#pragma once

#include <TFT_eSPI.h> // Hardware-specific library

//--- Utils ---
namespace Utils {
/**
 * @brief Átalakít egy milliszekundum értéket "perc:mp:ms" formátumú szöveggé
 * @param msec Időérték milliszekundumban
 * @return Formázott string (pl. "2:05:123")
 */
String msecToString(uint32_t msec);

/**
 * @brief  Formáz egy lebegőpontos számot stringgé, a tizedesjegyek számát paraméterként adva meg.
 * @param value A lebegőpontos szám értéke
 * @param decimalPlaces A tizedesjegyek száma (alapértelmezett: 2)
 */
String floatToString(float value, int decimalPlaces = 2);

/**
 * Várakozás a soros port megnyitására DEBUG esetén
 */
void debugWaitForSerial(TFT_eSPI &tft);

//--- TFT ---
void tftTouchCalibrate(TFT_eSPI &tft, uint16_t (&calData)[5]);

/**
 * Ékezetes karakterek ASCII karakterekre konvertálása
 * @param text A konvertálandó szöveg (in-place módosítás)
 */
void convertToASCII(char *text);

//--- Beep ----
/**
 *  Pitty hangjelzés
 */
void beepTick();
void beepError();

/**
 * Trafipax riasztó hangjelzés
 */
void beepAlert();

/**
 * Sziréna hangjelzés (felfutó-lefutó)
 * @param cycles hány sziréna ciklus
 * @param minFreq kezdő frekvencia (Hz)
 * @param maxFreq vég frekvencia (Hz)
 * @param step lépésköz (Hz)
 * @param toneMs egy lépés hossza (ms)
 * @param pauseMs ciklusok közti szünet (ms)
 */
void beepSiren(int cycles, int minFreq, int maxFreq, int step, int toneMs, int pauseMs);

/**
 * @brief CRC16 számítás (CCITT algoritmus)
 * Használhatnánk a CRC könyvtárat is, de itt saját implementációt adunk
 *
 * @param data Adat pointer
 * @param length Adat hossza bájtokban
 * @return Számított CRC16 érték
 */
uint16_t calcCRC16(const uint8_t *data, size_t length);

/**
 * Tömb elemei nullák?
 */
template <typename T, size_t N> bool isZeroArray(T (&arr)[N]) {
    for (size_t i = 0; i < N; ++i) {
        if (arr[i] != 0) {
            return false; // Ha bármelyik elem nem nulla, akkor false-t adunk vissza
        }
    }
    return true; // Ha minden elem nulla, akkor true-t adunk vissza
}

/**
 * Eltelt már annyi idő?
 */
inline bool timeHasPassed(long fromWhen, int howLong) { return millis() - fromWhen >= howLong; }

}; // namespace Utils
