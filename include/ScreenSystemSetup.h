#pragma once
#include "UIScreen.h"
class ScreenSystemSetup : public UIScreen {
  public:
    ScreenSystemSetup() : UIScreen(SCREEN_NAME_SYSTEM_SETUP) { layoutComponents(); }
    void layoutComponents() override;
    void drawContent() override;
};
