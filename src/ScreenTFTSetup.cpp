#include "ScreenTFTSetup.h"
#include "Config.h"
#include "Utils.h"
#include "ValueChangeDialog.h"
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
        10,                                                                                           //
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH),                                         //
        "Auto Brightness",                                                                            //
        UIButton::ButtonType::Toggleable,                                                             //
        config.data.tftAutoBrightnessActive ? UIButton::ButtonState::On : UIButton::ButtonState::Off, //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                config.data.tftAutoBrightnessActive = event.state == UIButton::EventButtonState::On;

                if (manualBrightnessBtn) {
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

                // Lokális int változó a dialog számára
                static int brightnessValue = static_cast<int>(config.data.tftManualBrightnessValue);

                auto dialog = std::make_shared<ValueChangeDialog>(
                    this,                           // parent screen
                    "Manual Brightness",            // title
                    "Set brightness level (5-255)", // message
                    &brightnessValue,               // lokális int pointer
                    5,                              // min value
                    255,                            // max value
                    5,                              // step value
                    [this](const std::variant<int, float, bool> &newValue) {
                        // Value change callback - frissítjük a fényerőt azonnal
                        if (std::holds_alternative<int>(newValue)) {
                            int brightness = std::get<int>(newValue);
                            // Biztosítsuk, hogy a tartományban maradjon
                            brightness = constrain(brightness, 5, 255);
                            config.data.tftManualBrightnessValue = static_cast<uint8_t>(brightness);
                            tftBackLightAdjuster.setBacklightLevel(brightness);
                        }
                    },
                    [this](UIDialogBase *dialog, UIDialogBase::DialogResult result) {
                        // Dialog close callback
                        if (result == UIDialogBase::DialogResult::Accepted) {
                            // OK esetén mentjük a konfigurációt
                            // config.checkSave();
                        }
                        // Cancel esetén a ValueChangeDialog automatikusan visszaállítja az eredeti értéket
                        // De nekünk is vissza kell állítani a config értéket
                        else if (result == UIDialogBase::DialogResult::Rejected) {
                            // Visszaállítjuk az eredeti értéket Cancel esetén
                            brightnessValue = static_cast<int>(config.data.tftManualBrightnessValue);
                        }
                    });

                showDialog(dialog);
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
                getScreenManager()->goBack();
            }
        }));
}

/**
 * Kirajzolja a képernyő saját tartalmát
 */
void ScreenTFTSetup::drawContent() {

    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("TFT Settings", ::SCREEN_W / 2, 20);
    tft.setFreeFont();
}
