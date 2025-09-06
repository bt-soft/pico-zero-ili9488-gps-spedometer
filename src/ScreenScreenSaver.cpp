#include "ScreenScreenSaver.h"

#include "TftBackLightAdjuster.h"
extern TftBackLightAdjuster tftBackLightAdjuster;

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
 * @brief Loop hívás felülírása
 */
void ScreenScreenSaver::handleOwnLoop() {}

/**
 * @brief Érintés esemény kezelése
 */
bool ScreenScreenSaver::handleTouch(const TouchEvent &event) {
    if (event.pressed) {
        if (getScreenManager()) {
            getScreenManager()->goBack();
        }
        return true;
    }
    return false;
}

/**
 * @brief Képernyő saját tartalmának kirajzolása
 */
void ScreenScreenSaver::drawContent() {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_COLOR_BACKGROUND);
    tft.setFreeFont();
    tft.setTextSize(3);
    tft.drawString(SCREEN_NAME_SCREENSAVER, ::SCREEN_W / 2, ::SCREEN_H / 2 - 20);
    tft.setTextSize(1);
    tft.drawString("ScreenSaver", ::SCREEN_W / 2, ::SCREEN_H / 2 + 20);
}

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
    // Kényszeríti a háttérvilágítást a minimum szintre
    tftBackLightAdjuster.setBacklightLevel(NIGHTLY_BRIGHTNESS);
}