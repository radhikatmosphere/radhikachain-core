#!/usr/bin/env bash
# Instalador mínimo — minero PoK y utilidades (sin binarios C++ aún).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
PREFIX="${PREFIX:-/usr/local}"

echo "== RadhikaChain Core — instalador (scripts) =="

if ! command -v python3 >/dev/null; then
  echo "ERROR: python3 requerido"
  exit 1
fi

if ! python3 -c "import blake3" 2>/dev/null; then
  echo "Instalando blake3..."
  python3 -m pip install --user -r "$ROOT/requirements-miner.txt"
fi

make -C "$ROOT" PREFIX="$PREFIX" install-miner

ENV_TARGET="${RADHIKA_DIR:-$HOME/.radhika}"
mkdir -p "$ENV_TARGET"
if [[ ! -f "$ENV_TARGET/.env" ]]; then
  cp "$ROOT/.env.example" "$ENV_TARGET/.env"
  echo "Creado $ENV_TARGET/.env — edita RPC_USER y RPC_PASS antes de minar."
fi

echo ""
echo "Listo. Uso:"
echo "  export \$(grep -v '^#' $ENV_TARGET/.env | xargs)  # o source manual"
echo "  karma-mine [dirección_coinbase_opcional]"