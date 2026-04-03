from sklearn.model_selection import train_test_split
from sklearn.linear_model import LinearRegression
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_squared_error, r2_score
from sklearn.metrics import root_mean_squared_error
import numpy as np
from data_process import merged_data
from sklearn.impute import SimpleImputer
import matplotlib.pyplot as plt
import pandas as pd
merged_data = merged_data.drop('Rank', axis=1)
print(merged_data.columns)
# from xgboost import XGBRegressor

features = ['is_host', 'Year', 'Total Events','Athlete Count', 'Total Participations', 'Cumulative_Total_Medals', 'Cumulative_Gold_Medals']
target = 'Gold'
# prepare data
X = merged_data[features]
y = merged_data[target]
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# linear regression model
linear_model = LinearRegression()
linear_model.fit(X_train, y_train)

# random forest model
rf_model = RandomForestRegressor(n_estimators=100, random_state=42)
rf_model.fit(X_train, y_train)
rf_predictions = rf_model.predict(X_test)


y_pred_linear = linear_model.predict(X_test)
y_pred_rf = rf_model.predict(X_test)
# model evaluation
rmse_linear = root_mean_squared_error(y_test, y_pred_linear)
print("Linear Regression - RMSE:", rmse_linear)
# Compute RMSE for Random Forest Regression
rmse_rf = root_mean_squared_error(y_test, y_pred_rf)
print("Random Forest - RMSE:", rmse_rf)
print("Linear Regression - R²:", r2_score(y_test, y_pred_linear))
print("Random Forest - R²:", r2_score(y_test, y_pred_rf))

countries_2024 = merged_data[merged_data['Year'] == 2024]
countries_2028 = countries_2024.copy()
countries_2028['Year'] = 2028
countries_2028['Total Events'] = 341  # from official website

# cumulative medals
countries_2028['Cumulative_Total_Medals'] += countries_2024['Total']
countries_2028['Cumulative_Gold_Medals'] += countries_2024['Gold']

# 2028 host country is United States
countries_2028['is_host'] = countries_2028['NOC'].apply(lambda x: 1 if x == 'United States' else 0)

# use random forest model to predict gold medals
countries_2028['Predicted_Gold_Medals'] = rf_model.predict(countries_2028[features])

# print out the prediction
print(countries_2028[['NOC', 'Year', 'Predicted_Gold_Medals']])
# save the prediction to a csv file
countries_2028[['NOC', 'Year', 'Predicted_Gold_Medals']].to_csv('Predictions_2028_Olympics.csv', index=False)

all_tree_preds = np.array([tree.predict(countries_2028[features]) for tree in rf_model.estimators_])

# calculate mean and standard deviation
countries_2028['Pred_Mean'] = np.mean(all_tree_preds, axis=0)  
countries_2028['Pred_Std'] = np.std(all_tree_preds, axis=0)    
countries_2028['CI_lower'] = countries_2028['Pred_Mean'] - 1.96*countries_2028['Pred_Std']  # 95%
countries_2028['CI_upper'] = countries_2028['Pred_Mean'] + 1.96*countries_2028['Pred_Std']  # 95%

actual_2024 = merged_data[merged_data.Year==2024][['NOC','Gold']].rename(columns={'Gold':'Gold_2024'})
combined = countries_2028.merge(actual_2024, on='NOC')

# determine if the country is likely to improve or decline
combined['Improve'] = combined['CI_lower'] > combined['Gold_2024']  # beyond 2024 performance
combined['Decline'] = combined['CI_upper'] < combined['Gold_2024']  # lower than 2024 performance
selected_countries = ['Italy', 'Netherlands', 'Australia', 'Hungary', 'Canada', 'Uzbekistan']

# filter selected countries
viz_data = combined[combined.NOC.isin(selected_countries)].sort_values('Pred_Mean', ascending=False)

# visualization
plt.figure(figsize=(12, 6), dpi=100)
colors = ['#2ca02c' if improve else '#d62728' for improve in viz_data['Improve']]
plt.errorbar(viz_data['Pred_Mean'], viz_data['NOC'],
             xerr=[viz_data['Pred_Mean']-viz_data['CI_lower'], 
                   viz_data['CI_upper']-viz_data['Pred_Mean']],
             fmt='o', color='grey', label='95% Confidence Interval')
plt.scatter(viz_data['Gold_2024'], viz_data['NOC'], 
           s=100, c=colors, edgecolor='black', zorder=10,
           label=['Likely Improvement','Likely Decline'])
for i, row in viz_data.iterrows():
    plt.text(row['Pred_Mean']+0.8, row['NOC'], 
             f"{row['Pred_Mean']:.1f} ± {(row['CI_upper']-row['Pred_Mean']):.1f}",
             va='center', fontsize=9)
plt.title('2028 Gold Medal Predictions vs 2024 Actuals (Selected Countries)')
plt.xlabel('Gold Medals Count')
plt.xlim(0, 25)
plt.grid(axis='x', linestyle='--', alpha=0.7)
plt.legend()
plt.tight_layout()
plt.show()