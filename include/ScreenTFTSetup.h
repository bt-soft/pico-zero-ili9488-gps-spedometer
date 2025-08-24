#pragma once
#include "UIScreen.h"
class ScreenTFTSetup : public UIScreen {
  public:
    ScreenTFTSetup() : UIScreen(SCREEN_NAME_TFT_SETUP) { layoutComponents(); }
    void layoutComponents() override;
    void drawContent() override;
};
