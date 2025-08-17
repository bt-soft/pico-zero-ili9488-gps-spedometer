#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include <stdint.h> // uint8_t, uint16_t, stb.

// Konfig struktúra típusdefiníció
struct Config_t {

    //--- TFT
    uint16_t tftCalibrateData[5];     // TFT touch kalibrációs adatok
    bool tftAutoBrightnessActive;     // TFT automatikus fényerő
    uint8_t tftManualBrightnessValue; // TFT Háttérvilágítás értéke kézi fényerő szabályozás esetén

    //--- System
    bool beeperEnabled; // Hangjelzés engedélyezése

    bool gpsTrafiAlarmEnabled; // GPS traffipax riasztás engedélyezése
};

#endif // CONFIG_DATA_H
