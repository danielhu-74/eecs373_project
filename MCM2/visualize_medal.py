# import pandas as pd
# import matplotlib.pyplot as plt
# import seaborn as sns

# # read the data
# df = pd.read_csv('Predictions_2028_Olympics.csv')
# rank the countries by the predicted total medals
# df_sorted = df.sort_values(by='Predicted_Total_Medals', ascending=False)

# # choose the top 30 countries
# df_top30 = df_sorted.head(30)
# sns.set(style="whitegrid")

# # create a barplot
# plt.figure(figsize=(14, 10))  # set the size of the figure
# sns.barplot(x='Predicted_Total_Medals', y='NOC', data=df_top30, palette='mako')

##  add labels
# plt.xlabel('Predicted Total Medals', fontsize=14)
# plt.ylabel('Country', fontsize=14)

# # show the plot
# plt.tight_layout()
# plt.show()

# import pandas as pd
# import matplotlib.pyplot as plt
# import seaborn as sns

# # read the data
# df_gold = pd.read_csv('Predictions_2028_Olympics_Gold.csv')

##  rank the countries by the predicted gold medals
# df_gold_sorted = df_gold.sort_values(by='Predicted_Gold_Medals', ascending=False)

# # choose the top 30 countries
# df_top_gold = df_gold_sorted.head(30)

# sns.set(style="whitegrid", context='talk')

##  create a barplot
# plt.figure(figsize=(14, 10))
# barplot = sns.barplot(x='Predicted_Gold_Medals', y='NOC', data=df_top_gold, palette='viridis')

# #add labels
# # plt.title('Predicted Gold Medals for Top 30 Countries in 2028 Olympics', fontsize=20)
# plt.xlabel('Predicted Gold Medals', fontsize=16)
# plt.ylabel('Country', fontsize=16)
# plt.xticks(fontsize=12)
# plt.yticks(fontsize=12)

# # add the values on the bars
# # for p in barplot.patches:
# #     width = p.get_width()
# #     plt.text(p.get_width(), p.get_y() + p.get_height() / 2 + 0.2,
# #              f'{width:.2f}', va='center')

# plt.tight_layout()
# plt.show()
import plotly.express as px
import pandas as pd

# read the data
df_gold = pd.read_csv('Predictions_2028_Olympics_Gold.csv')

# create a treemap
fig = px.treemap(df_gold, path=['NOC'], values='Predicted_Gold_Medals',
                 title='Predicted Gold Medals in 2028 Olympics',
                 color='Predicted_Gold_Medals',  # according to the number of gold medals
                 color_continuous_scale='Blues') 

# refresh the layout
fig.update_layout(
    font_family="Arial",
    font_size=16,
    font_color="black",
    title_font_family="Times New Roman",
    title_font_size=24,
    title_font_color="blue"
)

fig.show()


# the above code is for the visualization of the predicted medals for the top 30 countries in the 2028 Olympics. THREE FIGURES
