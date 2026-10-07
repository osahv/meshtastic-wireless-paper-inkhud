#ifdef MESHTASTIC_INCLUDE_INKHUD

/*

Own-node status page: battery, mesh size, airtime, uptime, free heap.
Redraws when brought to the screen, then every few minutes while visible.

*/

#pragma once

#include "configuration.h"

#include "graphics/niche/InkHUD/Applet.h"

namespace NicheGraphics::InkHUD
{

class NodeStatusApplet : public Applet, public concurrency::OSThread
{
  public:
    NodeStatusApplet();
    void onActivate() override;
    void onDeactivate() override;
    void onForeground() override;
    void onRender(bool full) override;

  protected:
    int32_t runOnce() override;
};

} // namespace NicheGraphics::InkHUD

#endif
