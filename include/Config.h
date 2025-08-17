#ifndef __CONFIG_H
#define __CONFIG_H

#include "ConfigData.h"
#include "DebugDataInspector.h"
#include "StoreBase.h"

// Alapértelmezett konfigurációs adatok (readonly, const)
extern const Config_t DEFAULT_CONFIG;

/**
 * Konfigurációs adatok kezelése
 */
class Config : public StoreBase<Config_t> {
  public:
    // A 'config' változó, alapértelmezett értékeket veszi fel a konstruktorban
    // Szándékosan public, nem kell a sok getter egy embedded rendszerben
    Config_t data;

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
    void loadDefaults() override { memcpy(&data, &DEFAULT_CONFIG, sizeof(Config_t)); }
};

// Globális config példány deklaráció
extern Config config;

#endif // __CONFIG_H
