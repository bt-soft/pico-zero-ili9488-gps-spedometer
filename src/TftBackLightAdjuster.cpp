#include "TftBackLightAdjuster.h"
#include "Utils.h"

/**
 * LED háttérvilágítás PWM állítgatás
 */
void TftBackLightAdjuster::loop(void) {

    // SENSOR_CHECK_MSEC msec-enként frissítjük a célértéket
    if (Utils::timeHasPassed(lastSensorCheckMsec, SENSOR_CHECK_MSEC)) {
        lastSensorCheckMsec = millis();

        if (_tftAutoBrightnessActive) {
            // Automata mód: szenzor alapján állítunk
            int lightSensorValue = analogRead(PIN_LIGHT_SENSOR);

            if (lightSensorValue < SENSOR_VALUE_NIGHT) {
                new_brightness = NIGHTLY_BRIGHTNESS;
            } else if (lightSensorValue > SENSOR_VALUE_DAILY) {
                new_brightness = DAILY_BRIGHTNESS;
            } else {
                new_brightness = map(lightSensorValue, SENSOR_VALUE_NIGHT, SENSOR_VALUE_DAILY, DAILY_BRIGHTNESS, NIGHTLY_BRIGHTNESS);
            }

        } else {
            // Manuális mód: a beállított értéket használjuk
            new_brightness = _manualBrightnessValue;
        }
    }

    // Fényerő állítása, ha szükséges
    if (new_brightness != brightness) {
        // 15 msec-enként léptetünk a fényerőn
        if (millis() - lastAdjustMsec > LED_ADJUST_MSEC) {
            lastAdjustMsec = millis();

            if (new_brightness > brightness) {
                brightness++;
            } else {
                brightness--;
            }

            DEBUG("Backlight adjust to %d (target: %d)\n", brightness, new_brightness);

            // A setBacklightLevel már kezeli a 0 és 255 speciális esetet, de PWM-mel.
            // A direkt digitalWrite hatékonyabb lehet, de a sima analogWrite is megteszi.
            setBacklightLevel(brightness);
        }
    }
}