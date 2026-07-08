#!/usr/bin/env bash
# Smoke test: binarios compilados responden en regtest.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
BITCOIND="${BITCOIND:-$BUILD_DIR/bin/bitcoind}"
BITCOIN_CLI="${BITCOIN_CLI:-$BUILD_DIR/bin/bitcoin-cli}"

for bin in "$BITCOIND" "$BITCOIN_CLI"; do
  [[ -x "$bin" ]] || { echo "FAIL: no encontrado $bin"; exit 1; }
done

TEST_DIR="$(mktemp -d)"
trap 'rm -rf "$TEST_DIR"' EXIT

echo "== smoke-node =="
"$BITCOIND" -version
"$BITCOIN_CLI" -version

"$BITCOIND" -regtest -datadir="$TEST_DIR" -daemon
sleep 4

info="$("$BITCOIN_CLI" -regtest -datadir="$TEST_DIR" getblockchaininfo)"
echo "$info" | grep -q '"chain": "regtest"' || { echo "FAIL: regtest no activo"; exit 1; }
echo "OK: getblockchaininfo regtest"

"$BITCOIN_CLI" -regtest -datadir="$TEST_DIR" stop
sleep 2
echo "PASS: smoke-node"