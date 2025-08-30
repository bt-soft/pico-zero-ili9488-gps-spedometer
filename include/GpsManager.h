#pragma once

#include <Arduino.h>
#include <TinyGPS++.h>

#include "DayLightSaving.h"
#include "SatelliteDb.h"

class GpsManager {

  public:
    struct Satellites_type {
        int prn;
        int elevation;
        int azimuth;
        int snr;
        bool updated; // Flag to indicate if this satellite's data was updated in the current GSV block
    };

    SatelliteDb satelliteDb;

    /**
     * Konstruktor
     */
    GpsManager(HardwareSerial *serial);

    /**
     * loop
     */
    void loop();

    /**
     * LED debug mode beállítása
     */
    void setLedDebug(bool state) { debugGpsSerialOnInternalFastLed = state; }
    /**
     * Soros debug mód beállítása
     */
    void setSerialDebug(bool state) { debugGpsSerialData = state; }

    /**
     * Debug GPS műhold adatbázis logolása
     */
    void setDebugGpsSatellitesDatabase(bool state) { debugGpsSatellitesDatabase = state; }

    /**
     * Thread-safe hozzáférés a műhold adatbázishoz UI számára (Core0)
     */
    std::vector<SatelliteDb::SatelliteData> getSatelliteSnapshotForUI(SatelliteDb::SortType_t sortType = SatelliteDb::NONE) const {
        //
        return satelliteDb.getSnapshotForUI(sortType);
    }
    /**
     * Thread-safe műholdak számának lekérdezése UI számára (Core0)
     */
    uint8_t getSatelliteCountForUI() const { return satelliteDb.countSatsForUI(); }

    /**
     * Thread-safe GPS adatok lekérdezése UI számára (Core0)
     */
    TinyGPSLocation getLocation() { return gps.location; }
    TinyGPSInteger getSatellites() { return gps.satellites; }
    TinyGPSHDOP getHdop() { return gps.hdop; }
    TinyGPSSpeed getSpeed() { return gps.speed; }
    TinyGPSDate getDate() { return gps.date; }
    TinyGPSTime getTime() { return gps.time; }
    TinyGPSCourse getCourse() { return gps.course; }
    TinyGPSAltitude getAltitude() { return gps.altitude; }

    /**
     * Helyi időzóna szerint korrigált dátum és idő lekérdezése (CET/CEST)
     */
    struct LocalDateTime {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        bool timeValid;
        uint8_t day;
        uint8_t month;
        uint16_t year;
        bool dateValid;
    };

    LocalDateTime getLocalDateTime();
    LocalDateTime getLocalTime() { return getLocalDateTime(); } // Alias

    uint32_t getGpsBootTime() { return gpsBootTime; }

    String getGpsQualityString();
    String getGpsModeToString();

  private:
    HardwareSerial *gpsSerial;
    TinyGPSPlus gps;

    uint8_t currentSatelliteCount = 0;  // Number of satellites currently being tracked
    TinyGPSCustom gsv_msg_num;          // gsv_msg_num
    TinyGPSCustom gsv_total_msgs;       // gsv_total_msgs
    TinyGPSCustom gsv_num_sats_in_view; // gsv_num_sats_in_view

    TinyGPSCustom gsv_prn[4];       // gsv_prn
    TinyGPSCustom gsv_elevation[4]; // gsv_elevation
    TinyGPSCustom gsv_azimuth[4];   // gsv_azimuth
    TinyGPSCustom gsv_snr[4];       // gsv_snr

    // Debugging GPS adatok kiírása
    bool debugGpsSerialData;

    // Debugging a beépített RGB LED-el
    bool debugGpsSerialOnInternalFastLed;

    bool debugGpsSatellitesDatabase;

    // Boot time - a GPS mikor látott érvényes műholdat?
    uint32_t bootStartTime;
    uint32_t gpsBootTime;

    void processGSVMessages();
};
