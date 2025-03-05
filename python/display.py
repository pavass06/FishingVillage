import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Load the CSV file
df = pd.read_csv("/Users/avass/Documents/1SSE/Code/FishingVillage/data/simulation_summary.csv")
df.columns = df.columns.str.strip()  # clean up column names

# Compute GDP Growth as percentage change
df['GDP Growth'] = df['DailyGDP'].pct_change() * 100
df['GDP Growth'].fillna(0, inplace=True)

# Decide whether to subsample
if len(df) < 100:
    df_to_plot = df
else:
    # Plot every 10th row if more than 100 cycles
    df_to_plot = df.iloc[::10, :]

# Create the figure and subplots
fig, axs = plt.subplots(2, 3, figsize=(24, 10))
fig.suptitle("Simulation Indicators over Cycles", fontsize=16)

###############################################################################
# 1) GDP & Population
###############################################################################
ax1 = axs[0, 0]
color_gdp = 'tab:blue'
color_pop = 'tab:red'

ax1.set_title("GDP & Population")
ax1.set_xlabel("Cycle (Day)")
ax1.set_ylabel("Daily GDP", color=color_gdp)

ax1.plot(df_to_plot['Cycle'], df_to_plot['DailyGDP'], 'o-', color=color_gdp, label="Daily GDP")
ax1.tick_params(axis='y', labelcolor=color_gdp)
ax1.grid(True)

ax2 = ax1.twinx()
ax2.set_ylabel("Population", color=color_pop)
ax2.plot(df_to_plot['Cycle'], df_to_plot['Population'], 's-', color=color_pop, label="Population")
ax2.tick_params(axis='y', labelcolor=color_pop)

###############################################################################
# 2) GDP & Inflation
###############################################################################
ax3 = axs[0, 1]
color_gdp = 'tab:blue'
color_infl = 'tab:green'

ax3.set_title("GDP & Inflation")
ax3.set_xlabel("Cycle (Day)")
ax3.set_ylabel("Daily GDP", color=color_gdp)
ax3.plot(df_to_plot['Cycle'], df_to_plot['DailyGDP'], 'o-', color=color_gdp, label="Daily GDP")
ax3.tick_params(axis='y', labelcolor=color_gdp)
ax3.grid(True)

ax4 = ax3.twinx()
ax4.set_ylabel("Inflation (%)", color=color_infl)
ax4.plot(df_to_plot['Cycle'], df_to_plot['Inflation'], '^-', color=color_infl, label="Inflation")
ax4.tick_params(axis='y', labelcolor=color_infl)

###############################################################################
# 3) GDP & Unemployment
###############################################################################
ax5 = axs[0, 2]
color_gdp = 'tab:blue'
color_unemp = 'tab:purple'

ax5.set_title("GDP & Unemployment")
ax5.set_xlabel("Cycle (Day)")
ax5.set_ylabel("Daily GDP", color=color_gdp)
ax5.plot(df_to_plot['Cycle'], df_to_plot['DailyGDP'], 'o-', color=color_gdp, label="Daily GDP")
ax5.tick_params(axis='y', labelcolor=color_gdp)
ax5.grid(True)

ax6 = ax5.twinx()
ax6.set_ylabel("Unemployment (%)", color=color_unemp)
ax6.plot(df_to_plot['Cycle'], df_to_plot['Unemployment'], 'd-', color=color_unemp, label="Unemployment")
ax6.tick_params(axis='y', labelcolor=color_unemp)

###############################################################################
# 4) GDP per Capita (single axis)
###############################################################################
axs[1, 0].plot(df_to_plot['Cycle'], df_to_plot['GDPperCapita'], 'o-', color='orange')
axs[1, 0].set_title("GDP per Capita")
axs[1, 0].set_xlabel("Cycle (Day)")
axs[1, 0].set_ylabel("GDP per Capita")
axs[1, 0].grid(True)

###############################################################################
# 5) Population (single axis)
###############################################################################
axs[1, 1].plot(df_to_plot['Cycle'], df_to_plot['Population'], 's-', color='red')
axs[1, 1].set_title("Population")
axs[1, 1].set_xlabel("Cycle (Day)")
axs[1, 1].set_ylabel("Population")
axs[1, 1].grid(True)

###############################################################################
# 6) GDP Growth (single axis)
###############################################################################
axs[1, 2].plot(df_to_plot['Cycle'], df_to_plot['GDP Growth'], '^-', color='brown')
axs[1, 2].set_title("GDP Growth (%)")
axs[1, 2].set_xlabel("Cycle (Day)")
axs[1, 2].set_ylabel("GDP Growth (%)")
axs[1, 2].grid(True)

# Control how many x-ticks we show depending on the dataset size
for ax_row in axs:
    for ax in ax_row:
        if len(df) < 100:
            # Show a tick per cycle if not too many cycles
            ax.xaxis.set_major_locator(plt.MaxNLocator(len(df)))
        else:
            # For large number of cycles, limit to ~10 ticks to avoid overcrowding
            ax.xaxis.set_major_locator(plt.MaxNLocator(10))

plt.tight_layout()
plt.show()

