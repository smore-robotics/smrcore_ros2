#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd -P)"
rm -rf "$ROOT_DIR/build" "$ROOT_DIR/install" "$ROOT_DIR/log"
