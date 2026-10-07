# How it works & customisation

## InkHUD in two minutes

[InkHUD](https://github.com/meshtastic/firmware/tree/master/src/graphics/niche/InkHUD) is Meshtastic's UI for E‑Ink devices.
It is built from **applets** (small pages) shown in **tiles**. An applet draws itself with `onRender()` when asked to
(`requestUpdate()`), and the renderer pushes one image to the panel. A health policy (`DisplayHealth`) mixes fast
partial updates with occasional full refreshes to limit ghosting – upstream behaviour, unchanged here.

This project adds two applets and switches the fonts. It lives entirely in four new files and one registration file.

## Files

```
src/graphics/niche/InkHUD/Applets/User/Clock/ClockApplet.{h,cpp}
src/graphics/niche/InkHUD/Applets/User/NodeStatus/NodeStatusApplet.{h,cpp}
variants/esp32s3/heltec_wireless_paper/nicheGraphics.h         (fonts + registration)
```

## Clock applet

- **Geometry** – the same seven segments as Meshtastic's classic clock: a 16×4 base segment, beveled with two triangles,
  1 px gaps. The scale is chosen as the largest value (≤ 3.5, step 0.05) for which the time still fits the width and the
  available height. 12 h mode drops the leading zero.
- **Time** – `getValidTime(RTCQuality::RTCQualityDevice, true)` returns *local* epoch seconds (the device timezone is
  applied by the firmware at boot). If it is `0` the applet shows `--:--` and `Time not set`.
- **Refresh policy** – an `OSThread` fires at the next minute boundary (+200 ms). It requests an update **only if the
  applet is in the foreground and the minute on screen is out of date** (`lastDrawnMinute`). When the applet is shown
  it draws immediately and re‑aligns the timer.
- **Layout** – 18 px at the top stay empty (`topReserve`) so the system battery icon never overlaps the digits; the date
  line sits at the bottom (`Sun 04.10.2026`, plus `AM/PM` in 12 h mode).

## Node status applet

Reads only local state: `powerStatus` (battery %, voltage, charging/USB), `nodeDB->getNumOnlineMeshNodes()` /
`getNumMeshNodes()`, `airTime->channelUtilizationPercent()` / `utilizationTXPercent()`, `millis()` and `ESP.getFreeHeap()`.
It refreshes when opened and every 5 minutes (`REFRESH_MS`) while visible – the page is informational, not time‑critical.

## Fonts / Cyrillic

`nicheGraphics.h` sets

```cpp
InkHUD::Applet::fontLarge  = FREESANS_12PT_WIN1251;
InkHUD::Applet::fontMedium = FREESANS_9PT_WIN1251;
InkHUD::Applet::fontSmall  = FREESANS_6PT_WIN1251;
```

These Windows‑1251 fonts already exist in upstream InkHUD. Text passes through `Applet::parse()` which converts UTF‑8 to
the font's code page. Windows‑1251 has Latin + Cyrillic but not the accented Western‑European letters of Windows‑1252.

## Customising

| Want | Change |
|---|---|
| Other weekday names (e.g. Ukrainian) | `WEEKDAYS[]` in `ClockApplet.cpp` (use the Win1251 font; text goes through `parse()`) |
| Date format | the `snprintf` format string in `ClockApplet::onRender` |
| More/less top margin | `topReserve` in `ClockApplet::onRender` |
| Bigger/smaller digits | the scale search loop (max `3.5f`) and `W - 12` width margin |
| Status refresh rate | `REFRESH_MS` in `NodeStatusApplet.cpp` |
| Other status lines | add `snprintf` + `printAt` lines in `NodeStatusApplet::onRender` (five lines fit the 122 px height) |
| Show applets by default | the two flags of `addApplet("Clock", …, activated, autoshow)` in `nicheGraphics.h` |

## Writing your own applet

Copy `NodeStatusApplet` as a template: derive from `InkHUD::Applet`, implement `onRender()`, call `requestUpdate()` when your
data changes (only while `isForeground()`), register it with `addApplet(...)` before `inkhud->begin()`.
