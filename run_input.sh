#!/bin/bash

EXAMPLE=(1 2 3 4 5 6 7 8 9)
RUN="./wrk/agent.exe"
INPUT_DIR="input"
OUTPUT_DIR="output"
SOURCE_CSV_DIR="wrk"

# Création du dossier global de sortie s’il n’existe pas
mkdir -p "$OUTPUT_DIR"

for exam in "${EXAMPLE[@]}"; do
  INPUT_FILE="$INPUT_DIR/input_test$exam"
  OUTDIR="$OUTPUT_DIR/out_test$exam"

  echo "→ Lancement test $exam avec $INPUT_FILE"

  if [ ! -f "$INPUT_FILE" ]; then
    echo "⚠️  Fichier $INPUT_FILE introuvable"
    continue
  fi

  # Nettoyage du dossier de sortie
  rm -rf "$OUTDIR"
  mkdir -p "$OUTDIR"

  # Lancement de la simulation
  "$RUN" "$INPUT_FILE" > screen 2>&1

  # Déplacement des CSVs générés dans wrk/ → dans output/out_testX
  for file in economicdatas.csv firm_revenu.csv fisher_avg_income.csv; do
    if [ -f "$SOURCE_CSV_DIR/$file" ]; then
      mv "$SOURCE_CSV_DIR/$file" "$OUTDIR/"
    else
      echo "⚠️  Fichier $file manquant pour test $exam"
    fi
  done

  # Déplacement du screen
  mv screen "$OUTDIR/"

  echo "✔️ Résultats enregistrés dans $OUTDIR"
done

