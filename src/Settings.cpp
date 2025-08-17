#include "Settings.h"
#include "Config.h"               // Include the full header for implementation
#include "TftBackLightAdjuster.h" // Include full definition
#include "TrafipaxManager.h"
#include "Utils.h" // For beeping and other utilities
#include "defines.h"

// Define title bar height
const int TITLE_BAR_HEIGHT = 80; // Increased height for better visual

// Constants for button layout optimization
static const int BUTTON_MARGIN = 40;
static const int BUTTON_HEIGHT = 60; // Increased height for better touch
static const int BUTTON_SPACING = 75;
static const int CONTROL_BUTTON_WIDTH = 120; // Wider for better touch
static const int BACK_BUTTON_WIDTH = 160;

// Visual enhancement constants
static const int BUTTON_CORNER_RADIUS = 15;
static const int TITLE_SHADOW_OFFSET = 3;

// Color scheme for modern look
static const uint16_t HEADER_BG_COLOR = RGB565(45, 45, 75); // Dark blue-grey
static const uint16_t HEADER_TEXT_COLOR = TFT_WHITE;
static const uint16_t BUTTON_BG_COLOR = RGB565(60, 60, 90); // Slightly lighter
static const uint16_t BUTTON_TEXT_COLOR = TFT_WHITE;
static const uint16_t BUTTON_BORDER_ACTIVE = RGB565(100, 150, 255); // Nice blue
static const uint16_t BUTTON_BORDER_DISABLED = RGB565(100, 100, 100);
static const uint16_t ACCENT_GREEN = RGB565(50, 200, 100);
static const uint16_t ACCENT_RED = RGB565(255, 80, 80);
static const uint16_t ACCENT_ORANGE = RGB565(255, 165, 0);
static const uint16_t VALUE_DISPLAY_BG = RGB565(30, 30, 50);

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
    int yPos = 100; // Start lower to accommodate taller header

    // Screen Button
    createButton(_mainButtons, BUTTON_MARGIN, yPos, buttonWidth, BUTTON_HEIGHT, "Screen", BUTTON_BORDER_ACTIVE, [this]() {
        _currentState = ScreenState::BRIGHTNESS;
        _needsRedraw = true;
        Utils::beepTick();
    });

    yPos += BUTTON_SPACING;
    // Alarm Button
    createButton(_mainButtons, BUTTON_MARGIN, yPos, buttonWidth, BUTTON_HEIGHT, "Alarm", BUTTON_BORDER_ACTIVE, [this]() {
        _currentState = ScreenState::ALARM;
        _needsRedraw = true;
        Utils::beepTick();
    });

    yPos += BUTTON_SPACING;
    // Info Button
    createButton(_mainButtons, BUTTON_MARGIN, yPos, buttonWidth, BUTTON_HEIGHT, "Info", BUTTON_BORDER_ACTIVE, [this]() {
        _currentState = ScreenState::INFORMATION;
        _needsRedraw = true;
        Utils::beepTick();
    });

    // Exit Button
    createButton(_mainButtons, 0, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Exit", ACCENT_RED, [this]() { exit(); });

    // Save Button
    createButton(_mainButtons, screenWidth - BACK_BUTTON_WIDTH, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Save", ACCENT_GREEN, [this]() {
        _config.checkSave();
        Utils::beepTick();
        delay(100);
        exit();
    });
}

void Settings::initInformationButtons() {
    // Back Button
    createButton(_informationButtons, _tft.width() - BACK_BUTTON_WIDTH, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Back", ACCENT_ORANGE, [this]() {
        _currentState = ScreenState::MAIN;
        _needsRedraw = true;
        Utils::beepTick();
    });
}

void Settings::initBrightnessButtons() {
    int screenWidth = _tft.width();
    int buttonWidth = screenWidth - 2 * BUTTON_MARGIN;

    // Auto Brightness Toggle Button
    createButton(_brightnessButtons, BUTTON_MARGIN, 100, buttonWidth, BUTTON_HEIGHT, "", BUTTON_BORDER_ACTIVE, [this]() {
        _config.data.tftAutoBrightnessActive = !_config.data.tftAutoBrightnessActive;
        _tftBackLightAdjuster.setAutoBrightnessActive(_config.data.tftAutoBrightnessActive);
        _needsRedraw = true;
        Utils::beepTick();
    });

    // Manual Brightness Down Button
    createButton(_brightnessButtons, BUTTON_MARGIN, 190, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "-", ACCENT_RED, [this]() {
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
    createButton(_brightnessButtons, screenWidth - BUTTON_MARGIN - CONTROL_BUTTON_WIDTH, 190, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "+", ACCENT_GREEN, [this]() {
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
    createButton(_brightnessButtons, screenWidth - BACK_BUTTON_WIDTH, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Back", ACCENT_ORANGE, [this]() {
        _currentState = ScreenState::MAIN;
        _needsRedraw = true;
        Utils::beepTick();
    });
}

void Settings::initAlarmButtons() {
    int screenWidth = _tft.width();
    int buttonWidth = screenWidth - 2 * BUTTON_MARGIN;

    // GPS Trafi Alarm Enabled Toggle Button
    createButton(_alarmButtons, BUTTON_MARGIN, 100, buttonWidth, BUTTON_HEIGHT, "", BUTTON_BORDER_ACTIVE, [this]() {
        _config.data.gpsTrafiAlarmEnabled = !_config.data.gpsTrafiAlarmEnabled;
        _needsRedraw = true;
        Utils::beepTick();
    });

    // GPS Trafi Alarm Distance Down Button
    createButton(_alarmButtons, BUTTON_MARGIN, 190, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "-", ACCENT_RED, [this]() {
        if (_config.data.gpsTrafiAlarmEnabled) {
            uint16_t &val = _config.data.gpsTrafiAlarmDistance;
            val = (val > 400) ? val - 100 : 400;
            updateAlarmValueDisplay();
            Utils::beepTick();
        }
    });

    // GPS Trafi Alarm Distance Up Button
    createButton(_alarmButtons, screenWidth - BUTTON_MARGIN - CONTROL_BUTTON_WIDTH, 190, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "+", ACCENT_GREEN, [this]() {
        if (_config.data.gpsTrafiAlarmEnabled) {
            uint16_t &val = _config.data.gpsTrafiAlarmDistance;
            val = (val < 2000) ? val + 100 : 2000;
            updateAlarmValueDisplay();
            Utils::beepTick();
        }
    });

    // Back Button
    createButton(_alarmButtons, screenWidth - BACK_BUTTON_WIDTH, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Back", ACCENT_ORANGE, [this]() {
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
    _tft.fillScreen(RGB565(25, 25, 35)); // Dark modern background
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
 * Képernyő címének megjelenítése modern dizájnnal
 */
void Settings::drawScreenTitle(const char *title) {
    // Modern gradient-like header background
    _tft.fillRect(0, 0, _tft.width(), TITLE_BAR_HEIGHT, HEADER_BG_COLOR);

    // Add a subtle border at the bottom
    _tft.fillRect(0, TITLE_BAR_HEIGHT - 3, _tft.width(), 3, BUTTON_BORDER_ACTIVE);

    // Shadow effect for text
    _tft.setTextFont(4);
    _tft.setTextSize(2);
    _tft.setTextDatum(MC_DATUM);

    // Draw shadow
    _tft.setTextColor(RGB565(20, 20, 40)); // Dark shadow
    _tft.drawString(title, _tft.width() / 2 + TITLE_SHADOW_OFFSET, TITLE_BAR_HEIGHT / 2 + TITLE_SHADOW_OFFSET);

    // Draw main text
    _tft.setTextColor(HEADER_TEXT_COLOR);
    _tft.drawString(title, _tft.width() / 2, TITLE_BAR_HEIGHT / 2);

    // Add decorative corner elements
    _tft.fillCircle(20, 20, 8, BUTTON_BORDER_ACTIVE);
    _tft.fillCircle(_tft.width() - 20, 20, 8, BUTTON_BORDER_ACTIVE);

    _tft.setTextSize(1); // Restore default
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
    uint16_t disabledColor = _config.data.tftAutoBrightnessActive ? BUTTON_BORDER_DISABLED : ACCENT_RED;
    uint16_t enabledColor = _config.data.tftAutoBrightnessActive ? BUTTON_BORDER_DISABLED : ACCENT_GREEN;

    _brightnessButtons[1].setBorderColor(disabledColor);
    _brightnessButtons[2].setBorderColor(enabledColor);

    // Draw all buttons
    for (auto &button : _brightnessButtons) {
        button.draw();
    }

    // Draw value display background with rounded corners
    int centerX = _tft.width() / 2;
    int centerY = 220;
    int boxWidth = 100;
    int boxHeight = 50;

    _tft.fillRoundRect(centerX - boxWidth / 2, centerY - boxHeight / 2, boxWidth, boxHeight, 10, VALUE_DISPLAY_BG);
    _tft.drawRoundRect(centerX - boxWidth / 2, centerY - boxHeight / 2, boxWidth, boxHeight, 10, BUTTON_BORDER_ACTIVE);

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
    uint16_t disabledColor = !_config.data.gpsTrafiAlarmEnabled ? BUTTON_BORDER_DISABLED : ACCENT_RED;
    uint16_t enabledColor = !_config.data.gpsTrafiAlarmEnabled ? BUTTON_BORDER_DISABLED : ACCENT_GREEN;

    _alarmButtons[1].setBorderColor(disabledColor);
    _alarmButtons[2].setBorderColor(enabledColor);

    // Draw all buttons
    for (auto &button : _alarmButtons) {
        button.draw();
    }

    // Draw value display background with rounded corners
    int centerX = _tft.width() / 2;
    int centerY = 220;
    int boxWidth = 120;
    int boxHeight = 50;

    _tft.fillRoundRect(centerX - boxWidth / 2, centerY - boxHeight / 2, boxWidth, boxHeight, 10, VALUE_DISPLAY_BG);
    _tft.drawRoundRect(centerX - boxWidth / 2, centerY - boxHeight / 2, boxWidth, boxHeight, 10, BUTTON_BORDER_ACTIVE);

    // Display alarm distance value
    updateAlarmValueDisplay();

    // Add unit text
    if (_config.data.gpsTrafiAlarmEnabled) {
        _tft.setTextFont(2);
        _tft.setTextColor(TFT_LIGHTGREY);
        _tft.setTextDatum(MC_DATUM);
        _tft.drawString("meters", centerX, centerY + 35);
    }
}

/**
 * Információs képernyő megjelenítése
 */
void Settings::drawInformationScreen() {
    // Képernyő címének megjelenítése
    drawScreenTitle("Information");

    char buf[64];
    int yPos = 120;
    int lineSpacing = 45;

    // Create info card background
    int cardX = 30;
    int cardY = 110;
    int cardWidth = _tft.width() - 60;
    int cardHeight = 160;

    _tft.fillRoundRect(cardX, cardY, cardWidth, cardHeight, 15, VALUE_DISPLAY_BG);
    _tft.drawRoundRect(cardX, cardY, cardWidth, cardHeight, 15, BUTTON_BORDER_ACTIVE);

    _tft.setTextDatum(TL_DATUM);
    _tft.setTextFont(2);

    // Program Version with icon-like bullet
    _tft.fillCircle(50, yPos + 10, 5, ACCENT_GREEN);
    _tft.setTextColor(TFT_WHITE);
    sprintf(buf, "Version: V%s", APP_VERSION);
    _tft.drawString(buf, 70, yPos);

    yPos += lineSpacing;
    // Build Time
    _tft.fillCircle(50, yPos + 10, 5, BUTTON_BORDER_ACTIVE);
    _tft.setTextColor(TFT_LIGHTGREY);
    sprintf(buf, "Build: %s %s", __DATE__, __TIME__);
    _tft.drawString(buf, 70, yPos);

    yPos += lineSpacing;
    // Trafipax Count
    _tft.fillCircle(50, yPos + 10, 5, ACCENT_ORANGE);
    _tft.setTextColor(TFT_YELLOW);
    sprintf(buf, "Trafipax Count: %d", _trafipaxManager.count());
    _tft.drawString(buf, 70, yPos);

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
    buttonList.emplace_back(_tft, x, y, w, h, label, BUTTON_BG_COLOR, BUTTON_TEXT_COLOR, borderColor, 4);
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
    // Display new value without clearing background (already drawn)
    _tft.setTextFont(4);
    _tft.setTextSize(2);
    _tft.setTextDatum(MC_DATUM);

    int centerX = _tft.width() / 2;
    int centerY = 220;

    if (isEnabled) {
        _tft.setTextColor(TFT_WHITE);
        _tft.drawString(String(value), centerX, centerY);
    } else {
        _tft.setTextColor(BUTTON_BORDER_DISABLED);
        _tft.drawString(disabledText, centerX, centerY);
    }

    _tft.setTextFont(1); // Restore default font
    _tft.setTextSize(1); // Restore default text size
}
