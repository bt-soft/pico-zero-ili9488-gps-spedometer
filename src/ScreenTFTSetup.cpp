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

    DEBUG("ScreenTFTSetup: layoutComponents called\n");

    // Függőlegesen egymás alá
    int btnW = 180;
    int btnH = 60;
    int btnX = (::SCREEN_W - btnW) / 2;
    int btnY = 60;
    int btnGap = 20;

    // Auto Brightness gomb
    int row = 0;
    addChild(std::make_shared<UIButton>(                                                              //
        10,                                                                                           //
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH),                                         //
        "Auto Brightness",                                                                            //
        UIButton::ButtonType::Toggleable,                                                             //
        config.data.tftAutoBrightnessActive ? UIButton::ButtonState::On : UIButton::ButtonState::Off, //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                config.data.tftAutoBrightnessActive = event.state == UIButton::EventButtonState::On;
                tftBackLightAdjuster.setAutoBrightnessActive(config.data.tftAutoBrightnessActive);

                DEBUG("Auto Brightness changed to: %s\n", config.data.tftAutoBrightnessActive ? "ON" : "OFF");

                if (manualBrightnessBtn) {
                    DEBUG("Setting manualBrightnessBtn enabled to: %s\n", config.data.tftAutoBrightnessActive ? "false" : "true");
                    manualBrightnessBtn->setEnabled(!config.data.tftAutoBrightnessActive);
                    manualBrightnessBtn->markForRedraw();
                }

                // Frissítsük a képernyőt, hogy a Manual gomb állapota is frissüljön
                markForRedraw(true);
            }
        }) //
    );

    // Manual Brightness gomb
    row++;
    manualBrightnessBtn = std::make_shared<UIButton>(         //
        11,                                                   //
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH), //
        "Manual Brightness",                                  //
        UIButton::ButtonType::Pushable,                       //
        UIButton::ButtonState::Off,                           //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                DEBUG("Manual Brightness button pressed - not implemented yet\n");
            }
        } //
    );
    manualBrightnessBtn->setEnabled(!config.data.tftAutoBrightnessActive);
    addChild(manualBrightnessBtn);

    // Vissza gomb
    addChild(std::make_shared<UIButton>( //                                                                                                                                  //
        1,                               //
        Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), //
        "Back", UIButton::ButtonType::Pushable, [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                config.checkSave();
                getScreenManager()->goBack();
            }
        }));
}

/**
 * Kirajzolja a képernyő saját tartalmát
 */
void ScreenTFTSetup::drawContent() {

    DEBUG("ScreenTFTSetup: drawContent called\n");

    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("TFT Settings", ::SCREEN_W / 2, 20);
    tft.setFreeFont();
}
