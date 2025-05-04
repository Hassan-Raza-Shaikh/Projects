import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from scipy import stats
import matplotlib as mpl

# Disable math text interpretation globally
mpl.rcParams['mathtext.default'] = 'regular'

# Load your dataset (change the filename if needed)
df = pd.read_csv("Spotify_final_dataset.csv")  # <-- Make sure this matches your file

# Clean column names
df.columns = df.columns.str.strip()

# Convert columns with '(x...)' values to float
df['Top 10 Times'] = df['Top 10 (xTimes)'].astype(str).str.replace('(x', '', regex=False).str.replace(')', '', regex=False).astype(float)
df['Peak Position Times'] = df['Peak Position (xTimes)'].astype(str).str.replace('(x', '', regex=False).str.replace(')', '', regex=False).astype(float)

# Convert other numeric columns
df['Days'] = df['Days'].astype(int)
df['Peak Position'] = df['Peak Position'].astype(int)
df['Peak Streams'] = df['Peak Streams'].astype(int)
df['Total Streams'] = df['Total Streams'].astype(int)

# =========================
# Descriptive Statistics
# =========================
print("Basic Statistics:\n")
print(df[['Days', 'Top 10 Times', 'Peak Position', 'Peak Position Times', 'Peak Streams', 'Total Streams']].describe())

# =========================
# Histogram: Total Streams
# =========================
plt.figure(figsize=(10, 6))
sns.histplot(df['Total Streams'], bins=30, kde=True, color='skyblue')
plt.title('Distribution of Total Streams')
plt.xlabel('Total Streams')
plt.ylabel('Frequency')
plt.tight_layout()
plt.savefig("histogram_total_streams.png")
plt.show()

# =========================
# Pie Chart: Top 5 Artists
# =========================
# Replace special characters in artist names to avoid matplotlib parsing issues
df['Artist Name'] = df['Artist Name'].replace(r'(\$)', r'\\$', regex=True)

top_artists = df['Artist Name'].value_counts().head(5)
plt.figure(figsize=(8, 8))
top_artists.plot.pie(autopct='%1.1f%%', startangle=90, colors=sns.color_palette("pastel"))
plt.title('Top 5 Artists by Number of Songs')
plt.ylabel('')
plt.tight_layout()
plt.savefig("pie_top_artists.png")
plt.show()

# =========================
# Frequency Distribution: Peak Position
# =========================
peak_position_bins = [1, 5, 10, 20, 50, 100]
df['Peak Pos Bin'] = pd.cut(df['Peak Position'], bins=peak_position_bins)
freq_distribution = df['Peak Pos Bin'].value_counts().sort_index()
print("\nFrequency Distribution of Peak Positions:")
print(freq_distribution)

# =========================
# Mean & Variance
# =========================
mean_streams = df['Total Streams'].mean()
var_streams = df['Total Streams'].var()
print(f"\nMean of Total Streams: {mean_streams:,.0f}")
print(f"Variance of Total Streams: {var_streams:,.0f}")

# =========================
# Confidence Interval (95%)
# =========================
sample = df.sample(frac=0.8, random_state=42)
remaining = df.drop(sample.index)
sample_mean = sample['Total Streams'].mean()
sample_se = stats.sem(sample['Total Streams'])
confidence_interval = stats.t.interval(0.95, len(sample)-1, loc=sample_mean, scale=sample_se)
print(f"\n95% Confidence Interval for Mean of Total Streams: {confidence_interval}")

# =========================
# Tolerance Interval (approx, normality assumed)
# =========================
z = stats.norm.ppf(0.975)
tolerance_interval = (
    sample_mean - z * np.std(sample['Total Streams'], ddof=1),
    sample_mean + z * np.std(sample['Total Streams'], ddof=1)
)
print(f"95% Tolerance Interval for Total Streams: {tolerance_interval}")

# Validate with 20% data
within_tolerance = remaining['Total Streams'].between(*tolerance_interval).mean()
print(f"\nPercentage of Remaining Data within Tolerance Interval: {within_tolerance * 100:.2f}%")

# =========================
# Hypothesis Testing
# =========================
# H0: No difference in mean streams based on Top 10 appearances
top_10_more = df[df['Top 10 Times'] > df['Top 10 Times'].median()]
top_10_less = df[df['Top 10 Times'] <= df['Top 10 Times'].median()]
t_stat, p_val = stats.ttest_ind(top_10_more['Total Streams'], top_10_less['Total Streams'], equal_var=False)

print(f"\nT-Test: Do songs in Top 10 more often have higher total streams?")
print(f"T-statistic: {t_stat:.4f}, P-value: {p_val:.4f}")

if p_val < 0.05:
    print("Result: Statistically significant difference.")
else:
    print("Result: No significant difference.")

# =========================
# Scatterplot: Total Streams vs. Top 10 Appearances
# =========================
plt.figure(figsize=(10, 6))
sns.scatterplot(data=df, x='Top 10 Times', y='Total Streams', hue='Artist Name', s=100, alpha=0.7)
plt.title('🎯 Total Streams vs. Top 10 Appearances')
plt.xlabel('Top 10 Times')
plt.ylabel('Total Streams')
plt.legend(loc='upper left', bbox_to_anchor=(1, 1))
plt.tight_layout()
plt.savefig("scatter_top10_vs_streams.png")
plt.show()

# =========================
# Average Streams by Artist (Top 10 Artists)
# =========================
top_artists_avg = df.groupby('Artist Name')['Total Streams'].mean().sort_values(ascending=False).head(10)

plt.figure(figsize=(12, 6))
sns.barplot(x=top_artists_avg.values, y=top_artists_avg.index, palette='viridis')
plt.title('💎 Average Streams by Top 10 Artists')
plt.xlabel('Average Total Streams')
plt.ylabel('Artist Name')
plt.tight_layout()
plt.savefig("bar_artist_avg_streams.png")
plt.show()

# =========================
# Lineplot: Peak Position vs. Total Streams
# =========================
plt.figure(figsize=(10, 6))
sns.lineplot(data=df.sort_values('Peak Position'), x='Peak Position', y='Total Streams', marker='o')
plt.title('📉 Peak Position vs. Total Streams')
plt.xlabel('Peak Position (1 is best)')
plt.ylabel('Total Streams')
plt.tight_layout()
plt.savefig("line_peak_vs_streams.png")
plt.show()

# =========================
# Correlation Heatmap
# =========================
plt.figure(figsize=(8, 6))
corr = df[['Days', 'Top 10 Times', 'Peak Position', 'Peak Position Times', 'Peak Streams', 'Total Streams']].corr()
sns.heatmap(corr, annot=True, cmap='coolwarm', fmt=".2f", linewidths=0.5)
plt.title('🧠 Correlation Heatmap')
plt.tight_layout()
plt.savefig("heatmap_correlation.png")
plt.show()

# =========================
# Violin Plot: Distribution of Peak Positions
# =========================
plt.figure(figsize=(8, 6))
sns.violinplot(data=df, x='Peak Position', inner='point', palette='Set2')
plt.title('🎻 Distribution of Peak Positions')
plt.tight_layout()
plt.savefig("violin_peak_position.png")
plt.show()

