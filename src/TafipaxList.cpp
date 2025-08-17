#include <Arduino.h>
#include <LittleFS.h>
#include <TinyGPS++.h>
#include <math.h>

#include "TafipaxList.h"
#include "Utils.h"
#include "defines.h"

// CSV forrás:
//   https://www.autopalyamatrica.hu/fix-traffipax-lista-veda-terkep
//
// CSV feltöltése a LittleFS-re:
//  pio run --target uploadfs
//
/**
 *
 */
TafipaxList::TafipaxList() {}

/**
 * Ellenőrizzük, hogy létezik-e a CSV fájl
 */
boolean TafipaxList::checkFile(const char *filename) {
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
void TafipaxList::loadFromCSV(const char *filename) {
    tafipaxCount = 0;
    File file = LittleFS.open(filename, "r");
    if (!file) {
        DEBUG("Failed to open file: %s\n", filename);
        return;
    }

    char line[192];

    // Skip header
    file.readBytesUntil('\n', line, sizeof(line));
    while (file.available() && tafipaxCount < MAX_TAFIPAX) {

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

        TrafipaxInternal &t = tafipaxList[tafipaxCount++];
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
int TafipaxList::count() const { return tafipaxCount; }

/**
 * Trafipax riasztás - csak közeledés esetén riaszt
 * @param currentLat aktuális GPS szélesség
 * @param currentLon aktuális GPS hosszúság
 * @param alertDistanceMeters riasztási távolság méterben
 * @return trafipax rekord ha közeledünk és a távolság <= alertDistanceMeters, egyébként nullptr
 */
const TrafipaxInternal *TafipaxList::checkTrafipaxApproach(double currentLat, double currentLon, double alertDistanceMeters) {

    // Legközelebbi trafipax keresése
    int closestIdx = -1;
    double minDistance = 999999.0;

    for (int i = 0; i < tafipaxCount; i++) {
        double distance = TinyGPSPlus::distanceBetween(currentLat, currentLon, tafipaxList[i].lat, tafipaxList[i].lon);
        if (distance < minDistance) {
            minDistance = distance;
            closestIdx = i;
        }
    }

    // Nincs trafipax
    if (closestIdx == -1) {
        return nullptr;
    }

    // Ha ez egy új legközelebbi trafipax, vagy első hívás
    if (closestIdx != lastClosestTrafipaxIdx) {
        lastClosestTrafipaxIdx = closestIdx;
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
    // 2. Közeledünk a trafipaxhoz
    if (minDistance <= alertDistanceMeters && isApproaching) {
        return &tafipaxList[closestIdx];
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
const TrafipaxInternal *TafipaxList::getClosestTrafipax(double currentLat, double currentLon, double &outDistance) const {

    if (tafipaxCount == 0) {
        outDistance = 999999.0;
        return nullptr;
    }

    int closestIdx = -1;
    double minDistance = 999999.0;

    for (int i = 0; i < tafipaxCount; i++) {
        double distance = TinyGPSPlus::distanceBetween(currentLat, currentLon, tafipaxList[i].lat, tafipaxList[i].lon);
        if (distance < minDistance) {
            minDistance = distance;
            closestIdx = i;
        }
    }

    outDistance = minDistance;

    if (closestIdx == -1) {
        return nullptr;
    }

    return &tafipaxList[closestIdx];
}

/**
 * Teszt metódus - litéri trafipax közeledés/távolodás szimulálása
 * Litéri trafipax koordinátái: 47.100934, 18.011792
 * A teszt 1000m-ről indul és 200m-ig közeledik, majd 1000m-ig távolodik
 */
void TafipaxList::testLiteriTrafipaxApproach() {
    const double LITERI_LAT = 47.100934;
    const double LITERI_LON = 18.011792;
    const double ALERT_DISTANCE = 500.0; // 500 méteres riasztási távolság

    // Kiindulási pont: 1000m távolságban délre a litéri trafipax-tól
    double startLat = LITERI_LAT - 0.009; // kb. 1000m délre
    double startLon = LITERI_LON;

    Serial.println("\n=== LITÉRI TRAFIPAX TESZT ===");
    Serial.println("Közeledés teszt (1000m -> 200m):");

    // 1. KÖZELEDÉS FÁZIS: 1000m-ről 200m-ig
    for (int step = 0; step <= 10; step++) {
        // Lineáris interpoláció a kiindulási pont és a trafipax között
        double progress = step / 10.0;
        double testLat = startLat + (LITERI_LAT - startLat) * progress * 0.8; // csak 80%-ig megyünk (200m marad)
        double testLon = startLon;

        // Távolság kiszámítása ellenőrzéshez
        double distance = TinyGPSPlus::distanceBetween(testLat, testLon, LITERI_LAT, LITERI_LON);

        // Trafipax közeledés ellenőrzése
        const TrafipaxInternal *result = checkTrafipaxApproach(testLat, testLon, ALERT_DISTANCE);

        Serial.print("Lépés ");
        Serial.print(step);
        Serial.print(": Távolság = ");
        Serial.print(distance, 0);
        Serial.print("m");

        if (result != nullptr) {
            Serial.print(" -> RIASZTÁS! ");
            Serial.print(result->city);
            Serial.print(", ");
            Serial.println(result->street_or_km);
        } else {
            Serial.println(" -> Nincs riasztás");
        }

        delay(500); // 500ms várakozás a lépések között
    }

    Serial.println("\nTávolodás teszt (200m -> 1000m):");

    // 2. TÁVOLODÁS FÁZIS: 200m-ről 1000m-ig
    for (int step = 0; step <= 10; step++) {
        // Vissza távolodunk a kiindulási pontig
        double progress = step / 10.0;
        double currentLat = LITERI_LAT - 0.0018;                   // 200m-ről indulunk
        double testLat = currentLat - (0.009 - 0.0018) * progress; // távolodunk 1000m-ig
        double testLon = startLon;

        // Távolság kiszámítása ellenőrzéshez
        double distance = TinyGPSPlus::distanceBetween(testLat, testLon, LITERI_LAT, LITERI_LON);

        // Trafipax közeledés ellenőrzése
        const TrafipaxInternal *result = checkTrafipaxApproach(testLat, testLon, ALERT_DISTANCE);

        Serial.print("Lépés ");
        Serial.print(step);
        Serial.print(": Távolság = ");
        Serial.print(distance, 0);
        Serial.print("m");

        if (result != nullptr) {
            Serial.print(" -> RIASZTÁS! ");
            Serial.print(result->city);
            Serial.print(", ");
            Serial.println(result->street_or_km);
        } else {
            Serial.println(" -> Nincs riasztás");
        }

        delay(500); // 500ms várakozás a lépések között
    }

    Serial.println("=== TESZT BEFEJEZVE ===\n");
}

/**
 * Demo indítása - 5mp várakozás, majd közeledés/távolodás szimulálása
 */
void TafipaxList::startDemo() {
    demo.isActive = true;
    demo.startTime = millis();
    demo.currentPhase = 0;

    DEBUG("\n=== TRAFIPAX DEMO INDÍTVA ===\n");
    DEBUG("Teszt fázisok:\n");
    DEBUG("0-5mp: Várakozás (nincs riasztás)\n");
    DEBUG("5-20mp: Közeledés a litéri trafipaxhoz\n");
    DEBUG("20-40mp: Távolodás a litéri trafipaxtól (lassítva)\n");
    DEBUG("40-45mp: Demo befejezése\n");
    DEBUG("------------------------------------------\n\n");
}

/**
 * Demo feldolgozása - szimulált GPS koordináták generálása
 */
void TafipaxList::processDemo() {
    if (!demo.isActive) {
        return;
    }

    unsigned long elapsed = (millis() - demo.startTime) / 1000; // másodpercek

    // Demo befejezése
    if (elapsed >= TrafipaxDemo::PHASE_END) {
        demo.isActive = false;
        demo.hasValidCoords = false;
        DEBUG("=== DEMO BEFEJEZVE ===\n");
        return;
    }

    // Szimulált GPS koordináták generálása a fázis alapján
    double simLat, simLon;

    if (elapsed < TrafipaxDemo::PHASE_WAIT) {
        // Várakozási fázis - messze vagyunk (1500m)
        simLat = TrafipaxDemo::LITERI_LAT - 0.0135; // kb. 1500m délre
        simLon = TrafipaxDemo::LITERI_LON;

        if (elapsed != demo.currentPhase) {
            demo.currentPhase = elapsed;
            DEBUG("Demo fázis: Várakozás (%lus/5s)\n", elapsed);
        }

    } else if (elapsed < TrafipaxDemo::PHASE_APPROACH) {
        // Közeledési fázis - 1500m-ről 200m-ig
        float progress = (elapsed - TrafipaxDemo::PHASE_WAIT) / 15.0f; // 0.0 - 1.0 (15s alatt)
        simLat = TrafipaxDemo::LITERI_LAT - 0.0135 + (0.0135 - 0.0018) * progress;
        simLon = TrafipaxDemo::LITERI_LON;

        if (elapsed != demo.currentPhase) {
            demo.currentPhase = elapsed;
            double distance = TinyGPSPlus::distanceBetween(simLat, simLon, TrafipaxDemo::LITERI_LAT, TrafipaxDemo::LITERI_LON);
            DEBUG("Demo fázis: Közeledés (%lus/20s) - %dm\n", elapsed, (int)distance);
        }

    } else if (elapsed < TrafipaxDemo::PHASE_DEPART) {
        // Távolodási fázis - 200m-ről 1500m-ig (lassítva 20s alatt)
        float progress = (elapsed - TrafipaxDemo::PHASE_APPROACH) / 20.0f; // 0.0 - 1.0 (20s alatt)
        simLat = TrafipaxDemo::LITERI_LAT - 0.0018 - (0.0135 - 0.0018) * progress;
        simLon = TrafipaxDemo::LITERI_LON;

        if (elapsed != demo.currentPhase) {
            demo.currentPhase = elapsed;
            double distance;
            const TrafipaxInternal *closest = getClosestTrafipax(simLat, simLon, distance);
            if (closest) {
                DEBUG("Demo fázis: Távolodás (%lus/40s) - %dm\n", elapsed, (int)distance);
            }
        }

    } else {
        // Befejező fázis - messze vagyunk
        simLat = TrafipaxDemo::LITERI_LAT - 0.0135;
        simLon = TrafipaxDemo::LITERI_LON;

        if (elapsed != demo.currentPhase) {
            demo.currentPhase = elapsed;
            DEBUG("Demo fázis: Befejezés (%lus/45s)\n", elapsed);
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
bool TafipaxList::isDemoActive() const { return demo.isActive; }

/**
 * Demo koordináták lekérése
 */
bool TafipaxList::getDemoCoords(double &lat, double &lon) const {
    if (demo.isActive && demo.hasValidCoords) {
        lat = demo.currentLat;
        lon = demo.currentLon;
        return true;
    }
    return false;
}
