#include "Utils.h"
#include "defines.h"
#include "pins.h"

namespace Utils {

// Nem-blokkoló sziréna állapotváltozói
namespace {
bool sirenActive = false;
int siren_cycles;
int siren_minFreq;
int siren_maxFreq;
int siren_step;
int siren_toneMs;
int siren_pauseMs;

unsigned long lastSirenActionTime = 0;
int currentFreq;
bool sweepingUp = true;
int cyclesDone = 0;
bool isPaused = false;
} // namespace

/**
 * @brief Átalakít egy milliszekundum értéket "perc:mp:ms" formátumú szöveggé
 * @param msec Időérték milliszekundumban
 * @return Formázott string (pl. "2:05:123")
 */
String msecToString(uint32_t msec) {
    uint32_t minutes = msec / 60000;
    uint32_t seconds = (msec % 60000) / 1000;
    uint32_t millis = msec % 1000;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02u mins, %02u sec, %03u msec", minutes, seconds, millis);
    return String(buf);
}

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
    const char *txt = "TFT touch kalibracio kell!\n";
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
 * ISO-8859-2 ékezetes karakterek cseréje
 */
void removeAccents(char *text) {

    int len = strlen(text);

    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)text[i];
        switch (c) {
            case 0xE1:
                text[i] = 'a';
                break; // á
            case 0xE9:
                text[i] = 'e';
                break; // é
            case 0xED:
                text[i] = 'i';
                break; // í
            case 0xF3:
                text[i] = 'o';
                break; // ó
            case 0xF6:
                text[i] = 'o';
                break; // ö
            case 0xF5:
                text[i] = 'o';
                break; // ő
            case 0xFA:
                text[i] = 'u';
                break; // ú
            case 0xFC:
                text[i] = 'u';
                break; // ü
            case 0xFB:
                text[i] = 'u';
                break; // ű
            case 0xC1:
                text[i] = 'A';
                break; // Á
            case 0xC9:
                text[i] = 'E';
                break; // É
            case 0xCD:
                text[i] = 'I';
                break; // Í
            case 0xD3:
                text[i] = 'O';
                break; // Ó
            case 0xD6:
                text[i] = 'O';
                break; // Ö
            case 0xD5:
                text[i] = 'O';
                break; // Ő
            case 0xDA:
                text[i] = 'U';
                break; // Ú
            case 0xDC:
                text[i] = 'U';
                break; // Ü
            case 0xDB:
                text[i] = 'U';
                break; // Ű
        }
    }
}

/**
 *  Pitty hangjelzés
 */
void beepTick() {
    tone(PIN_BUZZER, 800);
    delay(10);
    noTone(PIN_BUZZER);
}

/**
 * Hiba jelzés
 */
void beepError() {
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
 * Sziréna hangjelzés (felfutó-lefutó) - BLOKKOLÓ
 */
void beepSiren(int cycles, int minFreq, int maxFreq, int step, int toneMs, int pauseMs) {
    for (int c = 0; c < cycles; c++) {
        for (int f = minFreq; f <= maxFreq; f += step) {
            tone(PIN_BUZZER, f);
            delay(toneMs);
        }
        for (int f = maxFreq; f >= minFreq; f -= step) {
            tone(PIN_BUZZER, f);
            delay(toneMs);
        }
        noTone(PIN_BUZZER);
        delay(pauseMs);
    }
}

/**
 * Sziréna hangjelzés indítása - Nem blokkoló
 */
void startNonBlockingSiren(int cycles, int minFreq, int maxFreq, int step, int toneMs, int pauseMs) {
    if (sirenActive)
        return;
    siren_cycles = cycles;
    siren_minFreq = minFreq;
    siren_maxFreq = maxFreq;
    siren_step = step;
    siren_toneMs = toneMs;
    siren_pauseMs = pauseMs;
    sirenActive = true;
    isPaused = false;
    cyclesDone = 0;
    currentFreq = siren_minFreq;
    sweepingUp = true;
    lastSirenActionTime = millis();
    tone(PIN_BUZZER, currentFreq);
}

/**
 * Sziréna hangjelzés leállítása - Nem blokkoló
 */
void stopNonBlockingSiren() {
    sirenActive = false;
    noTone(PIN_BUZZER);
}

/**
 * Sziréna hangjelzés kezelése  - Nem blokkoló
 */
void handleNonBlockingSiren() {
    if (!sirenActive) {
        return;
    }

    unsigned long currentTime = millis();

    if (isPaused) {
        if (currentTime - lastSirenActionTime >= (unsigned long)siren_pauseMs) {
            isPaused = false;
            cyclesDone++;
            if (cyclesDone >= siren_cycles) {
                stopNonBlockingSiren();
                return;
            }
            sweepingUp = true;
            currentFreq = siren_minFreq;
            tone(PIN_BUZZER, currentFreq);
            lastSirenActionTime = currentTime;
        }
        return;
    }

    if (currentTime - lastSirenActionTime >= (unsigned long)siren_toneMs) {
        lastSirenActionTime = currentTime;

        if (sweepingUp) {
            currentFreq += siren_step;
            if (currentFreq > siren_maxFreq) {
                currentFreq = siren_maxFreq;
                sweepingUp = false;
            }
        } else {
            currentFreq -= siren_step;
            if (currentFreq < siren_minFreq) {
                currentFreq = siren_minFreq;
                noTone(PIN_BUZZER);
                isPaused = true;
                return;
            }
        }
        tone(PIN_BUZZER, currentFreq);
    }
}

/**
 * @brief CRC16 számítás (CCITT algoritmus)
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
