#include "Utils.h"
#include "defines.h"
#include "pins.h"

namespace Utils {
/**
 * @brief  Formáz egy lebegőpontos számot stringgé, a tizedesjegyek számát paraméterként adva meg.
 * @param value A lebegőpontos szám értéke
 * @param decimalPlaces A tizedesjegyek száma (alapértelmezett: 2)
 */
String floatToString(float value, int decimalPlaces = 2) {
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
 * Ékezetes karakterek ASCII karakterekre konvertálása
 */
void convertToASCII(char *text) {
    int len = strlen(text);
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)text[i];
        switch (c) {
            case 0xC3: // UTF-8 prefix for accented characters
                if (i + 1 < len) {
                    unsigned char next = (unsigned char)text[i + 1];
                    char replacement = '?';
                    switch (next) {
                        case 0xA1:
                            replacement = 'a';
                            break; // á
                        case 0xA9:
                            replacement = 'e';
                            break; // é
                        case 0xAD:
                            replacement = 'i';
                            break; // í
                        case 0xB3:
                            replacement = 'o';
                            break; // ó
                        case 0xB6:
                            replacement = 'o';
                            break; // ö
                        case 0xB5:
                            replacement = 'o';
                            break; // õ
                        case 0xBA:
                            replacement = 'u';
                            break; // ú
                        case 0xBC:
                            replacement = 'u';
                            break; // ü
                        case 0xBB:
                            replacement = 'u';
                            break; // û
                        case 0x81:
                            replacement = 'A';
                            break; // Á
                        case 0x89:
                            replacement = 'E';
                            break; // É
                        case 0x8D:
                            replacement = 'I';
                            break; // Í
                        case 0x93:
                            replacement = 'O';
                            break; // Ó
                        case 0x96:
                            replacement = 'O';
                            break; // Ö
                        case 0x95:
                            replacement = 'O';
                            break; // Õ
                        case 0x9A:
                            replacement = 'U';
                            break; // Ú
                        case 0x9C:
                            replacement = 'U';
                            break; // Ü
                        case 0x9B:
                            replacement = 'U';
                            break; // Û
                    }
                    // Shift left to overwrite the UTF-8 sequence
                    text[i] = replacement;
                    memmove(&text[i + 1], &text[i + 2], len - i - 1);
                    len--;
                }
                break;
        }
    }
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

/**
 * @brief CRC16 számítás (CCITT algoritmus)
 * Használhatnánk a CRC könyvtárat is, de itt saját implementációt adunk
 *
 * @param data Adat pointer
 * @param length Adat hossza bájtokban
 * @return Számított CRC16 érték
 */
uint16_t calcCRC16(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i] << 8;
        for (uint8_t j = 0; j < 8; j++) {
            crc = (crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1);
        }
    }
    return crc;
}

}; // namespace Utils