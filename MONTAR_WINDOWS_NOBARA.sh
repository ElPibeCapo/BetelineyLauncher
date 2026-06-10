#!/usr/bin/env bash
# =============================================================
#  MONTAR_WINDOWS_NOBARA.sh
#  Monta la particion Windows (NTFS) desde Nobara Linux
#  y da acceso directo al proyecto BetelineyLauncher.
#
#  Uso: bash MONTAR_WINDOWS_NOBARA.sh
# =============================================================

set -e
R='\033[0;31m' G='\033[0;32m' Y='\033[1;33m' C='\033[0;36m' N='\033[0m'

echo -e "${C}"
echo "  =============================================="
echo "    Beteliney - Montar Windows desde Nobara"
echo "  =============================================="
echo -e "${N}"

# ============================================================
# DETECTAR PARTICION NTFS DE WINDOWS
# ============================================================
echo -e "${Y}[INFO] Buscando particion NTFS (Windows)...${N}"

# Buscar la primera particion NTFS que no este montada ya
WIN_DEV=""
WIN_LABEL=""
while IFS= read -r line; do
    DEV=$(echo "$line" | awk '{print $1}')
    FSTYPE=$(echo "$line" | awk '{print $2}')
    LABEL=$(echo "$line" | awk '{print $3}')
    MOUNTPOINT=$(echo "$line" | awk '{print $4}')

    if [[ "$FSTYPE" == "ntfs" || "$FSTYPE" == "ntfs3" ]]; then
        if [[ -z "$MOUNTPOINT" || "$MOUNTPOINT" == "" ]]; then
            WIN_DEV="$DEV"
            WIN_LABEL="$LABEL"
            break
        elif [[ -n "$MOUNTPOINT" ]]; then
            # Ya montada — usarla directamente
            WIN_DEV="$DEV"
            WIN_LABEL="$LABEL"
            WIN_MOUNT="$MOUNTPOINT"
            echo -e "${G}[OK] Particion Windows ya montada en: ${WIN_MOUNT}${N}"
            break
        fi
    fi
done < <(lsblk -o PATH,FSTYPE,LABEL,MOUNTPOINT -n -p 2>/dev/null)

if [[ -z "$WIN_DEV" ]]; then
    echo -e "${R}[ERROR] No se encontro particion NTFS.${N}"
    echo "  Verifica con: lsblk -o PATH,FSTYPE,LABEL,MOUNTPOINT"
    exit 1
fi

echo -e "  Dispositivo : ${C}${WIN_DEV}${N}"
[[ -n "$WIN_LABEL" ]] && echo -e "  Etiqueta    : ${WIN_LABEL}"

# ============================================================
# MONTAR SI NO ESTA MONTADA
# ============================================================
if [[ -z "$WIN_MOUNT" ]]; then
    WIN_MOUNT="/mnt/windows"
    echo -e "${Y}[INFO] Montando en ${WIN_MOUNT}...${N}"

    sudo mkdir -p "$WIN_MOUNT"

    # ntfs3 es el driver moderno del kernel (disponible desde 5.15+)
    # Si falla, intentar con ntfs-3g
    if ! sudo mount -t ntfs3 -o uid=$(id -u),gid=$(id -g),fmask=0022,dmask=0022 \
            "$WIN_DEV" "$WIN_MOUNT" 2>/dev/null; then
        echo -e "${Y}[INFO] ntfs3 no disponible, usando ntfs-3g...${N}"
        if ! command -v ntfs-3g &>/dev/null; then
            echo -e "${Y}[INFO] Instalando ntfs-3g...${N}"
            sudo dnf install -y ntfs-3g 2>/dev/null || \
            sudo apt install -y ntfs-3g 2>/dev/null || \
            sudo pacman -S --needed ntfs-3g 2>/dev/null
        fi
        sudo ntfs-3g -o uid=$(id -u),gid=$(id -g),fmask=0022,dmask=0022 \
            "$WIN_DEV" "$WIN_MOUNT"
    fi
    echo -e "${G}[OK] Montado en ${WIN_MOUNT}${N}"
fi

# ============================================================
# UBICAR EL PROYECTO
# ============================================================

# Busca CMakeLists.txt del proyecto en toda la particion montada
# (evita hardcodear rutas de usuario como Users/SENA)
echo -e "${Y}[INFO] Buscando proyecto BetelineyLauncher en ${WIN_MOUNT}...${N}"

PROJECT_DIR=""

# 1. Busqueda rapida en rutas tipicas primero (mas comun)
QUICK_PATHS=(
    "$WIN_MOUNT/BetelineyLauncher_v7/BetelineyLauncher/source"
)
# Agrega Users/*/BetelineyLauncher_v7 sin hardcodear el usuario
if [[ -d "$WIN_MOUNT/Users" ]]; then
    while IFS= read -r userdir; do
        QUICK_PATHS+=("$userdir/BetelineyLauncher_v7/BetelineyLauncher/source")
        QUICK_PATHS+=("$userdir/Desktop/BetelineyLauncher_v7/BetelineyLauncher/source")
        QUICK_PATHS+=("$userdir/Documents/BetelineyLauncher_v7/BetelineyLauncher/source")
    done < <(find "$WIN_MOUNT/Users" -maxdepth 1 -mindepth 1 -type d 2>/dev/null | grep -v "Public\|Default\|All Users")
fi

for p in "${QUICK_PATHS[@]}"; do
    if [[ -f "$p/CMakeLists.txt" ]]; then
        PROJECT_DIR="$p"
        break
    fi
done

# 2. Fallback: busqueda profunda por CMakeLists con nombre de proyecto
if [[ -z "$PROJECT_DIR" ]]; then
    echo -e "${Y}[INFO] Busqueda rapida sin resultado, buscando en profundidad...${N}"
    PROJECT_DIR=$(find "$WIN_MOUNT" -maxdepth 8 -name "CMakeLists.txt" 2>/dev/null \
        -exec grep -l "BetelineyLauncher\|Beteliney" {} \; 2>/dev/null | \
        head -1 | xargs -r dirname)
fi

echo ""
if [[ -n "$PROJECT_DIR" ]]; then
    echo -e "${G}  =============================================="
    echo -e "   Proyecto encontrado:"
    echo -e "   ${PROJECT_DIR}"
    echo -e "  ==============================================${N}"
    echo ""
    echo "  Para compilar en Linux:"
    echo -e "  ${C}cd \"$PROJECT_DIR\" && bash COMPILAR_LINUX.sh${N}"
    echo ""
    echo "  Para copiar al home (recomendado para builds frecuentes):"
    DEST="$HOME/BetelineyLauncher_v7"
    echo -e "  ${C}cp -r \"$(dirname "$(dirname "$PROJECT_DIR")")\" \"$DEST\"${N}"
    echo -e "  ${C}cd \"$DEST/BetelineyLauncher/source\" && bash COMPILAR_LINUX.sh${N}"
    echo ""

    read -rp "  Copiar proyecto al home ahora? [s/N]: " COPY_CHOICE
    if [[ "${COPY_CHOICE,,}" == "s" ]]; then
        SRC_ROOT="$(dirname "$(dirname "$PROJECT_DIR")")"
        DEST="$HOME/BetelineyLauncher_v7"
        if [[ -d "$DEST" ]]; then
            echo -e "${Y}[INFO] Ya existe $DEST. Sobreescribir? [s/N]: ${N}"
            read -r OVR
            [[ "${OVR,,}" != "s" ]] && echo "Cancelado." && exit 0
            rm -rf "$DEST"
        fi
        echo -e "${Y}[INFO] Copiando...${N}"
        cp -r "$SRC_ROOT" "$DEST"
        echo -e "${G}[OK] Copiado a $DEST${N}"
        echo ""
        echo -e "  Ahora ejecuta:"
        echo -e "  ${C}cd $DEST/BetelineyLauncher/source && bash COMPILAR_LINUX.sh${N}"
    fi
else
    echo -e "${Y}[WARN] Proyecto BetelineyLauncher no encontrado en rutas conocidas.${N}"
    echo "  Busca manualmente en: $WIN_MOUNT"
    echo "  Luego ejecuta: bash COMPILAR_LINUX.sh desde la carpeta source/"
fi
echo ""
