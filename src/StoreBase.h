#ifndef STORE_BASE_H
#define STORE_BASE_H

#include <Arduino.h>

#include "StoreEepromBase.h"
#include "defines.h"
#include "utils.h"

/**
 * @brief Generikus wrapper alaposztály EEPROM kezeléshez
 *
 * Automatikus CRC ellenőrzéssel és mentéssel rendelkezik.
 * A leszármazott osztályok egyszerűen implementálhatják az EEPROM
 * funkcionalitást.
 *
 * @tparam T A tárolandó struktúra típusa
 */
template <typename T> class StoreBase {
  protected:
    /// @brief Az utoljára mentett adatok CRC16 ellenőrző összege
    uint16_t lastCRC = 0;
    /**
     * @brief Referencia a tárolt adatokra
     *
     * A leszármazott osztályban kötelező implementálni.
     *
     * @return T& Referencia a tárolt adatokra
     */
    virtual T &getData() = 0;

    /**
     * @brief Const referencia a tárolt adatokra
     *
     * CRC számításhoz és const függvényekhez használt.
     *
     * @return const T& Const referencia a tárolt adatokra
     */
    virtual const T &getData() const = 0;

    /**
     * @brief Leszármazott osztály nevének lekérdezése
     *
     * Debug üzenetekhez használt osztálynév.
     *
     * @return const char* Az osztály neve
     */
    virtual const char *getClassName() const = 0;

    /**
     * @brief EEPROM mentés végrehajtása
     *
     * A leszármazott felülírhatja a megfelelő cím megadásához.
     *
     * @return uint16_t Mentett adatok CRC-je (0 ha sikertelen)
     */
    virtual uint16_t performSave() { return StoreEepromBase<T>::save(getData(), 0, getClassName()); }

    /**
     * @brief EEPROM betöltés végrehajtása
     *
     * A leszármazott felülírhatja a megfelelő cím megadásához.
     *
     * @return uint16_t Betöltött adatok CRC-je
     */
    virtual uint16_t performLoad() { return StoreEepromBase<T>::load(getData(), 0, getClassName()); }

  public:
    /**
     * @brief Kényszerített mentés EEPROM-ba
     *
     * Feltétel nélkül elmenti az adatokat és frissíti a CRC-t.
     */
    virtual void forceSave() {
        DEBUG("[%s] Kényszerített mentés...\n", getClassName());
        uint16_t savedCrc = performSave();
        if (savedCrc != 0) {
            lastCRC = savedCrc;
        }
    }

    /**
     * @brief Adatok betöltése EEPROM-ból
     *
     * Ha az adatok érvénytelenek, alapértékek mentése történik.
     */
    virtual void load() {
        DEBUG("[%s] Betöltés...\n", getClassName());
        lastCRC = performLoad();
    }

    /**
     * @brief Alapértelmezett értékek beállítása
     *
     * A leszármazott osztályban kötelező implementálni.
     */
    virtual void loadDefaults() = 0;
    /**
     * @brief Automatikus mentés CRC ellenőrzés alapján
     *
     * Összehasonlítja a jelenlegi adatok CRC-jét az utoljára mentett értékkel.
     * Ha különböznek, automatikusan menti az adatokat.
     */
    virtual void checkSave() {
        DEBUG("[%s] CRC ellenőrzés a mentéshez\n", getClassName());

        uint16_t currentCrc = Utils::calcCRC16(reinterpret_cast<const uint8_t *>(&getData()), sizeof(T));

        if (lastCRC != currentCrc) {
            DEBUG("[%s] CRC eltérés (RAM: %d != EEPROM: %d). Mentés...\n", getClassName(), currentCrc, lastCRC);

            uint16_t savedCrc = performSave();
            DEBUG("[%s] Mentés OK\n", getClassName());

            if (savedCrc != 0) {
                lastCRC = savedCrc;
            } else {
                DEBUG("[%s] Mentés SIKERTELEN!\n", getClassName());
            }
        }
    }

    /**
     * @brief Utolsó CRC érték lekérdezése
     * @return uint16_t Az utoljára mentett CRC érték
     */
    uint16_t getLastCRC() const { return lastCRC; }

    /**
     * @brief Jelenlegi adatok CRC-jének számítása
     * @return uint16_t A jelenlegi adatok CRC értéke
     */
    uint16_t getCurrentCRC() const { return Utils::calcCRC16(reinterpret_cast<const uint8_t *>(&getData()), sizeof(T)); }

    /**
     * @brief Ellenőrzi, hogy szükséges-e mentés
     * @return true Ha a jelenlegi CRC eltér az utoljára mentetttől
     */
    bool needsSave() const { return lastCRC != getCurrentCRC(); }
};

#endif // STORE_BASE_H