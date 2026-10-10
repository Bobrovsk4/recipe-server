#!/bin/bash
set -e

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "== start compile =="
cmake -S "$ROOT" -B "$ROOT/build"
cmake --build "$ROOT/build"

echo "== start app =="
"$ROOT/build/backend"