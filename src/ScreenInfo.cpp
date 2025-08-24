#include "ScreenInfo.h"
#include "Utils.h"
#include "defines.h"

/**
 * UI komponensek elhelyezése
 */
void ScreenInfo::layoutComponents() {
    // Back gomb jobb alsó sarokban
    auto backButton = std::make_shared<UIButton>(
        1,                                                                                                                                                                // id
        Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), // bounds (jobb alsó sarok)
        "Back",                                                                                                                                                           // label
        UIButton::ButtonType::Pushable,                                                                                                                                   // type
        [this](const UIButton::ButtonEvent &event) { this->onBackButtonClicked(event); });

    addChild(backButton);
}

/**
 * Kirajzolja a képernyő saját tartalmát
 */
void ScreenInfo::drawContent() {
    // Háttér törlése
    tft.fillScreen(TFT_BLACK);

    // Címsor
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Info Screen", ::SCREEN_W / 2, 50);

    // Információs szöveg
    tft.setFreeFont();
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("GPS Speedometer v1.0", ::SCREEN_W / 2, 120);

    tft.setTextSize(1);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("Raspberry Pi Pico", ::SCREEN_W / 2, 160);
    tft.drawString("TFT ILI9488 Display", ::SCREEN_W / 2, 180);
    tft.drawString("DS18B20 Temperature Sensor", ::SCREEN_W / 2, 200);
    tft.drawString("GPS Module", ::SCREEN_W / 2, 220);
}

/**
 * Back gomb callback
 */
void ScreenInfo::onBackButtonClicked(const UIButton::ButtonEvent &event) {
    if (event.state == UIButton::EventButtonState::Clicked) {
        DEBUG("ScreenInfo: Back button clicked, returning to main screen\n");
        getScreenManager()->switchToScreen(SCREEN_NAME_MAIN);
    }
}
