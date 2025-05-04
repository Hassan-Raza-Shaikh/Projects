import streamlit as st
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
from sklearn.linear_model import LogisticRegression
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score, confusion_matrix, classification_report

# Set the page config first
st.set_page_config(page_title="Spotify Dashboard", layout="wide")

# Load dataset from a CSV file
# Assuming your dataset is called "spotify_data.csv" and contains 10,000 rows
df = pd.read_csv("Spotify_final_dataset.csv")

# Streamlit Layout
st.title("Spotify Song Data Dashboard")
st.subheader("Interactive Visualizations and Insights")

# Filter by Artist
artist = st.selectbox("Choose an Artist", df['Artist Name'].unique())
filtered_data = df[df['Artist Name'] == artist]

# Show Data Table
st.write(f"Data for {artist}")
st.dataframe(filtered_data)

# Basic Classification: Predicting if the song will make it to Top 10
st.subheader("Song Classification: Will It Make It to Top 10?")

# Label Encoding for categorical variables
label_encoder = LabelEncoder()
df['Artist Name Encoded'] = label_encoder.fit_transform(df['Artist Name'])
df['Song Name Encoded'] = label_encoder.fit_transform(df['Song Name'])

# Feature Selection (we're predicting 'Top 10 (xTimes)' as a binary classification)
# Let's create a binary target variable: 1 if the song was in the Top 10, 0 otherwise
df['Top 10 Binary'] = df['Top 10 (xTimes)'].apply(lambda x: 1 if x > 0 else 0)

# Features (you can add more or modify as needed)
X = df[['Days', 'Peak Position', 'Peak Streams', 'Total Streams', 'Artist Name Encoded', 'Song Name Encoded']]
y = df['Top 10 Binary']

# Train/Test Split
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42)

# Choose a model: Logistic Regression or Random Forest
model_type = st.selectbox("Choose a model", ["Logistic Regression", "Random Forest"])

if model_type == "Logistic Regression":
    model = LogisticRegression(max_iter=1000)
elif model_type == "Random Forest":
    model = RandomForestClassifier(n_estimators=100)

# Fit the model
model.fit(X_train, y_train)

# Predict
y_pred = model.predict(X_test)

# Model Evaluation
accuracy = accuracy_score(y_test, y_pred)
conf_matrix = confusion_matrix(y_test, y_pred)
class_report = classification_report(y_test, y_pred)

# Display Evaluation Metrics
st.write(f"Accuracy of {model_type}: {accuracy:.2f}")
st.write("Confusion Matrix:")
st.write(conf_matrix)
st.write("Classification Report:")
st.text(class_report)

# Visualization of Confusion Matrix
fig, ax = plt.subplots(figsize=(6, 6))
ax.matshow(conf_matrix, cmap='Blues', alpha=0.7)
for (i, j), val in np.ndenumerate(conf_matrix):
    ax.text(j, i, f'{val}', ha='center', va='center', color='black')
ax.set_title("Confusion Matrix")
ax.set_xlabel("Predicted Label")
ax.set_ylabel("True Label")
st.pyplot(fig)

# Scatter Plot - Days vs Total Streams
st.subheader("Scatter Plot of Days vs Total Streams")
fig, ax = plt.subplots(figsize=(10, 6))
scatter = ax.scatter(df['Days'], df['Total Streams'], c=df['Top 10 Binary'], cmap='viridis', s=100, alpha=0.7)
ax.set_title('Days vs Total Streams')
ax.set_xlabel('Days')
ax.set_ylabel('Total Streams')
fig.colorbar(scatter, ax=ax, label='Top 10 Binary (1=Yes, 0=No)')
st.pyplot(fig)

# Average Streams per Song
average_streams = df['Total Streams'].mean()
st.write(f"Average Total Streams per Song: {average_streams:,.0f}")

# Interactive with Peak Streams
peak_streams = st.slider('Select Peak Streams Range', min_value=0, max_value=int(df['Peak Streams'].max()), step=1000000)
filtered_by_peak_streams = df[df['Peak Streams'] >= peak_streams]

st.write(f"Songs with Peak Streams greater than {peak_streams:,}")
st.dataframe(filtered_by_peak_streams)

