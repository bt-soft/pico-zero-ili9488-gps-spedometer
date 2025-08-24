#ifndef __SCREEN_INFO_H
#define __SCREEN_INFO_H

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
    ScreenInfo() : UIScreen(SCREEN_NAME_INFO) {
        DEBUG("ScreenInfo: Constructor called\n");
        layoutComponents();
    }
    virtual ~ScreenInfo() = default;

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
     * @brief Back gomb callback
     */
    void onBackButtonClicked(const UIButton::ButtonEvent &event);
};

#endif // __SCREEN_INFO_H
