import pandas as pd
import matplotlib.pyplot as plt

# Load the CSV file
df = pd.read_csv("/Users/avass/Documents/1SSE/Code/FishingVillage/data/simulation_summary.csv")

# Compute GDP Growth as percentage change (first cycle set to 0)
df['GDP Growth'] = df['GDP'].pct_change() * 100
df['GDP Growth'].fillna(0, inplace=True)

# Create a 2x3 grid of subplots
fig, axs = plt.subplots(2, 3, figsize=(18, 10))
fig.suptitle("Simulation Indicators over Cycles", fontsize=16)

###############################################################################
# 1. GDP & Population (two y-axes)
###############################################################################
ax1 = axs[0, 0]
color_gdp = 'tab:blue'
color_pop = 'tab:red'

# Left axis for GDP
ax1.set_title("GDP & Population")
ax1.set_xlabel("Cycle")
ax1.set_ylabel("GDP", color=color_gdp)
ax1.plot(df['Cycle'], df['GDP'], marker='o', color=color_gdp, label="GDP")
ax1.tick_params(axis='y', labelcolor=color_gdp)
ax1.grid(True)

# Right axis for Population
ax2 = ax1.twinx()
ax2.set_ylabel("Population", color=color_pop)
ax2.plot(df['Cycle'], df['Population'], marker='s', color=color_pop, label="Population")
ax2.tick_params(axis='y', labelcolor=color_pop)

###############################################################################
# 2. GDP & Inflation (two y-axes)
###############################################################################
ax3 = axs[0, 1]
color_gdp = 'tab:blue'
color_infl = 'tab:green'

# Left axis for GDP
ax3.set_title("GDP & Inflation")
ax3.set_xlabel("Cycle")
ax3.set_ylabel("GDP", color=color_gdp)
ax3.plot(df['Cycle'], df['GDP'], marker='o', color=color_gdp, label="GDP")
ax3.tick_params(axis='y', labelcolor=color_gdp)
ax3.grid(True)

# Right axis for Inflation
ax4 = ax3.twinx()
ax4.set_ylabel("Inflation (%)", color=color_infl)
ax4.plot(df['Cycle'], df['Inflation'], marker='^', color=color_infl, label="Inflation")
ax4.tick_params(axis='y', labelcolor=color_infl)

###############################################################################
# 3. GDP & Unemployment (two y-axes)
###############################################################################
ax5 = axs[0, 2]
color_gdp = 'tab:blue'
color_unemp = 'tab:purple'

# Left axis for GDP
ax5.set_title("GDP & Unemployment")
ax5.set_xlabel("Cycle")
ax5.set_ylabel("GDP", color=color_gdp)
ax5.plot(df['Cycle'], df['GDP'], marker='o', color=color_gdp, label="GDP")
ax5.tick_params(axis='y', labelcolor=color_gdp)
ax5.grid(True)

# Right axis for Unemployment
ax6 = ax5.twinx()
ax6.set_ylabel("Unemployment (%)", color=color_unemp)
ax6.plot(df['Cycle'], df['Unemployment'], marker='d', color=color_unemp, label="Unemployment")
ax6.tick_params(axis='y', labelcolor=color_unemp)

###############################################################################
# 4. GDP per Capita alone
###############################################################################
axs[1, 0].plot(df['Cycle'], df['GDPperCapita'], marker='o', color='orange')
axs[1, 0].set_title("GDP per Capita")
axs[1, 0].set_xlabel("Cycle")
axs[1, 0].set_ylabel("GDP per Capita")
axs[1, 0].grid(True)

###############################################################################
# 5. Population alone
###############################################################################
axs[1, 1].plot(df['Cycle'], df['Population'], marker='s', color='red')
axs[1, 1].set_title("Population")
axs[1, 1].set_xlabel("Cycle")
axs[1, 1].set_ylabel("Population")
axs[1, 1].grid(True)

###############################################################################
# 6. GDP Growth alone
###############################################################################
axs[1, 2].plot(df['Cycle'], df['GDP Growth'], marker='^', color='brown')
axs[1, 2].set_title("GDP Growth (%)")
axs[1, 2].set_xlabel("Cycle")
axs[1, 2].set_ylabel("GDP Growth (%)")
axs[1, 2].grid(True)

plt.tight_layout()
plt.show()
