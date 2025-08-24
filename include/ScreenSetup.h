#ifndef __SCREEN_SETUP_H
#define __SCREEN_SETUP_H

#include "UIButton.h"
#include "UIScreen.h"

/**
 * @brief Setup képernyő osztály
 */
class ScreenSetup : public UIScreen {

  public:
    /**
     * @brief ScreenSetup konstruktor
     */
    ScreenSetup() : UIScreen(SCREEN_NAME_SETUP) { layoutComponents(); }
    virtual ~ScreenSetup() = default;

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

#endif // __SCREEN_SETUP_H
