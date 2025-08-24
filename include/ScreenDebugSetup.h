#pragma once
#include "UIScreen.h"
class ScreenDebugSetup : public UIScreen {
  public:
    ScreenDebugSetup() : UIScreen(SCREEN_NAME_DEBUG_SETUP) { layoutComponents(); }
    void layoutComponents() override;
    void drawContent() override;
};
