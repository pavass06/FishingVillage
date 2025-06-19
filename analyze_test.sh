#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

# === Vérification du paramètre ===
if [ $# -ne 1 ]; then
  echo "Usage : $0 <numéro_de_test>"
  exit 1
fi

TEST_NUM=$1
OUTPUT_DIR="output/out_test${TEST_NUM}"

# === Contrôles préalables ===
if [ ! -d "$OUTPUT_DIR" ]; then
  echo "❌ Dossier $OUTPUT_DIR introuvable !"
  exit 1
fi

# === Lancement des scripts Python ===
echo "📊 Analyse pour le test #${TEST_NUM} → $OUTPUT_DIR"
echo

# 1) Distribution / inégalité
python3 analysis/inequality.py "$OUTPUT_DIR" \
  && echo "   ✓ inequality.py exécuté"

# 2) Revenus historiques
python3 analysis/revenue.py "$OUTPUT_DIR" \
  && echo "   ✓ revenue.py exécuté"

# 3) Série temporelle (GDP, etc.)
python3 analysis/show.py "$OUTPUT_DIR" \
  && echo "   ✓ show.py exécuté"

echo
echo "🎉 Analyse terminée pour out_test${TEST_NUM}"

