#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

echo "== Validación de instalación RadhikaChain Core =="

command -v python3 >/dev/null || { echo "FAIL: python3 no encontrado"; exit 1; }
echo "OK: python3"

[[ -f scripts/karma-mine.py ]] || { echo "FAIL: falta scripts/karma-mine.py"; exit 1; }
echo "OK: minero PoK presente"

[[ -f .env.example ]] || { echo "FAIL: falta .env.example"; exit 1; }
echo "OK: plantilla .env.example"

if [[ -f .env ]]; then
  # shellcheck disable=SC1091
  set -a; source .env; set +a
  if [[ -n "${RPC_USER:-}" && -n "${RPC_PASS:-}" ]]; then
    echo "OK: RPC_USER/RPC_PASS definidos en .env local"
  else
    echo "WARN: .env existe pero RPC_USER/RPC_PASS vacíos"
  fi
else
  echo "INFO: copia .env.example → .env antes de minar"
fi

if python3 -c "import blake3" 2>/dev/null; then
  echo "OK: módulo blake3 instalado"
else
  echo "INFO: instala dependencias con: pip install -r requirements-miner.txt"
fi

if command -v radhika-cli >/dev/null; then
  echo "OK: radhika-cli en PATH"
elif command -v bitcoin-cli >/dev/null; then
  echo "WARN: radhika-cli no encontrado; puedes usar CLI_BIN=bitcoin-cli"
else
  echo "INFO: sin CLI local; el minero usará JSON-RPC si el nodo está accesible"
fi

echo "PASS: validación básica completada"