import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# 1) Lecture brut du CSV (sans header complet)
path = "../wrk/fisher_avg_income.csv"
# Si la première ligne ne comporte que "cycle", on saute
with open(path) as f:
    first = f.readline().strip()
skip = 1 if first == "cycle" else 0

df = pd.read_csv(path, header=None, skiprows=skip)
ncols = df.shape[1]
df.columns = ["cycle"] + [f"id_{i}" for i in range(ncols - 1)]

# Typage
df["cycle"] = df["cycle"].astype(int)
for c in df.columns[1:]:
    df[c] = df[c].astype(float)

# Colonnes de revenus
income_cols = df.columns[1:]

# Calcul de la variance
df["variance"] = df[income_cols].var(axis=1, ddof=0).fillna(0)

# === 2) Animation de l'histogramme ===
fig, ax = plt.subplots(figsize=(8,5))
def animate(i):
    ax.clear()
    t    = df.at[i, "cycle"]
    data = df.loc[i, income_cols].values
    var  = df.at[i, "variance"]
    ax.hist(data, bins=20, edgecolor="black")
    ax.set_title(f"Jour {t} • Variance = {var:.2f}")
    ax.set_xlabel("Revenu moyen")
    ax.set_ylabel("Nombre de pêcheurs")
    ax.grid(True)

ani = FuncAnimation(
    fig, animate,
    frames=len(df),
    interval=3000,   # 3000 ms = 3 secondes entre chaque frame
    repeat=False
)
plt.show()  # Affiche l'animation

# === 3) Puis on trace la courbe de la variance ===
plt.figure(figsize=(8,4))
plt.plot(df["cycle"], df["variance"], marker="o")
plt.xlabel("Jour (cycle)")
plt.ylabel("Variance des revenus moyens")
plt.title("Évolution de la variance des revenus moyens")
plt.grid(True)
plt.tight_layout()
plt.show()
