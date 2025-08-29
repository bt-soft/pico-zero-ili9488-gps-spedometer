#include "ScreenSetup.h"
#include "Config.h"
#include "Utils.h"
#include "defines.h"

/**
 * UI komponensek elhelyezése
 */
void ScreenSetup::layoutComponents() {

    int btnW = 180;
    int btnH = 60;
    int buttonXGap = 30;

    // 1. sor
    int buttonY = 80;

    // TFT beállítások gomb
    addChild(std::make_shared<UIButton>( //
        10, Rect(buttonXGap, buttonY, btnW, btnH), "TFT", UIButton::ButtonType::Pushable, [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_TFT_SETUP);
            }
        }));

    // System beállítások button
    addChild(std::make_shared<UIButton>( //
        11, Rect((::SCREEN_W - btnW) - buttonXGap, buttonY, btnW, btnH), "System", UIButton::ButtonType::Pushable, [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_SYSTEM_SETUP);
            }
        }));

    // 2. sor
    buttonY = 170;

    // GPS beállítások gomb
    addChild(std::make_shared<UIButton>(       //
        12,                                    //
        Rect(buttonXGap, buttonY, btnW, btnH), //
        "GPS Alarm",                           //
        UIButton::ButtonType::Pushable,        //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_GPS_SETUP);
            }
        }) //
    );

    // Debug beállítások gomb
    addChild(std::make_shared<UIButton>(                             //
        13,                                                          //
        Rect((::SCREEN_W - btnW) - buttonXGap, buttonY, btnW, btnH), //
        "Debug",                                                     //
        UIButton::ButtonType::Pushable,
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_DEBUG_SETUP);
            }
        }) //
    );

    // Back gomb jobb alsó sarokban
    addChild(std::make_shared<UIButton>(                                                                                                                                  //
        1,                                                                                                                                                                //
        Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), //
        "Back",                                                                                                                                                           //
        UIButton::ButtonType::Pushable,                                                                                                                                   //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {

                // Itt mentjük a menthetőt
                config.checkSave();
                getScreenManager()->goBack();
            }
        }) //
    );
}

/**
 * Kirajzolja a képernyő saját tartalmát
 */
void ScreenSetup::drawContent() {
    // Háttér törlése
    tft.fillScreen(TFT_BLACK);

    // Címsor
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Setup Screen", ::SCREEN_W / 2, 30);
}
