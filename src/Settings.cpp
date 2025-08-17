#include "Settings.h"
#include "Config.h"               // Include the full header for implementation
#include "TftBackLightAdjuster.h" // Include full definition
#include "TrafipaxManager.h"
#include "Utils.h" // For beeping and other utilities
#include "defines.h"

// Define title bar height
const int TITLE_BAR_HEIGHT = 61; // Adjust as needed

/**
 *  A beállítások menü inicializálása
 */
Settings::Settings(TFT_eSPI &tft, Config &config, TftBackLightAdjuster &tftBackLightAdjuster, TrafipaxManager &trafipaxManager)
    : _tft(tft), _config(config), _active(false), _currentState(ScreenState::MAIN), _tftBackLightAdjuster(tftBackLightAdjuster), _trafipaxManager(trafipaxManager) {}

/**
 *  A beállítások menü inicializálása
 */
void Settings::init() {
    _mainButtons.reserve(5);

    // Screen Button
    _mainButtons.emplace_back(_tft, 50, 80, _tft.width() - 100, 50, "Screen", TFT_BLACK, TFT_WHITE, TFT_WHITE, 4);
    _mainButtons.back().setCallback([this]() {
        _currentState = ScreenState::BRIGHTNESS;
        draw();
        Utils::beepTick();
    });

    // Alarm Button
    _mainButtons.emplace_back(_tft, 50, 140, _tft.width() - 100, 50, "Alarm", TFT_BLACK, TFT_WHITE, TFT_WHITE, 4);
    _mainButtons.back().setCallback([this]() {
        _currentState = ScreenState::ALARM;
        draw();
        Utils::beepTick();
    });

    // Info Button
    _mainButtons.emplace_back(_tft, 50, 200, _tft.width() - 100, 50, "Info", TFT_BLACK, TFT_WHITE, TFT_WHITE, 4);
    _mainButtons.back().setCallback([this]() {
        _currentState = ScreenState::INFORMATION;
        draw();
        Utils::beepTick();
    });

    // Exit Button
    _mainButtons.emplace_back(_tft, 0, _tft.height() - 50, 150, 50, "Exit", TFT_BLACK, TFT_WHITE, TFT_RED, 4);
    _mainButtons.back().setCallback([this]() { exit(); });

    // Save Button
    _mainButtons.emplace_back(_tft, _tft.width() - 150, _tft.height() - 50, 150, 50, "Save", TFT_BLACK, TFT_WHITE, TFT_GREEN, 4);
    _mainButtons.back().setCallback([this]() {
        _config.checkSave();
        Utils::beepTick();
        delay(100); // Short pause for feedback
        exit();
    });

    initBrightnessButtons();
    initAlarmButtons();
    initInformationButtons();
}

void Settings::initInformationButtons() {
    _informationButtons.reserve(1);

    // Back Button
    _informationButtons.emplace_back(_tft, _tft.width() - 150, _tft.height() - 50, 150, 50, "Back", TFT_BLACK, TFT_WHITE, TFT_ORANGE, 4);
    _informationButtons.back().setCallback([this]() {
        _currentState = ScreenState::MAIN;
        draw();
        Utils::beepTick();
    });
}

void Settings::initBrightnessButtons() {
    _brightnessButtons.reserve(4);

    // Auto Brightness Toggle Button
    _brightnessButtons.emplace_back(_tft, 50, 80, _tft.width() - 100, 50, "", TFT_BLACK, TFT_WHITE, TFT_WHITE, 4);
    _brightnessButtons.back().setCallback([this]() {
        _config.data.tftAutoBrightnessActive = !_config.data.tftAutoBrightnessActive;
        _tftBackLightAdjuster.setAutoBrightnessActive(_config.data.tftAutoBrightnessActive);

        draw(); // Redraw to update text and button states

        Utils::beepTick();
    });

    // Manual Brightness Down Button
    _brightnessButtons.emplace_back(_tft, 50, 150, 100, 50, "-", TFT_BLACK, TFT_WHITE, TFT_RED, 4);
    _brightnessButtons.back().setCallback([this]() {
        if (!_config.data.tftAutoBrightnessActive) {

            uint8_t &val = _config.data.tftManualBrightnessValue;
            val = (val > 5) ? val - 5 : NIGHTLY_BRIGHTNESS;

            _tftBackLightAdjuster.setBacklightLevel(val);                                      // beállítjuk
            _config.data.tftManualBrightnessValue = _tftBackLightAdjuster.getBacklightLevel(); // lekérdezzük az  értéket és azt toljuk be a konfigurációba
            updateBrightnessValueDisplay();

            Utils::beepTick();
        }
    });

    // Manual Brightness Up Button
    _brightnessButtons.emplace_back(_tft, _tft.width() - 150, 150, 100, 50, "+", TFT_BLACK, TFT_WHITE, TFT_GREEN, 4);
    _brightnessButtons.back().setCallback([this]() {
        if (!_config.data.tftAutoBrightnessActive) {

            uint8_t &val = _config.data.tftManualBrightnessValue;
            val = (val < TFT_BACKGROUND_LED_MAX_BRIGHTNESS - 5) ? val + 5 : TFT_BACKGROUND_LED_MAX_BRIGHTNESS;

            _tftBackLightAdjuster.setBacklightLevel(val);                                      // beállítjuk
            _config.data.tftManualBrightnessValue = _tftBackLightAdjuster.getBacklightLevel(); // lekérdezzük az  értéket és azt toljuk be a konfigurációba
            updateBrightnessValueDisplay();

            Utils::beepTick();
        }
    });

    // Back Button
    _brightnessButtons.emplace_back(_tft, _tft.width() - 150, _tft.height() - 50, 150, 50, "Back", TFT_BLACK, TFT_WHITE, TFT_ORANGE, 4);
    _brightnessButtons.back().setCallback([this]() {
        _currentState = ScreenState::MAIN;
        draw();
        Utils::beepTick();
    });
}

void Settings::initAlarmButtons() {
    _alarmButtons.reserve(4);

    // GPS Trafi Alarm Enabled Toggle Button
    _alarmButtons.emplace_back(_tft, 50, 80, _tft.width() - 100, 50, "", TFT_BLACK, TFT_WHITE, TFT_WHITE, 4);
    _alarmButtons.back().setCallback([this]() {
        _config.data.gpsTrafiAlarmEnabled = !_config.data.gpsTrafiAlarmEnabled;
        draw(); // Redraw to update text and button states
        Utils::beepTick();
    });

    // GPS Trafi Alarm Distance Down Button
    _alarmButtons.emplace_back(_tft, 50, 150, 100, 50, "-", TFT_BLACK, TFT_WHITE, TFT_RED, 4);
    _alarmButtons.back().setCallback([this]() {
        if (_config.data.gpsTrafiAlarmEnabled) {
            uint16_t &val = _config.data.gpsTrafiAlarmDistance;
            val = (val > 400) ? val - 100 : 400;
            updateAlarmValueDisplay();
            Utils::beepTick();
        }
    });

    // GPS Trafi Alarm Distance Up Button
    _alarmButtons.emplace_back(_tft, _tft.width() - 150, 150, 100, 50, "+", TFT_BLACK, TFT_WHITE, TFT_GREEN, 4);
    _alarmButtons.back().setCallback([this]() {
        if (_config.data.gpsTrafiAlarmEnabled) {
            uint16_t &val = _config.data.gpsTrafiAlarmDistance;
            val = (val < 2000) ? val + 100 : 2000;
            updateAlarmValueDisplay();
            Utils::beepTick();
        }
    });

    // Back Button
    _alarmButtons.emplace_back(_tft, _tft.width() - 150, _tft.height() - 50, 150, 50, "Back", TFT_BLACK, TFT_WHITE, TFT_ORANGE, 4);
    _alarmButtons.back().setCallback([this]() {
        _currentState = ScreenState::MAIN;
        draw();
        Utils::beepTick();
    });
}

bool Settings::isActive() { return _active; }

void Settings::enter() {
    _active = true;
    _currentState = ScreenState::MAIN;
    draw();
    Utils::beepTick();
    delay(200);
}

void Settings::exit() {
    _active = false;
    Utils::beepTick();
}

void Settings::draw() {
    _tft.fillScreen(TFT_BLACK);
    switch (_currentState) {
        case ScreenState::MAIN:
            drawMainScreen();
            break;
        case ScreenState::BRIGHTNESS:
            drawBrightnessScreen();
            break;
        case ScreenState::ALARM:
            drawAlarmScreen();
            break;
        case ScreenState::INFORMATION:
            drawInformationScreen();
            break;
    }
}

/**
 * Képernyő címének megjelenítése
 */
void Settings::drawScreenTitle(const char *title) {
    _tft.fillRect(0, 0, _tft.width(), TITLE_BAR_HEIGHT, TFT_WHITE);

    _tft.setTextFont(4);
    _tft.setTextSize(2);
    _tft.setTextColor(TFT_BLACK, TFT_WHITE); // Black text, white background
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString(title, _tft.width() / 2, TITLE_BAR_HEIGHT / 2 + 5); // Center text vertically within the bar
    _tft.setTextSize(1);
}

/**
 * Fő képernyő megjelenítése
 */
void Settings::drawMainScreen() {
    // Képernyő címének megjelenítése
    drawScreenTitle("Settings");

    for (auto &button : _mainButtons) {
        button.draw();
    }
}

/**
 * Képernyő világosítása
 */
void Settings::drawBrightnessScreen() {

    // Képernyő címének megjelenítése
    drawScreenTitle("Brightness");

    // Update Auto-Brightness button text
    String autoText = "Auto: ";
    autoText += (_config.data.tftAutoBrightnessActive ? "On" : "Off");
    _brightnessButtons[0].setText(autoText);

    // Update button states based on auto-brightness mode
    if (_config.data.tftAutoBrightnessActive) {
        _brightnessButtons[1].setBorderColor(TFT_DARKGREY);
        _brightnessButtons[2].setBorderColor(TFT_DARKGREY);
    } else {
        _brightnessButtons[1].setBorderColor(TFT_RED);
        _brightnessButtons[2].setBorderColor(TFT_GREEN);
    }

    // Draw all buttons
    for (auto &button : _brightnessButtons) {
        button.draw();
    }

    // Display manual brightness value
    updateBrightnessValueDisplay();
}

/**
 * Riasztási képernyő megjelenítése
 */
void Settings::drawAlarmScreen() {

    // Képernyő címének megjelenítése
    drawScreenTitle("Alarm Settings");

    // Update GPS Trafi Alarm Enabled button text
    String enabledText = "Trafi Alarm: ";
    enabledText += (_config.data.gpsTrafiAlarmEnabled ? "On" : "Off");
    _alarmButtons[0].setText(enabledText);

    // Update button states based on alarm enabled mode
    if (!_config.data.gpsTrafiAlarmEnabled) {
        _alarmButtons[1].setBorderColor(TFT_DARKGREY);
        _alarmButtons[2].setBorderColor(TFT_DARKGREY);
    } else {
        _alarmButtons[1].setBorderColor(TFT_RED);
        _alarmButtons[2].setBorderColor(TFT_GREEN);
    }

    // Draw all buttons
    for (auto &button : _alarmButtons) {
        button.draw();
    }

    // Display alarm distance value
    updateAlarmValueDisplay();
}

/**
 * Információs képernyő megjelenítése
 */
void Settings::drawInformationScreen() {

    // Képernyő címének megjelenítése
    drawScreenTitle("Information");

    char buf[64];

    // Program Version
    _tft.setTextDatum(TL_DATUM);
    _tft.setTextColor(TFT_CYAN);
    sprintf(buf, "Version: V%s", APP_VERSION);
    _tft.drawString(buf, 50, 100);

    // Build Time
    sprintf(buf, "Build: %s %s", __DATE__, __TIME__);
    _tft.drawString(buf, 50, 140);

    // Trafipax Count
    sprintf(buf, "Trafipax Count: %d", _trafipaxManager.count());
    _tft.drawString(buf, 50, 180);

    // Draw all buttons
    for (auto &button : _informationButtons) {
        button.draw();
    }
}

/**
 * Képernyő világosítása
 */
void Settings::updateBrightnessValueDisplay() {
    // Clear previous value
    _tft.setTextFont(4);
    _tft.setTextSize(2);
    int text_width = _tft.textWidth("255"); // Max 3 digits
    int text_height = _tft.fontHeight();
    int x_center = _tft.width() / 2;
    int y_center = 175;

    _tft.fillRect(x_center - text_width / 2, y_center - text_height / 2, text_width, text_height, TFT_BLACK);

    // Display new value
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(TFT_WHITE); // Set text color without background

    if (!_config.data.tftAutoBrightnessActive) {
        String valStr = String(_config.data.tftManualBrightnessValue);
        _tft.drawString(valStr, _tft.width() / 2, 175);
    } else {
        _tft.setTextColor(TFT_DARKGREY); // Set text color without background
        _tft.drawString("-", _tft.width() / 2, 175);
    }
    _tft.setTextFont(1); // Restore default font
    _tft.setTextSize(1); // Restore default text size
}

void Settings::updateAlarmValueDisplay() {
    // Clear previous value
    _tft.setTextFont(4);
    _tft.setTextSize(2);
    int text_width = _tft.textWidth("2000"); // Max 4 digits
    int text_height = _tft.fontHeight();
    int x_center = _tft.width() / 2;
    int y_center = 175;

    _tft.fillRect(x_center - text_width / 2, y_center - text_height / 2, text_width, text_height, TFT_BLACK);

    // Display new value
    _tft.setTextDatum(MC_DATUM);
    _tft.setTextColor(TFT_WHITE); // Set text color without background

    if (_config.data.gpsTrafiAlarmEnabled) {
        String valStr = String(_config.data.gpsTrafiAlarmDistance);
        _tft.drawString(valStr, _tft.width() / 2, 175);
    } else {
        _tft.setTextColor(TFT_DARKGREY); // Set text color without background
        _tft.drawString("-", _tft.width() / 2, 175);
    }
    _tft.setTextFont(1); // Restore default font
    _tft.setTextSize(1); // Restore default text size
}

void Settings::handleTouch() {
    uint16_t x, y;
    if (_tft.getTouch(&x, &y)) {
        switch (_currentState) {
            case ScreenState::MAIN:
                handleMainTouch(x, y);
                break;
            case ScreenState::BRIGHTNESS:
                handleBrightnessTouch(x, y);
                break;
            case ScreenState::ALARM:
                handleAlarmTouch(x, y);
                break;
            case ScreenState::INFORMATION:
                handleInformationTouch(x, y);
                break;
        }
        delay(200); // Debounce
    }
}

void Settings::handleMainTouch(uint16_t x, uint16_t y) {
    for (auto &button : _mainButtons) {
        if (button.contains(x, y)) {
            button.press();
            return;
        }
    }
}

void Settings::handleBrightnessTouch(uint16_t x, uint16_t y) {
    for (auto &button : _brightnessButtons) {
        if (button.contains(x, y)) {
            button.press();
            return;
        }
    }
}

void Settings::handleAlarmTouch(uint16_t x, uint16_t y) {
    for (auto &button : _alarmButtons) {
        if (button.contains(x, y)) {
            button.press();
            return;
        }
    }
}

void Settings::handleInformationTouch(uint16_t x, uint16_t y) {
    for (auto &button : _informationButtons) {
        if (button.contains(x, y)) {
            button.press();
            return;
        }
    }
}

void Settings::loop() {
    if (!_active) {
        return;
    }
    handleTouch();
}
