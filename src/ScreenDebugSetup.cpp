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

    // Vízszintes gombok kollekciója
    std::vector<ButtonGroupDefinition> horizontalButtonDefs;

    horizontalButtonDefs.push_back(ButtonGroupDefinition{
        10,                                          // Gomb ID
        "Internal LED",                              // Felirat
        UIButton::ButtonType::Toggleable,            // Gomb típusa
        [this](const UIButton::ButtonEvent &event) { //
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                // DEBUG("Button ID: %d, Label: %s, State: %s\n", event.id, event.label, event.state == UIButton::EventButtonState::On ? "ON" : "OFF");
                config.data.debugGpsSerialOnInternalFastLed = event.state == UIButton::EventButtonState::On;
                gpsManager->setLedDebug(config.data.debugGpsSerialOnInternalFastLed);
            }
        },
        config.data.debugGpsSerialOnInternalFastLed ? UIButton::ButtonState::On : UIButton::ButtonState::Off, // Kezdeti állapot
        0,                                                                                                    // Szélesség (0 = auto-méretezés)
        0                                                                                                     // Magasság (0 = alapértelmezett magasság)
    });

    horizontalButtonDefs.push_back(ButtonGroupDefinition{
        11,                                          // Gomb ID
        "GPS Data On Serial",                        // Felirat
        UIButton::ButtonType::Toggleable,            // Gomb típusa
        [this](const UIButton::ButtonEvent &event) { //
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                // DEBUG("Button ID: %d, Label: %s, State: %s\n", event.id, event.label, event.state == UIButton::EventButtonState::On ? "ON" : "OFF");
                config.data.debugGpsSerialData = event.state == UIButton::EventButtonState::On;
                gpsManager->setSerialDebug(config.data.debugGpsSerialData);
            }
        },
        config.data.debugGpsSerialData ? UIButton::ButtonState::On : UIButton::ButtonState::Off, // Kezdeti állapot
        0,                                                                                       // Szélesség (0 = auto-méretezés)
        0                                                                                        // Magasság (0 = alapértelmezett magasság)
    });

    horizontalButtonDefs.push_back(ButtonGroupDefinition{
        12,                                          // Gomb ID
        "Sats DataBase On Serial",                   // Felirat
        UIButton::ButtonType::Toggleable,            // Gomb típusa
        [this](const UIButton::ButtonEvent &event) { //
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                // DEBUG("Button ID: %d, Label: %s, State: %s\n", event.id, event.label, event.state == UIButton::EventButtonState::On ? "ON" : "OFF");
                config.data.debugGpsSatellitesDatabase = event.state == UIButton::EventButtonState::On;
                gpsManager->setDebugGpsSatellitesDatabase(config.data.debugGpsSatellitesDatabase);
            }
        },
        config.data.debugGpsSatellitesDatabase ? UIButton::ButtonState::On : UIButton::ButtonState::Off, // Kezdeti állapot
        0,                                                                                               // Szélesség (0 = auto-méretezés)
        0                                                                                                // Magasság (0 = alapértelmezett magasság)
    });

    // A layoutHorizontalButtonGroup a ButtonsGroupManager-ből öröklődik
    layoutHorizontalButtonGroup(horizontalButtonDefs);

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
    tft.drawString("Debug Settings", ::SCREEN_W / 2, 50);
    tft.setFreeFont();
    // ...ide jönnek majd a beállítási gombok/ValueChangeDialog...
}
