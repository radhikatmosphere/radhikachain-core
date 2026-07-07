#!/usr/bin/env bash
# Falla si hay credenciales reales hardcodeadas en archivos versionados.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

FAIL=0
SCAN_PATHS=(scripts src contrib)

echo "== RadhikaChain secret scan =="

# 1) Literales sospechosos (no variables de entorno ni plantillas)
while IFS= read -r line; do
  file="${line%%:*}"
  [[ "$file" == *check-secrets.sh ]] && continue
  # Ignorar referencias a variables (${VAR}, ENV.get, os.environ, RPC_PASS, etc.)
  if echo "$line" | grep -qE '(\$\{|ENV\.get|os\.environ|getenv|RPC_PASS|RPC_USER|__SET_ME__|secure_password_here|\.example)'; then
    continue
  fi
  echo "$line"
  FAIL=1
done < <(
  grep -rInE '(password\s*=\s*["\x27][^"\x27]{8,}|secret\s*=\s*["\x27][^"\x27]{8,}|api[_-]?key\s*=\s*["\x27]|private[_-]?key\s*=\s*["\x27]|rpcauth=[^$]+)' \
    "${SCAN_PATHS[@]}" 2>/dev/null || true
)

if [[ "$FAIL" -eq 0 ]]; then
  echo "OK: sin literales de secreto en código"
fi

# 2) Tokens de proveedores (excluye este script)
if grep -rInE '(sk_live_[A-Za-z0-9]+|sk_test_[A-Za-z0-9]+|ghp_[A-Za-z0-9]{20,}|AKIA[0-9A-Z]{16}|whsec_[A-Za-z0-9]+)' \
  "${SCAN_PATHS[@]}" 2>/dev/null | grep -v check-secrets.sh; then
  echo "FAIL: token de proveedor detectado"
  FAIL=1
else
  echo "OK: sin tokens de proveedor"
fi

# 3) .env no debe estar en git
if git ls-files --error-unmatch .env &>/dev/null; then
  echo "FAIL: .env está versionado"
  FAIL=1
else
  echo "OK: .env no versionado"
fi

if [[ "$FAIL" -ne 0 ]]; then
  exit 1
fi

echo "PASS: repositorio limpio de secretos hardcodeados"