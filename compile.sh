#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"
make build
echo "Binarios en $ROOT/build/bin/"
ls -la "$ROOT/build/bin/" 2>/dev/null || true