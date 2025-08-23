#ifndef __SCREEN_MAIN_H
#define __SCREEN_MAIN_H

#include "ButtonsGroupManager.h"
#include "MessageDialog.h"
#include "UIScreen.h"
#include "ValueChangeDialog.h"

/**
 * @file ScreenMain.h
 * @brief Test képernyő osztály, amely használja a ButtonsGroupManager-t
 */

class ScreenMain : public UIScreen, public ButtonsGroupManager<ScreenMain> {

  public:
    /**
     * @brief ScreenMain konstruktor
     * @param tft TFT display referencia
     */
    ScreenMain() : UIScreen(SCREEN_NAME_MAIN) {

        DEBUG("ScreenMain: Constructor called\n");
        layoutComponents();
    }
    virtual ~ScreenMain() = default;

    /**
     * @brief Loop hívás felülírása
     * animációs vagy egyéb saját logika végrehajtására
     * @note Ez a metódus nem hívja meg a gyerek komponensek loop-ját, csak saját logikát tartalmaz.
     */
    virtual void handleOwnLoop() override {}

    /**
     * @brief Kirajzolja a képernyő saját tartalmát
     */
    virtual void drawContent() override {
        // Szöveg középre igazítása
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_COLOR_BACKGROUND);
        tft.setFreeFont(); // Alapértelmezett (kisebb) font
        tft.setTextSize(3);

        // Képernyő cím kirajzolása
        tft.drawString(SCREEN_NAME_MAIN, ::SCREEN_W / 2, ::SCREEN_H / 2 - 20);

        // Információs szöveg
        tft.setTextSize(1);
        tft.drawString("ScreenMain for debugging", ::SCREEN_W / 2, ::SCREEN_H / 2 + 20);
    }

  private:
    /**
     * @brief UI komponensek létrehozása és elhelyezése
     */
    void layoutComponents() {}
};

#endif // __SCREEN_MAIN_H