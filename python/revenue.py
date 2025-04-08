import pandas as pd
import matplotlib.pyplot as plt


import pandas as pd
import matplotlib.pyplot as plt

# Path to your CSV file with firm revenues.
csv_file = "/Users/avass/Documents/1SSE/Code/FishingVillage/wrk/firm_revenu.csv"

# Read CSV file without header. The first row contains firm IDs.
df = pd.read_csv(csv_file, header=None)

# Extract the first row as firm IDs.
firm_ids = df.iloc[0].tolist()
print("Firm IDs:", firm_ids)

# The remaining rows contain revenue data; convert them to numeric values.
revenue_data = df.iloc[1:].astype(float)
print("Revenue data shape:", revenue_data.shape)

# The x-axis represents simulation cycles (starting at 1).
cycles = range(1, revenue_data.shape[0] + 1)

# Create a figure for the plot.
plt.figure(figsize=(12, 7))

# Set a rolling window size to smooth the data.
window_size = 10

# Plot revenue for each firm using a rolling average for smoothness.
for idx, firm_id in enumerate(firm_ids):
    # Select revenue data for this firm.
    data = revenue_data.iloc[:, idx]
    # Apply a rolling average (smoothed) with the defined window size.
    smoothed_data = data.rolling(window=window_size, min_periods=1).mean()
    # Plot without markers to show a continuous line.
    plt.plot(cycles, smoothed_data, label=f'Firm {firm_id}', linestyle='-')

# Determine the maximum revenue value to set the y-axis limit with some headroom.
max_val = revenue_data.max().max()
plt.ylim(0, max_val * 1.2)

# Set labels and title.
plt.xlabel("Cycle")
plt.ylabel("Revenue")
plt.title("Firm Revenue Over Time (Smoothed)")
plt.legend(title="Firm ID", loc='upper left')
plt.grid(True)
plt.tight_layout()
plt.show()

