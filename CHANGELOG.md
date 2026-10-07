# Changelog

All notable changes are documented here. Versions follow [Semantic Versioning](https://semver.org/);
the Meshtastic base version is part of every release name.

## [1.0.0] – 2026-10-08

Base: Meshtastic firmware `v2.7.26.54e0d8d` · Hardware: Heltec Wireless Paper

### Added
- **Clock applet** – big seven-segment clock (classic Meshtastic look), auto-scaled, 12/24 h, weekday + date, minute-aligned refresh only while visible, 18 px top margin for the battery icon.
- **Node status applet** – battery %, voltage and charging/USB state, nodes online/known, channel utilisation and TX airtime, uptime, free heap; refreshed when opened and every 5 minutes.
- **Cyrillic rendering** – InkHUD fonts switched to Windows‑1251.
- Patch, overlay files, build script, flash helper, CI workflow and documentation.
