#include "ScreenTFTSetup.h"
#include "Config.h"
#include "Utils.h"
#include "defines.h"

#include "TftBackLightAdjuster.h"
extern TftBackLightAdjuster tftBackLightAdjuster;

/**
 * UI komponensek elhelyezése
 */
void ScreenTFTSetup::layoutComponents() {

    // Függőlegesen egymás alá
    int btnW = 180;
    int btnH = 60;
    int btnX = (::SCREEN_W - btnW) / 2;
    int btnY = 60;
    int btnGap = 20;

    // Auto Brightness gomb
    int row = 0;
    addChild(std::make_shared<UIButton>(                                                              //
        10,                                                                                           // id
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH),                                         // rect
        "Auto Brightness",                                                                            // label
        UIButton::ButtonType::Toggleable,                                                             // type
        config.data.tftAutoBrightnessActive ? UIButton::ButtonState::On : UIButton::ButtonState::Off, // Kezdeti állapot
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                config.data.tftAutoBrightnessActive = event.state == UIButton::EventButtonState::On;
                tftBackLightAdjuster.setAutoBrightnessActive(config.data.tftAutoBrightnessActive);
            }
        }) //
    );

    // Vissza gomb
    addChild(std::make_shared<UIButton>(                                                                                                                                  //
        1,                                                                                                                                                                //
        Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), //
        "Back", UIButton::ButtonType::Pushable,                                                                                                                           //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                config.checkSave();
                getScreenManager()->goBack();
            }
        }) //
    );
}

/**
 * Kirajzolja a képernyő saját tartalmát
 */
void ScreenTFTSetup::drawContent() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("TFT Settings", ::SCREEN_W / 2, 20);
    tft.setFreeFont();
}
