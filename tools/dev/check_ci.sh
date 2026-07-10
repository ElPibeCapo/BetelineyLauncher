#!/usr/bin/env bash
# check_ci.sh — estado de CI de ambos repos (launcher + meta), siempre con
# --repo explícito. Motivación (sesión 20 de ESTADO.md): al agregar el
# remote 'upstream' (PrismLauncher/PrismLauncher), 'gh' empezó a resolver
# por defecto a ESE repo en vez de ElPibeCapo/BetelineyLauncher, y
# 'gh run list' sin --repo devolvía corridas irrelevantes sin avisar.
#
# Uso: tools/dev/check_ci.sh [N]   (N = cantidad de corridas a mostrar, default 3)
set -uo pipefail

LAUNCHER_REPO="ElPibeCapo/BetelineyLauncher"
META_REPO="ElPibeCapo/meta"
N="${1:-3}"

echo "=== Repo default de 'gh' en este directorio (confirmar antes de confiar en nada sin --repo) ==="
gh repo view --json nameWithOwner -q .nameWithOwner 2>/dev/null || echo "(sin default resuelto)"
echo

echo "=== $LAUNCHER_REPO — últimas $N corridas ==="
gh run list --repo "$LAUNCHER_REPO" --limit "$N"
echo

echo "=== $META_REPO — últimas $N corridas ==="
gh run list --repo "$META_REPO" --limit "$N"
echo

echo "=== Secrets configurados en $LAUNCHER_REPO ==="
gh secret list --repo "$LAUNCHER_REPO"
