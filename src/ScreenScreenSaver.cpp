#include "ScreenScreenSaver.h"

#include "TftBackLightAdjuster.h"
extern TftBackLightAdjuster tftBackLightAdjuster;

#include "GpsManager.h"
extern GpsManager *gpsManager;

/**
 * @brief Konstruktor
 */
ScreenScreenSaver::ScreenScreenSaver() : UIScreen(SCREEN_NAME_SCREENSAVER) {
    DEBUG("ScreenScreenSaver: Constructor called\n");
    layoutComponents();
}

/**
 * @brief Destruktor
 */
ScreenScreenSaver::~ScreenScreenSaver() = default;

/**
 * @brief Képernyő komponensek elrendezése
 */
void ScreenScreenSaver::layoutComponents() {}

/**
 * @brief Képernyő aktiválása
 *
 * Meghívódik amikor a képernyő aktívvá válik (pl. visszatérés Info/Setup képernyőről)
 */
void ScreenScreenSaver::activate() {

    tft.fillScreen(TFT_BLACK);

    // Kényszeríti a háttérvilágítást a minimum szintre
    tftBackLightAdjuster.setBacklightLevel(NIGHTLY_BRIGHTNESS);
}

/**
 * @brief Érintés esemény kezelése
 */
bool ScreenScreenSaver::handleTouch(const TouchEvent &event) {
    if (event.pressed) {
        if (getScreenManager()) {
            getScreenManager()->goBack();
            Utils::beepTick();
        }
        return true;
    }
    return false;
}

/**
 * @brief Képernyő saját tartalmának kirajzolása
 */
void ScreenScreenSaver::drawContent() {}

/**
 * @brief Loop hívás felülírása
 */
void ScreenScreenSaver::handleOwnLoop() {

    // 15 másodpercenként frissítünk
    static uint32_t lastUpdate = 0;
    if (!Utils::timeHasPassed(lastUpdate, 15000)) {
        return;
    }
    lastUpdate = millis();

    // GPS idő lekérése
    GpsManager::LocalDateTime localDateTime = gpsManager->getLocalDateTime();

    // Idő
    char timeStr[9] = "--:--";

    if (localDateTime.timeValid) {
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", localDateTime.hour, localDateTime.minute);
    }

    constexpr uint8_t fontSize = 6;

    tft.setFreeFont();
    tft.setTextSize(fontSize);

    static uint32_t lastX = 0, lastY = 0;
    constexpr uint8_t margin = 20;
    constexpr uint8_t fontHeight = 48;
    uint8_t fontWidth = tft.textWidth("88:88", fontSize); // Szélesség fix, mert mindig ugyanaz a karakterkészlet

    // Véletlenszerű pozíció generálása a képernyőn belül
    uint32_t maxX = ::SCREEN_W - fontWidth - margin;
    uint32_t maxY = ::SCREEN_H - fontHeight - margin;
    lastX = random(margin, maxX);
    lastY = random(margin, maxY);

    // Kirajzolás
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString(timeStr, lastX, lastY);
}
