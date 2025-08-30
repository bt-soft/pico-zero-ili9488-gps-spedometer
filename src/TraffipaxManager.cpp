#include <Arduino.h>
#include <LittleFS.h>
#include <TinyGPS++.h>
#include <math.h>

#include "TraffipaxManager.h"
#include "Utils.h"
#include "defines.h"

// CSV forrás:
//   https://www.autopalyamatrica.hu/fix-traffipax-lista-veda-terkep
//
// CSV feltöltése a LittleFS-re:
//  pio run --target uploadfs
//

constexpr double TRAFFIPAX_DEMO_FAR_DISTANCE_M = 2000.0;
constexpr double TRAFFIPAX_DEMO_FAR_LAT_OFFSET = TRAFFIPAX_DEMO_FAR_DISTANCE_M / 111000.0; // ≈ 0.0180
#define TRAFFIPAX_DEMO_FAR_DISTANCE (TraffipaxDemo::demoTraffipaxLat - TRAFFIPAX_DEMO_FAR_LAT_OFFSET)

constexpr double TRAFFIPAX_DEMO_NEAR_DISTANCE_M = 200.0;
constexpr double TRAFFIPAX_DEMO_NEAR_LAT_OFFSET = TRAFFIPAX_DEMO_NEAR_DISTANCE_M / 111000.0; // ≈ 0.0018

// TraffipaxManager.cpp vagy egy másik .cpp-ben:
double TraffipaxManager::TraffipaxDemo::demoTraffipaxLat = 0.0;
double TraffipaxManager::TraffipaxDemo::demoTraffipaxLon = 0.0;

/**
 *
 */
TraffipaxManager::TraffipaxManager() {}

/**
 * Ellenőrizzük, hogy létezik-e a CSV fájl
 */
boolean TraffipaxManager::checkFile(const char *filename) {
    //
    if (LittleFS.exists(filename)) {
        File file = LittleFS.open(filename, "r");
        DEBUG("CSV fájl mérete: %d bájt\n", file.size());
        file.close();
        return true;
    }

    DEBUG("HIBA: A(z) %s CSV fájl nem található!\n", filename);
    return false;
}

/**
 * Betölti a Trafipax adatokat CSV fájlból
 */
void TraffipaxManager::loadFromCSV(const char *filename) {
    traffipaxCount = 0;
    File file = LittleFS.open(filename, "r");
    if (!file) {
        DEBUG("Failed to open file: %s\n", filename);
        return;
    }

    char line[192];

    // Skip header
    file.readBytesUntil('\n', line, sizeof(line));
    while (file.available() && traffipaxCount < MAX_TRAFIPAX_COUNT) {

        int len = file.readBytesUntil('\n', line, sizeof(line) - 1);

        if (len <= 0) {
            continue;
        }
        line[len] = 0;

        // Skip empty lines (only whitespace)
        char *trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t' || *trimmed == '\r') {
            trimmed++;
        }
        if (*trimmed == '\0') {
            continue; // Skip empty line
        }

        // CSV: Vármegye,Település neve,Útszám,Kilométer-szelvény/utca,GPS koordináta szélesség,GPS koordináta hosszúság
        // Parse manually to handle commas in field values
        char *fields[6];
        int fieldCount = 0;
        char *start = trimmed;
        int trimmed_len = strlen(trimmed);

        for (int i = 0; i < trimmed_len && fieldCount < 6; i++) {
            if (trimmed[i] == ',') {
                trimmed[i] = '\0';
                fields[fieldCount++] = start;
                start = &trimmed[i + 1];
            }
        }
        // Add last field
        if (fieldCount < 6) {
            fields[fieldCount++] = start;
        }

        if (fieldCount != 6) {
            DEBUG("Invalid CSV line, fields: %d\n", fieldCount);
            continue;
        }

        // char *county = fields[0];
        char *city = fields[1];
        // char *road = fields[2];
        char *street = fields[3];
        char *latstr = fields[4];
        char *lonstr = fields[5];

        if (!city || !street || !latstr || !lonstr) {
            continue;
        }

        TraffipaxRecord &t = traffipaxList[traffipaxCount++];
        strncpy(t.city, city, MAX_CITY_LEN - 1);
        t.city[MAX_CITY_LEN - 1] = 0;
        strncpy(t.street_or_km, street, MAX_STREET_LEN - 1);
        t.street_or_km[MAX_STREET_LEN - 1] = 0;
        t.lat = atof(latstr);
        t.lon = atof(lonstr);

        // DEBUG("Loaded Tafipax: %s, %s, %s, %s\n", t.city, t.street_or_km, Utils::floatToString(t.lat, 6).c_str(), Utils::floatToString(t.lon, 6).c_str());
    }

    file.close();
}

/**
 * Visszaadja a Trafipaxok számát
 */
int TraffipaxManager::count() const { return traffipaxCount; }

/**
 * Trafipax riasztás - csak közeledés esetén riaszt
 * @param currentLat aktuális GPS szélesség
 * @param currentLon aktuális GPS hosszúság
 * @param alertDistanceMeters riasztási távolság méterben
 * @return trafipax rekord ha közeledünk és a távolság <= alertDistanceMeters, egyébként nullptr
 */
const TraffipaxManager::TraffipaxRecord *TraffipaxManager::checkTraffipaxApproach(double currentLat, double currentLon, double alertDistanceMeters) {

    // Legközelebbi trafipax keresése
    int closestIdx = -1;
    double minDistance = 999999.0;

    for (int i = 0; i < traffipaxCount; i++) {
        double distance = TinyGPSPlus::distanceBetween(currentLat, currentLon, traffipaxList[i].lat, traffipaxList[i].lon);
        if (distance < minDistance) {
            minDistance = distance;
            closestIdx = i;
        }
    }

    // Nincs trafipax
    if (closestIdx == -1) {
        return nullptr;
    }

    // Ha ez egy új legközelebbi traffipax, vagy első hívás
    if (closestIdx != lastClosestTraffipaxIdx) {
        lastClosestTraffipaxIdx = closestIdx;
        lastDistance = minDistance;
        lastLat = currentLat;
        lastLon = currentLon;
        return nullptr; // Első alkalommal ne riasszon
    }

    // Ugyanaz a trafipax - vizsgáljuk a közeledést
    bool isApproaching = minDistance < lastDistance;

    // Frissítjük az utolsó értékeket
    lastDistance = minDistance;
    lastLat = currentLat;
    lastLon = currentLon;

    // Riasztás feltételei:
    // 1. A távolság <= riasztási távolság
    // 2. Közeledünk a traffipaxhoz
    if (minDistance <= alertDistanceMeters && isApproaching) {
        return &traffipaxList[closestIdx];
    }

    return nullptr;
}

/**
 * Legközelebbi trafipax keresése távolsággal együtt
 * @param currentLat aktuális GPS szélesség
 * @param currentLon aktuális GPS hosszúság
 * @param outDistance kimeneti paraméter a távolsághoz
 * @return legközelebbi trafipax rekord vagy nullptr ha nincs
 */
const TraffipaxManager::TraffipaxRecord *TraffipaxManager::getClosestTraffipax(double currentLat, double currentLon, double &outDistance) const {

    if (traffipaxCount == 0) {
        outDistance = 999999.0;
        return nullptr;
    }

    int closestIdx = -1;
    double minDistance = 999999.0;

    for (int i = 0; i < traffipaxCount; i++) {
        double distance = TinyGPSPlus::distanceBetween(currentLat, currentLon, traffipaxList[i].lat, traffipaxList[i].lon);
        if (distance < minDistance) {
            minDistance = distance;
            closestIdx = i;
        }
    }

    outDistance = minDistance;

    if (closestIdx == -1) {
        return nullptr;
    }

    return &traffipaxList[closestIdx];
}

//-------------------------------------------------------- DEMO -------------------------------------------------------------------------------------

/**
 * Demo indítása - 5mp várakozás, majd közeledés/távolodás szimulálása
 */
void TraffipaxManager::startDemo() {
    demo.isActive = true;
    demo.startTime = millis();
    demo.currentPhase = 0;

    DEBUG("\n=== TRAFFIPAX DEMÓ INDÍTVA ===\n");
    DEBUG("Teszt fázisok:\n");
    DEBUG("0-5mp: Várakozás (nincs riasztás)\n");
    DEBUG("5-20mp: Közeledés egy véletlenül kiválasztott traffipaxhoz\n");
    DEBUG("20-40mp: Távolodás a véletlenül kiválasztott traffipaxtól\n");
    DEBUG("40-45mp: Demó befejezése\n");

    // Véletlenszerű trafipax kiválasztása
    if (traffipaxCount > 0) {
        const TraffipaxRecord &selected = traffipaxList[random(traffipaxCount)];
        TraffipaxDemo::demoTraffipaxLat = selected.lat;
        TraffipaxDemo::demoTraffipaxLon = selected.lon;
        DEBUG("Demo trafipax kiválasztva: %s, %s, lat: %.6f, lon: %.6f\n", selected.city, selected.street_or_km, selected.lat, selected.lon);
    } else {
        // Ha nincs trafipax, akkor a litéri VÉDA koordináták
        TraffipaxDemo::demoTraffipaxLat = 47.100934;
        TraffipaxDemo::demoTraffipaxLon = 18.011792;
        DEBUG("Demo trafipax: nincs elérhető rekord, default koordináták.\n");
    }

    DEBUG("------------------------------------------\n\n");
}

/**
 * Demo feldolgozása - szimulált GPS koordináták generálása
 */
void TraffipaxManager::processDemo() {

    if (!demo.isActive) {
        return;
    }

    unsigned long elapsed = (millis() - demo.startTime) / 1000; // másodpercek

    // Demo befejezése
    if (elapsed >= TraffipaxDemo::PHASE_END) {
        demo.isActive = false;
        demo.hasValidCoords = false;
        DEBUG("=== TRAFFIPAX DEMÓ BEFEJEZVE ===\n");
        return;
    }

    // Szimulált GPS koordináták generálása a fázis alapján
    double simLat, simLon;

    if (elapsed < TraffipaxDemo::PHASE_WAIT) {
        // Várakozási fázis - messze vagyunk (2000m)
        simLat = TRAFFIPAX_DEMO_FAR_DISTANCE; // kb. 2000m délre
        simLon = TraffipaxDemo::demoTraffipaxLon;

        if (elapsed != demo.currentPhase) {
            demo.currentPhase = elapsed;
            DEBUG("Traffi Demo Fázis: Várakozás (%lus/5s)\n", elapsed);
        }

    } else if (elapsed < TraffipaxDemo::PHASE_APPROACH) {
        // Közeledési fázis - 2000m-ről 200m-ig
        float progress = (elapsed - TraffipaxDemo::PHASE_WAIT) / 15.0f; // 0.0 - 1.0 (15s alatt)
        simLat = TRAFFIPAX_DEMO_FAR_DISTANCE + (TRAFFIPAX_DEMO_FAR_LAT_OFFSET - TRAFFIPAX_DEMO_NEAR_LAT_OFFSET) * progress;
        simLon = TraffipaxDemo::demoTraffipaxLon;

        if (elapsed != demo.currentPhase) {
            demo.currentPhase = elapsed;
            double distance = TinyGPSPlus::distanceBetween(simLat, simLon, TraffipaxDemo::demoTraffipaxLat, TraffipaxDemo::demoTraffipaxLon);
            DEBUG("Traffi Demo Fázis: Közeledés (%lus/20s) - %dm\n", elapsed, (int)distance);
        }

    } else if (elapsed < TraffipaxDemo::PHASE_DEPART) {
        // Távolodási fázis - 200m-ről 2000m-ig (lassítva 20s alatt)
        float progress = (elapsed - TraffipaxDemo::PHASE_APPROACH) / 20.0f; // 0.0 - 1.0 (20s alatt)
        simLat = TraffipaxDemo::demoTraffipaxLat - TRAFFIPAX_DEMO_NEAR_LAT_OFFSET - (TRAFFIPAX_DEMO_FAR_LAT_OFFSET - TRAFFIPAX_DEMO_NEAR_LAT_OFFSET) * progress;
        simLon = TraffipaxDemo::demoTraffipaxLon;

        if (elapsed != demo.currentPhase) {
            demo.currentPhase = elapsed;
            double distance;
            const TraffipaxRecord *closest = getClosestTraffipax(simLat, simLon, distance);
            if (closest) {
                DEBUG("Traffi Demo Fázis: Távolodás (%lus/40s) - %dm\n", elapsed, (int)distance);
            }
        }

    } else {
        // Befejező fázis - messze vagyunk
        simLat = TRAFFIPAX_DEMO_FAR_DISTANCE;
        simLon = TraffipaxDemo::demoTraffipaxLon;

        if (elapsed != demo.currentPhase) {
            demo.currentPhase = elapsed;
            DEBUG("Traffi Demo Fázis: Befejezés (%lus/45s)\n", elapsed);
        }
    }

    // Demo koordináták tárolása
    demo.currentLat = simLat;
    demo.currentLon = simLon;
    demo.hasValidCoords = true;
}

/**
 * Visszaadja, hogy aktív-e a demo
 */
bool TraffipaxManager::isDemoActive() const { return demo.isActive; }

/**
 * Demo koordináták lekérése
 */
bool TraffipaxManager::getDemoCoords(double &lat, double &lon) const {
    if (demo.isActive && demo.hasValidCoords) {
        lat = demo.currentLat;
        lon = demo.currentLon;
        return true;
    }
    return false;
}
