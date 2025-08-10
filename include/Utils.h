#ifndef __UTILS_H__
#define __UTILS_H__

#include <TFT_eSPI.h> // Hardware-specific library

//--- Utils ---
namespace Utils {

//--- TFT ---
void tftTouchCalibrate(TFT_eSPI &tft, uint16_t (&calData)[5]);

//--- Beep ----
/**
 *  Pitty hangjelzés
 */
void beepTick();
void beepError();

}; // namespace Utils

#endif // __UTILS_H__
