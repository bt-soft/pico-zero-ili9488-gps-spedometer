#pragma once

//---- Program Information ------------------------------------------

#define PROGRAM_NAME "Pico GPS"
#define PROGRAM_DESC "RP2040 Zero GPS SpeedoMeter"
#define PROGRAM_VERSION "V0.0.3"
#define PROGRAM_AUTHOR "BT-Soft"

//--- ScreenNames ----
#define SCREEN_NAME_MAIN "Main"
#define SCREEN_NAME_INFO "Info"
#define SCREEN_NAME_SETUP "Setup"
#define SCREEN_NAME_SATS "Satellites"
#define SCREEN_NAME_TFT_SETUP "ScreenTFTSetup"
#define SCREEN_NAME_SYSTEM_SETUP "ScreenSystemSetup"
#define SCREEN_NAME_GPS_SETUP "ScreenGPSSetup"
#define SCREEN_NAME_DEBUG_SETUP "ScreenDebugSetup"
#define SCREEN_NAME_SCREENSAVER "ScreenScreenSaver"

//---
#define SCREEN_NAME_EMPTY "Empty"
#define SCREEN_NAME_TEST "Test"

#define __DEBUG
//  #define __DEBUG_WAIT_FOR_SERIAL

// Színsémák
#define RED2RED 0
#define GREEN2GREEN 1
#define BLUE2BLUE 2
#define BLUE2RED 3
#define GREEN2RED 4
#define RED2GREEN 5
#define RED2VIOLET 6

/**
 * Az ILI9488 esetén az RGB színek tipikusan 565 formátumban vannak tárolva:
 *      5 bit a vörös
 *      6 bit a zöld
 *      5 bit a kék
 */
#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

// #define TFT_GREY 0x2104 // Dark grey 16-bit colour
#define TFT_GREY RGB565(50, 50, 50)
#define TFT_BORDER_COLOR RGB565(70, 70, 70)
#define RINGMETER_LIGHTBLUE RGB565(73, 73, 255);

// Akkumulátor
#define SATS_RINGETER_MIN 0
#define SATS_RINGETER_MAX 15

// Betáp feszültség
#define VBUS_BARMETER_MIN 3.0f
#define VBUS_BARMETER_MAX 15.0f
// 18650  lítium akkumulátor feszültségei
#define VSYS_BARMETER_MIN 2.7f
#define VSYS_BARMETER_MAX 5.5f

// Hőmérséklet
#define TEMP_BARMETER_MIN -20.0f
#define TEMP_BARMETER_MAX +70.0f

#define GPS_DATA_MAX_AGE 5000

// Minimum ennyi ideig látszik a traffipax alarm
#define ALARM_TIME_MS 10000

// TFT
#define TFT_BACKGROUND_LED_MAX_BRIGHTNESS 255

//--- C String compare -----
#define STREQ(a, b) (strcmp((a), (b)) == 0)

//--- Array Utils ---
#define ARRAY_ITEM_COUNT(array) (sizeof(array) / sizeof(array[0]))

//--- Debug ---
#ifdef __DEBUG
#define DEBUG(fmt, ...) Serial.printf(fmt __VA_OPT__(, ) __VA_ARGS__)
#else
#define DEBUG(fmt, ...) // Üres makró, ha __DEBUG nincs definiálva
#endif
