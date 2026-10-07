#!/usr/bin/env bash
# Flash only the application partition (keeps channels, keys and settings).
# Usage: scripts/flash.sh <firmware.app.bin> [serial-port]
set -euo pipefail

FILE="${1:-}"; PORT="${2:-}"
[ -n "$FILE" ] && [ -f "$FILE" ] || { echo "usage: $0 <firmware.app.bin> [port]"; exit 1; }
case "$FILE" in *factory*) echo "This is a *factory* image (flash at 0x0). Use an *.app.bin file with this script."; exit 1;; esac

ESPTOOL="$(command -v esptool.py || command -v esptool || true)"
[ -n "$ESPTOOL" ] || { echo "esptool not found: pip install esptool"; exit 1; }

if [ -z "$PORT" ]; then
  for c in /dev/cu.SLAB_USBtoUART /dev/cu.usbserial-* /dev/ttyUSB0 /dev/ttyACM0; do [ -e "$c" ] && PORT="$c" && break; done
fi
[ -n "$PORT" ] || { echo "no serial port found - pass it as the 2nd argument"; exit 1; }

# optional checksum verification if SHA256SUMS sits next to the file
DIR="$(cd "$(dirname "$FILE")" && pwd)"; BASE="$(basename "$FILE")"
if [ -f "$DIR/SHA256SUMS" ]; then
  ( cd "$DIR" && grep " $BASE\$" SHA256SUMS | shasum -a 256 -c - ) || { echo "checksum mismatch - aborting"; exit 1; }
fi

echo "Port: $PORT"; echo "File: $FILE"
echo "Make sure you have a backup:  meshtastic --export-config > backup.yaml"
read -r -p "Flash now? [y/N] " ans; [ "$ans" = "y" ] || exit 0

"$ESPTOOL" --chip esp32s3 --port "$PORT" --baud 460800 erase_region 0xE000 0x2000
"$ESPTOOL" --chip esp32s3 --port "$PORT" --baud 460800 --after hard_reset write_flash 0x10000 "$FILE"
echo "Done. The screen refreshes in ~20 s. Enable Clock and Node Status in the InkHUD menu."
