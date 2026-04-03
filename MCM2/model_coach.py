import pandas as pd
import statsmodels.formula.api as smf
import chardet

def detect_encoding(file_path):
    with open(file_path, 'rb') as f:
        raw_data = f.read()
        result = chardet.detect(raw_data)
        return result['encoding']

def read_csv(file_path):
    encoding = detect_encoding(file_path)
    return pd.read_csv(file_path, encoding=encoding)

# read data
df = read_csv('./2025_Problem_C_Data/summerOly_athletes.csv')
df['Medal'] = df['Medal'].fillna('No medal')

# calculate medal counts
medal_counts = (
    df.groupby(['NOC', 'Sport', 'Year'])['Medal']
    .value_counts()
    .unstack(fill_value=0)
    .reset_index()
    .rename_axis(columns=None)
)
medal_counts['Total'] = medal_counts[['Gold', 'Silver', 'Bronze']].sum(axis=1)

# add coach effects and create treatment variable
great_coach_periods = [
    {'NOC': 'USA', 'Sport': 'Gymnastics', 'Start': 1996, 'End': 2016},
    {'NOC': 'ROU', 'Sport': 'Gymnastics', 'Start': 1976, 'End': 1984},
    {'NOC': 'CHN', 'Sport': 'Volleyball', 'Start': 1984, 'End': 1986},
    {'NOC': 'USA', 'Sport': 'Volleyball', 'Start': 2005, 'End': 2008},
    {'NOC': 'USA', 'Sport': 'Athletics', 'Start': 2008, 'End': 2016},   # Alberto Salazar
]

medal_counts['Great_Coach'] = 0
for period in great_coach_periods:
    mask = (
        (medal_counts['NOC'] == period['NOC']) &
        (medal_counts['Sport'] == period['Sport']) &
        (medal_counts['Year'].between(period['Start'], period['End']))
    )
    medal_counts.loc[mask, 'Great_Coach'] = 1
medal_counts = medal_counts.sort_values(['NOC', 'Sport', 'Year'])

# use transformations to create additional features
medal_counts['Medal_Growth'] = (
    medal_counts.groupby(['NOC', 'Sport'])['Total']
    .transform(lambda x: x.pct_change())
)

medal_counts['Medal_Volatility'] = (
    medal_counts.groupby(['NOC', 'Sport'])['Total']
    .transform(lambda x: x.rolling(3, min_periods=1).std())
)

medal_counts['Historical_Mean'] = (
    medal_counts.groupby(['NOC', 'Sport'])['Total']
    .transform(lambda x: x.expanding().mean())
)

medal_counts['Breakthrough'] = (
    (medal_counts['Total'] > medal_counts['Historical_Mean'] + 2 * medal_counts['Medal_Volatility'])
    .astype(int)
)

# filter data for DID analysis
did_data = medal_counts[
    medal_counts.groupby(['NOC', 'Sport'])['Great_Coach']
    .transform('sum') > 0
]

# run DID analysis
model = smf.ols(
    formula='Total ~ Great_Coach + C(NOC) + C(Sport) + Year + Medal_Volatility + Breakthrough',
    data=did_data
).fit()

print(model.summary())

import matplotlib.pyplot as plt
import seaborn as sns

sample_noc_sport = did_data[
    (did_data['NOC'] == 'USA') & 
    (did_data['Sport'] == 'Gymnastics')
]

# plot medal trends
plt.figure(figsize=(10, 6))
sns.lineplot(data=sample_noc_sport, x='Year', y='Total', 
             hue='Great_Coach', style='Great_Coach',
             markers=True, dashes=False, palette='Set1')
plt.axvspan(1996, 2016, color='gray', alpha=0.2, label='Great Coach Period')
plt.title('USA Gymnastics Medal Trends: Coach Intervention vs Baseline')
plt.ylabel('Total Medals')
plt.legend(title='Great Coach Active')
plt.show()