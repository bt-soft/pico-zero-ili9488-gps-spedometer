#include "Utils.h"
#include "commons.h"
#include "pins.h"

namespace Utils {
/**
 * @brief  Formáz egy lebegőpontos számot stringgé, a tizedesjegyek számát paraméterként adva meg.
 * @param value A lebegőpontos szám értéke
 * @param decimalPlaces A tizedesjegyek száma (alapértelmezett: 2)
 */
String floatToString(float value, int decimalPlaces) {
    String result = String(value, decimalPlaces);
    return result;
}

/**
 * Várakozás a soros port megnyitására
 * @param tft a TFT kijelző példánya
 */
void debugWaitForSerial(TFT_eSPI &tft) {
#ifdef __DEBUG
    beepError();
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Nyisd meg a soros portot!", 0, 0);
    while (!Serial) {
    }
    tft.fillScreen(TFT_BLACK);
    beepTick();
#endif
}

/**
 * TFT érintőképernyő kalibráció
 * @param tft TFT kijelző példánya
 * @param calData kalibrációs adatok
 */
void tftTouchCalibrate(TFT_eSPI &tft, uint16_t (&calData)[5]) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextFont(2);
    tft.setTextSize(2);
    const char *txt = "TFT touch kalibrácio kell!\n";
    tft.setCursor((tft.width() - tft.textWidth(txt)) / 2, tft.height() / 2 - 60);
    tft.setTextColor(TFT_ORANGE, TFT_BLACK);
    tft.println(txt);

    tft.setTextSize(1);
    txt = "Erintsd meg a jelzett helyeken a sarkokat!\n";
    tft.setCursor((tft.width() - tft.textWidth(txt)) / 2, tft.height() / 2 + 20);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.println(txt);

    // TFT_eSPI 'bóti' kalibráció indítása
    tft.calibrateTouch(calData, TFT_YELLOW, TFT_BLACK, 15);

    txt = "Kalibracio befejezodott!";
    tft.fillScreen(TFT_BLACK);
    tft.setCursor((tft.width() - tft.textWidth(txt)) / 2, tft.height() / 2);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(1);
    tft.println(txt);

    DEBUG("// Használd ezt a kalibrációs kódot a setup()-ban:\n");
    DEBUG("  uint16_t calData[5] = { ");
    for (uint8_t i = 0; i < 5; i++) {
        DEBUG("%d", calData[i]);
        if (i < 4) {
            DEBUG(", ");
        }
    }
    DEBUG(" };\n");
    DEBUG("  tft.setTouch(calData);\n");
}

/**
 *  Pitty hangjelzés
 */
void beepTick() {
    // Csak akkor csipogunk, ha a beeper engedélyezve van
    // if (!config.data.beeperEnabled)
    //    return;
    tone(PIN_BUZZER, 800);
    delay(10);
    noTone(PIN_BUZZER);
}

/**
 * Hiba jelzés
 */
void beepError() {
    // Csak akkor csipogunk, ha a beeper engedélyezve van
    // if (!config.data.beeperEnabled)
    //    return;
    tone(PIN_BUZZER, 500);
    delay(100);
    tone(PIN_BUZZER, 500);
    delay(100);
    tone(PIN_BUZZER, 500);
    delay(100);
    noTone(PIN_BUZZER);
}

/**
 * Trafipax riasztó hangjelzés
 */
void beepAlert() {
    // Csak akkor csipogunk, ha a beeper engedélyezve van
    // if (!config.data.beeperEnabled)
    //    return;
    for (int i = 0; i < 3; i++) {
        tone(PIN_BUZZER, 1200);
        delay(60);
        noTone(PIN_BUZZER);
        delay(40);
    }
    delay(100);
    for (int i = 0; i < 2; i++) {
        tone(PIN_BUZZER, 1800);
        delay(80);
        noTone(PIN_BUZZER);
        delay(40);
    }
}

/**
 * Sziréna hangjelzés (felfutó-lefutó)
 */
void beepSiren(int cycles, int minFreq, int maxFreq, int step, int toneMs, int pauseMs) {
    for (int c = 0; c < cycles; c++) {
        // Felfutó
        for (int f = minFreq; f <= maxFreq; f += step) {
            tone(PIN_BUZZER, f);
            delay(toneMs);
        }
        // Lefutó
        for (int f = maxFreq; f >= minFreq; f -= step) {
            tone(PIN_BUZZER, f);
            delay(toneMs);
        }
        noTone(PIN_BUZZER);
        delay(pauseMs);
    }
}

}; // namespace Utils