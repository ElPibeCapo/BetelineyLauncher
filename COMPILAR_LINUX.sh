#!/usr/bin/env bash
# =============================================================
#  Beteliney Launcher - Script de compilacion para Linux
#  Autor: El_PibeCapo <elpibecapoofficial@gmail.com>  2026
#  Probado en: Nobara Linux (Fedora-based) · CachyOS Linux (Arch-based)
#  Uso: cd source && bash COMPILAR_LINUX.sh
# =============================================================

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

R='\033[0;31m' G='\033[0;32m' Y='\033[1;33m' C='\033[0;36m' N='\033[0m'

echo -e "${C}"
echo "  =============================================="
echo "    Beteliney Launcher  |  Build Linux v8"
echo "  =============================================="
echo -e "${N}"

echo "  Elige modo:"
echo ""
echo "    [1]  Build completo       (primera vez)"
echo "    [2]  Recompilar rapido    (usa cache)"
echo "    [3]  Limpiar y rebuild"
echo "    [4]  Solo instalar deps"
echo "    [5]  Crear AppImage       (distribuible)"
echo "    [0]  Salir"
echo ""
read -rp "  Opcion [1]: " MODE
MODE="${MODE:-1}"
[[ "$MODE" == "0" ]] && exit 0


detect_distro() {
    command -v dnf &>/dev/null && echo "fedora" && return
    command -v apt &>/dev/null && echo "debian" && return
    command -v pacman &>/dev/null && echo "arch" && return
    echo "unknown"
}
DISTRO=$(detect_distro)

install_deps() {
    echo -e "${Y}[INFO] Instalando dependencias (${DISTRO})...${N}"
    case "$DISTRO" in
        fedora)
            sudo dnf install -y cmake ninja-build git gcc-c++ mold \
                qt6-qtbase-devel qt6-qtnetworkauth-devel qt6-qtsvg-devel \
                qt6-qtbase-private-devel extra-cmake-modules \
                libarchive-devel cmark-devel qrencode-devel zlib-devel \
                tomlplusplus-devel gamemode-devel \
                java-21-openjdk-devel desktop-file-utils libappstream-glib
            ;;
        debian)
            sudo apt update && sudo apt install -y cmake ninja-build git g++ \
                qt6-base-dev libqt6networkauth6-dev libqt6svg6-dev \
                extra-cmake-modules libarchive-dev libcmark-dev \
                libqrencode-dev zlib1g-dev libtomlplusplus-dev \
                gamemode default-jdk-headless appstream
            ;;
        arch)
            # CachyOS / Arch — incluye zlib-ng-compat como reemplazo de zlib
            sudo pacman -Syu --needed cmake ninja git gcc \
                qt6-base qt6-networkauth qt6-svg extra-cmake-modules \
                libarchive cmark qrencode zlib-ng-compat tomlplusplus \
                gamemode jdk-openjdk vulkan-headers appstream
            ;;
        *)
            echo -e "${R}[ERROR] Distro no soportada. Instala deps manualmente.${N}"
            exit 1 ;;
    esac
    echo -e "${G}[OK] Dependencias instaladas${N}"
}


check_tools() {
    echo -e "\n  --- Verificando herramientas ---\n"
    for tool in cmake ninja git g++ java; do
        if command -v "$tool" &>/dev/null; then
            echo -e "  ${G}[OK]${N} $tool"
        else
            echo -e "  ${R}[ERROR] $tool no encontrado. Corre opcion [4].${N}"
            exit 1
        fi
    done
    if pkg-config --exists Qt6Core 2>/dev/null; then
        echo -e "  ${G}[OK]${N} Qt6: $(pkg-config --modversion Qt6Core)"
    elif command -v qmake6 &>/dev/null; then
        echo -e "  ${G}[OK]${N} Qt6 (qmake6)"
    else
        echo -e "  ${R}[ERROR] Qt6 no encontrado.${N}"; exit 1
    fi
    # Usar el JDK que tenga javac — no el JRE default
    if command -v javac &>/dev/null; then
        export JAVA_HOME="$(dirname "$(dirname "$(readlink -f "$(command -v javac)")")")"
    elif [[ -f "/usr/lib/jvm/java-26-openjdk/bin/javac" ]]; then
        export JAVA_HOME="/usr/lib/jvm/java-26-openjdk"
    elif [[ -f "/usr/lib/jvm/default/bin/javac" ]]; then
        export JAVA_HOME="/usr/lib/jvm/default"
    else
        echo -e "  ${R}[ERROR]${N} No se encontró javac. Instala un JDK completo (jdk-openjdk)." && exit 1
    fi
    export PATH="$JAVA_HOME/bin:$PATH"
    echo -e "  ${G}[OK]${N} JAVA_HOME: $JAVA_HOME\n"
}

ensure_libnbt() {
    if [[ ! -f "libraries/libnbtplusplus/CMakeLists.txt" ]]; then
        echo -e "${Y}[INFO] Descargando libnbtplusplus...${N}"
        # BUG-7 fix: rama explícita para evitar que un cambio upstream rompa el build
        # Para pinear a un commit fijo (recomendado cuando el upstream esté estable):
        #   1. Verificar el último commit funcional: git -C libraries/libnbtplusplus log --oneline -5
        #   2. Reemplazar LIBNBT_BRANCH por el hash (ej: "a1b2c3d4e5f6")
        #   3. Cambiar --branch por: git -C libraries/libnbtplusplus checkout "$LIBNBT_BRANCH"
        # Hash verificado: (pendiente — usar "main" hasta estabilizar upstream)
        LIBNBT_BRANCH="main"
        git clone --depth 1 --branch "$LIBNBT_BRANCH" \
            https://github.com/PrismaticallyMulticoloured/libnbtplusplus.git \
            "libraries/libnbtplusplus"
        echo -e "${G}[OK] libnbtplusplus (branch: ${LIBNBT_BRANCH}) descargado${N}"
    fi
}

ensure_git() {
    # Solo crear estructura .git falsa si no existe ningún repo git real.
    # El objetivo es que CMakeLists.txt no falle en git describe.
    if [[ -f ".git/HEAD" ]]; then
        return  # ya hay repo real, no tocar nada
    fi
    mkdir -p .git/refs/heads
    echo "ref: refs/heads/master" > .git/HEAD
    echo "0000000000000000000000000000000000000000" > .git/refs/heads/master
    touch .git/packed-refs
}


do_configure() {
    echo -e "${Y}[1/3] Configurando CMake...${N}\n"

    # ── Arquitectura ──────────────────────────────────────────────────────────
    # znver1 = Ryzen 7 3700U (Zen+); cualquier otra CPU usa -march=native
    ARCH_FLAGS="-march=native -mtune=native"
    if grep -q "Ryzen 7 3700U" /proc/cpuinfo 2>/dev/null; then
        ARCH_FLAGS="-march=znver1 -mtune=znver1"
        echo -e "  ${C}[CPU] Ryzen 7 3700U detectado -> znver1 (Zen+, AVX2/FMA/BMI2)${N}"
    else
        CPU_MODEL=$(grep -m1 "model name" /proc/cpuinfo 2>/dev/null | cut -d: -f2 | xargs || echo "desconocida")
        echo -e "  ${C}[CPU] ${CPU_MODEL} -> native${N}"
    fi

    # ── Linker ────────────────────────────────────────────────────────────────
    # NOTA: mold tiene un bug con GCC 15/16 LTO + static libs (tag_array<T> se pierde
    # en el LTO link pass). Con LTO=ON se usa GNU ld que funciona correctamente.
    # mold es útil solo si se compila con LTO=OFF (ej: builds de debug rápidos).
    LINKER_FLAG=""
    echo -e "  ${C}[LINKER] GNU ld (LTO activo — mold incompatible con GCC16+nbt++)${N}"

    # ── Flags de compilacion optimizados para znver1 / GCC 16 ────────────────
    # -fno-plt                               : elimina indirección PLT en libs compartidas (Qt6)
    # -fomit-frame-pointer                   : libera un registro extra
    # -fno-semantic-interposition            : GCC optimiza definiciones propias sin overhead de LD_PRELOAD
    # -fno-asynchronous-unwind-tables (solo C): reduce tamaño .eh_frame (no aplica a C++ por exceptions)
    # NOTA: -ftree-vectorize y -ftree-slp-vectorize ya están implícitos en -O3; no se repiten.
    C_OPTS="-O3 ${ARCH_FLAGS} -ffunction-sections -fdata-sections -fvisibility=hidden \
-fno-plt -fomit-frame-pointer -fno-semantic-interposition \
-fno-asynchronous-unwind-tables -pipe"
    CXX_OPTS="-O3 ${ARCH_FLAGS} -ffunction-sections -fdata-sections -fvisibility=hidden \
-fvisibility-inlines-hidden -fno-plt -fomit-frame-pointer -fno-semantic-interposition -pipe"

    # ── Flags del linker ──────────────────────────────────────────────────────
    # --gc-sections    : elimina secciones no usadas (reduce tamaño ~10-15%)
    # --as-needed      : no linkea libs no usadas
    # -O1              : el linker reordena secciones para mejor cache locality
    # --sort-common    : agrupa variables globales por tamaño (reduce padding)
    #   → deprecado en binutils ≥2.38 — solo aplicar si la versión lo soporta sin warnings
    LD_OPTS="-Wl,--gc-sections,--as-needed,-O1"
    # Agregar --sort-common solo si el linker lo acepta sin warning
    if ld --help 2>&1 | grep -q -- '--sort-common'; then
        LD_OPTS="${LD_OPTS},--sort-common"
    fi

    # tomlplusplus_DIR: path varía por distro.
    # Fedora: /usr/lib64/cmake/tomlplusplus — Arch/CachyOS: /usr/lib/cmake/tomlplusplus
    # Si no se encuentra, el CMakeLists hace fallback a pkg-config automáticamente.
    TOML_DIR=""
    if [[ -f "/usr/lib64/cmake/tomlplusplus/tomlplusplusConfig.cmake" ]]; then
        TOML_DIR="-Dtomlplusplus_DIR=/usr/lib64/cmake/tomlplusplus"
    elif [[ -f "/usr/lib/cmake/tomlplusplus/tomlplusplusConfig.cmake" ]]; then
        TOML_DIR="-Dtomlplusplus_DIR=/usr/lib/cmake/tomlplusplus"
    fi
    # LTO=ON con libnbt++ excluido en su propio CMakeLists (fix GCC 15/16 tag_array bug).
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DENABLE_LTO=ON \
        "-DCMAKE_C_FLAGS_RELEASE=${C_OPTS}" \
        "-DCMAKE_CXX_FLAGS_RELEASE=${CXX_OPTS}" \
        "-DCMAKE_EXE_LINKER_FLAGS_RELEASE=${LD_OPTS}" \
        ${TOML_DIR:+"$TOML_DIR"} \
        -G Ninja
    echo -e "${G}[OK] Configure exitoso${N}\n"
}

do_build() {
    CPU_CORES=$(nproc 2>/dev/null || echo 4)
    echo -e "${Y}[2/3] Compilando con ${CPU_CORES} nucleos...${N}\n"
    START=$(date +%s)
    cmake --build build --parallel "$CPU_CORES"
    ELAPSED=$(( $(date +%s) - START ))
    echo -e "\n${G}[OK] Compilacion exitosa en ${ELAPSED}s${N}\n"
    EXE=$(find build -maxdepth 2 \( -name "beteliney" -o -name "BetelineyLauncher" \) 2>/dev/null | head -1)
    [[ -n "$EXE" ]] && echo -e "  Ejecutable: ${C}${EXE}${N} ($(du -sh "$EXE" | cut -f1))"
}


do_appimage() {
    echo -e "${Y}[APPIMAGE] Creando AppImage distribuible...${N}\n"
    [[ ! -f "build/CMakeCache.txt" ]] && echo -e "${R}[ERROR] Compila primero (opcion 1).${N}" && exit 1

    APPDIR="build/AppDir"
    rm -rf "$APPDIR" && mkdir -p "$APPDIR"
    DESTDIR="$APPDIR" cmake --install build

    LINUXDEPLOY="build/linuxdeploy-x86_64.AppImage"
    LINUXDEPLOY_QT="build/linuxdeploy-plugin-qt-x86_64.AppImage"

    if [[ ! -f "$LINUXDEPLOY" ]]; then
        echo -e "${Y}[INFO] Descargando linuxdeploy...${N}"
        wget -q --show-progress -O "$LINUXDEPLOY" \
            "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
        chmod +x "$LINUXDEPLOY"
    fi
    if [[ ! -f "$LINUXDEPLOY_QT" ]]; then
        echo -e "${Y}[INFO] Descargando linuxdeploy-plugin-qt...${N}"
        wget -q --show-progress -O "$LINUXDEPLOY_QT" \
            "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
        chmod +x "$LINUXDEPLOY_QT"
    fi

    DESKTOP_FILE=$(find "$APPDIR" -name "*.desktop" 2>/dev/null | head -1)
    ICON=$(find "$APPDIR" -name "*256*.png" 2>/dev/null | head -1)
    [[ -z "$ICON" ]] && ICON=$(find "$APPDIR" -name "*.png" 2>/dev/null | head -1)
    QMAKE=$(command -v qmake6 2>/dev/null || command -v qmake 2>/dev/null || true)
    OUTPUT_NAME="BetelineyLauncher-$(date +%Y%m%d)-x86_64.AppImage"

    QMAKE="$QMAKE" OUTPUT="build/$OUTPUT_NAME" \
    "$LINUXDEPLOY" --appdir "$APPDIR" \
        ${DESKTOP_FILE:+--desktop-file "$DESKTOP_FILE"} \
        ${ICON:+--icon-file "$ICON"} \
        --plugin qt --output appimage

    if [[ -f "build/$OUTPUT_NAME" ]]; then
        echo -e "\n${G}  AppImage: build/$OUTPUT_NAME ($(du -sh "build/$OUTPUT_NAME" | cut -f1))${N}"
        echo -e "  Uso: chmod +x build/$OUTPUT_NAME && ./build/$OUTPUT_NAME"
    else
        echo -e "${R}[ERROR] AppImage no generado.${N}"; exit 1
    fi
}

# ============================================================
# FLUJO PRINCIPAL
# ============================================================
case "$MODE" in
    4) install_deps; exit 0 ;;
    5) check_tools; ensure_libnbt; ensure_git; do_appimage; exit 0 ;;
    3) echo -e "${Y}[INFO] Limpiando build/...${N}"; rm -rf build; MODE=1 ;;
esac

check_tools
ensure_libnbt
ensure_git

case "$MODE" in
    1) do_configure; do_build ;;
    2)
        [[ ! -f "build/CMakeCache.txt" ]] && do_configure
        do_build ;;
esac

echo -e "\n${G}  BUILD EXITOSO — Ejecuta: ./build/beteliney${N}\n"
