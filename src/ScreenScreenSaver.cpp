#include "ScreenScreenSaver.h"

ScreenScreenSaver::ScreenScreenSaver() : UIScreen(SCREEN_NAME_SCREENSAVER) {
    DEBUG("ScreenScreenSaver: Constructor called\n");
    layoutComponents();
}

ScreenScreenSaver::~ScreenScreenSaver() = default;

void ScreenScreenSaver::handleOwnLoop() {}

bool ScreenScreenSaver::handleTouch(const TouchEvent &event) {
    if (event.pressed) {
        if (getScreenManager()) {
            getScreenManager()->goBack();
        }
        return true;
    }
    return false;
}

void ScreenScreenSaver::drawContent() {
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_COLOR_BACKGROUND);
    tft.setFreeFont();
    tft.setTextSize(3);
    tft.drawString(SCREEN_NAME_SCREENSAVER, ::SCREEN_W / 2, ::SCREEN_H / 2 - 20);
    tft.setTextSize(1);
    tft.drawString("ScreenSaver", ::SCREEN_W / 2, ::SCREEN_H / 2 + 20);
}

void ScreenScreenSaver::layoutComponents() {}
