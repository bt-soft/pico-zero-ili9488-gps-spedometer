#ifndef SETTINGS_H
#define SETTINGS_H

#include <TFT_eSPI.h>
#include <vector>
#include "Button.h"

// Forward declare dependencies
class Config;
class TftBackLightAdjuster;

class Settings {
public:
    Settings(TFT_eSPI& tft, Config& config, TftBackLightAdjuster& tftBackLightAdjuster);

    void init();
    void enter();
    void loop();
    bool isActive();

private:
    enum class ScreenState { MAIN, BRIGHTNESS };

    void draw();
    void handleTouch();
    void exit();

    // Screen-specific methods
    void initBrightnessButtons();
    void drawMainScreen();
    void handleMainTouch(uint16_t x, uint16_t y);
    void drawBrightnessScreen();
    void handleBrightnessTouch(uint16_t x, uint16_t y);
    void updateBrightnessValueDisplay();


    TFT_eSPI& _tft;
    Config& _config;
    bool _active;
    ScreenState _currentState;
    TftBackLightAdjuster& _tftBackLightAdjuster;
    
    std::vector<Button> _mainButtons;
    std::vector<Button> _brightnessButtons;
};

#endif // SETTINGS_H
