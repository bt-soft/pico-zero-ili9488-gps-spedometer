#include "ScreenDebugSetup.h"
#include "Config.h"
#include "Utils.h"
#include "defines.h"

#include "GpsManager.h"
extern GpsManager *gpsManager;

/**
 * UI komponensek elhelyezése
 */
void ScreenDebugSetup::layoutComponents() {

    // Függőlegesen egymás alá
    int btnW = 180;
    int btnH = 60;
    int btnX = (::SCREEN_W - btnW) / 2;
    int btnY = 60;
    int btnGap = 20;

    // LED gomb
    int row = 0;
    addChild(std::make_shared<UIButton>(                                                                      //
        10,                                                                                                   // id
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH),                                                 // rect
        "Zero Internal LED",                                                                                  // label
        UIButton::ButtonType::Toggleable,                                                                     // type
        config.data.debugGpsSerialOnInternalFastLed ? UIButton::ButtonState::On : UIButton::ButtonState::Off, // Kezdeti állapot
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                // DEBUG("Button ID: %d, Label: %s, State: %s\n", event.id, event.label, event.state == UIButton::EventButtonState::On ? "ON" : "OFF");
                config.data.debugGpsSerialOnInternalFastLed = event.state == UIButton::EventButtonState::On;
                gpsManager->setLedDebug(config.data.debugGpsSerialOnInternalFastLed);
            }
        }) //
    );

    // GPS Data on Serial
    row++;
    addChild(std::make_shared<UIButton>(                                                         //
        11,                                                                                      // id
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH),                                    // rect
        "GPS On Serial",                                                                         // label
        UIButton::ButtonType::Toggleable,                                                        // type
        config.data.debugGpsSerialData ? UIButton::ButtonState::On : UIButton::ButtonState::Off, // Kezdeti állapot
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                // DEBUG("Button ID: %d, Label: %s, State: %s\n", event.id, event.label, event.state == UIButton::EventButtonState::On ? "ON" : "OFF");
                config.data.debugGpsSerialData = event.state == UIButton::EventButtonState::On;
                gpsManager->setSerialDebug(config.data.debugGpsSerialData);
            }
        }) //
    );

    // SatDB on Serial
    row++;
    addChild(std::make_shared<UIButton>(                                                                 //
        12,                                                                                              // id
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH),                                            // rect
        "SatDB On Serial",                                                                               // label
        UIButton::ButtonType::Toggleable,                                                                // type
        config.data.debugGpsSatellitesDatabase ? UIButton::ButtonState::On : UIButton::ButtonState::Off, // Kezdeti állapot
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                // DEBUG("Button ID: %d, Label: %s, State: %s\n", event.id, event.label, event.state == UIButton::EventButtonState::On ? "ON" : "OFF");
                config.data.debugGpsSatellitesDatabase = event.state == UIButton::EventButtonState::On;
                gpsManager->setDebugGpsSatellitesDatabase(config.data.debugGpsSatellitesDatabase);
            }
        }) //
    );

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

/**
 *
 */
void ScreenDebugSetup::drawContent() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("Debug Settings", ::SCREEN_W / 2, 20);
    tft.setFreeFont();
    // ...ide jönnek majd a beállítási gombok/ValueChangeDialog...
}
