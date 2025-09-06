#include "ScreenInfo.h"
#include "Config.h"
#include "Utils.h"
#include "defines.h"

#include "TraffipaxManager.h"
extern TraffipaxManager traffipaxManager;

#include "GpsManager.h"
extern GpsManager *gpsManager;

#include "SensorUtils.h"
extern SensorUtils sensorUtils;

// Demó mód
extern bool demoMode;

/**
 * @brief ScreenInfo konstruktor
 */
ScreenInfo::ScreenInfo() : UIScreen(SCREEN_NAME_INFO) {

    // Padding kiszámítása
    tft.setFreeFont();
    tft.setTextSize(1);
    textPadding = tft.textWidth("8888-88-88");
    bootTextPadding = tft.textWidth("88 mins, 88 sec, 888 msec");

    lastUpdated = 0;

    // Komponensek elrendezése
    layoutComponents();
}

/**
 * UI komponensek elhelyezése
 */
void ScreenInfo::layoutComponents() {

    // Vissza gomb
    addChild(std::make_shared<UIButton>( //                                                                                                                                  //
        1,                               //
        Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), //
        "Back",                                                                                                                                                           //
        UIButton::ButtonType::Pushable,                                                                                                                                   //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->goBack();
            }
        }) //
    );
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
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    snprintf(valueBuffer, sizeof(valueBuffer), "GPS Speedometer %s", PROGRAM_VERSION);
    tft.drawString(valueBuffer, ::SCREEN_W / 2, 50);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString(PROGRAM_AUTHOR, ::SCREEN_W / 2, 70);

    // Táblázat prompt
    tft.setTextSize(1);
    tft.setTextPadding(0);
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

    constexpr uint8_t lineHeight = 10;

    // 1. oszlop
    uint16_t tableX = 100;
    uint16_t tableY = 110;

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

    // 2. oszlop
    tableX = 330;
    tableY = 110;
    tft.drawString("CPU Clock", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Total Heap", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Free Heap", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("Used Heap", tableX, tableY);
    tableY += lineHeight;

    // Táblázat Értékek
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    // 1. oszlop értékek
    tableX = 120;
    tableY = 110;
    tft.drawString("Raspberry Pi Pico Zero", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("ILI9488", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("DS18B20", tableX, tableY);
    tableY += lineHeight;
    tft.drawString("uBlox Neo-6M/8M", tableX, tableY);
    tableY += lineHeight;
    snprintf(valueBuffer, sizeof(valueBuffer), "%s %s", __DATE__, __TIME__);
    tft.drawString(valueBuffer, tableX, tableY);
    tableY += lineHeight;
    snprintf(valueBuffer, sizeof(valueBuffer), "%d", ::traffipaxManager.count());
    tft.drawString(valueBuffer, tableX, tableY);

    // 2. oszlop értékek
    tableX = 350;
    tableY = 110;
    // snprintf(valueBuffer, sizeof(valueBuffer), "%.1f MHz", rp2040.f_cpu() / 1000000.0f); //valamiért elromlott az snprintf() float kiírása :()
    dtostrf(rp2040.f_cpu() / 1000000.0f, 0, 1, valueBuffer);
    tft.drawString(valueBuffer, tableX, tableY);
    tableY += lineHeight;
    // snprintf(valueBuffer, sizeof(valueBuffer), "%.1f kB", rp2040.getTotalHeap() / 1024.0f);
    dtostrf(rp2040.getTotalHeap() / 1024.0f, 0, 1, valueBuffer);
    tft.drawString(valueBuffer, tableX, tableY);
    tableY += lineHeight;
    // snprintf(valueBuffer, sizeof(valueBuffer), "%.1f kB", rp2040.getFreeHeap() / 1024.0f);
    dtostrf(rp2040.getFreeHeap() / 1024.0f, 0, 1, valueBuffer);
    tft.drawString(valueBuffer, tableX, tableY);
    tableY += lineHeight;
    // snprintf(valueBuffer, sizeof(valueBuffer), "%.1f kB", rp2040.getUsedHeap() / 1024.0f);
    dtostrf(rp2040.getUsedHeap() / 1024.0f, 0, 1, valueBuffer);
    tft.drawString(valueBuffer, tableX, tableY);
    tableY += lineHeight;

    // 2. tábla 2 oszlop prompt
    // mert az MR_DATUM törli az előtte lévő teljes tartalmat, így az 1. tábla promptokat is
    tableX = 330;
    tableY = 190;
    tft.setTextDatum(MR_DATUM);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

    tft.drawString("Accumulator", tableX, tableY);
    tableY += lineHeight;
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

    // 2. tábla 1 oszlop prompt
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

    // 5 másodperces frissítés
    if (!Utils::timeHasPassed(lastUpdated, 5000)) {
        return;
    }
    lastUpdated = millis();

    tft.setFreeFont();
    tft.setTextSize(1);
    tft.setTextPadding(textPadding);
    tft.setTextDatum(ML_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);

    // 2. oszlop
    uint16_t x = 350;
    uint16_t y = 190;
    constexpr uint8_t lineHeight = 10;

    // snprintf(valueBuffer, sizeof(valueBuffer), "%.2fV", sensorUtils.readVBusExternal()); //valamiért elromlott az snprintf() float kiírása :()
    dtostrf(sensorUtils.readVBusExternal(), 0, 2, valueBuffer);
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;
    dtostrf(sensorUtils.readVSysExternal(), 0, 2, valueBuffer);
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;
    dtostrf(sensorUtils.readExternalTemperature(), 0, 2, valueBuffer);
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;
    dtostrf(sensorUtils.readCoreTemperature(), 0, 2, valueBuffer);
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;

    // Nagyobb padding
    tft.setTextPadding(bootTextPadding);
    if (gpsManager->getGpsBootTime() == 0) {
        snprintf(valueBuffer, sizeof(valueBuffer), "--:--");
    } else {
        Utils::secToMinSecString(gpsManager->getGpsBootTime(), valueBuffer, sizeof(valueBuffer));
    }
    Utils::secToMinSecString(gpsManager->getGpsBootTime(), valueBuffer, sizeof(valueBuffer));
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;

    // Padding vissza
    tft.setTextPadding(textPadding);
    tft.drawString(::demoMode ? "Demo" : "Normal", x, y);
    y += lineHeight;

    // 1. oszlop
    x = 120;
    y = 190;

    snprintf(valueBuffer, sizeof(valueBuffer), "%u", gpsManager->getSatellites().value());
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;
    snprintf(valueBuffer, sizeof(valueBuffer), "%u", gpsManager->getSatelliteCountForUI());
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;
    // snprintf(valueBuffer, sizeof(valueBuffer), "%.6f", gpsManager->getLocation().lat());
    dtostrf(gpsManager->getLocation().lat(), 0, 6, valueBuffer);
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;
    // snprintf(valueBuffer, sizeof(valueBuffer), "%.6f", gpsManager->getLocation().lng());
    dtostrf(gpsManager->getLocation().lng(), 0, 6, valueBuffer);
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;
    // snprintf(valueBuffer, sizeof(valueBuffer), "%.1fm", gpsManager->getAltitude().meters());
    dtostrf(gpsManager->getAltitude().meters(), 0, 1, valueBuffer);
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;
    tft.drawString(gpsManager->getGpsQualityString(), x, y);
    y += lineHeight;
    tft.drawString(gpsManager->getGpsModeToString(), x, y);
    y += lineHeight;
    snprintf(valueBuffer, sizeof(valueBuffer), "%.1fkm/h", gpsManager->getSpeed().kmph());
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;
    // snprintf(valueBuffer, sizeof(valueBuffer), "%.2f", gpsManager->getHdop().hdop());
    dtostrf(gpsManager->getHdop().hdop(), 0, 2, valueBuffer);
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;

    GpsManager::LocalDateTime localDateTime = gpsManager->getLocalDateTime();
    sprintf(valueBuffer, "%04d-%02d-%02d", localDateTime.year, localDateTime.month, localDateTime.day);
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;

    sprintf(valueBuffer, "%02d:%02d:%02d", localDateTime.hour, localDateTime.minute, localDateTime.second);
    tft.drawString(valueBuffer, x, y);
    y += lineHeight;
}