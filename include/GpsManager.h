#ifndef __GPS_MANAGER_H
#define __GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPS++.h>

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
    void setLedDebug(bool state) { debugSerialOnInternalFastLed = state; }

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

    // Debugging a beépített RGB LED-el
    bool debugSerialOnInternalFastLed;

    unsigned long startTime;
    unsigned long gpsBootTime;

    void processGSVMessages();
    void readGPS();
};

#endif // __GPS_MANAGER_H