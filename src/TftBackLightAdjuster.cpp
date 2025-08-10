//#include <Streaming.h>

#include "TftBackLightAdjuster.h"

/**
 * LED háttérvilágítás PWM állítgatás
 */
void TftBackLightAdjuster::adjust(void) {

    if (new_brightness != brightness) {
        // 15 msec-enként állítgatjuk a fényerőt
        if (millis() - lastAdjustMsec > LED_ADJUST_MSEC) {
            lastAdjustMsec = millis();

            new_brightness > brightness ? ++brightness : --brightness;

            if (brightness == 255) {
                // Maximum fényerőnél: tiszta DC (digitalWrite HIGH)
                digitalWrite(PIN_TFT_BACKGROUND_LED, HIGH);
            } else if (brightness == 0) {
                // Minimumnál: tiszta DC (digitalWrite LOW)
                digitalWrite(PIN_TFT_BACKGROUND_LED, LOW);
            } else {
                // Köztes értékeknél: PWM használata
                analogWrite(PIN_TFT_BACKGROUND_LED, brightness);
            }
        }
    }

    // 200msec-enként mérünk rá a szenzorra
    if (millis() - lastSensorCheckMsec > SENSOR_CHECK_MSEC) {
        lastSensorCheckMsec = millis();

        // szenzor érték leolvasása
        int lightSensorValue = analogRead(PIN_LIGHT_SENSOR);

        // Sötétben legyen sötétebb, világosban világosabb
        if (lightSensorValue < SENSOR_VALUE_NIGHT) {
            new_brightness = NIGHTLY_BRIGHTNESS; // sötét legyen sötétben

        } else if (lightSensorValue > SENSOR_VALUE_DAILY) {
            new_brightness = DAILY_BRIGHTNESS; // világos legyen világosban

        } else {
            // A beállítandó érték kimatekozása (fordított mapping)
            new_brightness = map(lightSensorValue, SENSOR_VALUE_NIGHT, SENSOR_VALUE_DAILY, DAILY_BRIGHTNESS, NIGHTLY_BRIGHTNESS);
        }

        //Serial << "lightSensorValue: " << lightSensorValue << ", brightness: " << brightness << ", new_brightness: " << new_brightness << endl;
    }
}
