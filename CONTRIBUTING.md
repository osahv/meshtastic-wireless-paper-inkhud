# Contributing

Thanks for helping! This project is deliberately small, so contributions are easy to review.

## Good first contributions
- Rebase the patch on a newer Meshtastic tag and confirm it builds and runs.
- Test on another InkHUD board and report what is needed (each board has its own `nicheGraphics.h`).
- Translations of weekday names/labels (remember the Windows‑1251 code page).
- Photos of the screens for the README (real device, please mention panel revision).

## Ground rules
- Keep the patch **small and readable**; do not reformat unrelated upstream code.
- Never commit secrets: channel keys, config backups, tokens, coordinates, MAC addresses of your devices.
- Describe your hardware (board, panel variant), the Meshtastic version, and how you tested.
- The project is GPL‑3.0; by contributing you agree your changes are released under the same license.

## Development loop
1. `scripts/build.sh` (see [docs/BUILD.md](docs/BUILD.md)).
2. Edit files in the work checkout; regenerate the patch with `git diff > patches/0001-….patch` (and refresh `overlay/`).
3. Open a pull request – CI builds the firmware automatically.
