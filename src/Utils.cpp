#include "Utils.h"
#include "commons.h"
#include "pins.h"

namespace Utils {
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

}; // namespace Utils