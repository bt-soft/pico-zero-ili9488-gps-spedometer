#include "ScreenMain.h"
#include "Config.h"
#include "LinearMeter.h"
#include "UIButton.h"
#include "Utils.h"
#include "defines.h"
#include <Arduino.h>

extern TraffipaxManager traffipaxManager;

constexpr uint16_t SPRITE_VERTICAL_LINEAR_METER_HEIGHT = 10 * (10 + 2) + 40; // max n=10, h=10, g=2
constexpr uint8_t SPRITE_VERTICAL_LINEAR_METER_WIDTH = 70;

constexpr uint8_t ALERT_BAR_HEIGHT = 80;
constexpr uint8_t ALERT_TEXT_PADDING = 5;

// Sprite a vertikális bar-oknak
TFT_eSprite spriteVerticalLinearMeter(&tft);

// Demó mód
extern bool demoMode;

/**
 * UI komponensek elhelyezése
 */
void ScreenMain::layoutComponents() {
    // Info gomb bal alsó sarokban
    addChild(std::make_shared<UIButton>(                                                                                            //
        1,                                                                                                                          // id
        Rect(0, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH - 8, UIButton::DEFAULT_BUTTON_HEIGHT), // bounds (bal alsó sarok)
        "Info",                                                                                                                     // label
        UIButton::ButtonType::Pushable,                                                                                             // type
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_INFO);
            }
        }) //
    );

    // Setup gomb jobb alsó sarokban
    addChild(std::make_shared<UIButton>(                                                                                                                                  //
        2,                                                                                                                                                                // id
        Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), // bounds (jobb alsó sarok)
        "Setup",                                                                                                                                                          // label
        UIButton::ButtonType::Pushable,                                                                                                                                   // type
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_SETUP);
            }
        }) //
    );
}

/**
 * @brief Képernyő aktiválása
 *
 * Meghívódik amikor a képernyő aktívvá válik (pl. visszatérés Info/Setup képernyőről)
 */
void ScreenMain::activate() {

    // Beállítjuk a kényszerített újrarajzolás flag-et
    this->forceRedraw = true;

    // a következő ciklusban kényszerítjük az újrarajzolást
    markForRedraw(true); // a képernyőt és a gyerekeit  újrarajzolásra jelöljük

    // Ős activate() metódus hívása
    UIScreen::activate();
}

/**
 * Műhold ikon rajzolása
 */
void ScreenMain::drawSatelliteIcon(int16_t x, int16_t y) {
    // Központi műhold test (kör)
    tft.fillCircle(x + 12, y + 12, 5, TFT_LIGHTGREY);
    tft.drawCircle(x + 12, y + 12, 5, TFT_WHITE);

    // Felső napelem (egyszerű téglalap)
    tft.fillRect(x + 9, y + 2, 6, 3, TFT_BLUE);
    tft.drawRect(x + 9, y + 2, 6, 3, TFT_MAGENTA);

    // Alsó napelem (egyszerű téglalap)
    tft.fillRect(x + 9, y + 19, 6, 3, TFT_BLUE);
    tft.drawRect(x + 9, y + 19, 6, 3, TFT_MAGENTA);

    // Bal napelem (egyszerű téglalap)
    tft.fillRect(x + 2, y + 9, 3, 6, TFT_BLUE);
    tft.drawRect(x + 2, y + 9, 3, 6, TFT_WHITE);

    // Jobb napelem (egyszerű téglalap)
    tft.fillRect(x + 19, y + 9, 3, 6, TFT_BLUE);
    tft.drawRect(x + 19, y + 9, 3, 6, TFT_WHITE);

    // Összekötő vonalak (vékony vonalak a központból)
    tft.drawLine(x + 12, y + 7, x + 12, y + 5, TFT_WHITE);   // felfelé
    tft.drawLine(x + 12, y + 17, x + 12, y + 19, TFT_WHITE); // lefelé
    tft.drawLine(x + 7, y + 12, x + 5, y + 12, TFT_WHITE);   // balra
    tft.drawLine(x + 17, y + 12, x + 19, y + 12, TFT_WHITE); // jobbra

    // Központi jelzőfény (kicsi)
    tft.fillCircle(x + 12, y + 12, 2, TFT_RED);

    // Felirat
    tft.setTextDatum(ML_DATUM); // Middle Left - bal oldal, középre igazítva
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setFreeFont();
    tft.setTextSize(1);
    tft.drawString("sats", x, y + 28);
}

/**
 * Naptár ikon rajzolása
 */
void ScreenMain::drawCalendarIcon(int16_t x, int16_t y) {
    // Naptár alapja (téglalap)
    tft.fillRect(x + 2, y + 4, 26, 20, TFT_WHITE);
    tft.drawRect(x + 2, y + 4, 26, 20, TFT_DARKGREY);

    // Felső sáv (fejléc)
    tft.fillRect(x + 2, y + 4, 26, 6, TFT_RED);

    // Spirálok/kapcsok
    tft.fillRect(x + 6, y, 4, 8, TFT_DARKGREY);
    tft.fillRect(x + 20, y, 4, 8, TFT_DARKGREY);

    // Naptár rácsok (napok)
    tft.drawFastHLine(x + 4, y + 12, 22, TFT_LIGHTGREY);
    tft.drawFastHLine(x + 4, y + 16, 22, TFT_LIGHTGREY);
    tft.drawFastHLine(x + 4, y + 20, 22, TFT_LIGHTGREY);

    tft.drawFastVLine(x + 9, y + 10, 12, TFT_LIGHTGREY);
    tft.drawFastVLine(x + 15, y + 10, 12, TFT_LIGHTGREY);
    tft.drawFastVLine(x + 21, y + 10, 12, TFT_LIGHTGREY);

    // Aktuális nap kiemelése
    tft.fillRect(x + 10, y + 13, 4, 3, TFT_BLUE);
}

/**
 * Magasság ikon rajzolása (hegy + magassági vonalak)
 */
void ScreenMain::drawAltitudeIcon(int16_t x, int16_t y) {
    // Hegy/domb alakzat
    tft.fillTriangle(x + 5, y + 18, x + 20, y + 4, x + 35, y + 18, TFT_GREEN);
    tft.drawTriangle(x + 5, y + 18, x + 20, y + 4, x + 35, y + 18, TFT_DARKGREEN);

    // Magassági vonalak (szintvonalak)
    tft.drawFastHLine(x + 8, y + 15, 24, TFT_WHITE);
    tft.drawFastHLine(x + 10, y + 12, 20, TFT_WHITE);
    tft.drawFastHLine(x + 13, y + 9, 14, TFT_WHITE);
    tft.drawFastHLine(x + 16, y + 7, 8, TFT_WHITE);

    // Magasság jelölő nyíl
    tft.drawFastVLine(x + 2, y + 2, 14, TFT_WHITE);
    tft.drawLine(x + 2, y + 2, x + 5, y + 5, TFT_WHITE);
    tft.drawLine(x + 2, y + 2, x - 1, y + 5, TFT_WHITE);

    // Felirat
    tft.setTextDatum(MC_DATUM); // Middle Center - középre igazítva
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setFreeFont();
    tft.setTextSize(1);
    tft.drawString("altit", x + 20, y + 27);
}

/**
 * GPS pontosság ikon rajzolása (célkereszt/target stílusban)
 */
void ScreenMain::drawGpsAccuracyIcon(int16_t x, int16_t y) {
    // Külső kör (cél kerete)
    tft.drawCircle(x + 15, y + 12, 12, TFT_WHITE);
    tft.drawCircle(x + 15, y + 12, 11, TFT_WHITE);

    // Egyetlen belső kör (célpont)
    tft.fillCircle(x + 15, y + 12, 3, TFT_RED);
    tft.drawCircle(x + 15, y + 12, 3, TFT_WHITE);

    // Kereszt vonalak (célkereszt)
    // Vízszintes vonal
    tft.drawFastHLine(x + 3, y + 12, 24, TFT_WHITE);
    // Függőleges vonal
    tft.drawFastVLine(x + 15, y, 24, TFT_WHITE);

    // Kis szaggatás a kereszten (autentikus célkereszt look)
    tft.drawFastHLine(x + 13, y + 12, 4, TFT_BLACK); // kis rés középen vízszintesen
    tft.drawFastVLine(x + 15, y + 10, 4, TFT_BLACK); // kis rés középen függőlegesen

    // Felirat
    tft.setTextDatum(ML_DATUM); // Middle Left - bal oldal, középre igazítva
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setFreeFont();
    tft.setTextSize(1);
    tft.setTextPadding(0); // ne töröljön bele felesleges pixelbe
    tft.drawString("hdop", x + 5, y + 27);
}

/**
 * Speedometer ikon rajzolása (sebességmérő - félkör)
 */
void ScreenMain::drawSpeedometerIcon(int16_t x, int16_t y) {
    // Félkör alap (felső félkör speedometer)
    // Külső félkör (felső 180°)
    for (int i = 180; i <= 360; i++) {
        float rad = (i * PI) / 180.0;
        int x1 = x + 15 + 15 * cos(rad);
        int y1 = y + 15 + 15 * sin(rad);
        tft.drawPixel(x1, y1, TFT_WHITE);

        int x2 = x + 15 + 14 * cos(rad);
        int y2 = y + 15 + 14 * sin(rad);
        tft.drawPixel(x2, y2, TFT_LIGHTGREY);
    }

    // Belső félkör (műszer lap - felső rész)
    for (int i = 180; i <= 360; i++) {
        float rad = (i * PI) / 180.0;
        for (int r = 0; r < 12; r++) {
            int x1 = x + 15 + r * cos(rad);
            int y1 = y + 15 + r * sin(rad);
            tft.drawPixel(x1, y1, TFT_BLACK);
        }
    }

    // Felső egyenes vonal (speedometer alapja)
    tft.drawFastHLine(x + 3, y + 15, 24, TFT_WHITE);

    // Skála vonalak (csak felső félkörben)
    // 180° (bal szél) - 0 km/h
    tft.drawLine(x + 3, y + 15, x + 6, y + 15, TFT_WHITE);
    // 225° (bal-felső) - 50 km/h
    tft.drawLine(x + 4, y + 5, x + 7, y + 8, TFT_WHITE);
    // 270° (felső közép) - 100 km/h
    tft.drawLine(x + 15, y + 3, x + 15, y + 6, TFT_WHITE);
    // 315° (jobb-felső) - 150 km/h
    tft.drawLine(x + 26, y + 5, x + 23, y + 8, TFT_WHITE);
    // 360° / 0° (jobb szél) - 200 km/h
    tft.drawLine(x + 27, y + 15, x + 24, y + 15, TFT_WHITE);

    // Speedometer mutató 240 fokra (120° a felső félkörben)
    float angle = 240 * PI / 180.0; // 240 fok radiánban (felső félkör 60°-nál)
    int endX = x + 15 + 10 * cos(angle);
    int endY = y + 15 + 10 * sin(angle);

    // Piros mutató (vastagabb)
    tft.drawLine(x + 15, y + 15, endX, endY, TFT_RED);
    tft.drawLine(x + 15, y + 15, endX - 1, endY, TFT_RED);
    tft.drawLine(x + 15, y + 15, endX, endY - 1, TFT_RED);

    // Központi csavar/tengely
    tft.fillCircle(x + 15, y + 15, 2, TFT_YELLOW);
    tft.drawCircle(x + 15, y + 15, 2, TFT_WHITE);

    // Speed zónák jelzése (színes pontok a skálán)
    tft.drawPixel(x + 6, y + 12, TFT_GREEN);  // alacsony sebesség
    tft.drawPixel(x + 12, y + 7, TFT_YELLOW); // közepes sebesség
    tft.drawPixel(x + 21, y + 12, TFT_RED);   // nagy sebesség

    // Felirat
    tft.setTextDatum(MC_DATUM); // Middle Center - középre igazítva
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setFreeFont();
    tft.setTextSize(1);
    tft.setTextPadding(0); // ne töröljön bele felesleges pixelbe
    tft.drawString("max sp", x + 16, y + 22);
}

/**
 * Kirajzolja a képernyő saját tartalmát (statikus elemek)
 */
void ScreenMain::drawContent() {

    // Szöveges feliratok
    tft.setFreeFont(); // Alapértelmezett font
    tft.setTextSize(1);
    tft.setTextPadding(0); // ne töröljön bele felesleges pixelbe
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextDatum(MR_DATUM);

    // Magasság mértékegység felirat
    tft.drawString("m", ::SCREEN_W - 2, 18);

    // Maxspeed km/h felirat
    tft.drawString("km/h", ::SCREEN_W - 2, 60);

    // Sebesség mértékegység felirat
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("km/h", ::SCREEN_W / 2, 105);

    // Műhold ikon bal oldalon
    drawSatelliteIcon(0, 0);

    // Naptár ikon fent középen
    drawCalendarIcon(::SCREEN_W / 2 - 90, 0);

    // Magasság ikon jobb oldalon
    drawAltitudeIcon(::SCREEN_W - 135, 0);

    // GPS pontosság (hdop) ikon rajzolása
    drawGpsAccuracyIcon(0, 50);

    // Speedometer ikon (max speed)
    drawSpeedometerIcon(::SCREEN_W - 130, 50);
}

/**
 * @brief Normál módú adatok legyűjtése
 */
ScreenMain::DisplayData ScreenMain::collectRealData() {
    DisplayData data;

    // Műhold adatok
    data.satelliteValid = gpsManager->getSatellites().isValid() && gpsManager->getSatellites().age() < GPS_DATA_MAX_AGE;
    data.satelliteCount = data.satelliteValid ? gpsManager->getSatellites().value() : 0;
    data.gpsMode = gpsManager->getGpsModeToString();

    // Dátum
    GpsManager::LocalDateTime localDateTime = gpsManager->getLocalDateTime();
    if (localDateTime.dateValid) {
        char dateStr[11];
        sprintf(dateStr, "%04d-%02d-%02d", localDateTime.year, localDateTime.month, localDateTime.day);
        data.dateString = String(dateStr);
    } else {
        data.dateString = "----.--.--";
    }

    // Idő
    if (localDateTime.timeValid) {
        char timeStr[9];
        sprintf(timeStr, "%02d:%02d:%02d", localDateTime.hour, localDateTime.minute, localDateTime.second);
        data.timeString = String(timeStr);

    } else {
        data.timeString = "--:--:--";
    }

    // Pozíció
    data.positionValid = gpsManager->getLocation().isValid() && gpsManager->getLocation().age() < GPS_DATA_MAX_AGE;
    data.latitude = data.positionValid ? gpsManager->getLocation().lat() : 0.0;
    data.longitude = data.positionValid ? gpsManager->getLocation().lng() : 0.0;

    // Magasság
    data.altitudeValid = gpsManager->getAltitude().isValid() && gpsManager->getAltitude().age() < GPS_DATA_MAX_AGE;
    data.altitude = data.altitudeValid ? gpsManager->getAltitude().meters() : 0.0;

    // GPS pontosság
    data.hdopValid = gpsManager->getHdop().isValid() && gpsManager->getHdop().age() < GPS_DATA_MAX_AGE;
    data.hdop = data.hdopValid ? gpsManager->getHdop().hdop() : 0.0;

    // Sebesség - 1km/h esetén indulunk el
    data.speedValid = gpsManager->getSpeed().isValid() && gpsManager->getSpeed().age() < GPS_DATA_MAX_AGE && gpsManager->getSpeed().kmph() > 1;
    data.currentSpeed = data.speedValid ? gpsManager->getSpeed().kmph() : 0.0;

    // Maximum sebesség (statikus változó)
    static double maxSpeedValue = 0.0;
    if (data.speedValid && data.currentSpeed > maxSpeedValue) {
        maxSpeedValue = data.currentSpeed;
    }
    data.maxSpeed = maxSpeedValue;

    // Szenzorok
    data.busVoltage = sensorUtils.readVBusExternal();
    if (externalTemperatureMode) {
        data.temperature = sensorUtils.readExternalTemperature();
    } else {
        data.temperature = sensorUtils.readCoreTemperature();
    }

    return data;
}

/**
 * @brief Demó módú adatok generálása
 */
ScreenMain::DisplayData ScreenMain::collectDemoData() {
    DisplayData data;

    // Műhold adatok - 2 másodpercenként változik
    static unsigned long lastSatChange = 0;
    static uint8_t demoSatCount = 8;
    if (millis() - lastSatChange > 2000) {
        demoSatCount = random(0, 16);
        lastSatChange = millis();
    }
    data.satelliteCount = demoSatCount;
    data.satelliteValid = true;
    switch (random(0, 4)) {
        case 0:
            data.gpsMode = "No Fix";
            break;
        case 1:
            data.gpsMode = "Auto 2D/3D";
            break;
        case 2:
            data.gpsMode = "Differential";
            break;
        case 3:
            data.gpsMode = "Estimated";
            break;
        default:
            data.gpsMode = "Unknown";
            break;
    }

    // Dátum és idő - folyamatosan növekvő
    static unsigned long demoStartTime = millis();
    unsigned long elapsed = (millis() - demoStartTime) / 1000;

    data.dateString = "2025-08-23";

    int startHour = 17, startMin = 42, startSec = 30;
    int totalSeconds = startHour * 3600 + startMin * 60 + startSec + elapsed;
    int hour = (totalSeconds / 3600) % 24;
    int minute = (totalSeconds % 3600) / 60;
    int second = totalSeconds % 60;

    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);
    data.timeString = String(timeStr);
    data.dateTimeValid = true;

    // Pozíció
    data.positionValid = false;
    data.latitude = 0.0;
    data.longitude = 0.0;

    // Magasság - 3 másodpercenként változik
    static unsigned long lastAltChange = 0;
    static double demoAltitude = 150.0;
    if (millis() - lastAltChange > 3000) {
        demoAltitude = random(50, 2000);
        lastAltChange = millis();
    }
    data.altitude = demoAltitude;
    data.altitudeValid = true;

    // GPS pontosság - random érték 3.00-99.99 között
    data.hdop = random(300, 10000) / 100.0f; // 3.00 ... 99.99
    data.hdopValid = true;

    // Sebesség - 1.5 másodpercenként változik
    static unsigned long lastSpeedChange = 0;
    static double demoSpeed = 50.0;
    if (millis() - lastSpeedChange > 1500) {
        demoSpeed = random(0, 350);
        lastSpeedChange = millis();
    }
    data.currentSpeed = demoSpeed;
    data.speedValid = true;

    // Maximális sebesség szimulációja
    static double demoMaxSpeed = 0.0;
    if (data.currentSpeed > demoMaxSpeed) {
        demoMaxSpeed = data.currentSpeed;
    }
    data.maxSpeed = demoMaxSpeed;

    // Szenzorok - szimulált értékek
    // Ha 15.5 a felső határ, akkor random(0, 1201) / 100.0 = 3.5 ... 15.5
    data.busVoltage = 3.5 + (random(0, 1201) / 100.0); // 3.5 ... 15.5 V

    // temperature: -15.5 ... +65.5 °C
    data.temperature = -15.5 + (random(0, 811) / 10.0); // -15.5 ... +65.5 (811 lépés, 0.1°C)

    return data;
}

/**
 * Alert bar törlése
 */
void ScreenMain::clearTraffipaxAlert() {
    // Egyszerűen töröljük a riasztás sávját a tft-n
    tft.fillRect(0, 0, tft.width(), ALERT_BAR_HEIGHT, TFT_BLACK);

    // Ha vansziréna, akkor azt most lelőjük
    Utils::stopNonBlockingSiren();
}

/**
 * Traffipax figyelmeztető sáv megjelenítése
 */
void ScreenMain::displayTraffipaxAlert(const TraffipaxManager::TraffipaxRecord *traffipax, double distance) {

    static uint16_t lastBackgroundColor = 0xFFFF;
    static const TraffipaxManager::TraffipaxRecord *lastTraffipaxPtr = nullptr;
    static char lastCityText[MAX_CITY_LEN] = {0};
    static char lastStreetText[MAX_STREET_LEN] = {0};
    static int lastDistance = -1;

    if (traffipax == nullptr) {
        return;
    }

    // Háttérszín meghatározása állapot szerint
    uint16_t backgroundColor;
    uint16_t textColor;

    switch (traffipaxAlert.currentState) {
        case TraffipaxAlert::APPROACHING:
        case TraffipaxAlert::NEARBY_STOPPED:
            backgroundColor = TFT_RED;
            textColor = TFT_WHITE;
            break;
        case TraffipaxAlert::DEPARTING:
            backgroundColor = TFT_ORANGE;
            textColor = TFT_BLACK;
            break;
        default:
            return; // INACTIVE
    }

    bool fullRedraw = false;
    // Ha háttérszín vagy traffipax rekord változott, teljes újrarajzolás
    if (backgroundColor != lastBackgroundColor || traffipax != lastTraffipaxPtr) {
        fullRedraw = true;
        lastBackgroundColor = backgroundColor;
        lastTraffipaxPtr = traffipax;

        // Város és utca szöveg mentése
        strncpy(lastCityText, traffipax->city, MAX_CITY_LEN);
        strncpy(lastStreetText, traffipax->street_or_km, MAX_STREET_LEN);
    }

    if (fullRedraw) {
        // Háttér sáv
        tft.fillRect(0, 0, tft.width(), ALERT_BAR_HEIGHT, backgroundColor);

        // Város (első sor, balra igazítva)
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(textColor, backgroundColor);
        tft.setFreeFont(&FreeSans9pt7b);
        tft.setTextSize(2);
        tft.drawString(lastCityText, ALERT_TEXT_PADDING, 10);

        // Utca/km (második sor, balra igazítva)
        tft.setTextSize(1);
        tft.drawString(lastStreetText, ALERT_TEXT_PADDING, 55);
    }

    // Távolság (jobbra, vertikálisan középre) csak akkor töröljük, ha változott
    int intDistance = (int)distance;
    if (intDistance != lastDistance || fullRedraw) {

        char distanceText[16];
        snprintf(distanceText, sizeof(distanceText), "%dm", intDistance);

        tft.setTextDatum(MR_DATUM);
        tft.setFreeFont(&FreeSerifBold24pt7b);
        tft.setTextSize(1);
        tft.setTextPadding(tft.textWidth("8888m"));
        tft.setTextColor(textColor, backgroundColor);
        tft.drawString(distanceText, ::SCREEN_W - ALERT_TEXT_PADDING, ALERT_BAR_HEIGHT / 2);
        lastDistance = intDistance;
    }

    tft.setFreeFont();
}

/**
 * Intelligens traffipax figyelmeztető rendszer
 * - 800m-en belül: piros sáv + város/utca + távolság
 * - Közeledés esetén: 10mp-enként szirénázás
 * - Megállás esetén: nincs szirénázás, csak a piros sáv
 * - Távolodás esetén: narancssárga sáv, nincs szirénázás
 *
 * Állapotok:
 * - INACTIVE: Nincs közeli traffipax (> 800m)
 * - APPROACHING: Közeledik (piros háttér + szirénázás)
 * - NEARBY_STOPPED: Megállt közel (piros háttér, nincs szirénázás)
 * - DEPARTING: Távolodik (narancssárga háttér, nincs szirénázás)
 */
void ScreenMain::processIntelligentTraffipaxAlert(double currentLat, double currentLon, bool positionValid) {

    // Trafipax riasztás stabilizálása: csak tartósan nagy távolság után tűnik el a sprite
    static unsigned long outOfRangeStart = 0;

    // Nincs érvényes GPS pozíció adat - riasztás kikapcsolása ha éppen aktív
    if (!positionValid) {
        if (traffipaxAlert.currentState != TraffipaxAlert::INACTIVE) {
            traffipaxAlert.currentState = TraffipaxAlert::INACTIVE;
            traffipaxAlert.activeTraffipax = nullptr;
            traffiAlarmActive = false;
            clearTraffipaxAlert();
            this->forceRedraw = true;
            markForRedraw(true);
        }
        outOfRangeStart = 0;
        return;
    }

    // Legközelebbi trafipax keresése
    double minDistance = 999999.0;
    const TraffipaxManager::TraffipaxRecord *closestTraffipax = traffipaxManager.getClosestTraffipax(currentLat, currentLon, minDistance);
    const unsigned long currentTime = millis();

    // Ha nincs közeli traffipax a kritikus távolságon belül
    if (minDistance > config.data.gpsTraffiAlarmDistance) {
        // Először jegyezzük fel, mikor kerültünk ki a tartományból
        if (outOfRangeStart == 0) {
            outOfRangeStart = currentTime;
        }
        // Ha már legalább 3 másodperce kívül vagyunk, akkor kapcsoljuk ki a riasztást
        if (currentTime - outOfRangeStart > 3000) {
            if (traffipaxAlert.currentState != TraffipaxAlert::INACTIVE) {
                traffipaxAlert.currentState = TraffipaxAlert::INACTIVE;
                traffipaxAlert.activeTraffipax = nullptr;
                traffiAlarmActive = false;
                clearTraffipaxAlert();
                this->forceRedraw = true;
                markForRedraw(true);
            }
            return;
        } else {
            // Még nem telt le a stabilizációs idő, a sprite maradjon!
            if (traffipaxAlert.currentState != TraffipaxAlert::INACTIVE && traffipaxAlert.activeTraffipax) {
                traffiAlarmActive = true;
                traffipaxAlert.currentDistance = minDistance;
                displayTraffipaxAlert(traffipaxAlert.activeTraffipax, minDistance);
            }
            return;
        }
    } else {
        // Visszaléptünk a tartományba, nullázzuk az időzítőt
        outOfRangeStart = 0;
    }

    // Van közeli traffipax - állapot meghatározása
    bool isApproaching = minDistance < (traffipaxAlert.lastDistance - 10.0);
    bool isDeparting = minDistance > (traffipaxAlert.lastDistance + 10.0);

    TraffipaxAlert::State newState = traffipaxAlert.currentState;
    if (traffipaxAlert.currentState == TraffipaxAlert::INACTIVE) {
        newState = TraffipaxAlert::APPROACHING;
    } else if (isApproaching) {
        newState = TraffipaxAlert::APPROACHING;
    } else if (isDeparting) {
        newState = TraffipaxAlert::DEPARTING;
    }

    if (newState != traffipaxAlert.currentState) {
        traffipaxAlert.currentState = newState;
        traffipaxAlert.lastStateChange = currentTime;
        traffipaxAlert.activeTraffipax = closestTraffipax;
    }

    // Figyelmeztető sáv megjelenítése minden ciklusban, amíg aktív
    traffiAlarmActive = true;
    traffipaxAlert.currentDistance = minDistance;
    displayTraffipaxAlert(closestTraffipax, minDistance);

    // Szirénázás csak közeledés esetén, 10mp-enként, ha engedélyezve van
    if (config.data.gpsTraffiSirenAlarmEnabled) {
        if (traffipaxAlert.currentState == TraffipaxAlert::APPROACHING) {
            if (currentTime - traffipaxAlert.lastSirenTime >= TraffipaxAlert::SIREN_INTERVAL) {
                Utils::startNonBlockingSiren(2, 600, 1800, 40, 4, 100);
                traffipaxAlert.lastSirenTime = currentTime;
            }
        }
    }

    // Távolság frissítése - csak 5m+ változásnál
    if (abs(minDistance - traffipaxAlert.lastDistance) >= 5.0) {
        traffipaxAlert.lastDistance = minDistance;
    }
}

/**
 * Kezeli a képernyő saját ciklusát (dinamikus frissítés)
 */

void ScreenMain::handleOwnLoop() {
    // 1 másodperces frissítés
    if (!Utils::timeHasPassed(lastUpdate, 1000)) {
        return;
    }
    lastUpdate = millis();

    // Adatok legyűjtése (demó vagy valós mód szerint)
    DisplayData data = demoMode ? collectDemoData() : collectRealData();

    // Demó logika kiszervezése
    if (demoMode) {
        handleDemoMode(data);
    }

    // Trafipax figyelmeztetés feldolgozása, ha engedélyezve van
    if (config.data.gpsTraffiAlarmEnabled) {
        processIntelligentTraffipaxAlert(data.latitude, data.longitude, data.positionValid);
    }

    // Általános buffer a megjelenítéshez
    char buf[11];

    // Ha nincs traffipax riasztás, akkor mehet a felső részek rajzolása is
    if (!this->traffiAlarmActive) {

        // Műholdak száma + GPS működési mód
        if (this->forceRedraw) {
            lastSatCount = 255;
            lastGpsMode = "";
        }
        // Műholdak száma
        UIScreen::updateUIValue<uint8_t>(
            lastSatCount, data.satelliteCount,
            [&]() {
                tft.setTextDatum(ML_DATUM);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setFreeFont();
                tft.setTextSize(2);
                tft.setTextPadding(tft.textWidth("88") + 10);
                tft.drawString(String(data.satelliteCount), 30, 15, 2);
                tft.setFreeFont();
            },
            0, this->forceRedraw);

        // GPS működési mód
        UIScreen::updateUIValue<String>(
            lastGpsMode, data.gpsMode,
            [&]() {
                tft.setTextDatum(ML_DATUM);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setFreeFont();
                tft.setTextSize(1);
                uint16_t fgcolor = gpsManager->getLocation().FixMode() == TinyGPSLocation::N ? TFT_ORANGE : TFT_GREEN;
                tft.setTextColor(fgcolor, TFT_BLACK);
                tft.setTextPadding(tft.textWidth("Differential"));
                tft.drawString(data.gpsMode, 65, 15, 1);
                tft.setFreeFont();
            },
            0, this->forceRedraw);

        // Dátum és idő
        String currentDateTime = data.dateString + data.timeString;
        UIScreen::updateUIValue<String>(
            lastDateTime, currentDateTime,
            [&]() {
                tft.setTextSize(2);
                tft.setFreeFont();
                tft.setTextDatum(ML_DATUM);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setTextPadding(tft.textWidth("8888-88-88") + 10);
                tft.drawString(data.dateString, ::SCREEN_W / 2 - 50, 12, 1);
                tft.setTextSize(1);
                tft.setFreeFont(&FreeSansBold18pt7b);
                tft.setTextPadding(tft.textWidth("88:88:88") + 10);
                tft.drawString(data.timeString, ::SCREEN_W / 2 - 70, 40);
                tft.setFreeFont();
            },
            0, this->forceRedraw);

        // Magasság
        UIScreen::updateUIValue<double>(
            lastAltitude, data.altitudeValid ? data.altitude : -9999.0,
            [&]() {
                tft.setTextDatum(ML_DATUM);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setFreeFont();
                tft.setTextSize(2);
                int paddingWidth = tft.textWidth("8888", 2) + 10;
                tft.setTextPadding(paddingWidth);
                String altText = (data.altitudeValid ? String((int)data.altitude) : "-- ");
                tft.drawString(altText, ::SCREEN_W - 90, 13, 2);
            },
            1.0, this->forceRedraw);

        // GPS HDOP
        UIScreen::updateUIValue<double>(
            lastHdop, data.hdopValid ? data.hdop : -1.0,
            [&]() {
                tft.setTextDatum(ML_DATUM);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setFreeFont();
                tft.setTextSize(2);
                tft.setTextPadding(tft.textWidth("88.88") + 15);
                dtostrf(data.hdop, 0, 2, buf);
                tft.drawString(data.hdopValid ? String(buf) : "--", 35, 63, 2);
                tft.setFreeFont();
            },
            0.1, this->forceRedraw);

        // Maximum sebesség
        UIScreen::updateUIValue<double>(
            lastMaxSpeed, data.maxSpeed,
            [&]() {
                tft.setTextDatum(ML_DATUM);
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setFreeFont();
                tft.setTextSize(2);
                tft.setTextPadding(tft.textWidth("888") + 10);
                // Ha a maxSpeed > 0, kiírjuk, egyébként "--"
                String maxSpeedStr = (data.maxSpeed > 0) ? String((int)data.maxSpeed) : "--";
                tft.drawString(maxSpeedStr, ::SCREEN_W - 90, 60, 2);
                tft.setFreeFont();
            },
            0.1, this->forceRedraw);
    }

    // Aktuális sebesség
    UIScreen::updateUIValue<double>(
        lastSpeed, data.speedValid ? data.currentSpeed : -1.0,
        [&]() {
            tft.loadFont(Arial_Narrow_Bold120);
            tft.setTextDatum(MC_DATUM);
            tft.setTextPadding(tft.textWidth("888") + 10);
            dtostrf(data.speedValid ? data.currentSpeed : 0, 0, 0, buf);
            tft.setTextColor(data.speedValid ? TFT_WHITE : TFT_RED, TFT_BLACK);
            tft.drawString(buf, ::SCREEN_W / 2 - 11, 240);
            tft.unloadFont();
        },
        0.1, this->forceRedraw);

    // -- Vertikális bar komponensek  ------------------------------------
    if (lastVerticalLinearSpriteUpdate != 0 && !Utils::timeHasPassed(lastVerticalLinearSpriteUpdate, 5000)) {
        return;
    }
    lastVerticalLinearSpriteUpdate = millis();

    // Sprite legyártása, ha még nem létezik
    if (!spriteVerticalLinearMeter.created()) {
        spriteVerticalLinearMeter.createSprite(SPRITE_VERTICAL_LINEAR_METER_WIDTH, SPRITE_VERTICAL_LINEAR_METER_HEIGHT);
    }

#define VERTICAL_LINEAR_METER_BAR_Y 250
    // Vertical Line bar - Battery (sprite-os)
    verticalLinearMeter(&spriteVerticalLinearMeter, SPRITE_VERTICAL_LINEAR_METER_HEIGHT, SPRITE_VERTICAL_LINEAR_METER_WIDTH,
                        "Vbus [V]",                       // category
                        data.busVoltage,                  // value
                        BATT_BARMETER_MIN,                // minVal
                        BATT_BARMETER_MAX,                // maxVal
                        0,                                // x
                        VERTICAL_LINEAR_METER_BAR_Y + 10, // y: sprite alsó éle, +10 hogy ne lógjon le
                        30,                               // bar-w
                        10,                               // bar-h
                        2,                                // gap
                        10,                               // n
                        RED2GREEN);                       // color

    // Vertical Line bar - Temperature
    verticalLinearMeter(&spriteVerticalLinearMeter, SPRITE_VERTICAL_LINEAR_METER_HEIGHT, SPRITE_VERTICAL_LINEAR_METER_WIDTH,
                        externalTemperatureMode ? "Ext [C]" : "CPU [C]",  // category
                        data.temperature,                                 // value
                        TEMP_BARMETER_MIN,                                // minVal
                        TEMP_BARMETER_MAX,                                // maxVal
                        tft.width() - SPRITE_VERTICAL_LINEAR_METER_WIDTH, // x: sprite szélesség beszámítva
                        VERTICAL_LINEAR_METER_BAR_Y + 10,                 // y: sprite alsó éle, +10 hogy ne lógjon le
                        30,                                               // bar-w
                        10,                                               // bar-h
                        2,                                                // gap
                        10,                                               // n
                        BLUE2RED,                                         // color
                        true);                                            // bal oldalt legyenek az értékek

    // Ha kényszerített újrarajzolás volt, akkor reseteljük a flag-et
    if (this->forceRedraw) {
        this->forceRedraw = false;
    }
}

/**
 * Demó logika kiszervezése
 */
void ScreenMain::handleDemoMode(DisplayData &data) {
    bool isTraffipaxManagerDemoActive = traffipaxManager.isDemoActive();

    // Ha a demó aktív volt és most nem az, rögzítjük a befejezési időt.
    if (wasDemoActive && !isTraffipaxManagerDemoActive) {
        lastDemoEndTime = millis();
        DEBUG("Traffi Demo Fázis: 2 perc várakozás\n");
    }
    wasDemoActive = isTraffipaxManagerDemoActive;

    // Ha a demó nem aktív:
    if (!isTraffipaxManagerDemoActive) {
        // Ha ez az első futás (lastDemoEndTime 0 és wasDemoActive kezdetben hamis)
        // VAGY ha a várakozási időszak letelt az utolsó demó befejezése óta.
        if (lastDemoEndTime == 0 || Utils::timeHasPassed(lastDemoEndTime, 2 * 60 * 1000)) {
            traffipaxManager.startDemo();
            lastDemoEndTime = 0; // Visszaállítás a következő várakozási időszakra
        }
    }

    // Ha a demó aktív, dolgozzuk fel.
    if (isTraffipaxManagerDemoActive) {
        traffipaxManager.processDemo();
        traffipaxManager.getDemoCoords(data.latitude, data.longitude);
        data.positionValid = true;
    }
}

/**
 * @brief Touch esemény kezelése - hőmérsékleti mód váltás
 */
bool ScreenMain::handleTouch(const TouchEvent &event) {

    // Csak lenyomás eseményre reagálunk
    if (!event.pressed) {
        return UIScreen::handleTouch(event); // Továbbítjuk az ősosztálynak
    }

    // Műhold ikon és szám területének ellenőrzése (bal felső sarok)
    constexpr uint16_t satIconX = 0;
    constexpr uint16_t satIconY = 0;
    constexpr uint16_t satIconWidth = 60;  // Ikon + szám területe
    constexpr uint16_t satIconHeight = 35; // Ikon magassága + felirat
    if (event.x >= satIconX && event.x < satIconX + satIconWidth && event.y >= satIconY && event.y < satIconY + satIconHeight) {
        getScreenManager()->switchToScreen(SCREEN_NAME_SATS);
        return true; // Esemény kezelve
    }

    // Jobb oldali hőmérsékleti bar területének ellenőrzése
    uint16_t rightBarX = ::SCREEN_W - SPRITE_VERTICAL_LINEAR_METER_WIDTH;
    constexpr uint16_t rightBarY = VERTICAL_LINEAR_METER_BAR_Y + 10 - SPRITE_VERTICAL_LINEAR_METER_HEIGHT; // y koordináta teteje
    constexpr uint16_t rightBarWidth = SPRITE_VERTICAL_LINEAR_METER_WIDTH;
    constexpr uint16_t rightBarHeight = SPRITE_VERTICAL_LINEAR_METER_HEIGHT;
    // Ellenőrizzük, hogy a touch a jobb oldali bar területén belül van-e
    if (event.x >= rightBarX && event.x < rightBarX + rightBarWidth && event.y >= rightBarY && event.y < rightBarY + rightBarHeight) {

        // Váltás a hőmérsékleti módok között
        externalTemperatureMode = !externalTemperatureMode;

        // Sprite azonnal frissüljön a következő draw()-nál
        lastVerticalLinearSpriteUpdate = 0;

        return true; // Esemény kezelve
    }

    // Ha nem volt találat, továbbítjuk az alaposztálynak
    return UIScreen::handleTouch(event);
}