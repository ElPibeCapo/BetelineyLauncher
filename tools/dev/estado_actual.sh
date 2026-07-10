#!/usr/bin/env bash
# estado_actual.sh — imprime SOLO la sección "## ESTADO ACTUAL" de ESTADO.md.
# El propio documento (1358+ líneas) dice explícitamente que un chat nuevo
# solo necesita leer esa sección para continuar el trabajo; el resto es
# historial de sesiones, útil para auditar pero no para arrancar.
#
# Uso: tools/dev/estado_actual.sh
set -euo pipefail
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
ESTADO="$REPO_ROOT/ESTADO.md"

awk '
  /^## ESTADO ACTUAL/ { printing=1 }
  printing && /^## / && !/^## ESTADO ACTUAL/ { exit }
  printing { print }
' "$ESTADO"
