
#pragma once
#include <Arduino.h>

#define MAX_TAFIPAX 400
#define MAX_CITY_LEN 32
#define MAX_STREET_LEN 48

struct TafipaxInternal {
    char city[MAX_CITY_LEN];
    char street_or_km[MAX_STREET_LEN];
    double lat;
    double lon;
};

class TafipaxList {
  public:
    constexpr static const char *CSV_FILE_NAME = "/trafipaxes.csv";

    TafipaxList();

    // Fájl kezelés/betöltés
    boolean checkFile(const char *filename);
    void loadFromCSV(const char *filename);
    int count() const;

    // Trafipax riasztás - csak közeledés esetén riaszt
    const TafipaxInternal *checkTrafipaxApproach(double currentLat, double currentLon, double alertDistanceMeters);

    // Legközelebbi trafipax keresése távolsággal együtt
    const TafipaxInternal *getClosestTrafipax(double currentLat, double currentLon, double &outDistance) const;

    // Teszt metódus - litéri trafipax közeledés/távolodás szimulálása
    void testLiteriTrafipaxApproach();

  private:
    TafipaxInternal tafipaxList[MAX_TAFIPAX];
    int tafipaxCount = 0;

    // Távolság követés közeledés detektáláshoz
    double lastLat = 0.0;
    double lastLon = 0.0;
    int lastClosestTrafipaxIdx = -1;
    double lastDistance = 999999.0;

    // Távolság számítás Haversine formulával
    double calculateDistance(double lat1, double lon1, double lat2, double lon2) const;
};
