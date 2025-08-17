#ifndef SETTINGS_H
#define SETTINGS_H

#include "Button.h"
#include <TFT_eSPI.h>
#include <vector>

// Forward declare dependencies
class Config;
class TftBackLightAdjuster;

class TrafipaxManager;

class Settings {
  public:
    Settings(TFT_eSPI &tft, Config &config, TftBackLightAdjuster &tftBackLightAdjuster, TrafipaxManager &trafipaxManager);

    void init();
    void enter();
    void loop();
    bool isActive();

  private:
    enum class ScreenState { MAIN, BRIGHTNESS, ALARM, INFORMATION };

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

    void initInformationButtons();

    void drawInformationScreen();
    void handleInformationTouch(uint16_t x, uint16_t y);

    void drawScreenTitle(const char *title);

    TFT_eSPI &_tft;
    Config &_config;
    bool _active;
    ScreenState _currentState;
    TftBackLightAdjuster &_tftBackLightAdjuster;
    TrafipaxManager &_trafipaxManager;

    std::vector<Button> _mainButtons;
    std::vector<Button> _brightnessButtons;
    std::vector<Button> _alarmButtons;
    std::vector<Button> _informationButtons;
};

#endif // SETTINGS_H
