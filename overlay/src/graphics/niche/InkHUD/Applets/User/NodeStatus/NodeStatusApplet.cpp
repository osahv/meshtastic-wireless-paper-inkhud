#ifdef MESHTASTIC_INCLUDE_INKHUD

#include "./NodeStatusApplet.h"

#include "NodeDB.h"
#include "PowerStatus.h"
#include "airtime.h"
#include "main.h"

using namespace NicheGraphics;

static const uint32_t REFRESH_MS = 5 * 60 * 1000UL; // Slow refresh while visible: this page is not time-critical

InkHUD::NodeStatusApplet::NodeStatusApplet() : concurrency::OSThread("NodeStatusApplet")
{
    OSThread::disable();
}

void InkHUD::NodeStatusApplet::onActivate()
{
    OSThread::enabled = true;
    OSThread::setIntervalFromNow(REFRESH_MS);
}

void InkHUD::NodeStatusApplet::onDeactivate()
{
    OSThread::disable();
}

void InkHUD::NodeStatusApplet::onForeground()
{
    OSThread::enabled = true;
    OSThread::setIntervalFromNow(REFRESH_MS);
    requestUpdate();
}

int32_t InkHUD::NodeStatusApplet::runOnce()
{
    if (isForeground())
        requestUpdate();
    return REFRESH_MS;
}

void InkHUD::NodeStatusApplet::onRender(bool full)
{
    setFont(fontMedium);
    setTextColor(BLACK);

    const int16_t lh = fontMedium.lineHeight() + 2;
    int16_t y = 1;
    char buf[48];

    // Battery
    if (powerStatus && powerStatus->getHasBattery()) {
        snprintf(buf, sizeof(buf), "Batt: %u%%  %.2fV%s", (unsigned)powerStatus->getBatteryChargePercent(),
                 powerStatus->getBatteryVoltageMv() / 1000.0f,
                 powerStatus->getIsCharging() ? " CHG" : (powerStatus->getHasUSB() ? " USB" : ""));
    } else {
        snprintf(buf, sizeof(buf), "Power: %s", (powerStatus && powerStatus->getHasUSB()) ? "USB" : "no data");
    }
    printAt(2, y, parse(buf), LEFT, TOP);
    y += lh;

    // Mesh size
    if (nodeDB) {
        snprintf(buf, sizeof(buf), "Nodes: %u online / %u", (unsigned)nodeDB->getNumOnlineMeshNodes(true),
                 (unsigned)nodeDB->getNumMeshNodes());
        printAt(2, y, parse(buf), LEFT, TOP);
        y += lh;
    }

    // Airtime
    if (airTime) {
        snprintf(buf, sizeof(buf), "Airtime: %.1f%%  TX %.1f%%", airTime->channelUtilizationPercent(),
                 airTime->utilizationTXPercent());
        printAt(2, y, parse(buf), LEFT, TOP);
        y += lh;
    }

    // Uptime
    uint32_t s = millis() / 1000;
    uint32_t days = s / 86400, hours = (s % 86400) / 3600, mins = (s % 3600) / 60;
    if (days)
        snprintf(buf, sizeof(buf), "Uptime: %lud %luh %lum", (unsigned long)days, (unsigned long)hours, (unsigned long)mins);
    else
        snprintf(buf, sizeof(buf), "Uptime: %luh %lum", (unsigned long)hours, (unsigned long)mins);
    printAt(2, y, parse(buf), LEFT, TOP);
    y += lh;

    // Free heap
    snprintf(buf, sizeof(buf), "Free heap: %u KB", (unsigned)(ESP.getFreeHeap() / 1024));
    printAt(2, y, parse(buf), LEFT, TOP);
}

#endif
