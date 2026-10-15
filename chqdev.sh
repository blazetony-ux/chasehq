#!/usr/bin/env sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ARCH=$(uname -m)
case "$ARCH" in
  x86_64|amd64) BIN="$ROOT/tools/chqdev/bin/chqdev-linux-amd64" ;;
  aarch64|arm64) BIN="$ROOT/tools/chqdev/bin/chqdev-linux-arm64" ;;
  *) echo "ERROR: unsupported bundled chqdev Linux architecture: $ARCH" >&2; exit 1 ;;
esac
exec "$BIN" --root "$ROOT" "$@"
