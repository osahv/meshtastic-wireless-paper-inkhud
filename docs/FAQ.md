# FAQ & troubleshooting

**The clock shows `--:--` / "Time not set".**
The node has no valid time yet. Connect the Meshtastic phone app once (it syncs the time), or wait for a GPS fix if your
device has GPS. The Wireless Paper has no GPS of its own.

**The time is off by one or two hours.**
The applet shows the node's *local* time using the timezone configured on the device (`device.tzdef`). Set a correct POSIX
timezone string in the Meshtastic app/CLI, e.g. `EET-2EEST,M3.5.0/3,M10.5.0/4`, and reboot the node (the timezone is applied at boot).

**I don't see the Clock / Node Status in the applet list.**
InkHUD saves the applet list on the device, so an already configured node does not pick up new applets automatically.
Enable them once in the InkHUD menu (long‑press the user button). A fresh/erased node will have them activated by default.

**Accented letters like é, ü, ñ look wrong.**
Windows‑1251 (Cyrillic) fonts do not contain Western‑European accents. That is the trade‑off of this patch. Use the stock
firmware (Windows‑1252) if you mostly need those.

**The screen flashes from time to time.**
That is the E‑Ink panel's full refresh to clear ghosting, scheduled by upstream InkHUD (`setDisplayResilience`). With a
once‑a‑minute clock it will occur roughly every quarter of an hour of clock display.

**Does the clock drain the battery?**
Only while it is on screen. When another applet is shown, the clock does not request redraws (only a lightweight timer ticks).
The cost of a minute update is small next to the always‑on radio, but exact numbers depend on your settings and battery –
measure on your own unit.

**Will my channels/keys survive?**
Yes with the documented partial flash (`app` partition only). Always back up first: [INSTALL.md](INSTALL.md#0-back-up-your-configuration).

**It does not build with a newer Meshtastic version.**
See *Rebasing* in [BUILD.md](BUILD.md). PRs with updated patches are welcome.

**How do I go back to stock?**
Flash the official firmware for `heltec-wireless-paper-inkhud` (see [INSTALL.md](INSTALL.md#go-back-to-stock)).

**Is anything sent anywhere / does the patch collect data?**
No. The applets only read local state and draw on the screen.
