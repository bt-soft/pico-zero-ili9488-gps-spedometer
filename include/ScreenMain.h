#ifndef __SCREEN_MAIN_H
#define __SCREEN_MAIN_H

#include "ButtonsGroupManager.h"
#include "GpsManager.h"
#include "Large_Font.h"
#include "MessageDialog.h"
#include "UIScreen.h"
#include "ValueChangeDialog.h"

// Globális GPS manager
extern GpsManager *gpsManager;

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
    virtual void handleOwnLoop() override;

    /**
     * @brief Kirajzolja a képernyő saját tartalmát
     */
    virtual void drawContent() override;

  private:
    /**
     * @brief UI komponensek létrehozása és elhelyezése
     */
    void layoutComponents();

    /**
     * @brief Műhold ikon rajzolása
     */
    void drawSatelliteIcon(int16_t x, int16_t y);

    /**
     * @brief Magasság ikon rajzolása
     */
    void drawAltitudeIcon(int16_t x, int16_t y);

    /**
     * @brief Naptár ikon rajzolása
     */
    void drawCalendarIcon(int16_t x, int16_t y);
};

#endif // __SCREEN_MAIN_H