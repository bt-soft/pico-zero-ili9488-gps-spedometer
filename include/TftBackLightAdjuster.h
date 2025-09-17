#pragma once

#include <Arduino.h>
#include <memory>

#include "Config.h"
#include "defines.h"
#include "pins.h"

#define DAILY_BRIGHTNESS TFT_BACKGROUND_LED_MAX_BRIGHTNESS // A nappali fény esetén a fényerő
#define NIGHTLY_BRIGHTNESS 5                               // Az éjszakai fény esetén a fényerő
#define DEFAULT_BRIGHTNESS DAILY_BRIGHTNESS                // Alapértelmezett fényerő

#define SENSOR_VALUE_NIGHT 350  // Éjszakai fény érzékelő érték
#define SENSOR_VALUE_DAILY 1000 // Nappali fény érzékelő érték

#define LED_ADJUST_MSEC 20    // LED fényerő állítási idő
#define SENSOR_CHECK_MSEC 200 // Fényérzékelő ellenőrzési idő

/**
 * TFT háttérvilágítás állító osztály.
 */
class TftBackLightAdjuster {

  private:
    uint8_t _backlightLevel;
    uint32_t lastSensorCheckMsec;
    uint32_t lastAdjustMsec;
    byte brightness;
    byte new_brightness;
    bool _tftAutoBrightnessActive;
    uint8_t _manualBrightnessValue;

    // Config callback token az automatikus leiratkozáshoz
    std::unique_ptr<ConfigCallbackToken> configCallbackToken;

  public:
    /**
     * Konstruktor.
     */
    TftBackLightAdjuster() : lastSensorCheckMsec(0), lastAdjustMsec(0), brightness(DEFAULT_BRIGHTNESS), new_brightness(0) {}

    /**
     * Inicializálja a háttérvilágítást.
     */
    void begin();

    /**
     * @brief Callback függvény, amit a Config hív meg változás esetén
     */
    void onConfigChanged();

    /**
     * Beállítja a háttérvilágítás szintjét.
     */
    void setBacklightLevel(uint8_t level) {
        _backlightLevel = constrain(level, NIGHTLY_BRIGHTNESS, TFT_BACKGROUND_LED_MAX_BRIGHTNESS);
        analogWrite(PIN_TFT_BACKGROUND_LED, _backlightLevel);
        brightness = _backlightLevel; // Az aktuális fényerő is erre álljon
    }

    /**
     * Visszaadja a háttérvilágítás szintjét.
     */
    uint8_t getBacklightLevel() const { return _backlightLevel; }

    /**
     * Fényérzékelő értékének lekérdezése.
     */
    uint16_t getSensorValue() const { return analogRead(PIN_LIGHT_SENSOR); }

    /**
     * Állítgatja a háttérvilágítást a környezeti fényviszonyoknak megfelelően.
     */
    void loop(void);
};