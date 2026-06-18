#!/bin/bash
# EMPAQUETAR_APPIMAGE.sh — Genera un AppImage de BetelineyLauncher
# Ejecutar desde el directorio raíz del source después de compilar con:
#   cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ...
#   ninja -C build
#
# Requisitos: appimagetool, linuxdeploy (se descargan automáticamente si no están)
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR"
BUILD_DIR="$SOURCE_DIR/build"
DIST_DIR="$SOURCE_DIR/dist"
APPDIR="$DIST_DIR/BetelineyLauncher.AppDir"

# Obtener versión
VER=$(grep -oP 'Launcher_VERSION_NAME "\K[\d.]+' "$SOURCE_DIR/CMakeLists.txt" | head -1)
ARCH=$(uname -m)
OUTPUT="$DIST_DIR/BetelineyLauncher-${VER}-Linux-${ARCH}.AppImage"

echo "==> Construyendo AppImage v${VER} para ${ARCH}"

# ─── Descargar herramientas si no están ───────────────────────────────────────
TOOLS_DIR="$DIST_DIR/tools"
mkdir -p "$TOOLS_DIR"

if [ ! -f "$TOOLS_DIR/linuxdeploy" ]; then
    echo "==> Descargando linuxdeploy..."
    wget -q "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${ARCH}.AppImage" \
        -O "$TOOLS_DIR/linuxdeploy"
    chmod +x "$TOOLS_DIR/linuxdeploy"
fi

if [ ! -f "$TOOLS_DIR/linuxdeploy-plugin-qt" ]; then
    echo "==> Descargando linuxdeploy-plugin-qt..."
    wget -q "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-${ARCH}.AppImage" \
        -O "$TOOLS_DIR/linuxdeploy-plugin-qt"
    chmod +x "$TOOLS_DIR/linuxdeploy-plugin-qt"
fi

if [ ! -f "$TOOLS_DIR/appimagetool" ]; then
    echo "==> Descargando appimagetool..."
    wget -q "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-${ARCH}.AppImage" \
        -O "$TOOLS_DIR/appimagetool"
    chmod +x "$TOOLS_DIR/appimagetool"
fi

# ─── Preparar AppDir ──────────────────────────────────────────────────────────
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/share/beteliney/jars" \
         "$APPDIR/usr/share/icons/hicolor/256x256/apps" \
         "$APPDIR/usr/share/icons/hicolor/scalable/apps" \
         "$APPDIR/usr/share/applications" \
         "$APPDIR/usr/share/metainfo"

# Binario principal
cp "$BUILD_DIR/beteliney" "$APPDIR/usr/bin/beteliney"
chmod +x "$APPDIR/usr/bin/beteliney"

# JARs
find "$BUILD_DIR/jars" "$BUILD_DIR/libraries" -name "*.jar" -exec cp {} "$APPDIR/usr/share/beteliney/jars/" \; 2>/dev/null || true

# Íconos
cp "$SOURCE_DIR/program_info/com.beteliney.BetelineyLauncher_256.png" \
   "$APPDIR/usr/share/icons/hicolor/256x256/apps/com.beteliney.BetelineyLauncher.png"
cp "$SOURCE_DIR/program_info/com.beteliney.BetelineyLauncher.svg" \
   "$APPDIR/usr/share/icons/hicolor/scalable/apps/com.beteliney.BetelineyLauncher.svg"

# Desktop entry
sed "s|@Launcher_DisplayName@|Beteliney Launcher|g; \
     s|@Launcher_APP_BINARY_NAME@|beteliney|g; \
     s|@Launcher_AppID@|com.beteliney.BetelineyLauncher|g; \
     s|@Launcher_CommonName@|Beteliney Launcher|g" \
    "$SOURCE_DIR/program_info/com.beteliney.BetelineyLauncher.desktop.in" \
    > "$APPDIR/usr/share/applications/com.beteliney.BetelineyLauncher.desktop"

# Metainfo
sed "s|@Launcher_VERSION_NAME@|${VER}|g; s|@Launcher_BUILD_TIMESTAMP@|$(date +%Y-%m-%d)|g; \
     s|@Launcher_AppID@|com.beteliney.BetelineyLauncher|g" \
    "$SOURCE_DIR/program_info/com.beteliney.BetelineyLauncher.metainfo.xml.in" \
    > "$APPDIR/usr/share/metainfo/com.beteliney.BetelineyLauncher.metainfo.xml"

# AppStream symlink a appdata.xml (compatibilidad)
ln -sf "../metainfo/com.beteliney.BetelineyLauncher.metainfo.xml" \
       "$APPDIR/usr/share/applications/com.beteliney.BetelineyLauncher.appdata.xml" 2>/dev/null || true

# ─── Deploy Qt y generar AppImage ─────────────────────────────────────────────
echo "==> Deployando Qt con linuxdeploy..."
export QMAKE="$(which qmake6 2>/dev/null || which qmake)"
export OUTPUT="$OUTPUT"
export PATH="$TOOLS_DIR:$PATH"

APPIMAGE_EXTRACT_AND_RUN=1 linuxdeploy \
    --appdir "$APPDIR" \
    --plugin qt \
    --desktop-file "$APPDIR/usr/share/applications/com.beteliney.BetelineyLauncher.desktop" \
    --icon-file "$APPDIR/usr/share/icons/hicolor/256x256/apps/com.beteliney.BetelineyLauncher.png" \
    --output appimage

echo ""
echo "==> AppImage generado: $OUTPUT"
echo "==> Tamaño: $(du -sh "$OUTPUT" 2>/dev/null | cut -f1)"
