#include <Arduino.h>

#include <TinyGPS++.h>
TinyGPSPlus gps;

#define PICO_ZERO
#include <TFT_eSPI.h>      // Hardware-specific library
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library with default width and height

#include "commons.h"
#include "linearMeter.h"
#include "ringMeter.h"

// Hőmérés
#define DEBUG_DS18B20             // DB18B20 debug
#define PIN_TEMP_SENSOR 7         /* ATmega328P PIN:4, D10 a DS18B20 bemenete */
#define DS18B20_TEMP_SENSOR_NDX 0 /* Dallas DS18B20 hõmérõ szenzor indexe */
#include <OneWire.h>
#define REQUIRESALARMS false /* nem kell a DallasTemperature ALARM supportja */
#include <DallasTemperature.h>
#include <NonBlockingDallas.h>

OneWire oneWire(PIN_TEMP_SENSOR);
DallasTemperature dallasTemp(&oneWire);
NonBlockingDallas nonBlockingDallasTemperatureSensor(&dallasTemp); // Create a new instance of the NonBlockingDallas class
// NonBlockingDallas nonBlockingDallasTemperatureSensor(new DallasTemperature(new OneWire(PIN_TEMP_SENSOR)));

#define PIN_BATTERRY_MEASURE A0
#define AD_RESOLUTION 12
volatile float vBatterry = 0.0f;
volatile float temperature = 0.0f;
int maxSpeed = 0;

#include "DayLightSaving.h"
DaylightSaving dls;

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

// Serial1 piout átdefiniálása a PICO ZERO-hoz
// Meg kell hívni a Serial1.setRX(PIN_SERIAL1_RX_NEW) és a Serial1.setTX(PIN_SERIAL1_TX_NEW)-et a Serial1.begin(9600) előtt
#define PIN_SERIAL1_TX_NEW (12u)
#define PIN_SERIAL1_RX_NEW (13u)

#define __DEBUG_ON_SERIAL__

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

/**
 *
 */
void processTraffiAlarm() {

    static long lastAlarm = millis();

    // Aktív az alarm?
    if (traffiAlarmActive) {

        // De még nem járt le az időzítése?
        if (!timeHasPassed(lastAlarm, ALARM_TIME_MS)) {
            return;
        }

        // Aktív az alarm, és le is járt az időzítése -> töröljük a viewport-ot és az alarm flag-et is
        tft.resetViewport();
        tft.fillRect(100, 20, 280, 60, TFT_BLACK); // töröljük a viewport területét
        traffiAlarmActive = false;
        lastAlarm = millis();
        return;

    } else {
        // Nem aktív az alarm, de meghívtak -> Bekapcsoljuk az alarm flag-et és a viewport-ot is
        tft.setViewport(100, 20, 280, 60, true);
        tft.frameViewport(TFT_RED, 3);             // 3 pixel keret a viewport-on belül
        tft.fillRect(100, 20, 280, 60, TFT_BLACK); // töröljük a viewport területét
        traffiAlarmActive = true;
    }

    // Alarm aktualizálása
    tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW, TFT_RED);
    tft.drawString("TRAFFIPAX ALARM!!!", 150, 30, 2);
}

/**
 * Fejléc feliratok
 */
void displayHeaderText() {

    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

#define HEADER_TEXT_Y 6
    tft.drawString("Time/Date", 250, HEADER_TEXT_Y, 2);
    tft.drawString("Altitude", 455, HEADER_TEXT_Y, 2);
    tft.drawString("Hdop", 75, 100, 2);
    tft.drawString("Max Speed", 400, 100, 2);
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

    if (!traffiAlarmActive) {
        // Idő
        if (gps.time.isValid() && gps.time.age() < GPS_DATA_MAX_AGE) {
            int hours = gps.time.hour();
            int mins = gps.time.minute();
            dls.correctTime(mins, hours, gps.date.day(), gps.date.month(), gps.date.year());
            sprintf(buf, "%02d:%02d", hours, mins);
            tft.setTextSize(1);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextPadding(tft.textWidth(buf, 6));
            tft.drawString(buf, 250, 45, 6);
        }

        // Dátum
        if (gps.date.isValid() && gps.date.age() < GPS_DATA_MAX_AGE) {
            sprintf(buf, "%04d-%02d-%02d", gps.date.year(), gps.date.month(), gps.date.day());
            tft.setTextSize(1);
            tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
            tft.setTextPadding(tft.textWidth(buf, 2));
            tft.drawString(buf, 250, 70, 2);
        }
    }

    // Hdop
    double hdop = gps.satellites.isValid() && gps.hdop.age() < GPS_DATA_MAX_AGE ? gps.hdop.hdop() : 0;
    sprintf(buf, "%.2f", hdop);
    tft.setTextPadding(5 * 14);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(buf, 75, 120, 4);

    // Sebesség
    int speedValue = gps.speed.isValid() && gps.speed.age() < GPS_DATA_MAX_AGE && gps.speed.kmph() >= 4 ? gps.speed.kmph() : 0;
    ringMeter(&tft,
              speedValue,                                 // speedValue,                                 // current value
              0,                                          // minValue
              SPEED_RINGMETER_MAX_VALUE,                  // maxValue
              (tft.width() / 2 - SPEED_RINGMETER_RADIUS), // x
              100,                                        // y
              SPEED_RINGMETER_RADIUS,                     // radius
              230,                                        // angle
              true,                                       // coloredValue
              "km/h",                                     // felirat
              GREEN2RED);                                 // scheme

    // Max Speed
    maxSpeed = MAX(maxSpeed, speedValue);
    sprintf(buf, "%3d", maxSpeed);
    tft.setTextPadding(3 * 14);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(buf, 400, 120, 4);

#define VERTICAL_BARS_Y 290
    // Vertical Line bar - Batterry
    verticalLinearMeter(&tft,
                        "Batt [V]",        // category
                        ::vBatterry,       // val
                        BATT_BARMETER_MIN, // minVal
                        BATT_BARMETER_MAX, // maxVal
                        0,                 // x
                        VERTICAL_BARS_Y,   // bottom-left-y
                        30,                // bar-w
                        10,                // bar-h
                        2,                 // gap
                        10,                // n
                        BLUE2RED);         // color

    // Vertical Line bar - temperature
    verticalLinearMeter(&tft,
                        "Temp [C]",        // category
                        ::temperature,     // val
                        TEMP_BARMETER_MIN, // minVal
                        TEMP_BARMETER_MAX, // maxVal
                        tft.width() - 30,  // x = maxX - bar-w
                        VERTICAL_BARS_Y,   // bottom-left-y
                        30,                // bar-w
                        10,                // bar-h
                        2,                 // gap
                        10,                // n
                        BLUE2RED,          // color
                        true);             // bal oldalt legyenek az értékek

    // // Traffipax alarm aktív?
    // if (traffiAlarmActive) {
    //     processTraffiAlarm();
    // }

    // // Test alarm 30mp-enként
    // static long testTrafiAlarm = millis();
    // if (timeHasPassed(testTrafiAlarm, 30000)) {
    //     processTraffiAlarm();
    //     testTrafiAlarm = millis();
    // }
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

    // TFT
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // Non-blocking Dallas temperature sensor
    nonBlockingDallasTemperatureSensor.begin(NonBlockingDallas::resolution_12, 1500);
    // Non-blocking Dallas temperature sensor hőmérséklet változás callback
    nonBlockingDallasTemperatureSensor.onTemperatureChange(handleTemperatureChange);
    // Azonnal le is kérjük a hőmérsékletet
    nonBlockingDallasTemperatureSensor.requestTemperature();

    displayHeaderText();
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