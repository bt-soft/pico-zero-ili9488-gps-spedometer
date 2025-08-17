#include <Arduino.h>
#include <Streaming.h>

#include <TinyGPS++.h>
TinyGPSPlus gps;

#include <TFT_eSPI.h>      // Hardware-specific library
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library with default width and height

//-------------------- Config
#include "Config.h"

#include "Large_Font.h"
#include "Utils.h"
#include "defines.h"
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

#define AD_RESOLUTION 12
volatile float vBatterry = 0.0f;
volatile float temperature = 0.0f;
int maxSpeed = 0;

#include "DayLightSaving.h"
#include "TftBackLightAdjuster.h"
TftBackLightAdjuster tftBackLightAdjuster;

#include "TrafipaxManager.h"
TrafipaxManager trafipaxManager; // Automatikusan betölti a CSV-t

// Optimalizált konstansok
constexpr int ALERT_BAR_HEIGHT = 80;
constexpr int SPRITE_VERTICAL_LINEAR_METER_HEIGHT = 10 * (10 + 2) + 40; // max n=10, h=10, g=2
constexpr int SPRITE_VERTICAL_LINEAR_METER_WIDTH = 70;
constexpr int ALERT_TEXT_PADDING = 20; // Újrahasznosítható konstans

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

// Sprite a vertikális bar-oknak
TFT_eSprite spriteVerticalLinearMeter(&tft);
TFT_eSprite spriteAlertBar(&tft);

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
    const TrafipaxInternal *activeTrafipax = nullptr;
    double currentDistance = 0.0;
    double lastDistance = 999999.0;
    unsigned long lastSirenTime = 0;
    unsigned long lastStateChange = 0;

    static constexpr unsigned long SIREN_INTERVAL = 10000; // 10 sec szirénázási intervallum
};

TrafipaxAlert trafipaxAlert;

/**
 * Állandó feliratok kirajzolása feliratok
 */
void drawStaticLabels() {

    tft.fillScreen(TFT_BLACK);

    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    tft.drawString("Time/Date", 250, 6, 2);
    tft.drawString("Altitude", 455, 6, 2);
    tft.drawString("Hdop", 40, 90, 2);
    tft.drawString("Max Speed", 440, 90, 2);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("km/h", tft.width() / 2, 105, 2);
}

/**
 * Alert bar törlése
 */
void clearAlertBar() {
    spriteAlertBar.fillSprite(TFT_BLACK);
    spriteAlertBar.pushSprite(0, 0);
}

/**
 * Trafipax figyelmeztető sáv megjelenítése sprite-tal és villogással
 */
void displayTrafipaxAlert(const TrafipaxInternal *trafipax, double distance) {
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
            return; // INACTIVE
    }

    spriteAlertBar.fillSprite(backgroundColor);
    spriteAlertBar.setFreeFont(&FreeSansBold18pt7b);
    spriteAlertBar.setTextColor(textColor, backgroundColor);

    // Város (első sor, balra igazítva)
    spriteAlertBar.setTextDatum(TL_DATUM);
    char cityText[MAX_CITY_LEN];
    strncpy(cityText, trafipax->city, MAX_CITY_LEN);
    Utils::convertToASCII(cityText);
    spriteAlertBar.drawString(cityText, ALERT_TEXT_PADDING, 10);

    // Utca/km (második sor, balra igazítva)
    char streetText[MAX_STREET_LEN];
    strncpy(streetText, trafipax->street_or_km, MAX_STREET_LEN);
    Utils::convertToASCII(streetText);
    spriteAlertBar.drawString(streetText, ALERT_TEXT_PADDING, 45);

    // Távolság (jobbra, vertikálisan középre)
    spriteAlertBar.setTextDatum(MR_DATUM);
    char distanceText[16];
    snprintf(distanceText, sizeof(distanceText), "%dm", (int)distance);
    Utils::convertToASCII(distanceText);
    spriteAlertBar.drawString(distanceText, tft.width() - ALERT_TEXT_PADDING, ALERT_BAR_HEIGHT / 2);

    spriteAlertBar.unloadFont();
    spriteAlertBar.pushSprite(0, 0);
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

    // GPS adatok validálása - optimalizált verzió
    if (trafipaxManager.getDemoCoords(currentLat, currentLon)) {
        // Demo koordináták használata
        hasValidData = true;
    } else if (gps.location.isValid() && gps.location.age() < GPS_DATA_MAX_AGE) {
        // Valós GPS adatok használata - egy feltétel
        currentLat = gps.location.lat();
        currentLon = gps.location.lng();
        hasValidData = true;
    }

    // Nincs érvényes GPS adat - riasztás kikapcsolása
    if (!hasValidData) {
        if (trafipaxAlert.currentState != TrafipaxAlert::INACTIVE) {
            trafipaxAlert.currentState = TrafipaxAlert::INACTIVE;
            trafipaxAlert.activeTrafipax = nullptr;
            traffiAlarmActive = false;
            // Optimalizált sáv törlés sprite-tal
            clearAlertBar();
            drawStaticLabels();
        }
        return;
    }

    // Legközelebbi trafipax keresése
    double minDistance = 999999.0;
    const TrafipaxInternal *closestTrafipax = trafipaxManager.getClosestTrafipax(currentLat, currentLon, minDistance);

    const unsigned long currentTime = millis();

    // Ha nincs közeli trafipax a kritikus távolságon belül
    if (minDistance > config.data.gpsTrafiAlarmDistance) {
        if (trafipaxAlert.currentState != TrafipaxAlert::INACTIVE) {
            trafipaxAlert.currentState = TrafipaxAlert::INACTIVE;
            trafipaxAlert.activeTrafipax = nullptr;
            traffiAlarmActive = false; // Riasztás kikapcsolása
            // Töröljük a figyelmeztető sávot sprite-tal
            clearAlertBar();
            drawStaticLabels(); // Újrarajzoljuk a feliratokat
        }
        return;
    }

    // Van közeli trafipax - állapot meghatározása
    // Stabil állapotváltás - csak 10m+ változásnál váltunk
    bool isApproaching = minDistance < (trafipaxAlert.lastDistance - 10.0);
    bool isDeparting = minDistance > (trafipaxAlert.lastDistance + 10.0);

    TrafipaxAlert::State newState = trafipaxAlert.currentState; // Ha még nincs beállítva állapot, akkor a távolság alapján indítjuk
    if (trafipaxAlert.currentState == TrafipaxAlert::INACTIVE) {
        newState = TrafipaxAlert::APPROACHING; // Kezdetben közeledés
    } else if (isApproaching) {
        newState = TrafipaxAlert::APPROACHING;
    } else if (isDeparting) {
        newState = TrafipaxAlert::DEPARTING;
    }
    // Ha nincs jelentős változás, akkor marad a jelenlegi állapot

    // Állapotváltás detektálása
    if (newState != trafipaxAlert.currentState) {
        trafipaxAlert.currentState = newState;
        trafipaxAlert.lastStateChange = currentTime;
        trafipaxAlert.activeTrafipax = closestTrafipax;
    }

    // Figyelmeztető sáv megjelenítése
    traffiAlarmActive = true;
    trafipaxAlert.currentDistance = minDistance;

    // Megjelenítés
    displayTrafipaxAlert(closestTrafipax, minDistance);

    // Szirénázás csak közeledés esetén, 10mp-enként
    if (trafipaxAlert.currentState == TrafipaxAlert::APPROACHING) {
        if (currentTime - trafipaxAlert.lastSirenTime >= TrafipaxAlert::SIREN_INTERVAL) {
            Utils::beepSiren(2, 600, 1800, 20, 8, 100);
            trafipaxAlert.lastSirenTime = currentTime;
        }
    }

    // Távolság frissítése - csak 5m+ változásnál
    if (abs(minDistance - trafipaxAlert.lastDistance) >= 5.0) {
        trafipaxAlert.lastDistance = minDistance;
    }
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
    int16_t speedValue = (int16_t)(gps.speed.isValid() && gps.speed.age() < GPS_DATA_MAX_AGE && gps.speed.kmph() >= 4 ? gps.speed.kmph() : 0);
#ifdef DEMO_MODE
    speedValue = random(0, 288); // demo mód
#endif

    // Ha a riasztás nem aktív
    if (!traffiAlarmActive) {
        // Műholdak száma
        short sats = gps.satellites.isValid() && gps.satellites.age() < GPS_DATA_MAX_AGE ? gps.satellites.value() : 0;
#ifdef DEMO_MODE
        sats = random(0, 16); // Demo mód
#endif
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
        int16_t alt = (int16_t)(gps.satellites.isValid() && gps.altitude.age() < GPS_DATA_MAX_AGE ? gps.altitude.meters() : 0);
        sprintf(buf, "%4d", alt);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextPadding(14 * 4);
        tft.drawString(gps.altitude.isValid() ? buf : "--", 450, 30, 4);

        // Idő
        if (gps.time.isValid() && gps.time.age() < GPS_DATA_MAX_AGE) {
            uint8_t hours = gps.time.hour();
            uint8_t mins = gps.time.minute();
            DaylightSaving::correctTime(mins, hours, gps.date.day(), gps.date.month(), gps.date.year());
            sprintf(buf, "%02d:%02d", hours, mins);
            tft.setTextSize(1);
            tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
            tft.setTextPadding(tft.textWidth(buf, 6));
            tft.drawString(gps.time.isValid() ? buf : "--", 250, 45, 6);
        }

        // Dátum
        if (gps.date.isValid() && gps.date.age() < GPS_DATA_MAX_AGE) {
            sprintf(buf, "%04d-%02d-%02d", gps.date.year(), gps.date.month(), gps.date.day());
            tft.setTextSize(1);
            tft.setTextColor(TFT_ORANGE, TFT_BLACK);
            tft.setTextPadding(tft.textWidth(buf, 2));
            tft.drawString(gps.date.isValid() ? buf : "--", 250, 70, 2);
        }

        // Hdop
        double hdop = gps.satellites.isValid() && gps.hdop.age() < GPS_DATA_MAX_AGE ? gps.hdop.hdop() : 0;
        dtostrf(hdop, 0, 2, buf);
        tft.setTextPadding(5 * 14);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(gps.hdop.isValid() ? buf : "--", 40, 110, 4);

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

    // Sebesség - MegaFont méretekkel
    dtostrf(speedValue, 0, 0, buf);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM); // vízszintes közép
    tft.loadFont(Arial_Narrow_Bold120);
    tft.setTextPadding(tft.textWidth("888"));
    tft.drawString(buf, tft.width() / 2, 240);
    tft.unloadFont();

    // -- Vertikális bar komponensek
    // Sprite legyártása, ha még nem létezik
    if (!spriteVerticalLinearMeter.created()) {
        spriteVerticalLinearMeter.createSprite(SPRITE_VERTICAL_LINEAR_METER_WIDTH, SPRITE_VERTICAL_LINEAR_METER_HEIGHT);
    }

    // Alert bar sprite létrehozása
    if (!spriteAlertBar.created()) {
        spriteAlertBar.createSprite(tft.width(), ALERT_BAR_HEIGHT);
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
    ::temperature = nonBlockingDallasTemperatureSensor.rawToCelsius(temperatureRAW);
    DEBUG("handleTemperatureChange -> temperature: %s °C\n", Utils::floatToString(::temperature, 2).c_str());
}

/**
 * @brief Splash screen kirajzolása
 */
void drawSplashScreen() {
    // Gradient háttér
    for (int y = 0; y < tft.height(); y++) {
        uint16_t color = tft.color565(0, 40 + y / 4, 120 + y / 8); // kékes-lila gradient
        tft.drawFastHLine(0, y, tft.width(), color);
    }

    // Nagy cím árnyékkal
    tft.setTextSize(6);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_BLACK); // árnyék
    tft.drawString("Pico GPS", tft.width() / 2 + 4, 60 + 4);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Pico GPS", tft.width() / 2, 60);

    // leírás
    tft.setTextSize(2);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("Motoros GPS sebessegmero", tft.width() / 2, 110, 2);

    // Verzió
    char buf[64];
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE);
    sprintf(buf, "V%s", APP_VERSION);
    tft.drawString(buf, tft.width() / 2, 160, 1);

    // Build
    sprintf(buf, "%s %s", __DATE__, __TIME__);
    tft.setTextColor(TFT_CYAN);
    tft.drawString(buf, tft.width() / 2, 180, 1);

    // Trafipaxok száma
    sprintf(buf, "Traffi cnt: %d", trafipaxManager.count());
    tft.setTextColor(TFT_YELLOW);
    tft.drawString(buf, tft.width() / 2, 250, 4);
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

#ifdef DEBUG_WAIT_FOR_SERIAL
    Utils::debugWaitForSerial(tft);
#endif

    // Config
    StoreEepromBase<Config_t>::init(); // Meghívjuk a statikus init metódust
    uint16_t x, y;
    if (tft.getTouch(&x, &y)) {
        delay(3000); // Ha van touch, akkor várunk egy picit, hogy a TFT stabilizálódjon
        if (tft.getTouch(&x, &y)) {
            // Ha még mindig nyomva van..
            DEBUG("Restoring default settings...\n");
            Utils::beepTick();

            // akkor betöltjük a default konfigot
            config.loadDefaults();

            // és el is mentjük
            DEBUG("Save default settings...\n");
            config.checkSave();

            Utils::beepTick();
            DEBUG("Default settings restored!\n");
        }
    } else {
        // Konfig sima betöltése
        config.load();
    }

    // TFT háttérvilágítás beállítása
    tftBackLightAdjuster.begin(config.data.tftAutoBrightnessActive, config.data.tftManualBrightnessValue);

    // LittleFS filesystem indítása
    LittleFS.begin();

    // Trafipax adatok betöltése CSV-ből ha a LittleFS fájl létezik
    if (trafipaxManager.checkFile(TrafipaxManager::CSV_FILE_NAME)) {
        trafipaxManager.loadFromCSV(TrafipaxManager::CSV_FILE_NAME);
    }
    DEBUG("trafipaxManagerok száma: %d\n", trafipaxManager.count());

    drawSplashScreen();

    // TFT érintőképernyő kalibrálása
    // Kell kalibrálni a TFT Touch-t?
    if (Utils::isZeroArray(config.data.tftCalibrateData)) {
        Utils::beepError();
        Utils::tftTouchCalibrate(tft, config.data.tftCalibrateData);
        config.checkSave(); // el is mentjük a kalibrációs adatokat
    }
    // Beállítjuk a touch scren-t
    tft.setTouch(config.data.tftCalibrateData);

    // Még egy picit mutatjuk a splash screent
    delay(3000);

    // Kirajzoljuk az állandó feliratokat
    drawStaticLabels();

    // Pittyentünk egyet, hogy üzemkészek vagyunk
    Utils::beepTick();

    // Valós idejű demo indítása (5mp várakozás után közeledés/távolodás)
    trafipaxManager.startDemo();

    // --------------------------------------------------------------------------------------------------------
    // Figyelem!!!
    // NEM LEHET a loop() és a Non-blocking Dallas között hosszú idő, mert a lib leáll az idő méréssel.
    //  Szerintem hibás a matek a NonBlockingDallas::waitNextReading() metódusban
    // --------------------------------------------------------------------------------------------------------
    //  Hőmérséklet szenzor inicializálása
    //  Non-blocking Dallas temperature sensor - 1500ms ajánlott 12 bites felbontáshoz
    nonBlockingDallasTemperatureSensor.begin(NonBlockingDallas::resolution_12, 1500);

    // Non-blocking Dallas temperature sensor callback-ek
    nonBlockingDallasTemperatureSensor.onTemperatureChange(handleTemperatureChange);

    // Azonnal le is kérjük a hőmérsékletet
    nonBlockingDallasTemperatureSensor.requestTemperature();
}

/**
 * Core-0 Loop
 */
void loop() {

    // Hőmérséklet frissítése
    nonBlockingDallasTemperatureSensor.update();

    // Értékek kiírása - optimalizált intervallum
    static unsigned long lastDisplay = 0;
    if (millis() - lastDisplay >= 1000) {
        displayValues();
        lastDisplay = millis();
    }

    // Demo trafipax közeledés/távolodás (ha aktív)
    if (trafipaxManager.isDemoActive()) {
        trafipaxManager.processDemo();
    }

    // Intelligens trafipax figyelmeztető rendszer - optimalizált intervallum
    static unsigned long lastTrafipaxCheck = 0;
    if (millis() - lastTrafipaxCheck >= 500) { // 1000ms->500ms: gyorsabb reagálás
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

        // Akkumulátor feszültség mérése
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
