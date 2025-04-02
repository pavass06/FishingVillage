import pandas as pd
import matplotlib.pyplot as plt

# Fixed file path (change if necessary)
filepath = "/Users/avass/Documents/1SSE/Code/FishingVillage/data/output/economicdatas.csv"

# Load the data and clean column names
df = pd.read_csv(filepath)
df.columns = df.columns.str.strip()  # Remove any extra spaces from column names

# Define the x-axis and y-axis parameters
x_parameter = "Cycle"         # x-axis is always the cycle
y_parameter = "Unemployment"     # change this to "DailyGDP", "Population", etc.

# Downsampling configuration: use step=1 for full data, or change to e.g., 100 for every 100th row
downsample_step = 100  # Change to 100, 10, etc. if you want to reduce the number of points

# Downsample the DataFrame if needed
if downsample_step > 1:
    df = df.iloc[::downsample_step, :]

# Plotting
plt.figure(figsize=(10, 6))
plt.plot(df[x_parameter], df[y_parameter], label=y_parameter, linewidth=2)
plt.xlabel(x_parameter)
plt.ylabel(y_parameter)
plt.title(f"Time Series of {y_parameter}")
plt.legend()
plt.grid(True)
plt.show()
