#ifndef SETTINGS_H
#define SETTINGS_H

#include <TFT_eSPI.h>
#include <vector>
#include "Button.h"

// Forward declare dependencies
class Config;

class Settings {
public:
    Settings(TFT_eSPI& tft, Config& config);

    void init();
    void enter();
    void loop();
    bool isActive();

private:
    void draw();
    void handleTouch();
    void exit();

    TFT_eSPI& _tft;
    Config& _config;
    bool _active;
    
    std::vector<Button> _buttons;
};

#endif // SETTINGS_H
