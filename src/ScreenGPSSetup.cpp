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

    // Trafi Alarm gomb
    int row = 0;
    addChild(std::make_shared<UIButton>(                                                            //
        10,                                                                                         //
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH),                                       //
        "Traffi Alarm",                                                                             //
        UIButton::ButtonType::Toggleable,                                                           //
        config.data.gpsTraffiAlarmEnabled ? UIButton::ButtonState::On : UIButton::ButtonState::Off, //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                config.data.gpsTraffiAlarmEnabled = event.state == UIButton::EventButtonState::On;

                if (gpsTraffiAlarmDistanceBtn) {
                    gpsTraffiAlarmDistanceBtn->setEnabled(config.data.gpsTraffiAlarmEnabled);
                    gpsTraffiAlarmDistanceBtn->markForRedraw();
                }

                // Frissítsük a képernyőt, hogy a Manual gomb állapota is frissüljön
                markForRedraw(true);
            }
        }) //
    );

    constexpr int MIN_DISTANCE = 100;
    constexpr int MAX_DISTANCE = 1500;
    constexpr int STEP_DISTANCE = 10;

    // Alarm Distance gomb
    row++;
    gpsTraffiAlarmDistanceBtn = std::make_shared<UIButton>(   //
        11,                                                   //
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH), //
        "Alarm Distance",                                     //
        UIButton::ButtonType::Pushable,                       //
        UIButton::ButtonState::Off,                           //
        [this, MIN_DISTANCE, MAX_DISTANCE, STEP_DISTANCE](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                static int gpsTraffiAlarmDistance = static_cast<int>(config.data.gpsTraffiAlarmDistance);

                auto dialog = std::make_shared<ValueChangeDialog>(
                    this, "Trafi Alarm Distance", "Set distance level (100-1500 meters)", //
                    &gpsTraffiAlarmDistance, MIN_DISTANCE, MAX_DISTANCE, STEP_DISTANCE,   //
                    [this, MIN_DISTANCE, MAX_DISTANCE](const std::variant<int, float, bool> &newValue) {
                        if (std::holds_alternative<int>(newValue)) {
                            int gpsTraffiAlarmDistance = std::get<int>(newValue);
                            gpsTraffiAlarmDistance = constrain(gpsTraffiAlarmDistance, MIN_DISTANCE, MAX_DISTANCE);
                            config.data.gpsTraffiAlarmDistance = static_cast<uint16_t>(gpsTraffiAlarmDistance);
                        }
                    },
                    [this](UIDialogBase *dialog, UIDialogBase::DialogResult result) {
                        if (result == UIDialogBase::DialogResult::Accepted) {
                        } else if (result == UIDialogBase::DialogResult::Rejected) {
                            gpsTraffiAlarmDistance = static_cast<int>(config.data.gpsTraffiAlarmDistance);
                        }
                    });

                showDialog(dialog);
            }
        } //
    );
    gpsTraffiAlarmDistanceBtn->setEnabled(config.data.tftAutoBrightnessActive);
    addChild(gpsTraffiAlarmDistanceBtn);

    row++;
    addChild(std::make_shared<UIButton>(                                                                 //
        12,                                                                                              //
        Rect(btnX, btnY + row * (btnH + btnGap), btnW, btnH),                                            //
        "Alarm Siren",                                                                                   //
        UIButton::ButtonType::Toggleable,                                                                //
        config.data.gpsTraffiSirenAlarmEnabled ? UIButton::ButtonState::On : UIButton::ButtonState::Off, //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                config.data.gpsTraffiSirenAlarmEnabled = event.state == UIButton::EventButtonState::On;
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

/**
 * Kirajzolja a képernyő saját tartalmát
 */
void ScreenGPSSetup::drawContent() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("GPS Settings", ::SCREEN_W / 2, 20);
    tft.setFreeFont();
}
