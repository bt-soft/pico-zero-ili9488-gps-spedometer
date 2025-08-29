#include "ScreenGPSSetup.h"
#include "Config.h"
#include "Utils.h"
#include "ValueChangeDialog.h"
#include "defines.h"

/**
 * UI komponensek elhelyezése
 */
void ScreenGPSSetup::layoutComponents() {

    // Függőlegesen egymás alá
    int btnW = 180;
    int btnH = 60;
    int btnX = (::SCREEN_W - btnW) / 2;
    int btnY = 60;
    int btnGap = 20;

    // Auto Brightness gomb
    int row = 0;
    addChild(std::make_shared<UIButton>(                                                           //
        10,                                                                                        //
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH),                                      //
        "Trafi Alarm",                                                                             //
        UIButton::ButtonType::Toggleable,                                                          //
        config.data.gpsTrafiAlarmEnabled ? UIButton::ButtonState::On : UIButton::ButtonState::Off, //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                config.data.gpsTrafiAlarmEnabled = event.state == UIButton::EventButtonState::On;

                if (gpsTrafiAlarmDistanceBtn) {
                    gpsTrafiAlarmDistanceBtn->setEnabled(config.data.gpsTrafiAlarmEnabled);
                    gpsTrafiAlarmDistanceBtn->markForRedraw();
                }

                // Frissítsük a képernyőt, hogy a Manual gomb állapota is frissüljön
                markForRedraw(true);
            }
        }) //
    );

    constexpr int MIN_DISTANCE = 100;
    constexpr int MAX_DISTANCE = 1500;
    constexpr int STEP_DISTANCE = 10;

    // Manual Brightness gomb
    row++;
    gpsTrafiAlarmDistanceBtn = std::make_shared<UIButton>(    //
        11,                                                   //
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH), //
        "Alarm Distance",                                     //
        UIButton::ButtonType::Pushable,                       //
        UIButton::ButtonState::Off,                           //
        [this, MIN_DISTANCE, MAX_DISTANCE, STEP_DISTANCE](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                // ... itt már elérhetőek a konstansok!
                static int gpsTrafiAlarmDistance = static_cast<int>(config.data.gpsTrafiAlarmDistance);

                auto dialog = std::make_shared<ValueChangeDialog>(
                    this, "Trafi Alarm Distance", "Set distance level (100-1500 meters)", &gpsTrafiAlarmDistance, MIN_DISTANCE, MAX_DISTANCE, STEP_DISTANCE,
                    [this, MIN_DISTANCE, MAX_DISTANCE](const std::variant<int, float, bool> &newValue) {
                        if (std::holds_alternative<int>(newValue)) {
                            int gpsTrafiAlarmDistance = std::get<int>(newValue);
                            gpsTrafiAlarmDistance = constrain(gpsTrafiAlarmDistance, MIN_DISTANCE, MAX_DISTANCE);
                            config.data.gpsTrafiAlarmDistance = static_cast<uint16_t>(gpsTrafiAlarmDistance);
                        }
                    },
                    [this](UIDialogBase *dialog, UIDialogBase::DialogResult result) {
                        if (result == UIDialogBase::DialogResult::Accepted) {
                            // config.checkSave();
                        } else if (result == UIDialogBase::DialogResult::Rejected) {
                            gpsTrafiAlarmDistance = static_cast<int>(config.data.gpsTrafiAlarmDistance);
                        }
                    });

                showDialog(dialog);
            }
        } //
    );
    gpsTrafiAlarmDistanceBtn->setEnabled(config.data.tftAutoBrightnessActive);
    addChild(gpsTrafiAlarmDistanceBtn);

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
void ScreenGPSSetup::drawContent() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextSize(1);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("GPS Settings", ::SCREEN_W / 2, 20);
    tft.setFreeFont();
}
