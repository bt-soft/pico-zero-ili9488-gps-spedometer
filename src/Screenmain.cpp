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
    // Műhold ikon bal felső sarokban (statikus)
    drawSatelliteIcon(10, 10);

    // Magasság ikon jobb felső sarokban (statikus)
    drawAltitudeIcon(::SCREEN_W - 40, 10);

    // Naptár ikon fent középen (statikus)
    drawCalendarIcon(::SCREEN_W / 2 - 15, 10);

    // Információs szöveg alul (statikus)
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(); // Alapértelmezett font
    tft.setTextSize(1);

    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    tft.drawString("Hdop", 40, 90, 2);
    tft.drawString("Max Speed", 440, 90, 2);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("km/h", tft.width() / 2, 105, 2);
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
}

/**
 * Magasság ikon rajzolása (hegy + magassági vonalak)
 */
void ScreenMain::drawAltitudeIcon(int16_t x, int16_t y) {
    // Hegy/domb alakzat
    tft.fillTriangle(x + 5, y + 12, x + 15, y + 4, x + 25, y + 12, TFT_GREEN);
    tft.drawTriangle(x + 5, y + 12, x + 15, y + 4, x + 25, y + 12, TFT_DARKGREEN);

    // Magassági vonalak (szintvonalak)
    tft.drawFastHLine(x + 8, y + 10, 14, TFT_WHITE);
    tft.drawFastHLine(x + 10, y + 8, 10, TFT_WHITE);
    tft.drawFastHLine(x + 12, y + 6, 6, TFT_WHITE);

    // Magasság jelölő nyíl
    tft.drawFastVLine(x + 2, y + 2, 10, TFT_WHITE);
    tft.drawLine(x + 2, y + 2, x + 4, y + 4, TFT_WHITE);
    tft.drawLine(x + 2, y + 2, x, y + 4, TFT_WHITE);
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
 * Kezeli a képernyő saját ciklusát (dinamikus frissítés)
 */
void ScreenMain::handleOwnLoop() {

    // Update satellite information
    static long lastUpdate = millis();
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
    currentSatCount = random(0, 16); // Demo mód
#endif

    if (currentSatCount != lastSatCount) {
        // Műholdak száma az ikon alatt - csak a számot frissítjük
        tft.setTextDatum(TC_DATUM); // Top Center
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setFreeFont(); // Alapértelmezett font
        tft.setTextSize(1);
        tft.setTextPadding(tft.textWidth("99")); // Padding a villogás ellen

        // Szám pozíciója az ikon alatt (x=10+12=22 az ikon közepe, y=10+24 az ikon alatt)
        tft.drawString(String(currentSatCount), 22, 34, 2);
        lastSatCount = currentSatCount;
    }

    // Magasság ellenőrzése
    double currentAltitude = 0.0;
    bool altitudeValid = gpsManager->getAltitude().isValid() && gpsManager->getAltitude().age() < GPS_DATA_MAX_AGE;
#ifdef DEMO_MODE
    currentAltitude = random(50, 2000); // Demo mód: 50-2000m
    altitudeValid = true;
#endif

    if (altitudeValid) {
        currentAltitude = gpsManager->getAltitude().meters();
    }

    if (abs(currentAltitude - lastAltitude) > 1.0 || (altitudeValid != (lastAltitude != -9999.0))) {
        // Magasság jobb felső sarokban - csak ha változott
        tft.setTextDatum(TC_DATUM); // Top Center
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setFreeFont(); // Alapértelmezett font
        tft.setTextSize(1);
        tft.setTextPadding(tft.textWidth("9999m")); // Padding a villogás ellen

        // Szám pozíciója az ikon alatt (jobb felső sarok)
        if (altitudeValid) {
            String altText = String((int)currentAltitude) + "m";
            tft.drawString(altText, ::SCREEN_W - 15, 26, 1);
        } else {
            tft.drawString("--m", ::SCREEN_W - 15, 26, 1);
        }
        lastAltitude = altitudeValid ? currentAltitude : -9999.0;
    }

    // GPS dátum és idő ellenőrzése (helyi időzóna szerint korrigálva)
    String currentDate = "";
    String currentTime = "";
    GpsManager::LocalDateTime localDateTime = gpsManager->getLocalDateTime();
    bool dateTimeValid = localDateTime.valid;

#ifdef DEMO_MODE
    currentDate = "2025-08-23";
    currentTime = "17:42:30"; // Helyi idő (CEST)
    dateTimeValid = true;
#endif

    if (dateTimeValid) {
        char dateStr[11], timeStr[9];
        sprintf(dateStr, "%04d-%02d-%02d", localDateTime.year, localDateTime.month, localDateTime.day);
        sprintf(timeStr, "%02d:%02d:%02d", localDateTime.hour, localDateTime.minute, localDateTime.second);
        currentDate = String(dateStr);
        currentTime = String(timeStr);
    }

    String currentDateTime = currentDate + currentTime; // Összefűzés a változás ellenőrzéséhez
    if (currentDateTime != lastDateTime) {
        // GPS dátum és idő a naptár ikon alatt - csak ha változott
        tft.setTextDatum(TC_DATUM); // Top Center
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setFreeFont(); // Alapértelmezett font
        tft.setTextSize(1);
        tft.setTextPadding(tft.textWidth("8888-88-88")); // Padding a villogás ellen

        // Pozíció a naptár ikon alatt (középen) - első sor: dátum
        if (dateTimeValid) {
            tft.drawString(currentDate, ::SCREEN_W / 2, 40, 1);
            tft.drawString(currentTime, ::SCREEN_W / 2, 52, 1); // Második sor: idő (12 pixel lejjebb)
        } else {
            tft.drawString("----/--/--", ::SCREEN_W / 2, 40, 1);
            tft.drawString("--:--:--", ::SCREEN_W / 2, 52, 1);
        }
        lastDateTime = currentDateTime;
    }

    // Sebesség ellenőrzése
    // Aktuális sebesség
    int16_t currentSpeed = 0; //(int16_t)(gpsManager->getSpeed().isValid() && gpsManager->getSpeed().age() < GPS_DATA_MAX_AGE && >= 4 ? gpsManager->getSpeed().kmph() : 0);
#ifdef DEMO_MODE
    currentSpeed = random(0, 288); // demo mód
#endif

    bool speedValid = gpsManager->getSpeed().isValid();
    if (speedValid) {
        currentSpeed = gpsManager->getSpeed().kmph();
    }

    if (abs(currentSpeed - lastSpeed) > 0.1 || (gpsManager->getSpeed().isValid() != (lastSpeed >= 0))) {
        // Sebesség középen Large_Font-al - csak ha változott
        tft.loadFont(Arial_Narrow_Bold120);
        tft.setTextDatum(MC_DATUM);               // vízszintes közép
        tft.setTextPadding(tft.textWidth("888")); // Padding a villogás ellen

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