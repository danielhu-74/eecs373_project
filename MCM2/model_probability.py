from sklearn.model_selection import train_test_split
from sklearn.linear_model import LinearRegression
from sklearn.ensemble import RandomForestRegressor
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import mean_squared_error, r2_score
import pandas as pd
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import classification_report, confusion_matrix
import numpy as np
import matplotlib.pyplot as plt
from MCM2.data_process_nonMedal import merged_data1
from data_process import merged_data
merged_data1['Has_Medal'] = 0
# merge the data
merged_data['Has_Medal'] = 1
merged_data = merged_data[['Year', 'NOC', 'Athlete Count', 'Total Participations', 'Has_Medal']]
merged_data2 = pd.concat([merged_data1,merged_data], ignore_index=True)

# features and target
features = ['Athlete Count', 'Year', 'Total Participations']
target = 'Has_Medal'

X = merged_data2[features]
y = merged_data2[target]
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# random forest model
rf_model = RandomForestClassifier(n_estimators=100, random_state=42) 
rf_model.fit(X_train, y_train)

# predict probabilities
predictions = rf_model.predict_proba(X_test)[:, 1]  # the probability of the label being 1

# model evaluation
print(classification_report(y_test, rf_model.predict(X_test)))
print("Confusion Matrix:")
print(confusion_matrix(y_test, rf_model.predict(X_test)))
countries_2024 = merged_data2[merged_data2['Year'] == 2024]
countries_2024 = countries_2024[countries_2024['Has_Medal'] == 0]
print(countries_2024.head(20))
print("done")
countries_2028 = countries_2024.copy()
countries_2028['Year'] = 2028
countries_2028['Total Events'] = 341 
countries_2028 = countries_2028[countries_2028['Has_Medal'] == 0]
countries_2028['Predicted_Medal_Probability'] = rf_model.predict_proba(countries_2028[features])[:, 1]
countries_2028.to_csv('Predictions_2028_Olympics_Medal_Probability.csv', index=False)