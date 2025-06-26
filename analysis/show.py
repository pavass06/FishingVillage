import sys, os
import pandas as pd
import matplotlib.pyplot as plt

import pandas as pd
import matplotlib.pyplot as plt

if len(sys.argv) != 2:
    print("Usage: python show.py <output_dir>")
    sys.exit(1)
output_dir = sys.argv[1]
infile = os.path.join(output_dir, "economicdatas.csv")

# Load the data and clean column names
# Adjust sep and decimal to your CSV format (',' or ';', '.' or ',')
raw = pd.read_csv(infile, sep=",", decimal='.', engine='python')
raw.columns = raw.columns.str.strip()  # Remove any extra spaces from column names

# Optional: Inspect the raw Cycle values to ensure they are the days you expect
print("Cycle head:", raw['Cycle'].head().tolist())
print(raw[['Cycle', 'DailyGDP']].describe())

# Make sure Cycle is treated as integers (day count)
raw['Cycle'] = raw['Cycle'].astype(int)

# Define the x-axis and y-axis parameters
x_parameter = "Cycle"      # x-axis set to the raw day count
y_parameter = "DailyGDP"   # change this to "DailyGDP", "Population", etc.

# Downsampling configuration: use step=1 for full data, or change to e.g., 100 for every 100th row
downsample_step = 1  # Use 1 to plot every day; increase to 10, 100, etc. to speed up

# Downsample the DataFrame if needed
if downsample_step > 1:
    df = raw.iloc[::downsample_step].copy()
else:
    df = raw.copy()

# Plotting
plt.figure(figsize=(10, 6))
plt.plot(df[x_parameter], df[y_parameter], label=y_parameter, linewidth=2)
plt.xlabel(x_parameter)
plt.ylabel(y_parameter)
plt.title(f"Time Series of {y_parameter}")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
