#include "ScreenSetup.h"
#include "Utils.h"
#include "defines.h"

/**
 * UI komponensek elhelyezése
 */
void ScreenSetup::layoutComponents() {

    // Függőlegesen egymás alá
    int btnW = 180;
    int btnH = 40;
    int btnX = (::SCREEN_W - btnW) / 2;
    int btnY = 80;
    int btnGap = 10;

    // TFT beállítások gomb
    addChild(std::make_shared<UIButton>( //
        10, Rect(btnX, btnY, btnW, btnH), "TFT Settings", UIButton::ButtonType::Pushable, [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_TFT_SETUP);
            }
        }));

    // System beállítások button
    addChild(std::make_shared<UIButton>( //
        11, Rect(btnX, btnY + (btnH + btnGap), btnW, btnH), "System Settings", UIButton::ButtonType::Pushable, [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_SYSTEM_SETUP);
            }
        }));

    // GPS beállítások gomb
    addChild(std::make_shared<UIButton>( //
        12, Rect(btnX, btnY + 2 * (btnH + btnGap), btnW, btnH), "GPS Settings", UIButton::ButtonType::Pushable, [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_GPS_SETUP);
            }
        }));

    // Debug beállítások gomb
    auto dbgButton = std::make_shared<UIButton>( //
        13, Rect(btnX, btnY + 3 * (btnH + btnGap), btnW, btnH), "Debug Settings", UIButton::ButtonType::Pushable, [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_DEBUG_SETUP);
            }
        });
#ifndef __DEBUG
    // Ha nincs debug mód, akkor a Debug gomb le van tiltva
    dbgButton->setButtonState(UIButton::ButtonState::Disabled);
#endif
    addChild(dbgButton);

    // Back gomb jobb alsó sarokban
    addChild(std::make_shared<UIButton>( //
        1, Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), "Back", UIButton::ButtonType::Pushable,
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_MAIN);
            }
        }));
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
    tft.drawString("Setup Screen", ::SCREEN_W / 2, 30);
}
