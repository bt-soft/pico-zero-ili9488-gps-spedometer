#include "GpsManager.h"
#include "Config.h"
#include "Utils.h"
#include "defines.h"

// Belső LED;  PICO: LED_BUILTIN (sima LED), Zero: GP16 (WS2812 RGB LED)
#define INTERNAL_RGB_LED_PIN 16
#define INTERNAL_RGB_LED_BRIGHTNESS 10
#define INTERNAL_RGB_LED_NUM 1

// PICO ZERO WS2812 RGB LED driver
#define FASTLED_INTERNAL
#include <FastLED.h>
#define FASTLED_FORCE_SOFTWARE_SPI
#define FASTLED_FORCE_SOFTWARE_PINS
CRGB leds[INTERNAL_RGB_LED_NUM];
#define INTERNAL_LED_COLOR CRGB::Red // Pirosan villogjon, ha GPS adat érkezik

constexpr uint8_t MAX_SATELLITES = 50;

/**
 * Konstruktor
 */
GpsManager::GpsManager(HardwareSerial &serial)
    : gpsSerial(serial), configCallbackToken(config.registerChangeCallback([this]() { this->onConfigChanged(); })) //
{
    // Kezdeti értékek felvétele
    onConfigChanged();

    // Initialize FastLED for Pico Zero WS2812 RGB LED
    FastLED.addLeds<WS2812, INTERNAL_RGB_LED_PIN, GRB>(leds, INTERNAL_RGB_LED_NUM);
    FastLED.setBrightness(INTERNAL_RGB_LED_BRIGHTNESS); // Set brightness
    FastLED.clear();
    FastLED.show();

    // TinyGPSCustom objektumok inicializálása GSV feldolgozáshoz - fő GSV mezők
    gsv_msg_num.begin(gps, "GPGSV", 1);          // Üzenet sorszáma
    gsv_total_msgs.begin(gps, "GPGSV", 2);       // Üzenetek száma
    gsv_num_sats_in_view.begin(gps, "GPGSV", 3); // Látható műholdak száma

    // TinyGPSCustom objektumok inicializálása GSV feldolgozáshoz
    //    Minden GSV mondat legfeljebb 4 műhold adatait tartalmazza
    //    A mezők: PRN, Eleváció, Azimut, SNR
    //    A mező indexek 0-tól indulnak, tehát $GPGSV,1,2,3,4,5,6,7,...
    //    4. mező: PRN, 5. mező: Eleváció, 6. mező: Azimut, 7. mező: SNR

    // Inicializáljuk az összes TinyGPSCustom objektumot
    for (byte i = 0; i < 4; ++i) {
        gsv_prn[i].begin(gps, "GPGSV", 4 + 4 * i);       // offsets 4, 8, 12, 16
        gsv_elevation[i].begin(gps, "GPGSV", 5 + 4 * i); // offsets 5, 9, 13, 17
        gsv_azimuth[i].begin(gps, "GPGSV", 6 + 4 * i);   // offsets 6, 10, 14, 18
        gsv_snr[i].begin(gps, "GPGSV", 7 + 4 * i);       // offsets 7, 11, 15, 19
    }

    // Ekkor indultunk
    bootStartTime = millis();
    gpsBootTime = 0;
}

/**
 * @brief Callback függvény, amit a Config hív meg változás esetén
 */
void GpsManager::onConfigChanged() {
    // DEBUG("GpsManager::onConfigChanged() - Debug flag-ek frissítése.\n");
    debugGpsSerialOnInternalFastLed = config.data.debugGpsSerialOnInternalFastLed;
    debugGpsSerialData = config.data.debugGpsSerialData;
    debugGpsSatellitesDatabase = config.data.debugGpsSatellitesDatabase;
}

/**
 * Helyi időzóna szerint korrigált dátum és idő lekérdezése (CET/CEST)
 */
GpsManager::LocalDateTime GpsManager::getLocalDateTime() {

    // Visszatérési érték
    LocalDateTime result = {0, 0, 0, false, 0, 0, 0, false};

    // Érvényes GPS időadatok másolása
    if (gps.time.isValid() && gps.time.age() < GPS_DATA_MAX_AGE) {
        // Sanity check the values
        if (gps.time.hour() <= 23 && gps.time.minute() <= 59 && gps.time.second() <= 59) {
            result.hour = gps.time.hour();
            result.minute = gps.time.minute();
            result.second = gps.time.second();
            result.timeValid = true;
        }
    }

    // Érvényes GPS dátumok másolása
    if (gps.date.isValid() && gps.date.age() < GPS_DATA_MAX_AGE) {
        // Sanity check the values
        if (gps.date.year() > 2020 && gps.date.year() < 2100 && gps.date.month() >= 1 && gps.date.month() <= 12 && gps.date.day() >= 1 && gps.date.day() <= 31) {
            result.day = gps.date.day();
            result.month = gps.date.month();
            result.year = gps.date.year();
            result.dateValid = true;
        }
    }

    // Ha érvényesek a GPS adatok, akkor nyári/téli időszámítás korrekciót is végrehajtunk
    if (result.dateValid && result.timeValid) {

        // Nyári/téli időszámítás korrekció
        DaylightSaving::correctTime(result.minute, result.hour, result.day, result.month, result.year);
    }

    return result;
}

/**
 * GPS minőségi szint lekérdezése
 */
String GpsManager::getGpsQualityString() {

    switch (gps.location.FixQuality()) {
        case TinyGPSLocation::Invalid:
            return "Invalid";
        case TinyGPSLocation::GPS:
            return "GPS";
        case TinyGPSLocation::DGPS:
            return "DGPS";
        case TinyGPSLocation::PPS:
            return "PPS";
        case TinyGPSLocation::RTK:
            return "RTK";
        case TinyGPSLocation::FloatRTK:
            return "FloatRTK";
        case TinyGPSLocation::Estimated:
            return "Estimated";
        case TinyGPSLocation::Manual:
            return "Manual";
        case TinyGPSLocation::Simulated:
            return "Simulated";
        default:
            return "Unknown";
    }
}

/**
 * GPS üzemmód lekérdezése
 */
String GpsManager::getGpsModeToString() {
    switch (gps.location.FixMode()) {
        case TinyGPSLocation::N:
            return "No Fix";
        case TinyGPSLocation::A:
            return "Auto 2D/3D";
        case TinyGPSLocation::D:
            return "Differential";
        case TinyGPSLocation::E:
            return "Estimated";
        default:
            return "Unknown";
    }
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

    if (debugGpsSatellitesDatabase) {
        satelliteDb.debugSatDb(num_sats_in_view);
    }
}

/**
 * GPS olvasása
 */
void GpsManager::loop() {

    bool isValidSentence = false;

    while (gpsSerial.available() > 0) {
        char c = gpsSerial.read();
        if (gps.encode(c)) {
            isValidSentence = true;
        }

        // Debug: kiírjuk a GPS soros porton küldött karaktereit
        if (debugGpsSerialData) {
            DEBUG("%c", c);
        }
    }

    // Ha van érvényes GPS NMEA mondat
    if (isValidSentence) {

        // GPS mondatok feldolgozása
        processGSVMessages();

        // GPS boot idő számítása (első érvényes műholdadat)
        if (gpsBootTime == 0 && gps.satellites.isValid() && gps.satellites.age() < GPS_DATA_MAX_AGE && gps.satellites.value() > 0) {
            gpsBootTime = (millis() - bootStartTime) / 1000;
        }

        // beépített RGB LED villogtatása, ha volt érvényes bejövő GPS mondat
        if (debugGpsSerialOnInternalFastLed) {
            leds[0] = INTERNAL_LED_COLOR;
            FastLED.show();

            leds[0] = CRGB::Black;
            FastLED.show();
        }
    }

    // Műhold adatbázis karbantartása másodpercenként
    static long lastWiseSatellitesData = millis();
    if (Utils::timeHasPassed(lastWiseSatellitesData, 1000)) {
        satelliteDb.deleteUntrackedSatellites();
    }
}