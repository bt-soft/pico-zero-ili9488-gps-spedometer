#include "Config.h"

/**
 * Alapértelmezett readonly konfigurációs adatok
 */
const Config_t DEFAULT_CONFIG = {

    //--- TFT
    .tftCalibrateData = {0, 0, 0, 0, 0}, // TFT touch kalibrációs adatok
    //.tftCalibrateData = {214, 3721, 239, 3606, 7},
    .tftAutoBrightnessActive = true,                               // TFT automatikus fényerő engedélyezése
    .tftManualBrightnessValue = TFT_BACKGROUND_LED_MAX_BRIGHTNESS, // TFT Háttérvilágítás értéke kézi fényerő szabályozás esetén

    //--- System
    .beeperEnabled = true, // Hangjelzés engedélyezése    // MiniAudioFft módok

    .gpsTrafiAlarmEnabled = true, // GPS traffipax riasztás engedélyezése

};

// Globális konfiguráció példány
Config config;
