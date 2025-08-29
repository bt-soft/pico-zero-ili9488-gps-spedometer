#include "ScreenSystemSetup.h"
#include "Config.h"
#include "Utils.h"
#include "defines.h"

void ScreenSystemSetup::layoutComponents() {

    // Függőlegesen egymás alá
    int btnW = 180;
    int btnH = 60;
    int btnX = (::SCREEN_W - btnW) / 2;
    int btnY = 60;
    int btnGap = 20;

    // Auto Brightness gomb
    int row = 0;
    addChild(std::make_shared<UIButton>(                                                    //
        10,                                                                                 //
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH),                               //
        "Beper",                                                                            //
        UIButton::ButtonType::Toggleable,                                                   //
        config.data.beeperEnabled ? UIButton::ButtonState::On : UIButton::ButtonState::Off, //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                config.data.beeperEnabled = event.state == UIButton::EventButtonState::On;
                DEBUG("Beeper changed to: %s\n", config.data.beeperEnabled ? "ON" : "OFF");
            }
        }) //
    );

    // Vissza gomb
    addChild(std::make_shared<UIButton>( //                                                                                                                                  //
        1,                               //
        Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), //
        "Back", UIButton::ButtonType::Pushable, [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->goBack();
            }
        }));
}

void ScreenSystemSetup::drawContent() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("System Settings", ::SCREEN_W / 2, 20);
    tft.setFreeFont();
}
