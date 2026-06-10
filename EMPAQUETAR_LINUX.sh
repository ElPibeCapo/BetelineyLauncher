#!/usr/bin/env bash
# ╔══════════════════════════════════════════════════════════════╗
# ║   BETELINEY LAUNCHER — EMPAQUETAR_LINUX.sh                  ║
# ║   Genera distribución portable para Linux                   ║
# ║   Opciones: AppImage · tar.gz portable · deb (Debian/Ubuntu)║
# ╚══════════════════════════════════════════════════════════════╝
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
DIST_DIR="$SCRIPT_DIR/dist"

R='\033[0;31m' G='\033[0;32m' Y='\033[1;33m' C='\033[0;36m'
W='\033[1;37m' DIM='\033[2m' BOLD='\033[1m' N='\033[0m'

ok()   { echo -e "  ${G}✔${N}  $*"; }
info() { echo -e "  ${C}●${N}  $*"; }
warn() { echo -e "  ${Y}⚠${N}  $*"; }
fail() { echo -e "  ${R}✘${N}  $*"; exit 1; }
sep()  { echo -e "  ${DIM}──────────────────────────────────────${N}"; }

get_version() {
    grep -m1 "Launcher_VERSION_MAJOR\|Launcher_VERSION_MINOR\|Launcher_VERSION_PATCH" \
        "$SCRIPT_DIR/CMakeLists.txt" 2>/dev/null | \
        grep -oP '[0-9]+' | paste -sd '.' || echo "7.3.0"
}

find_exe() {
    find "$BUILD_DIR" -maxdepth 4 \( -name "beteliney" -o -name "BetelineyLauncher" \) \
        -type f -perm /111 2>/dev/null | head -1 || true
}

check_build() {
    EXE=$(find_exe)
    if [[ -z "$EXE" ]]; then
        fail "Ejecutable no encontrado. Compila primero: bash COMPILAR_LINUX.sh → [1]"
    fi
    ok "Ejecutable: $EXE  ($(du -sh "$EXE" | cut -f1))"
}

make_targz() {
    sep; echo -e "  ${BOLD}${W}[MODO] tar.gz portable${N}"; sep
    local VERSION; VERSION=$(get_version)
    local ARCH; ARCH=$(uname -m)
    local OUT_NAME="BetelineyLauncher-${VERSION}-Linux-${ARCH}.tar.gz"
    local STAGE="$DIST_DIR/stage_targz/BetelineyLauncher"

    rm -rf "$DIST_DIR/stage_targz"
    mkdir -p "$STAGE"

    info "Copiando ejecutable..."
    cp "$EXE" "$STAGE/beteliney"
    chmod +x "$STAGE/beteliney"

    info "Resolviendo dependencias dinámicas con patchelf + ldd..."
    local LIBS_DIR="$STAGE/lib"
    mkdir -p "$LIBS_DIR"
    local skip_libs="libc.so|libm.so|libpthread.so|libdl.so|librt.so|ld-linux|libGL.so|libEGL.so|libvulkan"

    while IFS= read -r lib; do
        local libpath; libpath=$(echo "$lib" | awk '{print $3}')
        [[ -z "$libpath" || "$libpath" == "not" ]] && continue
        local basename; basename=$(basename "$libpath")
        echo "$basename" | grep -qE "$skip_libs" && continue
        [[ -f "$libpath" ]] && cp -n "$libpath" "$LIBS_DIR/" && echo -e "    ${DIM}+ $basename${N}"
    done < <(ldd "$EXE" 2>/dev/null)

    info "Copiando recursos Qt (qml, plugins, translations)..."
    local QT_PREFIX
    QT_PREFIX=$(pkg-config --variable=prefix Qt6Core 2>/dev/null || \
                dirname "$(dirname "$(which qmake6 2>/dev/null || which qmake)")" 2>/dev/null || echo "")
    if [[ -n "$QT_PREFIX" ]]; then
        for subdir in plugins translations; do
            [[ -d "$QT_PREFIX/$subdir" ]] && cp -r "$QT_PREFIX/$subdir" "$STAGE/" || true
        done
    fi

    info "Copiando jars y recursos del build..."
    [[ -d "$BUILD_DIR/jars" ]]   && cp -r "$BUILD_DIR/jars"   "$STAGE/"
    [[ -d "$BUILD_DIR/launcher" ]] && find "$BUILD_DIR/launcher" -name "*.qm" \
        -exec cp {} "$STAGE/" \; 2>/dev/null || true

    info "Creando script de lanzamiento..."
    cat > "$STAGE/run.sh" << 'RUN_EOF'
#!/usr/bin/env bash
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export LD_LIBRARY_PATH="$DIR/lib:$LD_LIBRARY_PATH"
export QT_PLUGIN_PATH="$DIR/plugins"
exec "$DIR/beteliney" "$@"
RUN_EOF
    chmod +x "$STAGE/run.sh"

    mkdir -p "$DIST_DIR"
    info "Comprimiendo en $OUT_NAME..."
    tar -czf "$DIST_DIR/$OUT_NAME" -C "$DIST_DIR/stage_targz" BetelineyLauncher
    rm -rf "$DIST_DIR/stage_targz"

    ok "Listo: $DIST_DIR/$OUT_NAME  ($(du -sh "$DIST_DIR/$OUT_NAME" | cut -f1))"
    echo ""
    echo -e "  ${DIM}Uso: tar -xzf $OUT_NAME && cd BetelineyLauncher && bash run.sh${N}"
}

make_appimage() {
    sep; echo -e "  ${BOLD}${W}[MODO] AppImage${N}"; sep

    # Verificar que el build está configurado
    [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]] && \
        fail "Build no configurado. Corre primero: bash COMPILAR_LINUX.sh → [1]"

    local VERSION; VERSION=$(get_version)
    local APPDIR="$BUILD_DIR/AppDir"
    local LINUXDEPLOY="$BUILD_DIR/linuxdeploy-x86_64.AppImage"
    local LINUXDEPLOY_QT="$BUILD_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"
    local OUTPUT_NAME="BetelineyLauncher-${VERSION}-x86_64.AppImage"

    # cmake --install para poblar el AppDir
    info "Instalando en AppDir..."
    rm -rf "$APPDIR" && mkdir -p "$APPDIR"
    DESTDIR="$APPDIR" cmake --install "$BUILD_DIR" 2>&1 | grep -v "^--" || true
    ok "AppDir poblado"

    # Descargar linuxdeploy si no está (con verificación de integridad básica)
    if [[ ! -f "$LINUXDEPLOY" || ! -x "$LINUXDEPLOY" ]]; then
        info "Descargando linuxdeploy..."
        local URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
        wget -q --show-progress -O "$LINUXDEPLOY" "$URL" || \
            fail "No se pudo descargar linuxdeploy. Verifica conexión."
        chmod +x "$LINUXDEPLOY"
        ok "linuxdeploy descargado"
    else
        ok "linuxdeploy ya presente"
    fi

    if [[ ! -f "$LINUXDEPLOY_QT" || ! -x "$LINUXDEPLOY_QT" ]]; then
        info "Descargando linuxdeploy-plugin-qt..."
        local URL_QT="https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
        wget -q --show-progress -O "$LINUXDEPLOY_QT" "$URL_QT" || \
            fail "No se pudo descargar linuxdeploy-plugin-qt. Verifica conexión."
        chmod +x "$LINUXDEPLOY_QT"
        ok "linuxdeploy-plugin-qt descargado"
    else
        ok "linuxdeploy-plugin-qt ya presente"
    fi

    # Verificar que FUSE está disponible (requerido para ejecutar AppImages)
    if ! command -v fusermount &>/dev/null && ! command -v fusermount3 &>/dev/null; then
        warn "fusermount no encontrado. Si linuxdeploy falla, instala: sudo dnf install fuse fuse3"
    fi

    local DESKTOP_FILE; DESKTOP_FILE=$(find "$APPDIR" -name "*.desktop" 2>/dev/null | head -1)
    local ICON; ICON=$(find "$APPDIR" -name "*256*.png" 2>/dev/null | head -1)
    [[ -z "$ICON" ]] && ICON=$(find "$APPDIR" -name "*.png" 2>/dev/null | head -1)
    local QMAKE; QMAKE=$(which qmake6 2>/dev/null || which qmake 2>/dev/null || echo "")

    [[ -z "$DESKTOP_FILE" ]] && warn "No se encontró .desktop en AppDir — el AppImage puede no tener metadata"
    [[ -z "$ICON" ]]         && warn "No se encontró ícono PNG — usando ícono genérico"
    [[ -z "$QMAKE" ]]        && fail "qmake6/qmake no encontrado. Necesario para linuxdeploy-plugin-qt"

    info "Generando AppImage con linuxdeploy..."
    mkdir -p "$DIST_DIR"
    QMAKE="$QMAKE" OUTPUT="$DIST_DIR/$OUTPUT_NAME" \
    "$LINUXDEPLOY" --appdir "$APPDIR" \
        ${DESKTOP_FILE:+--desktop-file "$DESKTOP_FILE"} \
        ${ICON:+--icon-file "$ICON"} \
        --plugin qt --output appimage 2>&1

    if [[ -f "$DIST_DIR/$OUTPUT_NAME" ]]; then
        chmod +x "$DIST_DIR/$OUTPUT_NAME"
        ok "AppImage generado: $DIST_DIR/$OUTPUT_NAME  ($(du -sh "$DIST_DIR/$OUTPUT_NAME" | cut -f1))"
        echo ""
        echo -e "  ${DIM}Uso: chmod +x $OUTPUT_NAME && ./$OUTPUT_NAME${N}"
        echo -e "  ${DIM}Nota: requiere FUSE en el sistema destino (fusermount o fusermount3)${N}"
    else
        fail "AppImage no generado — ver salida de linuxdeploy arriba"
    fi
}

make_deb() {
    sep; echo -e "  ${BOLD}${W}[MODO] .deb (Debian / Ubuntu / Nobara)${N}"; sep
    command -v dpkg-deb &>/dev/null || fail "dpkg-deb no encontrado. Solo disponible en sistemas Debian/Ubuntu."
    local VERSION; VERSION=$(get_version)
    local ARCH; ARCH=$(dpkg --print-architecture 2>/dev/null || echo "amd64")
    local PKG_NAME="beteliney-launcher_${VERSION}_${ARCH}"
    local PKG_DIR="$DIST_DIR/$PKG_NAME"

    rm -rf "$PKG_DIR"
    mkdir -p "$PKG_DIR/usr/bin"
    mkdir -p "$PKG_DIR/usr/share/applications"
    mkdir -p "$PKG_DIR/usr/share/icons/hicolor/256x256/apps"
    mkdir -p "$PKG_DIR/usr/share/icons/hicolor/scalable/apps"
    mkdir -p "$PKG_DIR/DEBIAN"

    cp "$EXE" "$PKG_DIR/usr/bin/beteliney"
    chmod +x "$PKG_DIR/usr/bin/beteliney"

    local PI="$SCRIPT_DIR/program_info"
    [[ -f "$PI/com.beteliney.BetelineyLauncher_256.png" ]] && \
        cp "$PI/com.beteliney.BetelineyLauncher_256.png" \
           "$PKG_DIR/usr/share/icons/hicolor/256x256/apps/com.beteliney.BetelineyLauncher.png"
    [[ -f "$PI/com.beteliney.BetelineyLauncher.svg" ]] && \
        cp "$PI/com.beteliney.BetelineyLauncher.svg" \
           "$PKG_DIR/usr/share/icons/hicolor/scalable/apps/"

    cat > "$PKG_DIR/usr/share/applications/com.beteliney.BetelineyLauncher.desktop" << DESK_EOF
[Desktop Entry]
Name=Beteliney Launcher
Comment=Minecraft Launcher personalizado
Exec=beteliney %u
Icon=com.beteliney.BetelineyLauncher
Terminal=false
Type=Application
Categories=Game;
MimeType=application/x-modrinth-modpack+zip;
StartupWMClass=beteliney
DESK_EOF

    local INSTALLED_SIZE; INSTALLED_SIZE=$(du -sk "$PKG_DIR/usr" | cut -f1)
    cat > "$PKG_DIR/DEBIAN/control" << CTRL_EOF
Package: beteliney-launcher
Version: ${VERSION}
Section: games
Priority: optional
Architecture: ${ARCH}
Installed-Size: ${INSTALLED_SIZE}
Maintainer: pibe <beteliney.github.io>
Description: Beteliney Launcher
 Custom Minecraft launcher basado en Prism Launcher.
 Incluye perfiles JVM Aikar, tema gamer neon, soporte No-Premium.
CTRL_EOF

    dpkg-deb --build "$PKG_DIR" "$DIST_DIR/${PKG_NAME}.deb" 2>&1
    rm -rf "$PKG_DIR"
    ok "Listo: $DIST_DIR/${PKG_NAME}.deb  ($(du -sh "$DIST_DIR/${PKG_NAME}.deb" | cut -f1))"
}

# ── MENÚ PRINCIPAL ──────────────────────────────────────────────
echo ""
echo -e "  ${C}${BOLD}  ⬡  BETELINEY LAUNCHER — EMPAQUETAR LINUX${N}"
echo -e "  ${DIM}──────────────────────────────────────────${N}"
echo ""
echo -e "  ${C}[1]${N}  ${W}tar.gz portable${N}   ${DIM}sin FUSE, funciona en WSL/containers/cualquier Linux${N}"
echo -e "  ${C}[2]${N}  ${W}AppImage${N}           ${DIM}requiere FUSE (la mayoría de escritorios lo tienen)${N}"
echo -e "  ${C}[3]${N}  ${W}.deb${N}               ${DIM}solo Debian / Ubuntu / sistemas basados en apt${N}"
echo -e "  ${C}[0]${N}  ${R}Salir${N}"
echo ""
read -rp "  Opción [1]: " MODE
MODE="${MODE:-1}"
[[ "$MODE" == "0" ]] && exit 0

check_build
mkdir -p "$DIST_DIR"

case "$MODE" in
    1) make_targz ;;
    2) make_appimage ;;
    3) make_deb ;;
    *) warn "Opción inválida." ;;
esac
