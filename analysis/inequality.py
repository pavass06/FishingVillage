import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np

# 1. Lecture des snapshots générés par main.cpp
df = pd.read_csv("../wrk/fisher_avg_income.csv")

# 2) Extraction des cycles et des colonnes de revenus
cycles     = df["cycle"].values
income_cols = [c for c in df.columns if c.startswith("id_")]

# 3) Préparation de la figure
fig, ax = plt.subplots(figsize=(10, 6))

def animate(i):
    ax.clear()
    t    = cycles[i]
    data = df.loc[i, income_cols].values.astype(float)
    var  = np.var(data, ddof=0)

    ax.hist(
        data,
        bins=20,
        edgecolor="black"
    )
    ax.set_xlabel("Revenu moyen")
    ax.set_ylabel("Nombre de pêcheurs")
    ax.set_title(f"t = {t} jours • Variance = {var:.2f}")
    ax.grid(True)

# 4) Création de l’animation
ani = FuncAnimation(
    fig,
    animate,
    frames=len(cycles),
    interval=500,
    repeat=False
)

# 5a) Pour voir directement l’animation dans une fenêtre
plt.show()