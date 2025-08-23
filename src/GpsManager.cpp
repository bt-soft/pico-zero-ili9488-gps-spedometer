#include "GpsManager.h"
#include "Utils.h"
#include "defines.h"

// Belső LED;  PICO: LED_BUILTIN (sima LED), Zero: GP16 (WS2812 RGB LED)
#define LED_PIN 16
// PICO ZERO WS2812 RGB LED driver
#define FASTLED_INTERNAL
#include <FastLED.h>
#define FASTLED_FORCE_SOFTWARE_SPI
#define FASTLED_FORCE_SOFTWARE_PINS
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];
#define INTERNAL_LED_COLOR CRGB::Red // Pirosan villogjon, ha GPS adat érkezik

constexpr uint8_t MAX_SATELLITES = 50;

/**
 * Konstruktor
 */
GpsManager::GpsManager(HardwareSerial *serial) : gpsSerial(serial), debugSerialOnInternalFastLed(true) {

    // Initialize FastLED for Pico Zero WS2812 RGB LED
    FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(50); // Set brightness to 50%
    FastLED.clear();
    FastLED.show();

    // Initialize TinyGPSCustom objects for GSV parsing - main GSV fields
    gsv_msg_num.begin(gps, "GPGSV", 1);          // Message number
    gsv_total_msgs.begin(gps, "GPGSV", 2);       // Total messages
    gsv_num_sats_in_view.begin(gps, "GPGSV", 3); // Number of satellites in view

    // Initialize TinyGPSCustom objects for GSV parsing
    //    Each GSV sentence contains data for up to 4 satellites
    //    The fields are: PRN, Elevation, Azimuth, SNR
    //    Field indices start from 0, so for $GPGSV,1,2,3,4,5,6,7,...
    //    4th field is PRN, 5th is Elevation, 6th is Azimuth, 7th is SNR
    //    For the first satellite in the sentence:
    // Initialize all the uninitialized TinyGPSCustom objects
    for (byte i = 0; i < 4; ++i) {
        gsv_prn[i].begin(gps, "GPGSV", 4 + 4 * i);       // offsets 4, 8, 12, 16
        gsv_elevation[i].begin(gps, "GPGSV", 5 + 4 * i); // offsets 5, 9, 13, 17
        gsv_azimuth[i].begin(gps, "GPGSV", 6 + 4 * i);   // offsets 6, 10, 14, 18
        gsv_snr[i].begin(gps, "GPGSV", 7 + 4 * i);       // offsets 7, 11, 15, 19
    }

    // Ekkon indultunk
    startTime = millis();
    gpsBootTime = 0;
}

/**
 * Read GSV messages from the GPS module
 */
void GpsManager::processGSVMessages() {

    // Check if all GSV messages have been received
    if (!gsv_total_msgs.isValid()) {
        // DEBUG("Not all $GPGSV messages processed yet\n");
        return;
    }

    // Check if this message is a new GSV block
    if (!gsv_msg_num.isUpdated()) {
        return;
    }

    uint8_t msg_num = atoi(gsv_msg_num.value());
    uint8_t total_msgs = atoi(gsv_total_msgs.value());
    uint8_t num_sats_in_view = atoi(gsv_num_sats_in_view.value());

    // Process up to 4 satellites per GSV message
    for (byte i = 0; i < 4; ++i) {
        if (gsv_prn[i].isUpdated() && gsv_prn[i].isValid()) {
            int _prn = atoi(gsv_prn[i].value());

            // ez valami szemét adat?
            if (_prn == 0) {
                continue;
            }

            int _elevation = atoi(gsv_elevation[i].value());
            int _azimuth = atoi(gsv_azimuth[i].value());
            int _snr = atoi(gsv_snr[i].value());

            satelliteDb.insertSatellite(_prn, _elevation, _azimuth, _snr);
        }
    }

    // totalMessages ==  currentMessage ?
    if (total_msgs != msg_num) {
        // DEBUG("Not complete yet\n");
        return;
    }

#ifdef __DEBUG
    satelliteDb.debugSatDb(num_sats_in_view);
#endif
}

/**
 * GPS adatok kiolvasása
 */
void GpsManager::readGPS() {

    // Kiolvassuk és dekódoljuk az összes GPS adatot
    while (gpsSerial->available() > 0) {

        char c = gpsSerial->read();

        if (gps.encode(c)) {

            // LED villogtatása, ha van érvényes bejövő GPS mondat
            if (debugSerialOnInternalFastLed) {
                leds[0] = INTERNAL_LED_COLOR;
                FastLED.show();

                leds[0] = CRGB::Black;
                FastLED.show();
            }
        }

        Serial.print(c); // Debug: kiírjuk a bejövő karaktereket
    }

    // Mikor bootolt be a GPS?
    if (gpsBootTime == 0 && gps.speed.isValid()) {
        gpsBootTime = (millis() - startTime) / 1000;
    }
}

/**
 * GPS olvasás
 */
void GpsManager::loop() {

    // Van GPS adat?
    if (Serial1.available()) {
        readGPS();
        processGSVMessages();
    }

    // Update satellite information
    static long lastWiseSatellitesData = millis();
    if (Utils::timeHasPassed(lastWiseSatellitesData, 1000)) {
        satelliteDb.deleteUntrackedSatellites();
    }
}
