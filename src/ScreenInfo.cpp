#include "ScreenInfo.h"
#include "Config.h"
#include "Utils.h"
#include "defines.h"

#include "TrafipaxManager.h"
extern TrafipaxManager trafipaxManager;

#include "GpsManager.h"
extern GpsManager *gpsManager;

#include "SensorUtils.h"
extern SensorUtils sensorUtils;

/**
 * UI komponensek elhelyezése
 */
void ScreenInfo::layoutComponents() {

    // Vissza gomb
    addChild(std::make_shared<UIButton>( //                                                                                                                                  //
        1,                               //
        Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), //
        "Back", UIButton::ButtonType::Pushable, [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->goBack();
            }
        }));
}

/**
 * Kirajzolja a képernyő saját tartalmát
 */
void ScreenInfo::drawContent() {
    // Háttér törlése
    tft.fillScreen(TFT_BLACK);

    // Címsor
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Info Screen", ::SCREEN_W / 2, 20);

    // Információs szöveg
    tft.setFreeFont();
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(String("GPS Speedometer ") + PROGRAM_VERSION, ::SCREEN_W / 2, 60);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString(PROGRAM_AUTHOR, ::SCREEN_W / 2, 80);

    // Táblázat prompt
    tft.setTextSize(1);
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

    uint16_t tableX = 100;
    uint16_t tableY = 110;
    constexpr uint16_t lineHeight = 10;

    tft.drawString("CPU", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("TFT", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Temp Sensor", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("GPS Module", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Compiled", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Trafipax DB size", tableX, tableY);

    // Táblázat Értékek
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    tableX = 130;
    tableY = 110;
    tft.drawString("Raspberry Pi Pico Zero", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("ILI9488", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("DS18B20", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("uBlox Neo-6M/8M", tableX, tableY);
    tableY += lineHeight;
    tft.drawString(String(__DATE__) + " " + String(__TIME__), tableX, tableY);
    tableY += lineHeight;
    tft.drawString(String(::trafipaxManager.count()), tableX, tableY);

    // Változó adatok prompt
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

    tableX = 100;
    tableY = 190;

    tft.drawString("Satellites", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Sat in DB", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Lat", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Lng", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Alt", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Quality", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Mode", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Speed", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("HDop", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Time", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Bat", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Temp", tableX, tableY);
    tableY += lineHeight;
}

/**
 * Kezeli a képernyő saját ciklusát (dinamikus frissítés)
 */
void ScreenInfo::handleOwnLoop() {

    // 1 másodperces frissítés
    static long lastUpdate = millis() - 1000;
    if (!Utils::timeHasPassed(lastUpdate, 1000)) {
        return;
    }
    lastUpdate = millis();

    uint16_t tableX = 130;
    uint16_t tableY = 190;
    constexpr uint16_t lineHeight = 10;

    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setFreeFont();
    tft.setTextSize(1); // Kisebb szöveg, hogy elférjen

    // Háttér törlése csak a változó adatok területén
    tft.fillRect(tableX, tableY, 150, 200, TFT_BLACK);

    tft.drawString(String(gpsManager->getSatellites().value()), tableX, tableY);
    tableY += lineHeight;
    tft.drawString(String(gpsManager->getSatelliteCountForUI()), tableX, tableY);
    tableY += lineHeight;
    tft.drawString(String(gpsManager->getLocation().lat(), 6), tableX, tableY);
    tableY += lineHeight;
    tft.drawString(String(gpsManager->getLocation().lng(), 6), tableX, tableY);
    tableY += lineHeight;
    tft.drawString(String(gpsManager->getAltitude().meters(), 1) + "m", tableX, tableY);
    tableY += lineHeight;
    tft.drawString(String(gpsManager->getGpsQualityString()), tableX, tableY);
    tableY += lineHeight;
    tft.drawString(String(gpsManager->getGpsModeToString()), tableX, tableY);
    tableY += lineHeight;
    tft.drawString(String(gpsManager->getSpeed().kmph(), 1) + "km/h", tableX, tableY);
    tableY += lineHeight;
    tft.drawString(String(gpsManager->getHdop().hdop(), 2), tableX, tableY);
    tableY += lineHeight;

    char timeStr[20];
    GpsManager::LocalDateTime localDateTime = gpsManager->getLocalDateTime();
    sprintf(timeStr, "%02d:%02d:%02d", localDateTime.hour, localDateTime.minute, localDateTime.second);
    tft.drawString(String(timeStr), tableX, tableY);
    tableY += lineHeight;

    tft.drawString(String(sensorUtils.readVBusExternal(), 1) + "V", tableX, tableY);
    tableY += lineHeight;
    tft.drawString(String(sensorUtils.readExternalTemperature(), 1) + "C", tableX, tableY);
    tableY += lineHeight;
}