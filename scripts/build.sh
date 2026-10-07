#!/usr/bin/env bash
# Build the patched firmware from the official Meshtastic tag.
# Usage: scripts/build.sh [work-dir]      (default: ./build-work)
set -euo pipefail

FW_TAG="v2.7.26.54e0d8d"
ENV_NAME="heltec-wireless-paper-inkhud"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
WORK="${1:-$HERE/build-work}"
OUT="$HERE/build-output"

for cmd in git python3; do command -v "$cmd" >/dev/null || { echo "missing: $cmd"; exit 1; }; done
PIO="$(command -v pio || command -v platformio || true)"
[ -n "$PIO" ] || { echo "PlatformIO not found: pip install platformio"; exit 1; }

mkdir -p "$WORK" "$OUT"
if [ ! -d "$WORK/firmware/.git" ]; then
  echo ">> cloning Meshtastic firmware $FW_TAG (with submodules)"
  git clone --recurse-submodules --branch "$FW_TAG" https://github.com/meshtastic/firmware.git "$WORK/firmware"
fi
cd "$WORK/firmware"
git checkout -q "$FW_TAG"
git reset -q --hard "$FW_TAG" && git clean -qfd -e .pio

echo ">> applying patch"
git apply "$HERE"/patches/*.patch

echo ">> building $ENV_NAME (first run downloads toolchains; be patient)"
"$PIO" run -e "$ENV_NAME"

rm -f "$OUT"/*.bin "$OUT"/SHA256SUMS
cp .pio/build/"$ENV_NAME"/firmware-"$ENV_NAME"-*.bin "$OUT"/
( cd "$OUT" && shasum -a 256 *.bin | tee SHA256SUMS )
echo ">> done: $OUT"
