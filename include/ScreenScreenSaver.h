#pragma once

#include "ButtonsGroupManager.h"
#include "MessageDialog.h"
#include "UIScreen.h"
#include "ValueChangeDialog.h"

/**
 * @file ScreenMain.h
 * @brief Test képernyő osztály, amely használja a ButtonsGroupManager-t
 */

class ScreenScreenSaver : public UIScreen, public ButtonsGroupManager<ScreenScreenSaver> {

  public:
    /**
     * @brief ScreenScreenSaver konstruktor
     * @param tft TFT display referencia
     */
    ScreenScreenSaver();

    /**
     * @brief ScreenScreenSaver destruktor
     */
    virtual ~ScreenScreenSaver();

    /**
     * @brief Loop hívás felülírása
     * animációs vagy egyéb saját logika végrehajtására
     * @note Ez a metódus nem hívja meg a gyerek komponensek loop-ját, csak saját logikát tartalmaz.
     */
    virtual void handleOwnLoop() override;

    /**
     * @brief Érintés esemény kezelése
     * @param event Érintés esemény adatok
     * @return true ha kezelte az eseményt (mindig), false egyébként
     * @details Bármilyen érintés ébreszti a képernyővédőt és visszatér az előző képernyőre
     */
    virtual bool handleTouch(const TouchEvent &event) override;

    /**
     * @brief Kirajzolja a képernyő saját tartalmát
     */
    virtual void drawContent() override;

  private:
    /**
     * @brief UI komponensek létrehozása és elhelyezése
     */
    void layoutComponents();
};
