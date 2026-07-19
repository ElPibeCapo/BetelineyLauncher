#!/usr/bin/env bash
# claude_guard_git_push.sh — hook PreToolUse de Claude Code (NO es un git hook,
# ver tools/dev/install_hooks.sh para eso). Intercepta cualquier tool_use de
# Bash antes de ejecutarse: si el comando contiene "git push" (cualquier
# variante: --force, con remote/rama explícitos, en este repo o en otro vía
# cd/ruta absoluta como ~/Descargas/meta_beteliney), fuerza el prompt de
# permiso normal de Claude Code en vez de dejarlo pasar solo porque el agente
# lo considera autorizado por el system prompt.
#
# Por qué existe: la regla "nunca git push sin autorización explícita" vivía
# solo como texto en CLAUDE.md y en cada agente — confiable la mayoría de las
# veces, pero no determinístico (un system prompt largo, un jailbreak, o el
# modelo simplemente equivocándose puede saltarla). Este script la convierte
# en una capa que no depende de que el modelo "se acuerde" de la regla.
set -euo pipefail

INPUT=$(cat)
COMMAND=$(echo "$INPUT" | jq -r '.tool_input.command // empty')

if [ -z "$COMMAND" ]; then
  exit 0
fi

# Detecta "git push" como subcomando real (no como substring de otra palabra,
# ej. no dispara con "echo 'git push'" dentro de un string de otro comando que
# no es git — igual se prioriza no dejar pasar falsos negativos sobre evitar
# falsos positivos, dado el costo asimétrico de cada error acá).
if echo "$COMMAND" | grep -qiE '(^|[;&|]|\bgit[[:space:]]+)push([[:space:]]|$)' && echo "$COMMAND" | grep -qiE '\bgit\b'; then
  cat <<EOF
{
  "hookSpecificOutput": {
    "hookEventName": "PreToolUse",
    "permissionDecision": "ask",
    "permissionDecisionReason": "git push detectado — requiere tu autorización explícita en esta conversación (regla del proyecto, ver CLAUDE.md). Aplica a este repo y a cualquier otro (ej. meta) alcanzado por el comando."
  }
}
EOF
  exit 0
fi

exit 0
