#include <Arduino.h>
#include <Streaming.h>

#include "Config.h"
#include "Utils.h"
#include "pins.h"

//------------------ TFT
#include <TFT_eSPI.h>
TFT_eSPI tft;
uint16_t SCREEN_W;
uint16_t SCREEN_H;

#include "SensorUtils.h"
SensorUtils sensorUtils;

#include "TftBackLightAdjuster.h"
TftBackLightAdjuster tftBackLightAdjuster;

#include "TrafipaxManager.h"
TrafipaxManager trafipaxManager; // Automatikusan betölti a CSV-t

//-------------------- Screens
// Globális képernyőkezelő pointer - inicializálás a setup()-ban történik
#include "ScreenManager.h"
ScreenManager *screenManager = nullptr;
IScreenManager **iScreenManager = (IScreenManager **)&screenManager; // A UIComponent használja

//-------------------- GPS
#include "GpsManager.h"
GpsManager *gpsManager = nullptr;

// ------------------------------------------------------------------------------------------------------------------------------
// Core0
// ------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Splash screen kirajzolása
 */
void drawSplashScreen() {
    // Gradient háttér
    for (int y = 0; y < tft.height(); y++) {
        uint16_t color = tft.color565(0, 40 + y / 4, 120 + y / 8); // kékes-lila gradient
        tft.drawFastHLine(0, y, tft.width(), color);
    }

    tft.setTextDatum(MC_DATUM);

    // Nagy cím árnyékkal
    tft.setTextSize(6);
    tft.setTextColor(TFT_BLACK); // árnyék
    tft.drawString(PROGRAM_NAME, tft.width() / 2 + 4, 60 + 4);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(PROGRAM_NAME, tft.width() / 2, 60);

    // leírás
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(PROGRAM_DESC, tft.width() / 2, 110, 2);

    // Verzió
    tft.setTextColor(TFT_YELLOW);
    tft.drawString(PROGRAM_VERSION, tft.width() / 2, 140, 2);

    // Author
    tft.setTextColor(TFT_YELLOW);
    tft.drawString(PROGRAM_AUTHOR, tft.width() / 2, 170, 2);

    // Build
    tft.setTextColor(TFT_CYAN);
    tft.drawString(String(__DATE__) + " " + String(__TIME__), tft.width() / 2, 210, 1);

    // Trafipaxok száma
    tft.drawString("Traffi cnt: " + String(trafipaxManager.count()), tft.width() / 2, 272, 4);
}

/**
 * @brief Setup függvény
 */
void setup() {
#ifdef __DEBUG
    Serial.begin(115200);
#endif
    // Beeper
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);

    // TFT
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // UI komponensek számára képernyő méretek inicializálása
    SCREEN_W = tft.width();
    SCREEN_H = tft.height();

#ifdef DEBUG_WAIT_FOR_SERIAL
    tftBackLightAdjuster.begin(true);
    Utils::debugWaitForSerial(tft);
#endif

    // LittleFS filesystem indítása
    LittleFS.begin();

    // Trafipax adatok betöltése CSV-ből ha a LittleFS fájl létezik
    if (trafipaxManager.checkFile(TrafipaxManager::CSV_FILE_NAME)) {
        trafipaxManager.loadFromCSV(TrafipaxManager::CSV_FILE_NAME);
    }
    DEBUG("trafipaxok száma: %d\n", trafipaxManager.count());

    // Splash screen
    drawSplashScreen();

    // Még egy picit mutatjuk a splash screent
    delay(1000);

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

    // ScreenManager inicializálása itt, amikor minden más már kész
    if (screenManager == nullptr) {
        screenManager = new ScreenManager();
    }
    screenManager->switchToScreen(SCREEN_NAME_MAIN); // A kezdő képernyőre kapcsolás

    // Szenzor inicializálása
    sensorUtils.init();

    // Pittyentünk egyet, hogy üzemkészek vagyunk
    Utils::beepTick();
}

/**
 *
 */
void loop() {

    // Szenzorok karbantartása
    sensorUtils.loop();

    //------------------- Touch esemény kezelése
    uint16_t touchX, touchY;
    bool touchedRaw = tft.getTouch(&touchX, &touchY);
    bool validCoordinates = true;
    if (touchedRaw) {
        if (touchX > tft.width() || touchY > tft.height()) {
            validCoordinates = false;
        }
    }

    static bool lastTouchState = false;
    static uint16_t lastTouchX = 0, lastTouchY = 0;
    bool touched = touchedRaw && validCoordinates;

    // Touch press event (immediate response)
    if (touched && !lastTouchState) {
        TouchEvent touchEvent(touchX, touchY, true);
        screenManager->handleTouch(touchEvent);
        lastTouchX = touchX;
        lastTouchY = touchY;
    } else if (!touched && lastTouchState) { // Touch release event (immediate response)
        TouchEvent touchEvent(lastTouchX, lastTouchY, false);
        screenManager->handleTouch(touchEvent);
    }

    lastTouchState = touched;

    if (screenManager) {
        // Deferred actions feldolgozása - biztonságos képernyőváltások végrehajtása
        screenManager->loop();
    }

//------------------- EEPROM mentés figyelése
#define EEPROM_SAVE_CHECK_INTERVAL 1000 * 60 * 5 // 5 perc
    static uint32_t lastEepromSaveCheck = 0;
    if (millis() - lastEepromSaveCheck >= EEPROM_SAVE_CHECK_INTERVAL) {
        config.checkSave();
        lastEepromSaveCheck = millis();
    }
}

// ------------------------------------------------------------------------------------------------------------------------------
// Core1
// ------------------------------------------------------------------------------------------------------------------------------

/**
 * Core1 setup
 */
void setup1() {
    // GPS Serial
    Serial1.setRX(PIN_SERIAL1_RX_NEW);
    Serial1.setTX(PIN_SERIAL1_TX_NEW);
    Serial1.begin(9600);

    // Init + konfiguráció
    gpsManager = new GpsManager(&Serial1);
    gpsManager->setLedDebug(config.data.debugGpsSerialOnInternalFastLed);
    gpsManager->setSerialDebug(config.data.debugGpsSerialData);
    gpsManager->setDebugGpsSatellitesDatabase(config.data.debugGpsSatellitesDatabase);
}

/**
 * Core1 loop
 */
void loop1() {

    // GPS olvasás
    gpsManager->loop();

    // Háttérvilágitás bazseválása
    tftBackLightAdjuster.loop();
}
