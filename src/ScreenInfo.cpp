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

// Demó mód
extern bool demoMode;

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

    // 2. tábla prompt - Azért kell a 2. tábla promptjait előbb kiírni,
    // mert az MR_DATUM törli az előtte lévő teljes tartalmat, így az 1. tábla promptokat is
    tableX = 330;
    tableY = 190;
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

    tft.drawString("Battery", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Temp External", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Temp CPU Core", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Boot time", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Mode", tableX, tableY);
    tableY += lineHeight;

    // 1. tábla prompt
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
    tft.drawString("Date", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Time", tableX, tableY);
    tableY += lineHeight;
}

/**
 * Kezeli a képernyő saját ciklusát (dinamikus frissítés)
 */
void ScreenInfo::handleOwnLoop() {

    // 1 másodperces frissítés
    static long lastUpdate = millis() - 5000;
    if (!Utils::timeHasPassed(lastUpdate, 5000)) {
        return;
    }
    lastUpdate = millis();

    // Sprite paraméterek
    int spriteX = 130;
    int spriteY = 187;
    constexpr int spriteW = 50;
    constexpr int spriteH = 150;

    constexpr int lineHeight = 10;

    TFT_eSprite infoSprite(&tft);

    // 1. oszlop
    infoSprite.createSprite(spriteW, spriteH);
    infoSprite.fillSprite(TFT_BLACK);
    infoSprite.setTextDatum(TL_DATUM);
    infoSprite.setTextColor(TFT_YELLOW, TFT_BLACK);
    infoSprite.setFreeFont();
    infoSprite.setTextSize(1);

    int y = 0;
    infoSprite.drawString(String(gpsManager->getSatellites().value()), 0, y);
    y += lineHeight;
    infoSprite.drawString(String(gpsManager->getSatelliteCountForUI()), 0, y);
    y += lineHeight;
    infoSprite.drawString(String(gpsManager->getLocation().lat(), 6), 0, y);
    y += lineHeight;
    infoSprite.drawString(String(gpsManager->getLocation().lng(), 6), 0, y);
    y += lineHeight;
    infoSprite.drawString(String(gpsManager->getAltitude().meters(), 1) + "m", 0, y);
    y += lineHeight;
    infoSprite.drawString(String(gpsManager->getGpsQualityString()), 0, y);
    y += lineHeight;
    infoSprite.drawString(String(gpsManager->getGpsModeToString()), 0, y);
    y += lineHeight;
    infoSprite.drawString(String(gpsManager->getSpeed().kmph(), 1) + "km/h", 0, y);
    y += lineHeight;
    infoSprite.drawString(String(gpsManager->getHdop().hdop(), 2), 0, y);
    y += lineHeight;

    char tmpBuf[20];
    GpsManager::LocalDateTime localDateTime = gpsManager->getLocalDateTime();

    sprintf(tmpBuf, "%04d-%02d-%02d", localDateTime.year, localDateTime.month, localDateTime.day);
    infoSprite.drawString(String(tmpBuf), 0, y);
    y += lineHeight;

    sprintf(tmpBuf, "%02d:%02d:%02d", localDateTime.hour, localDateTime.minute, localDateTime.second);
    infoSprite.drawString(String(tmpBuf), 0, y);
    y += lineHeight;

    // Sprite kirajzolása
    infoSprite.pushSprite(spriteX, spriteY);
    infoSprite.deleteSprite();

    // 2. oszlop
    spriteX = 355;
    spriteY = 187;

    infoSprite.createSprite(spriteW, spriteH);
    infoSprite.fillSprite(TFT_BLACK);
    infoSprite.setTextDatum(TL_DATUM);
    infoSprite.setTextColor(TFT_YELLOW, TFT_BLACK);
    infoSprite.setFreeFont();
    infoSprite.setTextSize(1);

    y = 0;

    infoSprite.drawString(String(sensorUtils.readVBusExternal(), 1) + "V", 0, y);
    y += lineHeight;
    infoSprite.drawString(String(sensorUtils.readExternalTemperature(), 1) + "C", 0, y);
    y += lineHeight;
    infoSprite.drawString(String(sensorUtils.readCoreTemperature(), 1) + "C", 0, y);
    y += lineHeight;
    infoSprite.drawString(Utils::msecToString(gpsManager->getGpsBootTime()), 0, y);
    y += lineHeight;
    infoSprite.drawString(::demoMode ? "Demo" : "Normal", 0, y);
    y += lineHeight;

    // Sprite kirajzolása
    infoSprite.pushSprite(spriteX, spriteY);
    infoSprite.deleteSprite();
}