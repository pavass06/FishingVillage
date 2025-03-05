import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Load the CSV file
df = pd.read_csv("/Users/avass/Documents/1SSE/Code/FishingVillage/data/simulation_summary.csv")
df.columns = df.columns.str.strip()  # clean up column names

# Compute GDP Growth as percentage change
df['GDP Growth'] = df['DailyGDP'].pct_change() * 100
df['GDP Growth'].fillna(0, inplace=True)

# EXAMPLE 1: Subsampling
# If you have very many rows, choose a subset for plotting (e.g., every 10th row):
df_sub = df.iloc[::10, :]

# EXAMPLE 2: Rolling average columns
# Suppose we smooth 'DailyGDP' over a 10-cycle window.
df['DailyGDP_Roll'] = df['DailyGDP'].rolling(window=10).mean()
df['Population_Roll'] = df['Population'].rolling(window=10).mean()
# ... add more rolling columns as needed.

# Create a 2x3 grid of subplots and make it larger (wider x-axis)
fig, axs = plt.subplots(2, 3, figsize=(24, 10))  # increased width from 18 -> 24
fig.suptitle("Simulation Indicators over Cycles", fontsize=16)

###############################################################################
# 1. GDP & Population (two y-axes)
###############################################################################
ax1 = axs[0, 0]
color_gdp = 'tab:blue'
color_pop = 'tab:red'
ax1.set_title("GDP & Population")
ax1.set_xlabel("Cycle (Day)")
ax1.set_ylabel("Daily GDP", color=color_gdp)

# Option A: Plot lines with no markers, maybe use rolling average
ax1.plot(df['Cycle'], df['DailyGDP_Roll'], '-', linewidth=2, color=color_gdp, label="Daily GDP (Roll)")
# Option B: Plot original data but smaller, more transparent markers
# ax1.plot(df_sub['Cycle'], df_sub['DailyGDP'], 'o', markersize=3, alpha=0.5, color=color_gdp, label="Daily GDP (Subsample)")

ax1.tick_params(axis='y', labelcolor=color_gdp)
ax1.grid(True)

ax2 = ax1.twinx()
ax2.set_ylabel("Population", color=color_pop)
ax2.plot(df['Cycle'], df['Population_Roll'], '-', linewidth=2, color=color_pop, label="Population (Roll)")
ax2.tick_params(axis='y', labelcolor=color_pop)

###############################################################################
# 2. GDP & Inflation (two y-axes)
###############################################################################
ax3 = axs[0, 1]
color_gdp = 'tab:blue'
color_infl = 'tab:green'

ax3.set_title("GDP & Inflation")
ax3.set_xlabel("Cycle (Day)")
ax3.set_ylabel("Daily GDP", color=color_gdp)
ax3.plot(df['Cycle'], df['DailyGDP_Roll'], '-', linewidth=2, color=color_gdp, label="Daily GDP (Roll)")
ax3.tick_params(axis='y', labelcolor=color_gdp)
ax3.grid(True)

ax4 = ax3.twinx()
ax4.set_ylabel("Inflation (%)", color=color_infl)
# Smaller markers, transparent:
ax4.plot(df_sub['Cycle'], df_sub['Inflation'], marker='^', markersize=3, alpha=0.5, linestyle='none', color=color_infl, label="Inflation (Subsample)")
ax4.tick_params(axis='y', labelcolor=color_infl)

###############################################################################
# 3. GDP & Unemployment (two y-axes)
###############################################################################
ax5 = axs[0, 2]
color_gdp = 'tab:blue'
color_unemp = 'tab:purple'

ax5.set_title("GDP & Unemployment")
ax5.set_xlabel("Cycle (Day)")
ax5.set_ylabel("Daily GDP", color=color_gdp)
ax5.plot(df['Cycle'], df['DailyGDP_Roll'], '-', linewidth=2, color=color_gdp, label="Daily GDP (Roll)")
ax5.tick_params(axis='y', labelcolor=color_gdp)
ax5.grid(True)

ax6 = ax5.twinx()
ax6.set_ylabel("Unemployment (%)", color=color_unemp)
# Subsample or rolling approach
ax6.plot(df_sub['Cycle'], df_sub['Unemployment'], marker='d', markersize=3, alpha=0.5, linestyle='none', color=color_unemp, label="Unemployment (Subsample)")
ax6.tick_params(axis='y', labelcolor=color_unemp)

###############################################################################
# 4. GDP per Capita alone
###############################################################################
# Remove markers, use a line
axs[1, 0].plot(df['Cycle'], df['GDPperCapita'].rolling(10).mean(), '-', linewidth=2, color='orange')
axs[1, 0].set_title("GDP per Capita (Roll)")
axs[1, 0].set_xlabel("Cycle (Day)")
axs[1, 0].set_ylabel("GDP per Capita")
axs[1, 0].grid(True)

###############################################################################
# 5. Population alone
###############################################################################
# Subsample with small markers
axs[1, 1].plot(df_sub['Cycle'], df_sub['Population'], 's', markersize=3, alpha=0.5, color='red')
axs[1, 1].set_title("Population (Subsample)")
axs[1, 1].set_xlabel("Cycle (Day)")
axs[1, 1].set_ylabel("Population")
axs[1, 1].grid(True)

###############################################################################
# 6. GDP Growth alone
###############################################################################
# Rolling approach
axs[1, 2].plot(df['Cycle'], df['GDP Growth'].rolling(10).mean(), '-', linewidth=2, color='brown')
axs[1, 2].set_title("GDP Growth (Roll, %)")
axs[1, 2].set_xlabel("Cycle (Day)")
axs[1, 2].set_ylabel("GDP Growth (%)")
axs[1, 2].grid(True)

# Optionally, reduce the number of tick labels on the x-axis (dilates visually)
for ax_row in axs:
    for ax in ax_row:
        ax.xaxis.set_major_locator(plt.MaxNLocator(10))  # show ~10 ticks on the x-axis

plt.tight_layout()
plt.show()
