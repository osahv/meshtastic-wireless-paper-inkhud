#ifdef MESHTASTIC_INCLUDE_INKHUD

/*

Big digital clock (seven-segment style), with weekday and date underneath.
Uses the local time (timezone from device config, config.device.tzdef).
Redraws once per minute, only while the applet is on screen.

*/

#pragma once

#include "configuration.h"

#include "graphics/niche/InkHUD/Applet.h"

namespace NicheGraphics::InkHUD
{

class ClockApplet : public Applet, public concurrency::OSThread
{
  public:
    ClockApplet();
    void onActivate() override;
    void onDeactivate() override;
    void onForeground() override;
    void onRender(bool full) override;

  protected:
    int32_t runOnce() override;

    uint32_t lastDrawnMinute = UINT32_MAX; // Minute (local epoch / 60) currently on screen, to skip redundant redraws

    uint32_t msToNextMinute();                                                     // For aligning updates to the minute
    void drawHSegment(int16_t x, int16_t y, int16_t w, int16_t h); // Segment shapes as in the classic Meshtastic clock
    void drawVSegment(int16_t x, int16_t y, int16_t w, int16_t h);
    void drawDigit(int16_t x, int16_t y, uint8_t digit, float scale);
    void drawColon(int16_t x, int16_t y, float scale);
};

} // namespace NicheGraphics::InkHUD

#endif
