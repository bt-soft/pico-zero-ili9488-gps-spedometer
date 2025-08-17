#include "Settings.h"
#include "Config.h" // Include the full header for implementation
#include "Utils.h"  // For beeping and other utilities

/**
 *  A beállítások menü inicializálása
 */
Settings::Settings(TFT_eSPI &tft, Config &config) : _tft(tft), _config(config), _active(false) {}

/**
 *  A beállítások menü inicializálása
 */
void Settings::init() {
    _buttons.reserve(4);

    // Screen Button
    // Screen Button
    _buttons.emplace_back(_tft, 50, 80, _tft.width() - 100, 50, "Screen", TFT_BLACK, TFT_WHITE, TFT_WHITE, 2);
    _buttons.back().setCallback([]() {
        // TODO: Screen settings
        Utils::beepTick();
    });

    // Alarm Button
    _buttons.emplace_back(_tft, 50, 150, _tft.width() - 100, 50, "Alarm", TFT_BLACK, TFT_WHITE, TFT_WHITE, 2);
    _buttons.back().setCallback([]() {
        // TODO: Alarm settings
        Utils::beepTick();
    });

    // Exit Button
    _buttons.emplace_back(_tft, 30, _tft.height() - 70, 150, 50, "Exit", TFT_BLACK, TFT_WHITE, TFT_RED, 2);
    _buttons.back().setCallback([this]() { exit(); });

    // Save Button
    _buttons.emplace_back(_tft, _tft.width() - 180, _tft.height() - 70, 150, 50, "Save", TFT_BLACK, TFT_WHITE, TFT_GREEN, 4);
    _buttons.back().setCallback([this]() {
        _config.checkSave();
        Utils::beepTick();
        delay(100); // Short pause for feedback
        exit();
    });
}

/**
 *  Ellenőrzi, hogy a beállítások aktívak-e
 */
bool Settings::isActive() { return _active; }

/**
 *  Belép a beállítások menübe
 */
void Settings::enter() {
    _active = true;
    draw();
    Utils::beepTick();
    delay(200); // Debounce to prevent immediate re-entry or double-tap issues
}

/**
 *  Kilép a beállítások menüből
 */
void Settings::exit() {
    _active = false;
    Utils::beepTick();
}

/**
 *  A beállítások menü kirajzolása
 */
void Settings::draw() {
    _tft.fillScreen(TFT_BLACK);
    _tft.setTextColor(TFT_WHITE, TFT_BLACK);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString("Settings", _tft.width() / 2, 30, 4);

    for (auto &button : _buttons) {
        button.draw();
    }
}

void Settings::handleTouch() {
    uint16_t x, y;
    if (_tft.getTouch(&x, &y)) {
        for (auto &button : _buttons) {
            if (button.contains(x, y)) {
                button.press();
                break; // Assume only one button can be pressed at a time
            }
        }
        delay(200); // Debounce after any touch
    }
}

void Settings::loop() {
    if (!_active) {
        return;
    }
    handleTouch();
}
