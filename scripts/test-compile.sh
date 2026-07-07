#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

echo "== Tests de scripts (fase scripts-only) =="

bash scripts/check-secrets.sh
python3 -m py_compile scripts/karma-mine.py
bash scripts/validate-install.sh

echo "PASS: scripts verificados"