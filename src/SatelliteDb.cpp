
#include <Arduino.h>

#include "SatelliteDb.h"
#include "defines.h"

// 30mp után töröljük a sat-ot ha nem frissült ez idő alatt
#define SAT_MAX_AGE_MSEC (30 * 1000)

/**
 * Konstruktor
 */
SatelliteDb::SatelliteDb() {
    // std::list automatikusan inicializálódik üres állapotban
}

/**
 * Műholdak számának lekérdezése
 */
uint8_t SatelliteDb::countSats() const { return satellites.size(); }

/**
 * Műhold keresése PRN alapján
 */
SatelliteDb::SatelliteData *SatelliteDb::searchSat(uint8_t prnNo) {
    auto it = findSatellite(prnNo);
    return (it != satellites.end()) ? &(*it) : nullptr;
}

/**
 * Műhold törlése PRN alapján
 */
void SatelliteDb::deleteSat(uint8_t prn) {
    auto it = findSatellite(prn);
    if (it != satellites.end()) {
        satellites.erase(it);
    }
}

/**
 * Régi műholdak törlése (amelyek nem frissültek a megadott időn belül)
 */
void SatelliteDb::deleteUntrackedSatellites() {
    unsigned long currentTime = millis();
    satellites.remove_if([currentTime](const SatelliteData &sat) { return (currentTime - sat.timeStamp) > SAT_MAX_AGE_MSEC; });
}

/**
 * Műhold hozzáadása vagy frissítése
 */
void SatelliteDb::insertSatellite(uint8_t prn, int16_t elevation, int16_t azimuth, int16_t snr) {
    unsigned long currentTime = millis();

    // Keresés: van már ilyen PRN?
    auto it = findSatellite(prn);

    if (it != satellites.end()) {
        // Frissítés
        if (snr == 0) {
            // Ha SNR 0, töröljük
            satellites.erase(it);
        } else {
            // Frissítjük az adatokat
            it->elevation = elevation;
            it->azimuth = azimuth;
            it->snr = snr;
            it->timeStamp = currentTime;
        }
    } else {
        // Új műhold hozzáadása (csak ha SNR > 0)
        if (snr > 0) {
            satellites.emplace_back(prn, elevation, azimuth, snr, currentTime);
        }
    }
}

/**
 * Összes műhold törlése
 */
void SatelliteDb::clear() { satellites.clear(); }

/**
 * Thread-safe snapshot létrehozása UI számára (Core0)
 * Gyors másolat készítése az aktuális állapotról
 * @param sortType Rendezési mód (alapértelmezett: NONE)
 */
std::vector<SatelliteDb::SatelliteData> SatelliteDb::getSnapshotForUI(SortType_t sortType) const {

    // Gyors másolat készítése - ez atomic operation std::list-nél
    std::vector<SatelliteData> copiedSatellites(satellites.begin(), satellites.end());

    // Le is rendezzük a másolatot a beállított mód szerint, ha kell
    if (sortType == BY_PRN) {
        std::sort(copiedSatellites.begin(), copiedSatellites.end(), [](const SatelliteData &a, const SatelliteData &b) {
            return a.prn < b.prn; // PRN növekvő sorrendben
        });

    } else if (sortType == BY_SNR) {
        std::sort(copiedSatellites.begin(), copiedSatellites.end(), [](const SatelliteData &a, const SatelliteData &b) {
            return a.snr > b.snr; // SNR csökkenő sorrendben
        });
    }

    return copiedSatellites;
}

/**
 * Thread-safe műholdak számának lekérdezése UI számára (Core0)
 */
uint8_t SatelliteDb::countSatsForUI() const {
    // size() olvasás általában atomic, de a snapshot biztonságosabb
    return satellites.size();
}

/**
 * Debugging műhold adatbázis
 * @param num_sats_in_view A GSV üzenetből érkező műholdak száma
 */
void SatelliteDb::debugSatDb(uint8_t num_sats_in_view) {
    unsigned long currentTime = millis();

    DEBUG("--------------------------------\n");
    DEBUG("\n--- Visible Satellites ---\n");
    DEBUG("Total in view (from GSV): %d\n", num_sats_in_view);
    DEBUG("Total in DB: %d\n", countSats());
    DEBUG("PRN | Elev | Azim | SNR | TTL(s)\n");
    DEBUG("----|------|------|-----|-------\n");

    for (const auto &sat : satellites) {
        long age = currentTime - sat.timeStamp;
        long timeToLive = (SAT_MAX_AGE_MSEC - age) / 1000; // másodpercben

        // Ha negatív, akkor már lejárt (de még nem lett törölve)
        if (timeToLive < 0) {
            timeToLive = 0;
        }

        DEBUG("%2d  | %2d   | %3d  | %2d  | %2ld\n", sat.prn, sat.elevation, sat.azimuth, sat.snr, timeToLive);
    }
    DEBUG("--------------------------------\n");
}
