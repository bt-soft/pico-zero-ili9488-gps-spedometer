#include "ScreenSystemSetup.h"
#include "Config.h"
#include "Utils.h"
#include "defines.h"

void ScreenSystemSetup::layoutComponents() {
    // Vissza gomb
    auto backButton = std::make_shared<UIButton>( //
        1, Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), "Back", UIButton::ButtonType::Pushable,
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                config.checkSave();
                getScreenManager()->goBack();
            }
        });
    addChild(backButton);
}

void ScreenSystemSetup::drawContent() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("System Settings", ::SCREEN_W / 2, 50);
    tft.setFreeFont();
    // ...ide jönnek majd a beállítási gombok/ValueChangeDialog...
}
