#!/usr/bin/env python3
"""
verify_presets.py — Verifica en vivo que las URLs y hashes SHA-512 de los
mods embebidos en BetelineyPresets.h sigan siendo válidos contra el CDN
real de Modrinth.

Motivación (sesión 26 de ESTADO.md): los 5 mods de los presets built-in
("Vanilla Optimizado" y "PvP Competitivo") quedaron con URLs 404 porque
Modrinth movió/eliminó esas versiones — nadie lo detectó hasta una
auditoría manual, y mientras tanto el 100% de las instalaciones de esos
presets fallaban. Este script automatiza esa verificación para no
depender de que alguien lo note por accidente.

Uso:
    python3 tools/dev/verify_presets.py [ruta/a/BetelineyPresets.h]

Exit code 0 si todo está OK, 1 si algo falló (URL muerta o hash no coincide).
"""
import hashlib
import re
import sys
import urllib.request

DEFAULT_PATH = "launcher/modplatform/beteliney/BetelineyPresets.h"

URL_RE = re.compile(r'"(https://cdn\.modrinth\.com/[^"]+)"')
SHA512_RE = re.compile(r'"([a-fA-F0-9]{128})"')
FILENAME_RE = re.compile(r'"([^"/]+\.jar)"')


def find_mod_blocks(text: str):
    """Devuelve cada bloque { ... } balanceado del archivo (a cualquier
    profundidad de anidamiento, no solo los de nivel superior — ese fue
    un bug real de la primera versión de este script: solo capturaba el
    bloque exterior completo del namespace, y luego .search() se quedaba
    con la primera coincidencia de todo el archivo, ignorando el resto).

    Nos quedamos únicamente con los bloques "hoja": los que tienen
    exactamente una URL y un hash SHA-512 (un bloque PackMod real).
    Los bloques que los contienen (preset, namespace) también matchean
    los patrones pero con más de una ocurrencia — se descartan con eso."""
    stack = []
    all_blocks = []
    for i, ch in enumerate(text):
        if ch == "{":
            stack.append(i)
        elif ch == "}":
            if stack:
                start = stack.pop()
                all_blocks.append(text[start:i + 1])

    leaf_blocks = []
    for block in all_blocks:
        if len(URL_RE.findall(block)) == 1 and len(SHA512_RE.findall(block)) == 1:
            leaf_blocks.append(block)
    return leaf_blocks


def check_mod(url: str, expected_sha512: str):
    try:
        req = urllib.request.Request(url, method="GET")
        with urllib.request.urlopen(req, timeout=30) as resp:
            if resp.status != 200:
                return False, f"HTTP {resp.status}"
            data = resp.read()
    except Exception as e:
        return False, f"error de red: {e}"

    real_hash = hashlib.sha512(data).hexdigest()
    if real_hash.lower() != expected_sha512.lower():
        return False, f"hash no coincide (esperado {expected_sha512[:16]}…, real {real_hash[:16]}…)"
    return True, f"OK ({len(data):,} bytes)"


def main():
    path = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_PATH
    try:
        with open(path, encoding="utf-8") as f:
            text = f.read()
    except FileNotFoundError:
        print(f"No encontré {path}. Pasá la ruta como argumento.")
        sys.exit(2)

    # C++ concatena literales de string adyacentes ("a" "b" == "ab"), y
    # BetelineyPresets.h parte URLs largas en dos líneas justo así. Sin
    # esto, la URL quedaba truncada en el directorio y siempre daba 404
    # aunque el archivo real existiera — bug real encontrado al probar
    # este mismo script contra el archivo de verdad.
    text = re.sub(r'"\s*"', "", text)

    checked = 0
    failed = 0
    for block in find_mod_blocks(text):
        url_m = URL_RE.search(block)
        sha_m = SHA512_RE.search(block)
        if not (url_m and sha_m):
            continue
        url = url_m.group(1)
        sha512 = sha_m.group(1)
        name_m = FILENAME_RE.search(block)
        name = name_m.group(1) if name_m else url.rsplit("/", 1)[-1]

        checked += 1
        ok, detail = check_mod(url, sha512)
        print(f"{'✅' if ok else '❌'} {name}: {detail}")
        if not ok:
            failed += 1

    print()
    print(f"Total: {checked} mods verificados, {failed} con problemas.")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
