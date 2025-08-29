#pragma once
#include "UIButton.h"
#include "UIScreen.h"

/**
 * @brief Info képernyő osztály
 */
class ScreenInfo : public UIScreen {

  public:
    /**
     * @brief ScreenInfo konstruktor
     */
    ScreenInfo() : UIScreen(SCREEN_NAME_INFO) { layoutComponents(); }
    virtual ~ScreenInfo() = default;

    /**
     * @brief Kirajzolja a képernyő saját tartalmát
     */
    virtual void drawContent() override;

    /**
     * @brief Loop hívás felülírása
     * animációs vagy egyéb saját logika végrehajtására
     * @note Ez a metódus nem hívja meg a gyerek komponensek loop-ját, csak saját logikát tartalmaz.
     */
    virtual void handleOwnLoop() override;

  private:
    /**
     * @brief UI komponensek létrehozása és elhelyezése
     */
    void layoutComponents();
};
