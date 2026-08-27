#!/usr/bin/env bash
# win_collect_dlls.sh - copia al destino TODAS las DLL de MinGW que necesita un
# binario de Windows, recorriendo la tabla de importaciones de forma recursiva.
#
#   tools/dev/win_collect_dlls.sh <destino> <binario> [binario...]
#
# Reemplaza la lista escrita a mano que habia antes. Esa lista no podia seguirle
# el paso a las dependencias transitivas: cada vez que Qt o una libreria sumaba
# una dependencia nueva, el paquete salia roto y solo se descubria al ejecutarlo
# en un Windows limpio.
#
# Solo copia lo que vive bajo el prefijo de MinGW. Las DLL del sistema
# (KERNEL32, USER32, ...) se quedan donde estan, que es lo correcto.

set -euo pipefail

DESTINO="${1:?Falta el directorio destino}"
shift
[ $# -gt 0 ] || { echo "Falta al menos un binario"; exit 1; }

PREFIJO="${MINGW_PREFIX:-/mingw64}"
BIN_DIR="$PREFIJO/bin"
mkdir -p "$DESTINO"

declare -A vistos=()
pendientes=("$@")
i=0
copiadas=0

while [ $i -lt ${#pendientes[@]} ]; do
    actual="${pendientes[$i]}"
    i=$((i + 1))
    [ -f "$actual" ] || continue
    while read -r nombre; do
        [ -n "$nombre" ] || continue
        clave="${nombre,,}"
        [ -n "${vistos[$clave]:-}" ] && continue
        vistos[$clave]=1
        origen="$BIN_DIR/$nombre"
        if [ -f "$origen" ]; then
            cp -f "$origen" "$DESTINO/"
            pendientes+=("$origen")
            copiadas=$((copiadas + 1))
        fi
    done < <(objdump -p "$actual" 2>/dev/null | awk '/DLL Name:/ {print $3}')
done

echo "  $copiadas DLL de MinGW copiadas a $DESTINO"

# Verificacion final: ninguna importacion de MinGW puede quedar sin resolver.
# Si falta algo, corta aca y no se publica un paquete roto.
faltan=0
for binario in "$DESTINO"/*.exe "$DESTINO"/*.dll "$DESTINO"/*/*.dll; do
    [ -f "$binario" ] || continue
    while read -r nombre; do
        [ -n "$nombre" ] || continue
        [ -f "$DESTINO/$nombre" ] && continue
        if [ -f "$BIN_DIR/$nombre" ]; then
            echo "  FALTA: $nombre (la necesita $(basename "$binario"))"
            faltan=$((faltan + 1))
        fi
    done < <(objdump -p "$binario" 2>/dev/null | awk '/DLL Name:/ {print $3}')
done

if [ $faltan -gt 0 ]; then
    echo "ERROR: $faltan dependencia(s) sin resolver en el paquete."
    exit 1
fi
echo "  Sin dependencias de MinGW sin resolver."
