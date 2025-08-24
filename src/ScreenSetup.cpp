#include "ScreenSetup.h"
#include "Utils.h"
#include "defines.h"

/**
 * UI komponensek elhelyezése
 */
void ScreenSetup::layoutComponents() {
    // Back gomb bal alsó sarokban
    auto backButton = std::make_shared<UIButton>(1,                                  // id
                                                 Rect(10, ::SCREEN_H - 50, 100, 40), // bounds
                                                 "Back",                             // label
                                                 UIButton::ButtonType::Pushable,     // type
                                                 [this](const UIButton::ButtonEvent &event) { this->onBackButtonClicked(event); });

    addChild(backButton);
}

/**
 * Kirajzolja a képernyő saját tartalmát
 */
void ScreenSetup::drawContent() {
    // Háttér törlése
    tft.fillScreen(TFT_BLACK);

    // Címsor
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Setup Screen", ::SCREEN_W / 2, 50);

    // Beállítások
    tft.setFreeFont();
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("Configuration Options", ::SCREEN_W / 2, 120);

    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("Temperature Unit", ::SCREEN_W / 2, 160);
    tft.drawString("Display Brightness", ::SCREEN_W / 2, 180);
    tft.drawString("GPS Update Rate", ::SCREEN_W / 2, 200);
    tft.drawString("Speed Units", ::SCREEN_W / 2, 220);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Under Development", ::SCREEN_W / 2, 260);
}

/**
 * Back gomb callback
 */
void ScreenSetup::onBackButtonClicked(const UIButton::ButtonEvent &event) {
    if (event.state == UIButton::EventButtonState::Clicked) {
        DEBUG("ScreenSetup: Back button clicked, returning to main screen\n");
        getScreenManager()->switchToScreen(SCREEN_NAME_MAIN);
    }
}
