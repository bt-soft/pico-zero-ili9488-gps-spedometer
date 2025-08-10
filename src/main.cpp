#include <Arduino.h>
#include <Streaming.h>

#include <TinyGPS++.h>
TinyGPSPlus gps;

#include <TFT_eSPI.h>      // Hardware-specific library
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library with default width and height

#include "Large_Font.h"
#include "Utils.h"
#include "commons.h"
#include "linearMeter.h"
#include "pins.h"
#include "ringMeter.h"

// Hőmérés
#define DEBUG_DS18B20             // DB18B20 debug
#define DS18B20_TEMP_SENSOR_NDX 0 /* Dallas DS18B20 hõmérõ szenzor indexe */
#include <OneWire.h>
#define REQUIRESALARMS false /* nem kell a DallasTemperature ALARM supportja */
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>

OneWire oneWire(PIN_TEMP_SENSOR);
DallasTemperature dallasTemp(&oneWire);
NonBlockingDallas nonBlockingDallasTemperatureSensor(&dallasTemp); // Create a new instance of the NonBlockingDallas class
// NonBlockingDallas nonBlockingDallasTemperatureSensor(new DallasTemperature(new OneWire(PIN_TEMP_SENSOR)));

#define AD_RESOLUTION 12
volatile float vBatterry = 0.0f;
volatile float temperature = 0.0f;
int maxSpeed = 0;

#include "DayLightSaving.h"
DaylightSaving dls;

#include "TftBackLightAdjuster.h"
TftBackLightAdjuster tftBackLightAdjuster;

#include "TafipaxList.h"
TafipaxList tafipax; // Automatikusan betölti a CSV-t

// GPS Mutex
auto_init_mutex(_gpsMutex);

// Belső LED;  PICO: LED_BUILTIN (sima LED), Zero: GP16 (WS2812 RGB LED)
#define LED_PIN 16
// PICO ZERO WS2812 RGB LED driver
#define FASTLED_INTERNAL
#include <FastLED.h>
#define FASTLED_FORCE_SOFTWARE_SPI
#define FASTLED_FORCE_SOFTWARE_PINS
#define NUM_LEDS 1
CRGB leds[NUM_LEDS];
#define INTERNAL_LED_COLOR CRGB::Green; // Zölden villogjon, ha GPS adat érkezik

#define __DEBUG_ON_SERIAL__

// Sprite méretek kiszámítása (fix, hogy mindig ugyanakkora legyen, így csak egyszer kell allokálni)
constexpr int SPRITE_VERTICAL_LINEAR_METER_HEIGHT = 10 * (10 + 2) + 40; // max n=10, h=10, g=2
constexpr int SPRITE_VERTICAL_LINEAR_METER_WIDTH = 70;

// Sprite a vertikális bar-oknak
TFT_eSprite spriteVerticalLinearMeter(&tft);

//---------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * Eltelt már annyi idő?
 */
bool timeHasPassed(long fromWhen, int howLong) { return millis() - fromWhen >= howLong; }

//---------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------
//
//      CORE - 0
//
//---------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------

bool traffiAlarmActive = false;

// Intelligens trafipax figyelmeztető rendszer
struct TrafipaxAlert {
    enum State {
        INACTIVE,       // Nincs aktív riasztás
        APPROACHING,    // Közeledik - piros háttér + szirénázás
        NEARBY_STOPPED, // Közel van, de megállt - piros háttér, nincs szirénázás
        DEPARTING       // Távolodik - narancssárga háttér, nincs szirénázás
    };

    State currentState = INACTIVE;
    const TafipaxInternal *activeTrafipax = nullptr;
    double currentDistance = 0.0;
    double lastDistance = 999999.0;
    unsigned long lastStateChange = 0;
    unsigned long lastSirenTime = 0;
    bool wasApproachingLastCheck = false;

    static constexpr double CRITICAL_DISTANCE = 800.0;     // 800m kritikus távolság
    static constexpr unsigned long SIREN_INTERVAL = 10000; // 10 sec szirénázási intervallum
    static constexpr unsigned long MIN_STATE_TIME = 2000;  // Min 2 sec egy állapotban
};

TrafipaxAlert trafipaxAlert;

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
    static constexpr unsigned long PHASE_WAIT = 10;     // 10mp várakozás
    static constexpr unsigned long PHASE_APPROACH = 20; // 10mp közeledés (10-20mp)
    static constexpr unsigned long PHASE_DEPART = 30;   // 10mp távolodás (20-30mp)
    static constexpr unsigned long PHASE_END = 35;      // 5mp befejezés (30-35mp)

    // Litéri trafipax koordinátái
    static constexpr double LITERI_LAT = 47.100934;
    static constexpr double LITERI_LON = 18.011792;
};

TrafipaxDemo trafipaxDemo;

/**
 * Állandó feliratok kirajzolása feliratok
 */
void drawStaticLabels() {

    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    tft.drawString("Time/Date", 250, 6, 2);
    tft.drawString("Altitude", 455, 6, 2);
    tft.drawString("Hdop", 40, 90, 2);
    tft.drawString("Max Speed", 440, 90, 2);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("km/h", tft.width() / 2, 110, 2);
}

/**
 * Trafipax figyelmeztető sáv megjelenítése
 */
void displayTrafipaxAlert(const TafipaxInternal *trafipax, double distance) {
    if (trafipax == nullptr)
        return;

    // Háttérszín meghatározása állapot szerint
    uint16_t backgroundColor;
    uint16_t textColor;
    switch (trafipaxAlert.currentState) {
        case TrafipaxAlert::APPROACHING:
        case TrafipaxAlert::NEARBY_STOPPED:
            backgroundColor = TFT_RED;
            textColor = TFT_WHITE;
            break;
        case TrafipaxAlert::DEPARTING:
            backgroundColor = TFT_ORANGE;
            textColor = TFT_BLACK;
            break;
        default:
            return; // INACTIVE esetén nem rajzolunk semmit
    }

    // Figyelmeztető sáv háttere (nagyobb sáv)
    constexpr int ALERT_BAR_HEIGHT = 45;
    tft.fillRect(0, 0, tft.width(), ALERT_BAR_HEIGHT, backgroundColor);

    // Szöveg összeállítása
    char alertText[100];
    snprintf(alertText, sizeof(alertText), "%s, %s - %dm", trafipax->city, trafipax->street_or_km, (int)distance);

    // Ékezetes karakterek ASCII-ra konvertálása
    Utils::convertToASCII(alertText);

    // Szöveg megjelenítése nagyobb fonttal
    tft.setTextSize(1);
    tft.setFreeFont(&FreeSansBold18pt7b); // nagyobb font
    tft.setTextColor(textColor, backgroundColor);
    tft.setTextDatum(MC_DATUM);

    // Szöveg középre igazítása
    int textWidth = tft.textWidth(alertText);
    int textX = tft.width() / 2;
    int textY = ALERT_BAR_HEIGHT / 2 + 2; // függőlegesen középre

    // Ha túl hosszú a szöveg, rövidítjük
    if (textWidth > tft.width() - 10) {
        snprintf(alertText, sizeof(alertText), "%s - %.0fm", trafipax->city, distance);
        Utils::convertToASCII(alertText);
    }

    tft.drawString(alertText, textX, textY);
    tft.setFreeFont();
}

/**
 * Távolság számítás Haversine formulával (GPS koordináták között)
 */
double calculateGPSDistance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000; // Föld sugara méterben
    double dLat = (lat2 - lat1) * PI / 180.0;
    double dLon = (lon2 - lon1) * PI / 180.0;
    double a = sin(dLat / 2) * sin(dLat / 2) + cos(lat1 * PI / 180.0) * cos(lat2 * PI / 180.0) * sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

/**
 * Demo indítása - 10mp várakozás, majd közeledés/távolodás szimulálása
 */
void startTrafipaxDemo() {
    trafipaxDemo.isActive = true;
    trafipaxDemo.startTime = millis();
    trafipaxDemo.currentPhase = 0;

    DEBUG("\n=== TRAFIPAX DEMO INDÍTVA ===\n");
    DEBUG("Teszt fázisok:\n");
    DEBUG("0-10mp: Várakozás (nincs riasztás)\n");
    DEBUG("10-20mp: Közeledés a litéri trafipaxhoz\n");
    DEBUG("20-30mp: Távolodás a litéri trafipaxtól\n");
    DEBUG("30-35mp: Demo befejezése\n");
    DEBUG("------------------------------------------\n\n");
}

/**
 * Demo feldolgozása - szimulált GPS koordináták generálása
 */
void processTrafipaxDemo() {
    if (!trafipaxDemo.isActive) {
        return;
    }

    unsigned long elapsed = (millis() - trafipaxDemo.startTime) / 1000; // másodpercek

    // Demo befejezése
    if (elapsed >= TrafipaxDemo::PHASE_END) {
        trafipaxDemo.isActive = false;
        trafipaxDemo.hasValidCoords = false;
        DEBUG("=== DEMO BEFEJEZVE ===\n");
        return;
    }

    // Szimulált GPS koordináták generálása a fázis alapján
    double simLat, simLon;

    if (elapsed < TrafipaxDemo::PHASE_WAIT) {
        // Várakozási fázis - messze vagyunk (1500m)
        simLat = TrafipaxDemo::LITERI_LAT - 0.0135; // kb. 1500m délre
        simLon = TrafipaxDemo::LITERI_LON;

        if (elapsed != trafipaxDemo.currentPhase) {
            trafipaxDemo.currentPhase = elapsed;
            DEBUG("Demo fázis: Várakozás (%lus/10s)\n", elapsed);
        }

    } else if (elapsed < TrafipaxDemo::PHASE_APPROACH) {
        // Közeledési fázis - 1500m-ről 200m-ig
        float progress = (elapsed - TrafipaxDemo::PHASE_WAIT) / 10.0f; // 0.0 - 1.0
        simLat = TrafipaxDemo::LITERI_LAT - 0.0135 + (0.0135 - 0.0018) * progress;
        simLon = TrafipaxDemo::LITERI_LON;

        if (elapsed != trafipaxDemo.currentPhase) {
            trafipaxDemo.currentPhase = elapsed;
            double distance = calculateGPSDistance(simLat, simLon, TrafipaxDemo::LITERI_LAT, TrafipaxDemo::LITERI_LON);
            DEBUG("Demo fázis: Közeledés (%lus/20s) - %dm\n", elapsed, (int)distance);
        }

    } else if (elapsed < TrafipaxDemo::PHASE_DEPART) {
        // Távolodási fázis - 200m-ről 1500m-ig
        float progress = (elapsed - TrafipaxDemo::PHASE_APPROACH) / 10.0f; // 0.0 - 1.0
        simLat = TrafipaxDemo::LITERI_LAT - 0.0018 - (0.0135 - 0.0018) * progress;
        simLon = TrafipaxDemo::LITERI_LON;

        if (elapsed != trafipaxDemo.currentPhase) {
            trafipaxDemo.currentPhase = elapsed;
            // Távolság számítás a TafipaxList-en keresztül
            double distance;
            const TafipaxInternal *closest = tafipax.getClosestTrafipax(simLat, simLon, distance);
            if (closest) {
                DEBUG("Demo fázis: Távolodás (%lus/30s) - %dm\n", elapsed, (int)distance);
            }
        }

    } else {
        // Befejező fázis - messze vagyunk
        simLat = TrafipaxDemo::LITERI_LAT - 0.0135;
        simLon = TrafipaxDemo::LITERI_LON;

        if (elapsed != trafipaxDemo.currentPhase) {
            trafipaxDemo.currentPhase = elapsed;
            DEBUG("Demo fázis: Befejezés (%lus/35s)\n", elapsed);
        }
    }

    // Demo koordináták tárolása
    trafipaxDemo.currentLat = simLat;
    trafipaxDemo.currentLon = simLon;
    trafipaxDemo.hasValidCoords = true;

    // GPS koordináták beállítása a demo számára (demo override a processIntelligentTrafipaxAlert-ben)
    // A demo koordinátákat a processTrafipaxDemo-ban tároljuk és a processIntelligentTrafipaxAlert-ben használjuk
}

/**
 * Intelligens trafipax figyelmeztető rendszer
 * - 800m-en belül: piros sáv + város/utca + távolság
 * - Közeledés esetén: 10mp-enként szirénázás
 * - Megállás esetén: nincs szirénázás, csak a piros sáv
 * - Távolodás esetén: narancssárga sáv, nincs szirénázás
 *
 * Állapotok:
 * - INACTIVE: Nincs közeli trafipax (> 800m)
 * - APPROACHING: Közeledik (piros háttér + szirénázás)
 * - NEARBY_STOPPED: Megállt közel (piros háttér, nincs szirénázás)
 * - DEPARTING: Távolodik (narancssárga háttér, nincs szirénázás)
 */
void processIntelligentTrafipaxAlert() {

    double currentLat, currentLon;
    bool hasValidData = false;

    // Demo mód ellenőrzése
    if (trafipaxDemo.isActive && trafipaxDemo.hasValidCoords) {
        // Demo koordináták használata
        currentLat = trafipaxDemo.currentLat;
        currentLon = trafipaxDemo.currentLon;
        hasValidData = true;
    } else {
        // Valós GPS adatok használata
        if (gps.location.isValid() && gps.location.age() < GPS_DATA_MAX_AGE) {
            currentLat = gps.location.lat();
            currentLon = gps.location.lng();
            hasValidData = true;
        }
    }

    // Nincs érvényes GPS adat
    if (!hasValidData) {
        if (trafipaxAlert.currentState != TrafipaxAlert::INACTIVE) {
            trafipaxAlert.currentState = TrafipaxAlert::INACTIVE;
            trafipaxAlert.activeTrafipax = nullptr;
            traffiAlarmActive = false; // Riasztás kikapcsolása
            // Töröljük a figyelmeztető sávot
            constexpr int ALERT_BAR_HEIGHT = 45;
            tft.fillRect(0, 0, tft.width(), ALERT_BAR_HEIGHT, TFT_BLACK);
            drawStaticLabels(); // Újrarajzoljuk a feliratokat
        }
        return;
    }

    // Legközelebbi trafipax keresése
    double minDistance = 999999.0;
    const TafipaxInternal *closestTrafipax = tafipax.getClosestTrafipax(currentLat, currentLon, minDistance);

    unsigned long currentTime = millis();

    // Ha nincs közeli trafipax a kritikus távolságon belül
    if (minDistance > TrafipaxAlert::CRITICAL_DISTANCE) {
        if (trafipaxAlert.currentState != TrafipaxAlert::INACTIVE) {
            trafipaxAlert.currentState = TrafipaxAlert::INACTIVE;
            trafipaxAlert.activeTrafipax = nullptr;
            traffiAlarmActive = false; // Riasztás kikapcsolása
            // Töröljük a figyelmeztető sávot
            constexpr int ALERT_BAR_HEIGHT = 45;
            tft.fillRect(0, 0, tft.width(), ALERT_BAR_HEIGHT, TFT_BLACK);
            drawStaticLabels(); // Újrarajzoljuk a feliratokat
        }
        return;
    }

    // Van közeli trafipax - állapot meghatározása
    bool isApproaching = minDistance < trafipaxAlert.lastDistance;
    bool isStopped = abs(minDistance - trafipaxAlert.lastDistance) < 5.0; // 5m tolerancia "megálláshoz"

    TrafipaxAlert::State newState = trafipaxAlert.currentState;

    if (isApproaching && !isStopped) {
        newState = TrafipaxAlert::APPROACHING;
    } else if (isStopped) {
        newState = TrafipaxAlert::NEARBY_STOPPED;
    } else {
        newState = TrafipaxAlert::DEPARTING;
    }

    // Állapotváltás detektálása
    if (newState != trafipaxAlert.currentState) {
        trafipaxAlert.currentState = newState;
        trafipaxAlert.lastStateChange = currentTime;
        trafipaxAlert.activeTrafipax = closestTrafipax;
    }

    // Figyelmeztető sáv megjelenítése
    traffiAlarmActive = true;
    displayTrafipaxAlert(closestTrafipax, minDistance);

    // Szirénázás csak közeledés esetén, 10mp-enként
    if (trafipaxAlert.currentState == TrafipaxAlert::APPROACHING) {
        if (currentTime - trafipaxAlert.lastSirenTime >= TrafipaxAlert::SIREN_INTERVAL) {
            Utils::beepSiren(2, 600, 1800, 20, 8, 100);
            trafipaxAlert.lastSirenTime = currentTime;
        }
    }

    // Távolság frissítése a következő összehasonlításhoz
    trafipaxAlert.lastDistance = minDistance;
    trafipaxAlert.currentDistance = minDistance;
}

/**
 * Értékek kiírása
 */
void displayValues() {

    // Lockolunk egyet
    CoreMutex m(&_gpsMutex);

    // Ha nem sikerül a lock, akkor nem megyünk tovább
    if (!m) {
        return;
    }

    char buf[11];

    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // Aktuális sebesség
    int speedValue = gps.speed.isValid() && gps.speed.age() < GPS_DATA_MAX_AGE && gps.speed.kmph() >= 4 ? gps.speed.kmph() : 0;

    // Ha a riasztás nem aktív
    if (!traffiAlarmActive) {
        // Műholdak száma
        short sats = gps.satellites.isValid() && gps.satellites.age() < GPS_DATA_MAX_AGE ? gps.satellites.value() : 0;
        ringMeter(&tft, sats,
                  SATS_RINGETER_MIN, // min
                  SATS_RINGETER_MAX, // max
                  0,                 // xpos
                  8,                 // ypos
                  40,                // radius
                  230,               // angle
                  true,              // coloredValue
                  "Sats",            // Text
                  RED2GREEN);

        // Magasság
        int alt = gps.satellites.isValid() && gps.altitude.age() < GPS_DATA_MAX_AGE ? gps.altitude.meters() : 0;
        sprintf(buf, "%4d", alt);
        tft.setTextPadding(14 * 4);
        tft.drawString(buf, 450, 30, 4);

        // Idő
        if (gps.time.isValid() && gps.time.age() < GPS_DATA_MAX_AGE) {
            int hours = gps.time.hour();
            int mins = gps.time.minute();
            dls.correctTime(mins, hours, gps.date.day(), gps.date.month(), gps.date.year());
            sprintf(buf, "%02d:%02d", hours, mins);
            tft.setTextSize(1);
            tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
            tft.setTextPadding(tft.textWidth(buf, 6));
            tft.drawString(buf, 250, 45, 6);
        }

        // Dátum
        if (gps.date.isValid() && gps.date.age() < GPS_DATA_MAX_AGE) {
            sprintf(buf, "%04d-%02d-%02d", gps.date.year(), gps.date.month(), gps.date.day());
            tft.setTextSize(1);
            tft.setTextColor(TFT_ORANGE, TFT_BLACK);
            tft.setTextPadding(tft.textWidth(buf, 2));
            tft.drawString(buf, 250, 70, 2);
        }

        // Hdop
        double hdop = gps.satellites.isValid() && gps.hdop.age() < GPS_DATA_MAX_AGE ? gps.hdop.hdop() : 0;
        dtostrf(hdop, 0, 2, buf);
        tft.setTextPadding(5 * 14);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(buf, 40, 110, 4);

        // Max Speed
        maxSpeed = MAX(maxSpeed, speedValue);
        sprintf(buf, "%3d", maxSpeed);
        tft.setTextPadding(3 * 14);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(buf, 440, 110, 4);
    }

    // Sebesség - Ringmeter
    // ringMeter(&tft,
    //           speedValue,                                 // speedValue,                                 // current value
    //           0,                                          // minValue
    //           SPEED_RINGMETER_MAX_VALUE,                  // maxValue
    //           (tft.width() / 2 - SPEED_RINGMETER_RADIUS), // x
    //           100,                                        // y
    //           SPEED_RINGMETER_RADIUS,                     // radius
    //           230,                                        // angle
    //           true,                                       // coloredValue
    //           "km/h",                                     // felirat
    //           GREEN2RED);                                 // scheme
    //
    // speedValue = random(0, 288);

    // Sebesség - MegaFont méretekkel
    dtostrf(speedValue, 0, 0, buf);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM); // vízszintes közép
    tft.loadFont(Arial_Narrow_Bold120);
    tft.setTextPadding(tft.textWidth("288"));
    tft.drawString(buf, tft.width() / 2, 240);
    tft.unloadFont();

    // -- Vertikális bar komponensek
    // Sprite legyártása, ha még nem létezik
    if (!spriteVerticalLinearMeter.created()) {
        spriteVerticalLinearMeter.createSprite(SPRITE_VERTICAL_LINEAR_METER_WIDTH, SPRITE_VERTICAL_LINEAR_METER_HEIGHT);
    }

#define VERTICAL_BARS_Y 290
    // Vertical Line bar - Batterry (sprite-os)
    verticalLinearMeter(&spriteVerticalLinearMeter, SPRITE_VERTICAL_LINEAR_METER_HEIGHT, SPRITE_VERTICAL_LINEAR_METER_WIDTH,
                        "Batt [V]",           // category
                        ::vBatterry,          // val
                        BATT_BARMETER_MIN,    // minVal
                        BATT_BARMETER_MAX,    // maxVal
                        0,                    // x
                        VERTICAL_BARS_Y + 10, // y: sprite alsó éle, +10 hogy ne lógjon le
                        30,                   // bar-w
                        10,                   // bar-h
                        2,                    // gap
                        10,                   // n
                        BLUE2RED);            // color

    // Vertical Line bar - temperature (sprite-os)
    verticalLinearMeter(&spriteVerticalLinearMeter, SPRITE_VERTICAL_LINEAR_METER_HEIGHT, SPRITE_VERTICAL_LINEAR_METER_WIDTH,
                        "Temp [C]",                                       // category
                        ::temperature,                                    // val
                        TEMP_BARMETER_MIN,                                // minVal
                        TEMP_BARMETER_MAX,                                // maxVal
                        tft.width() - SPRITE_VERTICAL_LINEAR_METER_WIDTH, // x: sprite szélesség beszámítva
                        VERTICAL_BARS_Y + 10,                             // y: sprite alsó éle, +10 hogy ne lógjon le
                        30,                                               // bar-w
                        10,                                               // bar-h
                        2,                                                // gap
                        10,                                               // n
                        BLUE2RED,                                         // color
                        true);                                            // bal oldalt legyenek az értékek
}

/**
 * @brief callback function a hőmérséklet változás kezelésére
 * CSAK akkor hívódik meg, ha a hőmérséklet két ÉRVÉNYES szenzorleolvasás között megváltozik.
 * @param deviceIndex A szenzor eszköz indexe
 * @param temperatureRAW A nyers hőmérséklet érték
 */
void handleTemperatureChange(int deviceIndex, int32_t temperatureRAW) {
    //
    ::temperature = nonBlockingDallasTemperatureSensor.rawToCelsius(temperatureRAW);
}

/**
 * Core-0 Setup
 */
void setup(void) {
#ifdef __DEBUG_ON_SERIAL__
    Serial.begin(115200);
#endif

    // Beeper
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);

    // TFT
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // TFT háttérvilágítás beállítása
    tftBackLightAdjuster.begin();

    // LittleFS filesystem indítása
    LittleFS.begin();

#ifdef DEBUG_WAIT_FOR_SERIAL
    Utils::debugWaitForSerial(tft);
#endif

    // Non-blocking Dallas temperature sensor
    nonBlockingDallasTemperatureSensor.begin(NonBlockingDallas::resolution_12, 1500);
    // Non-blocking Dallas temperature sensor hőmérséklet változás callback
    nonBlockingDallasTemperatureSensor.onTemperatureChange(handleTemperatureChange);
    // Azonnal le is kérjük a hőmérsékletet
    nonBlockingDallasTemperatureSensor.requestTemperature();

    // Kirajzoljuk az állandó feliratokat
    drawStaticLabels();

    // Trafipax adatok betöltése CSV-ből
    if (tafipax.checkFile(TafipaxList::CSV_FILE_NAME)) {
        tafipax.loadFromCSV(TafipaxList::CSV_FILE_NAME);
    }
    DEBUG("Tafipaxok száma: %d\n", tafipax.count());

    // Pittyentünk egyet, hogy üzemkészek vagyunk
    Utils::beepTick();

    // Valós idejű demo indítása (10mp várakozás után közeledés/távolodás)
    startTrafipaxDemo();

    // Közeledés-távolodás teszt - aktív
    // tafipax.testLiteriTrafipaxApproach();
}

/**
 * Core-0 Loop
 */
void loop() {

    // Hőmérséklet frissítése
    nonBlockingDallasTemperatureSensor.update();

    // Értékek kiírása
    static long lastDisplay = millis() - 1000;
    if (timeHasPassed(lastDisplay, 1000)) {
        displayValues();
        lastDisplay = millis();
    }

    // Demo trafipax közeledés/távolodás (ha aktív)
    if (trafipaxDemo.isActive) {
        processTrafipaxDemo();
    }

    // Intelligens trafipax figyelmeztető rendszer (másodpercenként)
    static long lastTrafipaxCheck = millis() - 1000;
    if (timeHasPassed(lastTrafipaxCheck, 1000)) {
        processIntelligentTrafipaxAlert();
        lastTrafipaxCheck = millis();
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------
//
//      CORE - 1
//
//---------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * GPS adatok kiolvasása
 */
void readGPS() {

    // Kiolvassuk és dekódoljuk az összes GPS adatot
    while (Serial1.available() > 0) {
        char c = Serial1.read();

        // LED villogtatása, ha van érvéynes bejövő GPS mondat
        if (gps.encode(c)) {
            leds[0] = INTERNAL_LED_COLOR;
            FastLED.show();

            leds[0] = CRGB::Black;
            FastLED.show();
        }
    }
}

/**
 * Akkumulátor feszültség mérése
 */
float readBatterry() {
#define V_REFERENCE 3.3f
#define EXTERNAL_VBUSDIVIDER_RATIO ((22.0f + 4.69f) / 4.69f) // A feszültségosztó ellenállások értéke
#define CONVERSION_FACTOR (1 << AD_RESOLUTION)

    float voltageOut = (analogRead(PIN_BATTERRY_MEASURE) * V_REFERENCE) / CONVERSION_FACTOR;
    float vBusExtVoltage = voltageOut * EXTERNAL_VBUSDIVIDER_RATIO;
    // Serial << "Vout: " << vBusExtVoltage << endl;

    return vBusExtVoltage;
}
/**
 *
 */
void readSensorValues() {

    // Lockolunk egyet
    CoreMutex m(&_gpsMutex);

    // Ha nem sikerül a lock, akkor nem megyünk tovább
    if (!m) {
        return;
    }

    // GPS adatok olvasása
    readGPS();

    static long lastReadSensors = millis() - 1000;
    if (timeHasPassed(lastReadSensors, 1000)) {
        vBatterry = readBatterry();
        lastReadSensors = millis();
    }

    // Háttérvilágitás bazseválása
    tftBackLightAdjuster.adjust();
}

/**
 * Core-1 Setup
 */
void setup1(void) {
    // GPS Serial
    Serial1.setRX(PIN_SERIAL1_RX_NEW);
    Serial1.setTX(PIN_SERIAL1_TX_NEW);
    Serial1.begin(9600);

    // initialize digital pin LED_BUILTIN as an output.
    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(50);

    // AD felbontás beállítása a feszültségméréshez
    analogReadResolution(AD_RESOLUTION);
}

/**
 * Core-1 - GPS process
 */
void loop1(void) {

    // Van GPS adat?
    if (Serial1.available()) {
        readSensorValues();
    }
}