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
 * @brief ConfigCallbackToken destruktor implementáció
 */
ConfigCallbackToken::~ConfigCallbackToken() { unregister(); }

/**
 * @brief ConfigCallbackToken manuális leiratkozás
 */
void ConfigCallbackToken::unregister() {
    if (valid && config) {
        config->internalUnregister(callbackId);
        valid = false;
    }
}

/**
 * @brief Feliratkoztat egy komponenst a konfiguráció változásainak figyelésére
 * @param callback A függvény, amit változáskor meg kell hívni
 * @return RAII token, amely destruktora automatikusan leiratkozik
 */
ConfigCallbackToken Config::registerChangeCallback(ConfigChangeCallback callback) {
    size_t index;

    // Ha van szabad index, azt használjuk
    if (!freeIndices.empty()) {
        index = freeIndices.back();
        freeIndices.pop_back();
        changeCallbacks[index] = CallbackEntry(std::move(callback));
    } else {
        // Új elemet adunk hozzá
        changeCallbacks.emplace_back(std::move(callback));
        index = changeCallbacks.size() - 1;
    }

    return ConfigCallbackToken(this, index);
}

/**
 * @brief Belső leiratkozás implementáció
 */
void Config::internalUnregister(size_t callbackId) {
    if (callbackId < changeCallbacks.size() && changeCallbacks[callbackId].active) {
        changeCallbacks[callbackId].active = false;
        freeIndices.push_back(callbackId);
    }
}

/**
 * @brief Értesíti a feliratkozott komponenseket a változásról
 */
void Config::notifyChange() {
    for (auto &entry : changeCallbacks) {
        if (entry.active && entry.callback) { // Csak az aktív callback-eket hívjuk meg
            entry.callback();
        }
    }
}