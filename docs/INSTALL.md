# Install, update and recover

This guide flashes the prebuilt firmware from [Releases](../../releases) onto a **Heltec Wireless Paper**.
Total time: about 10 minutes.

> **Tested path:** flashing only the *app partition* with `esptool` (below). Your channels, keys and settings live in a
> separate partition and stay untouched.

## What you need

- Heltec Wireless Paper (ESP32‑S3, 2.13″ E‑Ink) running Meshtastic **2.7.x** with InkHUD.
- A USB data cable (charge‑only cables will not work).
- Python 3 with `esptool` (`pip install esptool`) and, for the backup, the Meshtastic CLI (`pip install meshtastic`).

## 0. Back up your configuration

```bash
meshtastic --port <PORT> --export-config > wireless-paper-backup.yaml
meshtastic --port <PORT> --info > wireless-paper-info.txt
```

⚠️ The backup contains your channel keys. **Keep it private** and never commit it to a repository.

Find `<PORT>`:

| OS | Typical port |
|---|---|
| macOS | `/dev/cu.usbserial-*` or `/dev/cu.SLAB_USBtoUART` |
| Linux | `/dev/ttyUSB0` |
| Windows | `COM3` (see Device Manager) |

Close other programs that use the port (browser flasher tabs, Meshtastic Web, serial monitors) – only one can use it at a time.

## 1. Download and verify

From the latest release download one pair:

- `…_fw2.7.26.54e0d8d.app.bin` – application only (**recommended for updating an existing node**)
- `…_fw2.7.26.54e0d8d.factory.bin` – full image (bootloader + partitions + app) for a fresh/recovered device

and `SHA256SUMS`.

```bash
shasum -a 256 -c SHA256SUMS --ignore-missing      # macOS / Linux
```

## 2. Flash the app partition (keeps your settings)

```bash
PORT=/dev/cu.SLAB_USBtoUART            # adjust
FILE=meshtastic-heltec-wireless-paper-inkhud-clock-status_v1.0.0_fw2.7.26.54e0d8d.app.bin

esptool.py --chip esp32s3 --port $PORT --baud 460800 erase_region 0xE000 0x2000
esptool.py --chip esp32s3 --port $PORT --baud 460800 --after hard_reset write_flash 0x10000 "$FILE"
```

The first command clears the OTA‑selection sector so the device boots the freshly written application.
The helper script does both steps and checks the checksum for you:

```bash
scripts/flash.sh "$FILE" $PORT
```

Wait ~20 s after the reset: the E‑Ink screen refreshes and the node boots.

## 3. Enable the new screens

1. Open the InkHUD menu (long‑press the user button).
2. In the applets list switch **Clock** and **Node Status** on.
3. Assign them to a tile or switch to them with a short press, as for any other applet.

Menu labels can differ slightly between Meshtastic versions.

The new applets are registered as *activated, not auto‑shown*, so they never steal the screen from incoming messages.
Because InkHUD stores the applet list on the device, an already configured node needs this one‑time manual step.

## 4. Check

- The Meshtastic app / `meshtastic --info` reports firmware `2.7.26.54e0d8d`.
- Channels and node name are unchanged.
- Clock shows the time after the phone has synced it (until then: `--:--`).

## Alternative: browser web flasher

The Meshtastic web flasher can write a custom `.factory.bin`. The author has **not** tested this route with these files.
If you use it, do **not** tick *Full erase* unless you want a factory reset (it wipes channels, keys and settings).

## Update to a newer release

Repeat step 2 with the newer `.app.bin`. If the new release is based on a different Meshtastic version, read its release
notes first.

## Go back to stock

Flash the official firmware for your board from <https://github.com/meshtastic/firmware/releases>
(the `heltec-wireless-paper-inkhud` variant), using the same `esptool` steps with the official `.bin`, or the official web flasher.
Your settings survive the same way.

## Recovery if something goes wrong

- `esptool` cannot connect: unplug, hold the **PRG/BOOT** button while plugging USB back in (download mode), then retry.
  Button labels can vary by board revision.
- Device does not boot after flashing: flash the full `…factory.bin` at address `0x0`
  (`esptool.py … write_flash 0x0 <file>.factory.bin`), then restore your configuration with `meshtastic --configure wireless-paper-backup.yaml`.
- Still stuck: open an [issue](../../issues) with your board revision, panel variant and the full `esptool` output.
