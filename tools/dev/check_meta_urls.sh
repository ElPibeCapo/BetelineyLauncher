#!/usr/bin/env bash
# check_meta_urls.sh — confirma que el contenido Beteliney en GitHub Pages
# del repo 'meta' sigue accesible. Motivación (sesión 15 de ESTADO.md):
# el workflow generate.yml usaba keep_files:false, que borraba TODO el
# contenido de gh-pages cada 6h (cron) y lo reemplazaba solo con lo que
# el script de metadata genera — beteliney-packs/, malware/ y news/
# sobrevivían por pura casualidad hasta la siguiente corrida automática.
#
# Corré esto después de cualquier cambio al workflow de 'meta', o
# periódicamente, para detectar la regresión antes que un usuario.
set -uo pipefail

BASE="https://ElPibeCapo.github.io/meta/v1"
URLS=(
  "$BASE/beteliney-packs/index.json"
  "$BASE/malware/known-hashes.json"
  "$BASE/news/feed.atom"
  "$BASE/net.fabricmc.fabric-loader/"
)

FAIL=0
for url in "${URLS[@]}"; do
  CODE=$(curl -s -o /dev/null -w "%{http_code}" --max-time 15 "$url")
  if [[ "$CODE" == "200" ]]; then
    echo "✅ $CODE  $url"
  else
    echo "❌ $CODE  $url"
    FAIL=1
  fi
done

exit $FAIL
