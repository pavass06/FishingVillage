import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Load the CSV file
df = pd.read_csv("../data/simulation_summary.csv")
df.columns = df.columns.str.strip()  # clean up column names

# Compute GDP Growth as percentage change
df['GDP Growth'] = df['DailyGDP'].pct_change() * 100
df['GDP Growth'].fillna(0, inplace=True)

df_red= df.iloc[::100, :]

pop="Population"
dailygdp="DailyGDP"
inf="Inflation"

###############################################################################
# 2) Polpulation
###############################################################################
color_gdp = 'tab:blue'
color_infl = 'tab:green'

#plt.plot(df["Cycle"], df["Inflation"], label="inflation", linewidth=2)
plt.plot(df_red["Cycle"], df_red[inf], label="inflation", linewidth=2)



plt.show()

