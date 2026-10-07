#ifdef MESHTASTIC_INCLUDE_INKHUD

#include "./ClockApplet.h"

#include "RTC.h"
#include "main.h"

#include <ctime>

using namespace NicheGraphics;

// Same look as the classic (non-InkHUD) Meshtastic digital clock: beveled segments, 1px gaps
#define SEG_W 16
#define SEG_H 4

//   ___1___
// 6 |     | 2
//   |_7___|
// 5 |     | 3
//   |___4_|
static const uint8_t SEGMENTS[10][7] = {
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 1, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}  // 9
};

InkHUD::ClockApplet::ClockApplet() : concurrency::OSThread("ClockApplet")
{
    // No scheduled tasks until activated
    OSThread::disable();
}

// How long until the next full minute of the (local) clock. Falls back to a short retry if time is not valid yet.
uint32_t InkHUD::ClockApplet::msToNextMinute()
{
    uint32_t localEpoch = getValidTime(RTCQuality::RTCQualityDevice, true);
    if (localEpoch == 0)
        return 30 * 1000UL; // No valid time yet, check again soon
    return (60 - (localEpoch % 60)) * 1000UL + 200UL; // Slightly after the rollover
}

void InkHUD::ClockApplet::onActivate()
{
    OSThread::enabled = true;
    OSThread::setIntervalFromNow(msToNextMinute());
}

void InkHUD::ClockApplet::onDeactivate()
{
    OSThread::disable();
}

// Shown on screen: draw right away, and re-align the periodic update to the minute boundary
void InkHUD::ClockApplet::onForeground()
{
    OSThread::enabled = true;
    OSThread::setIntervalFromNow(msToNextMinute());
    requestUpdate();
}

int32_t InkHUD::ClockApplet::runOnce()
{
    // Only worth refreshing the e-ink if the clock is actually visible,
    // and the minute on screen is really out of date (an early wake-up costs nothing)
    if (isForeground()) {
        uint32_t localEpoch = getValidTime(RTCQuality::RTCQualityDevice, true);
        if (localEpoch / 60 != lastDrawnMinute)
            requestUpdate();
    }
    return msToNextMinute();
}

void InkHUD::ClockApplet::drawHSegment(int16_t x, int16_t y, int16_t w, int16_t h)
{
    int16_t half = h / 2;
    fillRect(x, y, w, h, BLACK);
    fillTriangle(x, y, x, y + h - 1, x - half, y + half, BLACK);
    fillTriangle(x + w, y, x + w + half, y + half, x + w, y + h - 1, BLACK);
}

void InkHUD::ClockApplet::drawVSegment(int16_t x, int16_t y, int16_t w, int16_t h)
{
    int16_t half = h / 2;
    fillRect(x, y, h, w, BLACK);
    fillTriangle(x + half, y - half, x + h - 1, y, x, y, BLACK);
    fillTriangle(x, y + w, x + h - 1, y + w, x + half, y + w + half, BLACK);
}

void InkHUD::ClockApplet::drawDigit(int16_t x, int16_t y, uint8_t digit, float scale)
{
    const uint8_t *seg = SEGMENTS[digit];
    const int16_t sw = SEG_W * scale;
    const int16_t sh = SEG_H * scale;

    const int16_t x1 = x + sh + 2, y1 = y;
    const int16_t x2 = x1 + sw + 2, y2 = y1 + sh + 2;
    const int16_t y3 = y2 + sw + 2 + sh + 2;
    const int16_t y4 = y3 + sw + 2;
    const int16_t y7 = y2 + sw + 2;

    if (seg[0]) drawHSegment(x1, y1, sw, sh);
    if (seg[1]) drawVSegment(x2, y2, sw, sh);
    if (seg[2]) drawVSegment(x2, y3, sw, sh);
    if (seg[3]) drawHSegment(x1, y4, sw, sh);
    if (seg[4]) drawVSegment(x, y3, sw, sh);
    if (seg[5]) drawVSegment(x, y2, sw, sh);
    if (seg[6]) drawHSegment(x1, y7, sw, sh);
}

void InkHUD::ClockApplet::drawColon(int16_t x, int16_t y, float scale)
{
    const int16_t sw = SEG_W * scale;
    const int16_t sh = SEG_H * scale;
    const int16_t cellH = sw * 2 + sh * 3 + 8;
    const int16_t dotX = x + (int16_t)(4 * scale);
    fillRect(dotX, y + cellH / 4, sh, sh, BLACK);
    fillRect(dotX, y + (cellH / 4) * 3, sh, sh, BLACK);
}

void InkHUD::ClockApplet::onRender(bool full)
{
    const int16_t W = width();
    const int16_t H = height();

    // Text line under the clock
    setFont(fontMedium);
    setTextColor(BLACK);
    const int16_t dateH = fontMedium.lineHeight();

    uint32_t localEpoch = getValidTime(RTCQuality::RTCQualityDevice, true);

    // Area available for the digits
    const int16_t topReserve = 18; // Keep clear of the battery icon (top right)
    const int16_t areaH = H - dateH - 8 - topReserve;

    if (localEpoch == 0) {
        // No usable time yet (never synced from phone / GPS / NTP)
        printAt(X(0.5), Y(0.40), "--:--", CENTER, MIDDLE);
        setFont(fontSmall);
        printAt(X(0.5), Y(0.75), parse("Time not set"), CENTER, MIDDLE);
        return;
    }

    lastDrawnMinute = localEpoch / 60;

    // localEpoch already includes the timezone offset, so read it back as plain "UTC" fields
    time_t tt = (time_t)localEpoch;
    struct tm tmv;
    gmtime_r(&tt, &tmv);

    int hour = tmv.tm_hour;
    const bool use12h = config.display.use_12h_clock;
    const bool pm = hour > 11;
    if (use12h)
        hour = (hour % 12 == 0) ? 12 : (hour % 12);

    char timeStr[8];
    if (use12h)
        snprintf(timeStr, sizeof(timeStr), "%d:%02d", hour, tmv.tm_min);
    else
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, tmv.tm_min);
    const size_t len = strlen(timeStr);

    // Total width and height for a given scale, same arithmetic as the classic clock
    auto timeWidth = [&](float s) -> int {
        const int sw = SEG_W * s, sh = SEG_H * s;
        int total = 0;
        for (size_t i = 0; i < len; i++) {
            if (timeStr[i] == ':')
                total += sh + 6 + (s >= 2.0f ? (int)(4.5f * s) : 0) + 5;
            else
                total += sw + sh * 2 + 4 + 5;
        }
        return total - 5 + sh / 2; // no gap after the last digit; bevel of the end segments sticks out
    };
    auto cellHeight = [&](float s) -> int { return (int)(SEG_W * s) * 2 + (int)(SEG_H * s) * 3 + 8; };

    float scale = 0.5f;
    while (scale < 3.5f && timeWidth(scale + 0.05f) <= W - 12 && cellHeight(scale + 0.05f) <= areaH)
        scale += 0.05f;

    const int sw = SEG_W * scale, sh = SEG_H * scale;
    int16_t x = (W - timeWidth(scale)) / 2 + sh / 2; // left bevel of segment 1 sticks out to the left
    const int16_t y = topReserve + (areaH - cellHeight(scale)) / 2 + 2;

    for (size_t i = 0; i < len; i++) {
        if (timeStr[i] == ':') {
            drawColon(x, y, scale);
            x += sh + 6;
            if (scale >= 2.0f)
                x += (int16_t)(4.5f * scale);
        } else {
            drawDigit(x, y, timeStr[i] - '0', scale);
            x += sw + sh * 2 + 4;
        }
        x += 5;
    }

    // Weekday + date
    static const char *WEEKDAYS[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    char dateStr[40];
    snprintf(dateStr, sizeof(dateStr), "%s %02d.%02d.%04d%s", WEEKDAYS[tmv.tm_wday % 7], tmv.tm_mday, tmv.tm_mon + 1,
             tmv.tm_year + 1900, use12h ? (pm ? " PM" : " AM") : "");
    setFont(fontMedium);
    printAt(X(0.5), H - 2, parse(dateStr), CENTER, BOTTOM);
}

#endif
