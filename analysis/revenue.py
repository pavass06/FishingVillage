#!/usr/bin/env python3

import sys
import numpy as np
import matplotlib.pyplot as plt

# -----------------------------------------------------------------------------
# Configuration : modifiez ici si besoin
# -----------------------------------------------------------------------------
# Chemin vers votre fichier CSV contenant les revenus :
REVENUE_CSV = "../wrk/firm_revenu.csv"
# Identifiants des firms, dans l'ordre des colonnes du CSV
FIRM_IDS = [100, 101]

# -----------------------------------------------------------------------------
# Chargement des données
# -----------------------------------------------------------------------------
print(f"Firm IDs: {FIRM_IDS}")

try:
    # 1) Charger le CSV : chaque colonne = une firm, chaque ligne = 1 cycle
    revenue_data = np.genfromtxt(REVENUE_CSV, delimiter=",")
except Exception as e:
    print(f"Error loading revenue data from '{REVENUE_CSV}': {e}")
    sys.exit(1)

print(f"Revenue data shape: {revenue_data.shape}")

# -----------------------------------------------------------------------------
# Protection contre données vides
# -----------------------------------------------------------------------------
if revenue_data.size == 0 or revenue_data.shape[0] == 0:
    print("No revenue data to plot – revenue_data is empty.")
    sys.exit(0)

# Si on a une seule colonne, garantir une matrice 2D
if revenue_data.ndim == 1:
    revenue_data = revenue_data.reshape(-1, 1)

# -----------------------------------------------------------------------------
# Filtrage des NaN / Inf
# -----------------------------------------------------------------------------
finite_mask = np.all(np.isfinite(revenue_data), axis=1)
revenue_data = revenue_data[finite_mask]

if revenue_data.shape[0] == 0:
    print("No finite revenue data to plot after filtering NaN/Inf.")
    sys.exit(0)

# -----------------------------------------------------------------------------
# Préparation du tracé
# -----------------------------------------------------------------------------
n_cycles, n_firms = revenue_data.shape
cycles = np.arange(n_cycles)
max_val = np.max(revenue_data)

# -----------------------------------------------------------------------------
# Tracé
# -----------------------------------------------------------------------------
plt.figure()
for idx, firm_id in enumerate(FIRM_IDS):
    if idx < n_firms:
        plt.plot(cycles,
                 revenue_data[:, idx],
                 marker="o",
                 label=f"Firm {firm_id}")
    else:
        print(f"Warning: no data column for Firm ID {firm_id}")

plt.xlabel("Cycle")
plt.ylabel("Revenue")
plt.title(f"Revenue per cycle for firms: {FIRM_IDS}")
plt.ylim(0, max_val * 1.2)
plt.grid(True)
plt.legend()
plt.tight_layout()
plt.show()
