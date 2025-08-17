#include "Settings.h"
#include "Config.h"               // Include the full header for implementation
#include "TftBackLightAdjuster.h" // Include full definition
#include "TrafipaxManager.h"
#include "Utils.h" // For beeping and other utilities
#include "defines.h"

// Define title bar height
const int TITLE_BAR_HEIGHT = 61; // Adjust as needed

// Constants for button layout optimization
static const int BUTTON_MARGIN = 50;
static const int BUTTON_HEIGHT = 50;
static const int BUTTON_SPACING = 60;
static const int CONTROL_BUTTON_WIDTH = 100;
static const int BACK_BUTTON_WIDTH = 150;

/**
 *  A beállítások menü inicializálása
 */
Settings::Settings(TFT_eSPI &tft, Config &config, TftBackLightAdjuster &tftBackLightAdjuster, TrafipaxManager &trafipaxManager)
    : _tft(tft), _config(config), _active(false), _currentState(ScreenState::MAIN), _needsRedraw(true), _tftBackLightAdjuster(tftBackLightAdjuster), _trafipaxManager(trafipaxManager) {}

/**
 *  A beállítások menü inicializálása
 */
void Settings::init() {
    // Reserve memory to avoid reallocations
    _mainButtons.reserve(5);
    _brightnessButtons.reserve(4);
    _alarmButtons.reserve(4);
    _informationButtons.reserve(1);

    initMainButtons();
    initBrightnessButtons();
    initAlarmButtons();
    initInformationButtons();
}

void Settings::initMainButtons() {
    int screenWidth = _tft.width();
    int buttonWidth = screenWidth - 2 * BUTTON_MARGIN;
    int yPos = 80;

    // Screen Button
    createButton(_mainButtons, BUTTON_MARGIN, yPos, buttonWidth, BUTTON_HEIGHT, "Screen", TFT_WHITE, [this]() {
        _currentState = ScreenState::BRIGHTNESS;
        _needsRedraw = true;
        Utils::beepTick();
    });

    yPos += BUTTON_SPACING;
    // Alarm Button
    createButton(_mainButtons, BUTTON_MARGIN, yPos, buttonWidth, BUTTON_HEIGHT, "Alarm", TFT_WHITE, [this]() {
        _currentState = ScreenState::ALARM;
        _needsRedraw = true;
        Utils::beepTick();
    });

    yPos += BUTTON_SPACING;
    // Info Button
    createButton(_mainButtons, BUTTON_MARGIN, yPos, buttonWidth, BUTTON_HEIGHT, "Info", TFT_WHITE, [this]() {
        _currentState = ScreenState::INFORMATION;
        _needsRedraw = true;
        Utils::beepTick();
    });

    // Exit Button
    createButton(_mainButtons, 0, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Exit", TFT_RED, [this]() { exit(); });

    // Save Button
    createButton(_mainButtons, screenWidth - BACK_BUTTON_WIDTH, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Save", TFT_GREEN, [this]() {
        _config.checkSave();
        Utils::beepTick();
        delay(100);
        exit();
    });
}

void Settings::initInformationButtons() {
    // Back Button
    createButton(_informationButtons, _tft.width() - BACK_BUTTON_WIDTH, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Back", TFT_ORANGE, [this]() {
        _currentState = ScreenState::MAIN;
        _needsRedraw = true;
        Utils::beepTick();
    });
}

void Settings::initBrightnessButtons() {
    int screenWidth = _tft.width();
    int buttonWidth = screenWidth - 2 * BUTTON_MARGIN;

    // Auto Brightness Toggle Button
    createButton(_brightnessButtons, BUTTON_MARGIN, 80, buttonWidth, BUTTON_HEIGHT, "", TFT_WHITE, [this]() {
        _config.data.tftAutoBrightnessActive = !_config.data.tftAutoBrightnessActive;
        _tftBackLightAdjuster.setAutoBrightnessActive(_config.data.tftAutoBrightnessActive);
        _needsRedraw = true;
        Utils::beepTick();
    });

    // Manual Brightness Down Button
    createButton(_brightnessButtons, BUTTON_MARGIN, 150, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "-", TFT_RED, [this]() {
        if (!_config.data.tftAutoBrightnessActive) {
            uint8_t &val = _config.data.tftManualBrightnessValue;
            val = (val > 5) ? val - 5 : NIGHTLY_BRIGHTNESS;
            _tftBackLightAdjuster.setBacklightLevel(val);
            _config.data.tftManualBrightnessValue = _tftBackLightAdjuster.getBacklightLevel();
            updateBrightnessValueDisplay();
            Utils::beepTick();
        }
    });

    // Manual Brightness Up Button
    createButton(_brightnessButtons, screenWidth - 150, 150, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "+", TFT_GREEN, [this]() {
        if (!_config.data.tftAutoBrightnessActive) {
            uint8_t &val = _config.data.tftManualBrightnessValue;
            val = (val < TFT_BACKGROUND_LED_MAX_BRIGHTNESS - 5) ? val + 5 : TFT_BACKGROUND_LED_MAX_BRIGHTNESS;
            _tftBackLightAdjuster.setBacklightLevel(val);
            _config.data.tftManualBrightnessValue = _tftBackLightAdjuster.getBacklightLevel();
            updateBrightnessValueDisplay();
            Utils::beepTick();
        }
    });

    // Back Button
    createButton(_brightnessButtons, screenWidth - BACK_BUTTON_WIDTH, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Back", TFT_ORANGE, [this]() {
        _currentState = ScreenState::MAIN;
        _needsRedraw = true;
        Utils::beepTick();
    });
}

void Settings::initAlarmButtons() {
    int screenWidth = _tft.width();
    int buttonWidth = screenWidth - 2 * BUTTON_MARGIN;

    // GPS Trafi Alarm Enabled Toggle Button
    createButton(_alarmButtons, BUTTON_MARGIN, 80, buttonWidth, BUTTON_HEIGHT, "", TFT_WHITE, [this]() {
        _config.data.gpsTrafiAlarmEnabled = !_config.data.gpsTrafiAlarmEnabled;
        _needsRedraw = true;
        Utils::beepTick();
    });

    // GPS Trafi Alarm Distance Down Button
    createButton(_alarmButtons, BUTTON_MARGIN, 150, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "-", TFT_RED, [this]() {
        if (_config.data.gpsTrafiAlarmEnabled) {
            uint16_t &val = _config.data.gpsTrafiAlarmDistance;
            val = (val > 400) ? val - 100 : 400;
            updateAlarmValueDisplay();
            Utils::beepTick();
        }
    });

    // GPS Trafi Alarm Distance Up Button
    createButton(_alarmButtons, screenWidth - 150, 150, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "+", TFT_GREEN, [this]() {
        if (_config.data.gpsTrafiAlarmEnabled) {
            uint16_t &val = _config.data.gpsTrafiAlarmDistance;
            val = (val < 2000) ? val + 100 : 2000;
            updateAlarmValueDisplay();
            Utils::beepTick();
        }
    });

    // Back Button
    createButton(_alarmButtons, screenWidth - BACK_BUTTON_WIDTH, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Back", TFT_ORANGE, [this]() {
        _currentState = ScreenState::MAIN;
        _needsRedraw = true;
        Utils::beepTick();
    });
}

bool Settings::isActive() { return _active; }

void Settings::enter() {
    _active = true;
    _currentState = ScreenState::MAIN;
    _needsRedraw = true;
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
    char autoText[20];
    sprintf(autoText, "Auto: %s", _config.data.tftAutoBrightnessActive ? "On" : "Off");
    _brightnessButtons[0].setText(autoText);

    // Update button states based on auto-brightness mode
    uint16_t disabledColor = _config.data.tftAutoBrightnessActive ? TFT_DARKGREY : TFT_RED;
    uint16_t enabledColor = _config.data.tftAutoBrightnessActive ? TFT_DARKGREY : TFT_GREEN;

    _brightnessButtons[1].setBorderColor(disabledColor);
    _brightnessButtons[2].setBorderColor(enabledColor);

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
    char enabledText[20];
    sprintf(enabledText, "Trafi Alarm: %s", _config.data.gpsTrafiAlarmEnabled ? "On" : "Off");
    _alarmButtons[0].setText(enabledText);

    // Update button states based on alarm enabled mode
    uint16_t disabledColor = !_config.data.gpsTrafiAlarmEnabled ? TFT_DARKGREY : TFT_RED;
    uint16_t enabledColor = !_config.data.gpsTrafiAlarmEnabled ? TFT_DARKGREY : TFT_GREEN;

    _alarmButtons[1].setBorderColor(disabledColor);
    _alarmButtons[2].setBorderColor(enabledColor);

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
void Settings::updateBrightnessValueDisplay() { updateValueDisplay(_config.data.tftManualBrightnessValue, "-", !_config.data.tftAutoBrightnessActive); }

void Settings::updateAlarmValueDisplay() { updateValueDisplay(_config.data.gpsTrafiAlarmDistance, "-", _config.data.gpsTrafiAlarmEnabled); }

void Settings::handleTouch() {
    uint16_t x, y;
    if (_tft.getTouch(&x, &y)) {
        switch (_currentState) {
            case ScreenState::MAIN:
                handleTouchForButtonList(_mainButtons, x, y);
                break;
            case ScreenState::BRIGHTNESS:
                handleTouchForButtonList(_brightnessButtons, x, y);
                break;
            case ScreenState::ALARM:
                handleTouchForButtonList(_alarmButtons, x, y);
                break;
            case ScreenState::INFORMATION:
                handleTouchForButtonList(_informationButtons, x, y);
                break;
        }
        delay(200); // Debounce
    }
}

// Legacy touch handlers kept for compatibility but simplified
void Settings::handleMainTouch(uint16_t x, uint16_t y) { handleTouchForButtonList(_mainButtons, x, y); }

void Settings::handleBrightnessTouch(uint16_t x, uint16_t y) { handleTouchForButtonList(_brightnessButtons, x, y); }

void Settings::handleAlarmTouch(uint16_t x, uint16_t y) { handleTouchForButtonList(_alarmButtons, x, y); }

void Settings::handleInformationTouch(uint16_t x, uint16_t y) { handleTouchForButtonList(_informationButtons, x, y); }

void Settings::loop() {
    if (!_active) {
        return;
    }

    if (_needsRedraw) {
        draw();
        _needsRedraw = false;
    }

    handleTouch();
}

// Helper function implementations
void Settings::createButton(std::vector<Button> &buttonList, int16_t x, int16_t y, int16_t w, int16_t h, const char *label, uint16_t borderColor, std::function<void()> callback) {
    buttonList.emplace_back(_tft, x, y, w, h, label, TFT_BLACK, TFT_WHITE, borderColor, 4);
    buttonList.back().setCallback(callback);
}

void Settings::handleTouchForButtonList(std::vector<Button> &buttonList, uint16_t x, uint16_t y) {
    for (auto &button : buttonList) {
        if (button.contains(x, y)) {
            button.press();
            return;
        }
    }
}

void Settings::updateValueDisplay(int value, const char *disabledText, bool isEnabled) {
    // Clear previous value
    _tft.setTextFont(4);
    _tft.setTextSize(2);
    int text_width = _tft.textWidth("2000"); // Max width for safety
    int text_height = _tft.fontHeight();
    int x_center = _tft.width() / 2;
    int y_center = 175;

    _tft.fillRect(x_center - text_width / 2, y_center - text_height / 2, text_width, text_height, TFT_BLACK);

    // Display new value
    _tft.setTextDatum(MC_DATUM);

    if (isEnabled) {
        _tft.setTextColor(TFT_WHITE);
        _tft.drawString(String(value), x_center, y_center);
    } else {
        _tft.setTextColor(TFT_DARKGREY);
        _tft.drawString(disabledText, x_center, y_center);
    }

    _tft.setTextFont(1); // Restore default font
    _tft.setTextSize(1); // Restore default text size
}
