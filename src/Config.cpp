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
    .beeperEnabled = true,   // Hangjelzés engedélyezése
    .screenSaverTimeout = 5, // Képernyőkímélő időtúllépés percben

    // GPS traffipax beállítások
    .gpsTraffiAlarmDistance = 800,      // GPS traffipax riasztási távolság méterben
    .gpsTraffiAlarmEnabled = true,      // GPS traffipax riasztás engedélyezése
    .gpsTraffiSirenAlarmEnabled = true, // GPS traffipax sziréna riasztás engedélyezése

    // GPS debug opciók
    .debugGpsSerialOnInternalFastLed = false, // GPS érvényes NMEA mondatok esetén a Zero belső LED villogtatása
    .debugGpsSerialData = false,              // GPS soros adat kiírása a soros portra
    .debugGpsSatellitesDatabase = false,

    //--- Nem állítható de perszisztens adatok ->  MainScreen
    .externalTemperatureMode = true, // Hőmérsékleti mód: true = külső hőmérséklet, false = CPU hőmérséklet, true = external, false = CPU
    .externalVoltageMode = true,     // Feszültségmérő mód: true = VBus, false = VSys, true = external, false = CPU
};

// Globális konfiguráció példány
Config config;

/**
 * @brief Feliratkoztat egy komponenst a konfiguráció változásainak figyelésére
 * @param callback A függvény, amit változáskor meg kell hívni
 */
void Config::registerChangeCallback(ConfigChangeCallback callback) {
    changeCallbacks.push_back(callback);
}

/**
 * @brief Értesíti a feliratkozott komponenseket a változásról
 */
void Config::notifyChange() {
    for (auto &callback : changeCallbacks) {
        callback();
    }
}