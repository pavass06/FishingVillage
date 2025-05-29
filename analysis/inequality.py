import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Chargement des revenus moyens
df = pd.read_csv("../wrk/fisher_avg_income.csv")

# Calcul de la variance
variance = np.var(df['AverageIncome'], ddof=0)

# Histogramme des revenus moyens avec légende
plt.figure(figsize=(10, 6))
plt.hist(
    df['AverageIncome'],
    bins=10,
    edgecolor='black',
    label=f"Variance = {variance:.2f}"
)
plt.xlabel('Revenu moyen')
plt.ylabel("Nombre de pêcheurs")
plt.title("Histogramme des revenus moyens des pêcheurs")
plt.legend()           # affiche la légende contenant la variance
plt.grid(True)
plt.tight_layout()
plt.show()
