#!/usr/bin/env bash
# secret_scan.sh — grep de patrones de credenciales conocidos antes de
# commitear. Motivación (sesión 15 de ESTADO.md): la API key de CurseForge
# quedó commiteada en texto plano en ESTADO.md y estuvo expuesta
# públicamente hasta que se detectó a mano.
#
# Uso manual: tools/dev/secret_scan.sh
# Uso como hook: instalado automáticamente por install_hooks.sh
set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$REPO_ROOT"

FILES=$(git diff --cached --name-only --diff-filter=ACM 2>/dev/null)
if [[ -z "$FILES" ]]; then
  FILES=$(git ls-files)
fi

PATTERNS=(
  '\$2[aby]\$[0-9]{2}\$[A-Za-z0-9./]{53}'
  'AKIA[0-9A-Z]{16}'
  '-----BEGIN [A-Z ]*PRIVATE KEY-----'
  'ghp_[A-Za-z0-9]{36}'
  'gho_[A-Za-z0-9]{36}'
)

FOUND=0
while IFS= read -r f; do
  [[ -f "$f" ]] || continue
  for p in "${PATTERNS[@]}"; do
    MATCH=$(grep -nE "$p" -- "$f" 2>/dev/null || true)
    if [[ -n "$MATCH" ]]; then
      echo "⚠ POSIBLE SECRETO en $f:"
      echo "$MATCH"
      FOUND=1
    fi
  done
done <<< "$FILES"

if [[ "$FOUND" -eq 0 ]]; then
  echo "✅ Sin coincidencias de patrones de credenciales conocidos."
else
  echo
  echo "❌ Revisar antes de commitear (o ajustar los patrones si es falso positivo)."
fi
exit $FOUND
