#ifndef __TFT_BACKLIGHT_ADJUSTER_H
#define __TFT_BACKLIGHT_ADJUSTER_H

#include "pins.h"
#include <Arduino.h>

#define MAX_BRIGHTNESS 255
#define DAILY_BRIGHTNESS MAX_BRIGHTNESS   // A nappali fény esetén a fényerő
#define NIGHTLY_BRIGHTNESS 5             // Az éjszakai fény esetén a fényerő
#define DEFAULT_BRIGHTNESS MAX_BRIGHTNESS // Alapértelmezett fényerő

#define SENSOR_VALUE_NIGHT 350  // Éjszakai fény érzékelő érték
#define SENSOR_VALUE_DAILY 1000 // Nappali fény érzékelő érték

#define LED_ADJUST_MSEC 20    // LED fényerő állítási idő
#define SENSOR_CHECK_MSEC 200 // Fényérzékelő ellenőrzési idő

class TftBackLightAdjuster {

  private:
    uint8_t _backlightLevel;
    uint32_t lastSensorCheckMsec;
    uint32_t lastAdjustMsec;
    byte brightness;
    byte new_brightness;

  public:
    TftBackLightAdjuster() : _backlightLevel(255), lastSensorCheckMsec(0), lastAdjustMsec(0), brightness(DEFAULT_BRIGHTNESS), new_brightness(0) {}

    void begin() {
        pinMode(PIN_TFT_BACKGROUND_LED, OUTPUT);
        setBacklightLevel(MAX_BRIGHTNESS); // Default to max brightness
    }

    void setBacklightLevel(uint8_t level) {
        _backlightLevel = constrain(level, 0, MAX_BRIGHTNESS);
        analogWrite(PIN_TFT_BACKGROUND_LED, _backlightLevel);
    }

    uint8_t getBacklightLevel() const { return _backlightLevel; }

    void adjust(void);
};

#endif // __TFT_BACKLIGHT_ADJUSTER_H