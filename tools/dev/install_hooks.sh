#!/usr/bin/env bash
# install_hooks.sh — instala secret_scan.sh como git pre-commit hook.
set -euo pipefail
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
HOOK="$REPO_ROOT/.git/hooks/pre-commit"

cat > "$HOOK" <<'EOF'
#!/usr/bin/env bash
"$(git rev-parse --show-toplevel)/tools/dev/secret_scan.sh"
EOF
chmod +x "$HOOK"
echo "pre-commit hook instalado en $HOOK"
echo "A partir de ahora, cada 'git commit' corre secret_scan.sh automáticamente."
