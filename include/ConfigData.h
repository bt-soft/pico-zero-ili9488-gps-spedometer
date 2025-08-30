#pragma once

#include <stdint.h> // uint8_t, uint16_t, stb.

// Konfig struktúra típusdefiníció
struct Config_t {

    //--- TFT
    uint16_t tftCalibrateData[5];     // TFT touch kalibrációs adatok
    bool tftAutoBrightnessActive;     // TFT automatikus fényerő
    uint8_t tftManualBrightnessValue; // TFT Háttérvilágítás értéke kézi fényerő szabályozás esetén

    //--- System
    bool beeperEnabled; // Hangjelzés engedélyezése

    // GPS traffipax riasztási beállítások
    uint16_t gpsTraffiAlarmDistance; // GPS traffipax riasztási távolság méterben
    bool gpsTraffiAlarmEnabled;      // GPS traffipax riasztás engedélyezése
    bool gpsTraffiSirenAlarmEnabled; // GPS traffipax sziréna riasztás engedélyezése

    //--- Debug
    bool debugGpsSerialOnInternalFastLed; //  Az RGB LED villogtatása, ha van GPS soros adat
    bool debugGpsSerialData;              //  GPS adatok kiírása a Soros portra
    bool debugGpsSatellitesDatabase;      //  GPS műhold adatbázis debug logolása
};
