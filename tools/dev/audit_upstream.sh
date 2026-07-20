#!/usr/bin/env bash
# audit_upstream.sh — automatiza el diff heredado-vs-upstream que hasta la
# creación de docs/AUDITORIA_MODULOS.md se hacía a mano, comando por
# comando. Motivación: la única vía sostenible de mantener seguro código
# heredado de Prism sin reauditar línea por línea es aprovechar el trabajo
# de la comunidad upstream — pero solo si el diff se corre con regularidad,
# no una vez y se olvida (exactamente lo que pasó con minecraft/auth/ y
# modplatform/, cero commits de revisión propia desde el fork).
#
# Qué hace:
#   1. Verifica/agrega el remote 'upstream' (PrismLauncher/PrismLauncher) y
#      trae tags nuevos.
#   2. Para cada carpeta heredada listada abajo, busca en upstream, desde
#      el commit de fork hasta el último tag remoto, commits con palabras
#      clave de seguridad.
#   3. Para cada uno, chequea con 'merge-base --is-ancestor' si ya está
#      aplicado en el 'main' local — si no, lo marca como pendiente real.
#
# Correr periódicamente (no solo cuando surge la duda) y volcar lo que
# salga acá a docs/AUDITORIA_MODULOS.md — este script detecta, no decide
# ni aplica nada por su cuenta.

set -uo pipefail

REPO_ROOT="$(git rev-parse --show-toplevel 2>/dev/null)"
if [[ -z "$REPO_ROOT" ]]; then
  echo "No estas dentro de un repo git."
  exit 1
fi
cd "$REPO_ROOT"

# FORK_COMMIT es el primer commit de ESTE repo (fork sin historia
# compartida con upstream — no sirve como base de rango contra tags de
# upstream, git log A..B necesita que A viva en la misma linea que B).
# FORK_TAG es el tag real de upstream con el que este commit tiene diff
# limpio en archivos no tocados por branding (verificado con
# 'git diff 09eb67f74 11.0.0 -- <archivo-sin-branding>') — ese es el que
# se usa como base para comparar contra upstream.
FORK_COMMIT="09eb67f74"
FORK_TAG="11.0.0"
UPSTREAM_URL="https://github.com/PrismLauncher/PrismLauncher.git"

KEYWORDS='security|fix.*crash|overflow|traversal|CVE|leak|vulnerab|exploit|sanitiz|use-after-free|uaf|double.free|null.*deref'

# Carpetas heredadas (100% o mayormente) — ver docs/AUDITORIA_MODULOS.md
# para el detalle de estado real de cada una.
FOLDERS=(
  "launcher/net"
  "launcher/meta"
  "launcher/migration"
  "launcher/minecraft/mod"
  "launcher/minecraft/launch"
  "launcher/minecraft/auth"
  "launcher/minecraft/skins"
  "launcher/minecraft/update"
  "launcher/launch"
  "launcher/modplatform"
  "launcher/tasks"
  "launcher/java"
  "launcher/settings"
  "launcher/archive"
  "launcher/console"
  "launcher/filelink"
  "launcher/news"
  "launcher/icons"
)

echo "== 1. Remote upstream =="
if ! git remote get-url upstream >/dev/null 2>&1; then
  echo "Agregando remote upstream ($UPSTREAM_URL)..."
  git remote add upstream "$UPSTREAM_URL"
fi
git fetch upstream --tags --quiet 2>&1 | grep -v '^$' || true

LATEST_TAG=$(git ls-remote --tags --refs upstream 2>/dev/null \
  | awk -F/ '{print $NF}' \
  | grep -E '^[0-9]+\.[0-9]+(\.[0-9]+)?$' \
  | sort -V | tail -n1)

if [[ -z "$LATEST_TAG" ]]; then
  echo "No se pudo determinar el ultimo tag de upstream. Abortando."
  exit 1
fi
echo "Ultimo tag de upstream: $LATEST_TAG"
echo

if ! git rev-parse "$LATEST_TAG" >/dev/null 2>&1; then
  echo "Tag $LATEST_TAG no esta local todavia, trayendolo..."
  git fetch upstream "refs/tags/${LATEST_TAG}:refs/tags/${LATEST_TAG}" --quiet
fi

echo "== 2. Commits de seguridad en upstream, filtrados por carpeta =="
TOTAL_FOUND=0
TOTAL_PENDING=0

for folder in "${FOLDERS[@]}"; do
  if [[ ! -d "$folder" ]]; then
    continue
  fi

  if ! COMMITS=$(git log --oneline --no-merges -i -E --grep="$KEYWORDS" \
    "${FORK_TAG}..${LATEST_TAG}" -- "$folder"); then
    echo "ERROR corriendo git log sobre $folder — abortando (no asumir 'sin hallazgos')."
    exit 2
  fi

  if [[ -z "$COMMITS" ]]; then
    continue
  fi

  echo "--- $folder ---"
  while IFS= read -r line; do
    hash="${line%% *}"
    TOTAL_FOUND=$((TOTAL_FOUND + 1))
    if git merge-base --is-ancestor "$hash" HEAD 2>/dev/null; then
      echo "  [ya aplicado]  $line"
    else
      echo "  [PENDIENTE]    $line"
      TOTAL_PENDING=$((TOTAL_PENDING + 1))
    fi
  done <<< "$COMMITS"
  echo
done

echo "== Resumen =="
echo "Commits de seguridad encontrados en el rango: $TOTAL_FOUND"
echo "Pendientes de aplicar (no ancestros de HEAD): $TOTAL_PENDING"
echo
if [[ "$TOTAL_PENDING" -gt 0 ]]; then
  echo "Hay hallazgos pendientes. Antes de actuar sobre ellos:"
  echo "  1. Revisar cada commit con 'git show <hash>' contra upstream."
  echo "  2. Actualizar docs/AUDITORIA_MODULOS.md con lo encontrado."
  echo "  3. Cherry-pick requiere el mismo criterio de siempre para"
  echo "     cambios al arbol de trabajo — no aplicar nada sin evaluar"
  echo "     antes si el archivo fue modificado localmente (riesgo de"
  echo "     conflicto) o si el fix ya se hizo distinto en este fork."
  exit 1
fi

echo "Nada pendiente detectado con las palabras clave actuales. Esto NO"
echo "es lo mismo que 'todo revisado' — solo cubre lo que upstream mismo"
echo "etiqueto como fix de seguridad en su mensaje de commit."
exit 0
