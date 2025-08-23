#include "ScreenMain.h"
#include "Utils.h"

/**
 * UI komponensek elhelyezése
 */
void ScreenMain::layoutComponents() {
    // Komponensek elhelyezése
}

/**
 * Kirajzolja a képernyő saját tartalmát (statikus elemek)
 */
void ScreenMain::drawContent() {
    // Műhold ikon bal oldalon
    drawSatelliteIcon(0, 0);

    // Naptár ikon fent középen
    drawCalendarIcon(::SCREEN_W / 2 - 60, 0);

    // Magasság ikon jobb oldalon
    drawAltitudeIcon(::SCREEN_W - 120, 0);

    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(); // Alapértelmezett font
    tft.setTextSize(1);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    // GPS pontosság (hdop) ikon rajzolása
    drawGpsAccuracyIcon(0, 50);

    // Speedometer ikon (max speed)
    drawSpeedometerIcon(::SCREEN_W - 120, 50);

    // Sebesség mértékegység felirat
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("km/h", tft.width() / 2 - 20, 105, 2);
}

/**
 * Műhold ikon rajzolása (flaticon 2863489 stílusban - minimális)
 */
void ScreenMain::drawSatelliteIcon(int16_t x, int16_t y) {
    // Központi műhold test (kör)
    tft.fillCircle(x + 12, y + 12, 5, TFT_LIGHTGREY);
    tft.drawCircle(x + 12, y + 12, 5, TFT_WHITE);

    // Felső napelem (egyszerű téglalap)
    tft.fillRect(x + 9, y + 2, 6, 3, TFT_BLUE);
    tft.drawRect(x + 9, y + 2, 6, 3, TFT_WHITE);

    // Alsó napelem (egyszerű téglalap)
    tft.fillRect(x + 9, y + 19, 6, 3, TFT_BLUE);
    tft.drawRect(x + 9, y + 19, 6, 3, TFT_WHITE);

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
    tft.drawString("sats", x, y + 27);
}

/**
 * Naptár ikon rajzolása (flaticon stílusban)
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
 * Magasság ikon rajzolása (hegy + magassági vonalak) - nagyobb méret
 */
void ScreenMain::drawAltitudeIcon(int16_t x, int16_t y) {
    // Hegy/domb alakzat - nagyobb méret
    tft.fillTriangle(x + 5, y + 18, x + 20, y + 4, x + 35, y + 18, TFT_GREEN);
    tft.drawTriangle(x + 5, y + 18, x + 20, y + 4, x + 35, y + 18, TFT_DARKGREEN);

    // Magassági vonalak (szintvonalak) - nagyobb és több vonal
    tft.drawFastHLine(x + 8, y + 15, 24, TFT_WHITE);
    tft.drawFastHLine(x + 10, y + 12, 20, TFT_WHITE);
    tft.drawFastHLine(x + 13, y + 9, 14, TFT_WHITE);
    tft.drawFastHLine(x + 16, y + 7, 8, TFT_WHITE);

    // Magasság jelölő nyíl - nagyobb
    tft.drawFastVLine(x + 2, y + 2, 14, TFT_WHITE);
    tft.drawLine(x + 2, y + 2, x + 5, y + 5, TFT_WHITE);
    tft.drawLine(x + 2, y + 2, x - 1, y + 5, TFT_WHITE);

    // Felirat
    tft.setTextDatum(ML_DATUM); // Middle Left - bal oldal, középre igazítva
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setFreeFont();
    tft.setTextSize(1);
    tft.drawString("alti", x + 5, y + 27);
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
    tft.setTextDatum(ML_DATUM); // Middle Left - bal oldal, középre igazítva
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setFreeFont();
    tft.setTextSize(1);
    tft.drawString("max", x + 8, y + 20);
}

/**
 * Kezeli a képernyő saját ciklusát (dinamikus frissítés)
 */
void ScreenMain::handleOwnLoop() {

    // Update satellite information
    static long lastUpdate = millis() - 1000;
    if (!Utils::timeHasPassed(lastUpdate, 1000)) {
        return;
    }
    lastUpdate = millis();

    static uint8_t lastSatCount = 255;    // Kényszerített első frissítés
    static double lastSpeed = -1.0;       // Kényszerített első frissítés
    static double lastAltitude = -9999.0; // Kényszerített első frissítés
    static String lastDateTime = "";      // Kényszerített első frissítés

    char buf[11];

    // Műholdak száma
    uint8_t currentSatCount = gpsManager->getSatellites().isValid() && gpsManager->getSatellites().age() < GPS_DATA_MAX_AGE ? gpsManager->getSatellites().value() : 0;
#ifdef DEMO_MODE
    // Demo módban időnként változtatjuk a műholdak számát
    static unsigned long lastSatChange = 0;
    static uint8_t demoSatCount = 8; // Kezdő demo műholdszám

    if (millis() - lastSatChange > 2000) { // 2 másodpercenként változik
        demoSatCount = random(0, 16);      // Demo mód: 0-15 műhold
        lastSatChange = millis();
    }
    currentSatCount = demoSatCount;
#endif

    if (currentSatCount != lastSatCount) {
        // Műholdak száma az ikon mellé (jobbra)
        tft.setTextDatum(ML_DATUM); // Middle Left - bal oldal, középre igazítva
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setFreeFont(&FreeSansBold18pt7b);
        tft.setTextSize(1); // Normál font méret

        // Padding a szám számára
        tft.setTextPadding(tft.textWidth("88") + 10);

        // Szám pozíciója az ikon mellett
        tft.drawString(String(currentSatCount), 39, 12);
        lastSatCount = currentSatCount;
        tft.setFreeFont(); // Alapértelmezett font
    }

    // GPS dátum és idő ellenőrzése (helyi időzóna szerint korrigálva)
    char dateStr[11], timeStr[9];
    String currentDate = "";
    String currentTime = "";
    GpsManager::LocalDateTime localDateTime = gpsManager->getLocalDateTime();
    bool dateTimeValid = localDateTime.valid;

#ifdef DEMO_MODE
    // Demó dátum
    currentDate = "2025-08-23";

    // Demo módban az idő folyamatosan növekszik
    static unsigned long demoStartTime = millis();
    unsigned long elapsed = (millis() - demoStartTime) / 1000; // másodpercek

    // Kezdő idő: 17:42:30
    int startHour = 17, startMin = 42, startSec = 30;
    int totalSeconds = startHour * 3600 + startMin * 60 + startSec + elapsed;

    int hour = (totalSeconds / 3600) % 24;
    int minute = (totalSeconds % 3600) / 60;
    int second = totalSeconds % 60;

    sprintf(timeStr, "%02d:%02d:%02d", hour, minute, second);
    currentTime = String(timeStr);
    dateTimeValid = true;
#else
    if (dateTimeValid) {
        sprintf(dateStr, "%04d-%02d-%02d", localDateTime.year, localDateTime.month, localDateTime.day);
        sprintf(timeStr, "%02d:%02d:%02d", localDateTime.hour, localDateTime.minute, localDateTime.second);
        currentDate = String(dateStr);
        currentTime = String(timeStr);
    }
#endif

    String currentDateTime = currentDate + currentTime; // Összefűzés a változás ellenőrzéséhez
    if (currentDateTime != lastDateTime) {

        tft.setTextSize(1);
        tft.setTextDatum(ML_DATUM); // Middle Left - bal oldal, középre igazítva
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        // GPS dátum az ikon mellett (jobbra), idő alatta
        tft.setFreeFont(&FreeSansBold9pt7b);
        tft.setTextPadding(tft.textWidth("8888888888") + 10);
        tft.drawString(dateTimeValid ? currentDate : "----/--/--", ::SCREEN_W / 2 - 25, 10);

        // Idő kiírása
        tft.setFreeFont(&FreeSansBold18pt7b);
        tft.setTextPadding(tft.textWidth("88:88:88") + 10);
        tft.drawString(dateTimeValid ? currentTime : "--:--:--", ::SCREEN_W / 2 - 60, 40);

        lastDateTime = currentDateTime;
        tft.setFreeFont(); // Alapértelmezett font
    }

    // Magasság ellenőrzése
    double currentAltitude = 0.0;
    bool altitudeValid = gpsManager->getAltitude().isValid() && gpsManager->getAltitude().age() < GPS_DATA_MAX_AGE;
#ifdef DEMO_MODE
    // Demo módban időnként változtatjuk a magasságot
    static unsigned long lastAltChange = 0;
    static double demoAltitude = 150.0; // Kezdő demo magasság

    if (millis() - lastAltChange > 3000) { // 3 másodpercenként változik
        demoAltitude = random(50, 2000);   // Demo mód: 50-2000m
        lastAltChange = millis();
    }
    currentAltitude = demoAltitude;
    altitudeValid = true;
#else
    if (altitudeValid) {
        currentAltitude = gpsManager->getAltitude().meters();
    }
#endif

    if (abs(currentAltitude - lastAltitude) > 1.0 || (altitudeValid != (lastAltitude != -9999.0))) {
        // Magasság jobb oldalra igazítva az ikon után
        tft.setTextDatum(MR_DATUM); // Middle Right - jobb oldal, középre igazítva
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setFreeFont();  // Alapértelmezett font
        tft.setTextSize(2); // Normál font méret

        // Padding a magasság számára
        int paddingWidth = tft.textWidth("8888m", 2) + 10;
        tft.setTextPadding(paddingWidth);

        // Szám pozíciója a képernyő jobb szélén (x=SCREEN_W-5, y=10 az ikon közepe)
        String altText = (altitudeValid ? String((int)currentAltitude) : "-- ") + "m";
        tft.drawString(altText, ::SCREEN_W - 5, 10, 2); // Font méret 2, jobb szélre igazítva
        lastAltitude = altitudeValid ? currentAltitude : -9999.0;
    }

    // Sebesség ellenőrzése
    // Aktuális sebesség
    int16_t currentSpeed = 0;
    bool speedValid = gpsManager->getSpeed().isValid();

#ifdef DEMO_MODE
    // Demo módban időnként változtatjuk a sebességet
    static unsigned long lastSpeedChange = 0;
    static int16_t demoSpeed = 80; // Kezdő demo sebesség

    if (millis() - lastSpeedChange > 1500) { // 1.5 másodpercenként változik
        demoSpeed = random(0, 150);          // Demo mód: 0-150 km/h
        lastSpeedChange = millis();
    }
    currentSpeed = demoSpeed;
    speedValid = true;
#else
    if (speedValid) {
        currentSpeed = gpsManager->getSpeed().kmph();
    }
#endif

    if (abs(currentSpeed - lastSpeed) > 0.1 || (gpsManager->getSpeed().isValid() != (lastSpeed >= 0))) {
        // Sebesség középen Large_Font-al - csak ha változott
        tft.loadFont(Arial_Narrow_Bold120);
        tft.setTextDatum(MC_DATUM);                    // vízszintes közép
        tft.setTextPadding(tft.textWidth("888") + 10); // Padding a villogás ellen

        if (speedValid) {
            dtostrf(currentSpeed, 0, 0, buf);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.drawString(buf, ::SCREEN_W / 2, 240);
        } else {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.drawString("0", ::SCREEN_W / 2, 240);
        }
        tft.unloadFont(); // Visszaállítjuk az alapértelmezett fontot
        lastSpeed = speedValid ? currentSpeed : -1.0;
    }
}