#pragma once
#include "UIScreen.h"
class ScreenGPSSetup : public UIScreen {
  public:
    ScreenGPSSetup() : UIScreen(SCREEN_NAME_GPS_SETUP) { layoutComponents(); }
    void layoutComponents() override;
    void drawContent() override;
};
