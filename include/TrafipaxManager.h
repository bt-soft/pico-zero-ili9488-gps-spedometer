#ifndef __TRAFIPAX_MANAGER_H
#define __TRAFIPAX_MANAGER_H

#include <Arduino.h>

#define MAX_TAFIPAX 400
#define MAX_CITY_LEN 32
#define MAX_STREET_LEN 48

// Demo trafipax közeledés/távolodás szimulálása működés közben
struct TrafipaxDemo {
    bool isActive = false;
    unsigned long startTime = 0;
    unsigned long currentPhase = 0;

    // Demo koordináták
    double currentLat = 0.0;
    double currentLon = 0.0;
    bool hasValidCoords = false;

    // Demo fázisok (másodpercben)
    static constexpr unsigned long PHASE_WAIT = 5;      // 5mp várakozás
    static constexpr unsigned long PHASE_APPROACH = 20; // 15mp közeledés (5-20mp)
    static constexpr unsigned long PHASE_DEPART = 40;   // 20mp távolodás (20-40mp) - lassítva
    static constexpr unsigned long PHASE_END = 45;      // 5mp befejezés (40-45mp)

    // Litéri trafipax koordinátái
    static constexpr double LITERI_LAT = 47.100934;
    static constexpr double LITERI_LON = 18.011792;
};

struct TrafipaxInternal {
    char city[MAX_CITY_LEN];
    char street_or_km[MAX_STREET_LEN];
    double lat;
    double lon;
};

class TrafipaxManager {
  public:
    constexpr static const char *CSV_FILE_NAME = "/trafipaxes.csv";

    TrafipaxManager();

    // Fájl kezelés/betöltés
    boolean checkFile(const char *filename);
    void loadFromCSV(const char *filename);
    int count() const;

    // Trafipax riasztás - csak közeledés esetén riaszt
    const TrafipaxInternal *checkTrafipaxApproach(double currentLat, double currentLon, double alertDistanceMeters);

    // Legközelebbi trafipax keresése távolsággal együtt
    const TrafipaxInternal *getClosestTrafipax(double currentLat, double currentLon, double &outDistance) const;

    // Teszt metódus - litéri trafipax közeledés/távolodás szimulálása
    void testLiteriTrafipaxApproach();

    // Demo funkciók
    void startDemo();
    void processDemo();
    bool isDemoActive() const;
    bool getDemoCoords(double &lat, double &lon) const;

  private:
    TrafipaxInternal tafipaxList[MAX_TAFIPAX];
    int tafipaxCount = 0;

    // Távolság követés közeledés detektáláshoz
    double lastLat = 0.0;
    double lastLon = 0.0;
    int lastClosestTrafipaxIdx = -1;
    double lastDistance = 999999.0;

    // Demo objektum
    TrafipaxDemo demo;
};
#endif // __TRAFIPAX_MANAGER_H