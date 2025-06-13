import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# 1. Lecture des données générées par main.cpp
df = pd.read_csv("../wrk/fisher_avg_income_snapshots.csv")
cycles = df["cycle"].values
income_cols = [c for c in df.columns if c.startswith("id_")]

# 2. Préparation de la figure
fig, ax = plt.subplots(figsize=(6,4))

def animate(i):
    ax.clear()
    t = cycles[i]
    data = df.loc[i, income_cols].values.astype(float)
    ax.hist(data, bins=20, edgecolor="black")
    ax.set_title(f"Distribution des revenus moyens – jour {t}")
    ax.set_xlabel("Average Income")
    ax.set_ylabel("Nombre de pêcheurs")
    ax.grid(True)

# 3. Création de l’animation
ani = FuncAnimation(fig, animate, frames=len(cycles), interval=500)

plt.show()