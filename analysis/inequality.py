import sys, os
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# 1. Lecture des données
if len(sys.argv) != 2:
    print("Usage: python inequality.py <output_dir>")
    sys.exit(1)
output_dir = sys.argv[1]
infile = os.path.join(output_dir, "income_snapshots.csv")

# Lecture des données
df = pd.read_csv(infile)

# 2. Extraction des instants et pré-calcul des variances
times = sorted(df['time'].unique())
variances = [
    df.loc[df['time'] == t, 'avg_income'].var(ddof=0)
    for t in times
]

# 3. Préparation des figures
fig, (ax_hist, ax_var) = plt.subplots(1, 2, figsize=(12, 5))
fig.suptitle("Dynamique de la distribution des revenus")

# 4. Fonction d'animation
def update(frame):
    t = times[frame]
    data_t = df.loc[df['time'] == t, 'avg_income']

    # Histogramme
    ax_hist.clear()
    ax_hist.hist(data_t, bins=30, edgecolor='black')
    ax_hist.set_title(f"Histogramme des revenus (t = {t} jours)")
    ax_hist.set_xlabel("Revenu moyen")
    ax_hist.set_ylabel("Nombre de pêcheurs")

    # Courbe de variance
    ax_var.clear()
    ax_var.plot(times[:frame+1], variances[:frame+1], marker='o')
    ax_var.set_title("Variance des revenus dans le temps")
    ax_var.set_xlabel("Temps (jours)")
    ax_var.set_ylabel("Variance")

# 5. Lancer l’animation
ani = FuncAnimation(
    fig, update,
    frames=len(times),
    interval=500,
    repeat=False
)

# 6. Affichage
plt.tight_layout()
plt.show()
