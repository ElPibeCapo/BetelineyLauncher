#!/usr/bin/env bash
# build_fast.sh — evita el cuelgue de LTO documentado en ESTADO.md
# (sesiones 20, 27, 29, 31, 32, 33: "ninja -C build -j$(nproc)" sin
# restringir targets se cuelga en este hardware con LTO activo).
#
# Sesion 45: el problema real no era falta de paciencia, era que "build/"
# quedo cacheado como CMAKE_BUILD_TYPE=Release + ENABLE_LTO=ON, y CADA
# iteracion de desarrollo (verificar un fix de 2 lineas) pagaba el costo
# completo de LTO+O3 de un release final. Medido en este hardware (Ryzen
# 7 3700U, 8 threads): un build completo desde cero en "build/" (Release+
# LTO, pool de 2 links simultaneos, cada uno con -flto=auto = 8 particiones
# WHOPR -> hasta 16 procesos lto1 peleando por 8 nucleos reales, load
# average ~15) llego a 9 HORAS sin terminar (se quedo trabado en 31/51
# targets). El mismo build completo desde cero en "build-dev/" (RelWithDebInfo,
# ENABLE_LTO=OFF, linker lld en vez de ld por defecto) tardo 19m51s, 0 errores,
# 31/31 tests. "build-dev/" es el default de este script para iteracion diaria.
# "build/" (Release+LTO real) queda reservado para verificacion final antes
# de un release — usar BUILD_DIR=build explicito, o el modo "release" de abajo.
#
# Compila en background (nohup + disown) con targets restringidos según
# el modo, y deja un log + PID pollables con check_build.sh, en vez de
# bloquear la terminal esperando un build que puede tardar minutos u horas.
#
# Uso:
#   tools/dev/build_fast.sh full              # build-dev/, background (default: sin LTO)
#   tools/dev/build_fast.sh tests              # reconfigura BUILD_TESTING=ON,
#                                               # compila solo lo testeable
#   tools/dev/build_fast.sh updater            # reconfigura Launcher_BUILD_ARTIFACT,
#                                               # compila solo el updater
#   tools/dev/build_fast.sh target:NOMBRE      # compila un target puntual
#   tools/dev/build_fast.sh release            # build/ real, Release+LTO, para
#                                               # verificacion final pre-release
#   BUILD_DIR=build tools/dev/build_fast.sh    # override manual del directorio
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$REPO_ROOT/build-dev}"
LOG_DIR="$REPO_ROOT/tools/dev/.buildlogs"
mkdir -p "$LOG_DIR"
TS="$(date +%Y%m%d_%H%M%S)"
LOG_FILE="$LOG_DIR/build_$TS.log"

MODE="${1:-full}"
cd "$REPO_ROOT"

TARGETS=""
case "$MODE" in
  tests)
    echo "[build_fast] Reconfigurando con -DBUILD_TESTING=ON ..." | tee -a "$LOG_FILE"
    cmake -S . -B "$BUILD_DIR" -DBUILD_TESTING=ON >>"$LOG_FILE" 2>&1
    # Sesion 45: "Launcher_logic BetelineyTranslation_test" apuntaba a un target
    # que nunca existio (ninja: error: unknown target). El target real que
    # compila TODOS los ejecutables de test es "tests/all"; "test" aparte
    # es el driver de CTest que los corre (requiere que ya esten compilados).
    TARGETS="Launcher_logic tests/all"
    ;;
  updater)
    echo "[build_fast] Reconfigurando con -DLauncher_BUILD_ARTIFACT=linux-x86_64 ..." | tee -a "$LOG_FILE"
    cmake -S . -B "$BUILD_DIR" -DLauncher_BUILD_ARTIFACT="linux-x86_64" >>"$LOG_FILE" 2>&1
    TARGETS="prism_updater_logic BetelineyLauncher_updater"
    ;;
  target:*)
    TARGETS="${MODE#target:}"
    ;;
  release)
    BUILD_DIR="$REPO_ROOT/build"
    TARGETS=""
    ;;
  full)
    TARGETS=""
    ;;
  *)
    echo "Uso: build_fast.sh [full|tests|updater|target:<nombre>|release]" >&2
    exit 1
    ;;
esac

CORES="$(nproc)"
echo "[build_fast] Lanzando ninja en background (targets: ${TARGETS:-<todos>})"
echo "[build_fast] Log: $LOG_FILE"

# shellcheck disable=SC2086
nohup ninja -C "$BUILD_DIR" -j"$CORES" $TARGETS >>"$LOG_FILE" 2>&1 &
BUILD_PID=$!
disown

echo "$BUILD_PID" > "$LOG_DIR/last_pid"
echo "$LOG_FILE" > "$LOG_DIR/last_log"
echo "[build_fast] PID $BUILD_PID lanzado en background."
echo "[build_fast] Sondear con: tools/dev/check_build.sh"
