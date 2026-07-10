#!/usr/bin/env bash
# check_build.sh — sondea un build lanzado con build_fast.sh SIN bloquear
# la terminal. Aplica el patrón real ya usado a mano en sesiones 27/28/29/31
# de ESTADO.md: comparar el timestamp del binario contra el de los fuentes
# modificados, en vez de esperar a que el proceso de ninja termine.
#
# Uso: tools/dev/check_build.sh [ruta/al/binario]
set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
LOG_DIR="$REPO_ROOT/tools/dev/.buildlogs"
BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build}"
BINARY="${1:-$BUILD_DIR/beteliney}"

if [[ ! -f "$LOG_DIR/last_pid" ]]; then
  echo "No hay ningún build registrado. Corré build_fast.sh primero."
  exit 1
fi

PID="$(cat "$LOG_DIR/last_pid")"
LOG_FILE="$(cat "$LOG_DIR/last_log")"

if kill -0 "$PID" 2>/dev/null; then
  echo "=== CORRIENDO (PID $PID vivo) ==="
else
  echo "=== TERMINADO (PID $PID ya no existe) ==="
fi

echo
echo "--- Últimas 25 líneas del log ($LOG_FILE) ---"
tail -n 25 "$LOG_FILE" 2>/dev/null || echo "(log vacío o no encontrado)"

echo
if [[ -f "$BINARY" ]]; then
  echo "--- Binario: $BINARY ---"
  stat -c "Modificado: %y" "$BINARY"
else
  echo "El binario $BINARY todavía no existe (o el build no llega hasta ahí)."
fi

echo
if grep -qiE '\berror\b' "$LOG_FILE" 2>/dev/null; then
  echo "⚠ El log contiene la palabra 'error' — revisar antes de asumir éxito:"
  grep -inE '\berror\b' "$LOG_FILE" | tail -10
else
  echo "✅ Sin coincidencias de 'error' en el log hasta ahora."
fi
