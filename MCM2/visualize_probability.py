import plotly.express as px
import pandas as pd
import numpy as np

# read the data
data = pd.read_csv('Predictions_2028_Olympics_Medal_Probability.csv')
grouped_data = data.groupby('NOC').agg({
    'Athlete Count': 'mean',
    'Total Participations': 'mean',
    'Predicted_Medal_Probability': 'mean'
}).reset_index()

# create a 3D scatter plot
fig = px.scatter_3d(grouped_data,
                    x='Athlete Count',
                    y='Total Participations',
                    z='Predicted_Medal_Probability',
                    color='Predicted_Medal_Probability',
                    hover_name='NOC',
                    title='Aggregated Data: Athlete Count, Total Participations, and Medal Probability',
                    labels={
                        'Athlete Count': 'Average Number of Athletes',
                        'Total Participations': 'Average Total Participations',
                        'Predicted_Medal_Probability': 'Average Probability of Winning At Least One Medal'
                    },
                    color_continuous_scale=px.colors.sequential.Viridis)

fig.show()
