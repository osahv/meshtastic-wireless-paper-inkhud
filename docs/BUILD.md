# Build from source

The patch is applied on top of the **official Meshtastic firmware at an exact tag**, so the result is reproducible.

| Pinned base | Value |
|---|---|
| Upstream repo | <https://github.com/meshtastic/firmware> |
| Tag | `v2.7.26.54e0d8d` |
| PlatformIO environment | `heltec-wireless-paper-inkhud` |

## Prerequisites

- `git`, Python 3, [PlatformIO](https://platformio.org/install/cli) (`pip install platformio`)
- ~10 GB free disk space and a stable connection; the first build downloads toolchains and takes 5–20 minutes.

## One command

```bash
scripts/build.sh                 # builds into ./build-output/
scripts/build.sh /path/to/work   # use another work directory
```

The script: clones upstream at the pinned tag (with submodules), applies
[`patches/0001-…patch`](../patches), runs `pio run -e heltec-wireless-paper-inkhud` and copies the resulting
`.bin` and `.factory.bin` files next to a `SHA256SUMS`.

## Manual steps

```bash
git clone --recurse-submodules --branch v2.7.26.54e0d8d https://github.com/meshtastic/firmware.git
cd firmware
git apply ../meshtastic-wireless-paper-inkhud/patches/0001-*.patch
pio run -e heltec-wireless-paper-inkhud
ls .pio/build/heltec-wireless-paper-inkhud/*.bin
```

Outputs:

- `firmware-heltec-wireless-paper-inkhud-<version>.bin` – application (flash at `0x10000`)
- `firmware-heltec-wireless-paper-inkhud-<version>.factory.bin` – full image (flash at `0x0`)

## Without `git apply` (overlay)

[`overlay/`](../overlay) holds the changed/new files at their final paths. Copy it over a checkout of the same tag:

```bash
cp -R overlay/* /path/to/firmware/
```

## Rebasing on another Meshtastic version

1. Check out the new tag of the firmware repo.
2. `git apply --3way patches/0001-*.patch`
3. Resolve conflicts (almost always only in `nicheGraphics.h`, where InkHUD registers applets).
4. If InkHUD's applet API changed, adjust `ClockApplet` / `NodeStatusApplet` (they use `onRender`, `requestUpdate`,
   `printAt`, `fillRect`, `fillTriangle`, `isForeground`).
5. Build, flash, test – and please open a PR with the rebased patch.

## Continuous integration

[`.github/workflows/build.yml`](../.github/workflows/build.yml) builds the firmware from the patch on every pull request and
on demand, uploading the binaries as an artifact. Pushing a `v*` tag attaches them to a GitHub release.
