#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "ConfigData.h"
#include "DebugDataInspector.h"
#include "StoreBase.h"

// Alapértelmezett konfigurációs adatok (readonly, const)
extern const Config_t DEFAULT_CONFIG;

// Callback típus definíció a konfiguráció változásának jelzésére
using ConfigChangeCallback = std::function<void()>;

/**
 * Konfigurációs adatok kezelése
 */
class Config : public StoreBase<Config_t> {
  public:
    // A 'config' változó, alapértelmezett értékeket veszi fel a konstruktorban
    // Szándékosan public, nem kell a sok getter egy embedded rendszerben
    Config_t data;

  private:
    // Vektor a változást figyelő callback függvények tárolására
    std::vector<ConfigChangeCallback> changeCallbacks;

    /**
     * @brief Értesíti a feliratkozott komponenseket a változásról
     */
    void notifyChange();

  protected:
    const char *getClassName() const override { return "Config"; }

    /**
     * Referencia az adattagra, csak az ős használja
     */
    Config_t &getData() override { return data; };

    /**
     * Const referencia az adattagra, CRC számításhoz
     */
    const Config_t &getData() const override { return data; };

    // Felülírjuk a mentést/betöltést a debug kiíratás hozzáadásához

    /**
     * Mentési művelet
     */
    uint16_t performSave() override {
        uint16_t savedCrc = StoreEepromBase<Config_t>::save(getData(), 0, getClassName());
#ifdef __DEBUG
        if (savedCrc != 0) {
            DebugDataInspector::printConfigData(getData());
        }
#endif
        // Csak akkor értesítünk, ha a mentés sikeres volt (CRC nem nulla)
        if (savedCrc != 0) {
            notifyChange();
        }
        return savedCrc;
    }

    /**
     *  Betöltési művelet
     */
    uint16_t performLoad() override {
        uint16_t loadedCrc = StoreEepromBase<Config_t>::load(getData(), 0, getClassName());

        // Min/Max érték korlátozása
        data.tftManualBrightnessValue = constrain(data.tftManualBrightnessValue, 0, TFT_BACKGROUND_LED_MAX_BRIGHTNESS);

#ifdef __DEBUG
        DebugDataInspector::printConfigData(getData()); // Akkor is kiírjuk, ha defaultot töltött
#endif

        // Betöltés után is értesítünk, hogy a komponensek felvegyék a friss értékeket
        notifyChange();

        return loadedCrc;
    }

  public:
    /**
     * Konstruktor
     * @param pData Pointer a konfigurációs adatokhoz
     */
    Config() : StoreBase<Config_t>(), data(DEFAULT_CONFIG) {}

    /**
     * Alapértelmezett adatok betöltése
     */
    void loadDefaults() override {
        memcpy(&data, &DEFAULT_CONFIG, sizeof(Config_t));
        // Alapértelmezett értékek betöltésekor is értesítünk
        notifyChange();
    }

    /**
     * @brief Feliratkoztat egy komponenst a konfiguráció változásainak figyelésére
     * @param callback A függvény, amit változáskor meg kell hívni
     * @return Index a leiratkozáshoz
     */
    size_t registerChangeCallback(ConfigChangeCallback callback);

    /**
     * @brief Leiratkoztat egy komponenst index alapján
     * @param callbackId A registerChangeCallback által visszaadott index
     */
    void unregisterCallback(size_t callbackId);
};

// Globális config példány deklaráció
extern Config config;