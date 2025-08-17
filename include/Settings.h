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
    enum class ScreenState { MAIN, BRIGHTNESS, ALARM };

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

    void initAlarmButtons();
    void drawAlarmScreen();
    void handleAlarmTouch(uint16_t x, uint16_t y);
    void updateAlarmValueDisplay();


    TFT_eSPI& _tft;
    Config& _config;
    bool _active;
    ScreenState _currentState;
    TftBackLightAdjuster& _tftBackLightAdjuster;
    
    std::vector<Button> _mainButtons;
    std::vector<Button> _brightnessButtons;
    std::vector<Button> _alarmButtons;
};

#endif // SETTINGS_H
