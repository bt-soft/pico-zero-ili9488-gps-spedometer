#pragma once

#include "ButtonsGroupManager.h"
#include "UIScreen.h"

/**
 * Debug képernyő beállítása
 */
class ScreenDebugSetup : public UIScreen, public ButtonsGroupManager<ScreenDebugSetup> {
  public:
    ScreenDebugSetup() : UIScreen(SCREEN_NAME_DEBUG_SETUP) { layoutComponents(); }
    void layoutComponents() override;
    void drawContent() override;
};
