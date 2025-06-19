#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

# —————————————————————————
# 1) CONFIGURATION
# —————————————————————————
AGENT_DIR="wrk"                # dossier contenant agent.exe
AGENT="agent.exe"              # nom de l'exécutable
INPUT_DIR="input"              # dossier contenant input_testN
OUTPUT_BASE="output"           # dossier parent des out_testN
TESTS=(1 2 3 4 5 6 7 8 9)      # numéros de tests à lancer
# —————————————————————————

mkdir -p "$OUTPUT_BASE"

for n in "${TESTS[@]}"; do
  IN="$INPUT_DIR/input_test$n"
  OUT="$OUTPUT_BASE/out_test$n"

  echo "→ Test #$n : $IN → $OUT"

  # Vérifie l'existence de l'input
  if [ ! -f "$IN" ]; then
    echo "   ⚠️  $IN introuvable, skip."
    continue
  fi

  # (Re)crée le dossier de sortie
  rm -rf "$OUT"
  mkdir -p "$OUT"

  # 1️⃣ On passe dans wrk/
  pushd "$AGENT_DIR" >/dev/null

    # Nettoie d'anciens CSV
    rm -f ./*.csv

    # Désactive le 'exit on error' le temps de lancer l'agent
    set +e
    ./"$AGENT" "../$IN" > screen 2>&1
    AGENT_STATUS=$?
    set -e

    if [ "$AGENT_STATUS" -ne 0 ]; then
      echo "   ⚠️  agent.exe a retourné le code $AGENT_STATUS"
    else
      echo "   ✔️  Simulation terminée sans erreur"
    fi

  # Retour à la racine
  popd >/dev/null

  # 2️⃣ On déplace tous les CSV générés
  shopt -s nullglob
  csvs=( "$AGENT_DIR"/*.csv )
  if [ ${#csvs[@]} -gt 0 ]; then
    mv "${csvs[@]}" "$OUT"/
    echo "   ✔️  ${#csvs[@]} CSV déplacés vers $OUT"
  else
    echo "   ⚠️  Aucun CSV généré pour le test $n"
  fi
  shopt -u nullglob

  # 3️⃣ On bouge le log 'screen'
  if [ -f "$AGENT_DIR/screen" ]; then
    mv "$AGENT_DIR/screen" "$OUT"/
    echo "   ✔️  screen déplacé vers $OUT"
  else
    echo "   ⚠️  screen manquant pour le test $n"
  fi

  echo "✔️  Test #$n terminé."
done
