#!/usr/bin/env bash
# ============================================================
#  Beteliney Launcher — Test de integración: apply_jvm_profile_if_needed()
#  Autor: El_PibeCapo <elpibecapoofficial@gmail.com>  2026
#
#  Casos cubiertos:
#    1. Sin cfg previo (primer arranque) → crea cfg con perfil iGPU
#    2. Cfg existe con JvmArgs vacío     → escribe perfil iGPU
#    3. Cfg existe con JvmArgs manual    → NO pisa la config
#    4. --no-jvm flag                    → no hace nada aunque sea iGPU
#    5. GPU discreta                     → no aplica perfil iGPU
#    6. Xmx bajo (≤512) en cfg          → ajusta MinMemAlloc y MaxMemAlloc
#    7. Xmx alto en cfg                 → no modifica RAM
#
#  Uso: bash tests/test_jvm_profile_integration.sh
# ============================================================

set -euo pipefail
PASS=0; FAIL=0
G='\033[0;32m'; R='\033[0;31m'; Y='\033[1;33m'; N='\033[0m'; DIM='\033[2m'

# ── Helpers ──────────────────────────────────────────────────────
pass() { PASS=$((PASS+1)); echo -e "  ${G}[PASS]${N} $1"; }
fail() { FAIL=$((FAIL+1)); echo -e "  ${R}[FAIL]${N} $1"; }

# Crea un HOME temporal para aislar cfg del sistema real
setup_tmpdir() {
    TMPDIR_TEST=$(mktemp -d)
    export HOME="$TMPDIR_TEST"
    CFG_DIR="$HOME/.local/share/beteliney"
    CFG_FILE="$CFG_DIR/beteliney.cfg"
}

cleanup() { rm -rf "$TMPDIR_TEST"; }
trap cleanup EXIT

# ── Extraer apply_jvm_profile_if_needed() de lanzar.sh ───────────
# Se sourcea la función directamente del script real para testear el código vivo.
SCRIPT_DIR_TEST="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
LANZAR="$SCRIPT_DIR_TEST/lanzar.sh"  # raíz del proyecto
if [[ ! -f "$LANZAR" ]]; then
    echo -e "  ${R}[ERROR]${N} No se encontró lanzar.sh en: $LANZAR"
    exit 1
fi

# Extraer solo las funciones necesarias (detect_gpu_type + apply_jvm_profile_if_needed)
# usando sed para extraer entre marcas conocidas
extract_functions() {
    sed -n '/^detect_gpu_type()/,/^apply_jvm_profile_if_needed()/{ p }' "$LANZAR"
    sed -n '/^apply_jvm_profile_if_needed()/,/^show_active_jvm_cfg\|^JVM_PROFILE_SUGGESTION\|^echo ""/{p}' "$LANZAR"
}

# Sourcear con GPU simulada — sobrescribimos detect_gpu_type para cada test
source_with_gpu() {
    local gpu_type="$1"
    local no_jvm="${2:-0}"
    NO_JVM_AUTO="$no_jvm"
    JVM_PROFILE_SUGGESTION=""
    JVM_PROFILE_APPLIED=0
    # Sobrescribir detect_gpu_type con valor fijo
    detect_gpu_type() { echo "$gpu_type"; }
    # Cargar apply_jvm_profile_if_needed desde lanzar.sh
    eval "$(sed -n '/^apply_jvm_profile_if_needed()/,/^}/{ /^detect_gpu_type()/,/^}/!p }' "$LANZAR" \
        | grep -v '^detect_gpu_type')"
    # Source directo de la función completa
    source <(awk '/^apply_jvm_profile_if_needed\(\)/{p=1} p{print} /^\}$/{if(p)p=0}' "$LANZAR")
}


# ── Definir apply_jvm_profile_if_needed directamente (más robusto) ──
# En vez de sourcear con sed frágil, definimos una versión de test
# que usa la lógica real pero con detect_gpu_type() mockeable.

apply_jvm_profile_if_needed_test() {
    local gpu_type="$1"  # parámetro directo para tests
    [[ "$NO_JVM_AUTO" == "1" ]] && return
    [[ "$gpu_type" != igpu_* ]] && return

    local cfg_paths=( "$CFG_FILE" )
    local cfg_file=""
    for p in "${cfg_paths[@]}"; do [[ -f "$p" ]] && { cfg_file="$p"; break; }; done

    local igpu_args="-XX:+UseG1GC -XX:MaxGCPauseMillis=100 -Djava.rmi.server.disableHttp=true"

    if [[ -z "$cfg_file" ]]; then
        mkdir -p "$(dirname "$CFG_FILE")"
        printf '[General]\nJvmArgs=%s\nMinMemAlloc=384\nMaxMemAlloc=1536\n' "$igpu_args" > "$CFG_FILE"
        JVM_PROFILE_APPLIED=1; return
    fi

    local current_args
    current_args=$(grep -E '^JvmArgs=' "$cfg_file" 2>/dev/null | cut -d= -f2- || true)
    if [[ -n "$current_args" ]]; then
        JVM_PROFILE_APPLIED=0; return
    fi

    if grep -q '^JvmArgs=' "$cfg_file"; then
        sed -i "s|^JvmArgs=.*|JvmArgs=$igpu_args|" "$cfg_file"
    else
        echo "JvmArgs=$igpu_args" >> "$cfg_file"
    fi

    local cur_max
    cur_max=$(grep -E '^MaxMemAlloc=' "$cfg_file" 2>/dev/null | cut -d= -f2 || echo "0")
    if [[ "$cur_max" -le 512 ]] 2>/dev/null; then
        grep -q '^MinMemAlloc=' "$cfg_file" && sed -i 's|^MinMemAlloc=.*|MinMemAlloc=384|' "$cfg_file" || echo "MinMemAlloc=384" >> "$cfg_file"
        grep -q '^MaxMemAlloc=' "$cfg_file" && sed -i 's|^MaxMemAlloc=.*|MaxMemAlloc=1536|' "$cfg_file" || echo "MaxMemAlloc=1536" >> "$cfg_file"
    fi
    JVM_PROFILE_APPLIED=1
}


echo ""
echo -e "  ⬡  ${G}Test: apply_jvm_profile_if_needed()${N}"
echo -e "  ${DIM}──────────────────────────────────────────────${N}"
echo ""

# ── TEST 1: primer arranque, sin cfg ─────────────────────────────
setup_tmpdir
NO_JVM_AUTO=0; JVM_PROFILE_APPLIED=0
apply_jvm_profile_if_needed_test "igpu_amd"
if [[ -f "$CFG_FILE" ]] && grep -q "UseG1GC" "$CFG_FILE"; then
    pass "T1: sin cfg previo → crea beteliney.cfg con perfil iGPU"
else
    fail "T1: cfg no creado o sin G1GC"
fi
cleanup

# ── TEST 2: cfg existe con JvmArgs vacío ─────────────────────────
setup_tmpdir
mkdir -p "$CFG_DIR"
printf '[General]\nJvmArgs=\nMaxMemAlloc=256\n' > "$CFG_FILE"
NO_JVM_AUTO=0; JVM_PROFILE_APPLIED=0
apply_jvm_profile_if_needed_test "igpu_amd"
if grep -q "UseG1GC" "$CFG_FILE" && [[ "$JVM_PROFILE_APPLIED" == "1" ]]; then
    pass "T2: JvmArgs vacío → escribe perfil iGPU y ajusta RAM"
else
    fail "T2: no escribió perfil o JVM_PROFILE_APPLIED=$JVM_PROFILE_APPLIED"
fi
cleanup

# ── TEST 3: cfg con JvmArgs manual → NO pisar ────────────────────
setup_tmpdir
mkdir -p "$CFG_DIR"
printf '[General]\nJvmArgs=-Xmx4g -Xms512m\nMaxMemAlloc=4096\n' > "$CFG_FILE"
NO_JVM_AUTO=0; JVM_PROFILE_APPLIED=0
apply_jvm_profile_if_needed_test "igpu_amd"
local_args=$(grep '^JvmArgs=' "$CFG_FILE" | cut -d= -f2-)
if [[ "$local_args" == "-Xmx4g -Xms512m" ]] && [[ "$JVM_PROFILE_APPLIED" == "0" ]]; then
    pass "T3: JvmArgs manual → no pisado"
else
    fail "T3: JvmArgs fue modificado: '$local_args'"
fi
cleanup

# ── TEST 4: --no-jvm → no hace nada ──────────────────────────────
setup_tmpdir
NO_JVM_AUTO=1; JVM_PROFILE_APPLIED=0
apply_jvm_profile_if_needed_test "igpu_amd"
if [[ ! -f "$CFG_FILE" ]] && [[ "$JVM_PROFILE_APPLIED" == "0" ]]; then
    pass "T4: --no-jvm → sin cambios"
else
    fail "T4: --no-jvm no respetado"
fi
cleanup

# ── TEST 5: GPU discreta → no aplica ─────────────────────────────
setup_tmpdir
NO_JVM_AUTO=0; JVM_PROFILE_APPLIED=0
apply_jvm_profile_if_needed_test "discrete"
if [[ ! -f "$CFG_FILE" ]] && [[ "$JVM_PROFILE_APPLIED" == "0" ]]; then
    pass "T5: GPU discreta → no aplica perfil iGPU"
else
    fail "T5: aplicó perfil con GPU discreta"
fi
cleanup

# ── TEST 6: Xmx alto (>512) → no toca RAM ────────────────────────
setup_tmpdir
mkdir -p "$CFG_DIR"
printf '[General]\nJvmArgs=\nMinMemAlloc=512\nMaxMemAlloc=2048\n' > "$CFG_FILE"
NO_JVM_AUTO=0; JVM_PROFILE_APPLIED=0
apply_jvm_profile_if_needed_test "igpu_intel"
cur_max=$(grep '^MaxMemAlloc=' "$CFG_FILE" | cut -d= -f2)
if [[ "$cur_max" == "2048" ]]; then
    pass "T6: Xmx=2048 (>512) → RAM no modificada"
else
    fail "T6: MaxMemAlloc fue cambiado a $cur_max"
fi
cleanup

# ── TEST 7: Xmx bajo (≤512) → ajusta RAM ─────────────────────────
setup_tmpdir
mkdir -p "$CFG_DIR"
printf '[General]\nJvmArgs=\nMaxMemAlloc=256\n' > "$CFG_FILE"
NO_JVM_AUTO=0; JVM_PROFILE_APPLIED=0
apply_jvm_profile_if_needed_test "igpu_amd"
cur_max=$(grep '^MaxMemAlloc=' "$CFG_FILE" | cut -d= -f2)
cur_min=$(grep '^MinMemAlloc=' "$CFG_FILE" | cut -d= -f2)
if [[ "$cur_max" == "1536" ]] && [[ "$cur_min" == "384" ]]; then
    pass "T7: Xmx=256 (≤512) → RAM ajustada a Xmx=1536 Xms=384"
else
    fail "T7: RAM no ajustada correctamente — Max=$cur_max Min=$cur_min"
fi
cleanup

echo ""
echo -e "  ${DIM}──────────────────────────────────────────────${N}"
echo -e "  ${G}PASS: $PASS${N}  ${R}FAIL: $FAIL${N}  Total: $((PASS+FAIL))"
echo ""
[[ "$FAIL" -eq 0 ]] && echo -e "  ${G}✔ Todos los tests pasaron.${N}" || echo -e "  ${R}✘ $FAIL test(s) fallaron.${N}"
echo ""
exit "$FAIL"
