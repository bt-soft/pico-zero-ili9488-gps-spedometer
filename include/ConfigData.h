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

    uint16_t gpsTrafiAlarmDistance; // GPS traffipax riasztási távolság méterben
    bool gpsTrafiAlarmEnabled;      // GPS traffipax riasztás engedélyezése

    //--- Debug
    bool debugGpsSerialOnInternalFastLed; //  Az RGB LED villogtatása, ha van GPS soros adat
    bool debugGpsSerialData;              //  GPS adatok kiírása a Soros portra
};

#endif // CONFIG_DATA_H
