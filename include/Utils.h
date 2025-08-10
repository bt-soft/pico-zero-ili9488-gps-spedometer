#ifndef __UTILS_H__
#define __UTILS_H__

#include <TFT_eSPI.h> // Hardware-specific library

//--- Utils ---
namespace Utils {

/**
 * @brief  Formáz egy lebegőpontos számot stringgé, a tizedesjegyek számát paraméterként adva meg.
 * @param value A lebegőpontos szám értéke
 * @param decimalPlaces A tizedesjegyek száma (alapértelmezett: 2)
 */
String floatToString(float value, int decimalPlaces);

/**
 * Várakozás a soros port megnyitására DEBUG esetén
 */
void debugWaitForSerial(TFT_eSPI &tft);

//--- TFT ---
void tftTouchCalibrate(TFT_eSPI &tft, uint16_t (&calData)[5]);

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

}; // namespace Utils

#endif // __UTILS_H__
