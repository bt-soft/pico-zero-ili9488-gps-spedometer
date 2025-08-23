#ifndef __UI_COMPONENT_H
#define __UI_COMPONENT_H

#include <Arduino.h>

#include "IScreenManager.h"
#include "UIColorPalette.h"
#include "defines.h"

// A main.cpp-ben definiálva
extern IScreenManager **iScreenManager;
extern TFT_eSPI tft;
extern uint16_t SCREEN_W;
extern uint16_t SCREEN_H;

// Touch esemény struktúra
struct TouchEvent {
    uint16_t x, y;
    bool pressed;

    TouchEvent(uint16_t x, uint16_t y, bool pressed) : x(x), y(y), pressed(pressed) {}
};

// Téglalap struktúra
struct Rect {
    int16_t x, y;
    uint16_t width, height;

    Rect(int16_t x = 0, int16_t y = 0, uint16_t width = 0, uint16_t height = 0) : x(x), y(y), width(width), height(height) {}

    bool contains(int16_t px, int16_t py) const { return px >= x && px < x + width && py >= y && py < y + height; }

    int16_t centerX() const { return x + width / 2; }
    int16_t centerY() const { return y + height / 2; }
};

class UIComponent {
  protected:
    Rect bounds;
    ColorScheme colors;

    bool disabled = false;      // Komponens tiltott állapot
    bool pressed = false;       // Komponens lenyomva állapot
    bool needsRedraw = true;    // Dirty flag, kinduláskor minden komponens újrarajzolást igényel
    uint32_t touchDownTime = 0; // Érintés kezdetének ideje, tagváltozó lett
    // Touch debounce
    uint32_t lastClickTime = 0;                             // Utolsó érvényes kattintás ideje
    static constexpr uint32_t DEFAULT_DEBOUNCE_DELAY = 200; // ms - Alapértelmezett debounce idő

    /**
     * @brief Debounce delay getter
     * @return A debounce delay értéke (alapértelmezett 200 ms)
     * @note A származtatott osztályok felülírhatják, ha más debounce időt szeretnének használni.
     */
    virtual uint32_t getDebounceDelay() const { return DEFAULT_DEBOUNCE_DELAY; }

    /**
     * @brief Touch margin getter
     * @return A touch margin értéke (alapértelmezett 0, de a származtatott osztályok felülírhatják)
     */
    virtual int16_t getTouchMargin() const { return 0; } // x pixel alapértelmezett tolerancia

    /**
     * @brief Ellenőrzi, hogy a megadott pont a komponens határain belül van-e (kiterjesztett érzékenységgel)
     * @param x A pont X koordinátája
     * @param y A pont Y koordinátája
     * @return true, ha a pont a komponens határain belül van (figyelembe véve a touch margin-t), false egyébként
     */
    virtual bool isPointInside(int16_t x, int16_t y) const {
        // Konfigurálható margin a pontosabb érintéshez - lehet hogy a származtatott osztályok felülírják
        const int16_t TOUCH_MARGIN = getTouchMargin();
        return (x >= bounds.x - TOUCH_MARGIN && x <= bounds.x + bounds.width + TOUCH_MARGIN && y >= bounds.y - TOUCH_MARGIN && y <= bounds.y + bounds.height + TOUCH_MARGIN);
    }

    /**
     * @brief A komponens jelezheti, hogy vizuális lenyomott visszajelzést ad-e
     * @details Ha true, akkor a komponens vizuális visszajelzést ad a lenyomásra (pl. színváltás).
     * @note Alapértelmezett szerint true, de a származtatott osztályok felülírhatják, ha nem kívánják ezt a viselkedést.
     * pl.: az UIScreen osztály nem ad vizuális visszajelzést a lenyomásra, mert a képernyő nem interaktív elem.
     * Emiatt ő majd felülírja ezt a metódust, hogy false-t adjon vissza.
     */
    virtual bool allowsVisualPressedFeedback() const { return false; }

    //
    // Eseménykezelő virtuális metódusok
    //

    virtual void onTouchDown(const TouchEvent &event) {
        // DEBUG("UIComponent: Touch DOWN at (%d,%d)\n", event.x, event.y);
    }

    virtual void onTouchUp(const TouchEvent &event) {
        // DEBUG("UIComponent: Touch UP at (%d,%d)\n", event.x, event.y);
    }

    virtual void onTouchCancel(const TouchEvent &event) {
        // DEBUG("UIComponent: Touch CANCEL at (%d,%d)\n", event.x, event.y);
    }

    virtual bool onClick(const TouchEvent &event) {
        // DEBUG("UIComponent: CLICK at (%d,%d)\n", event.x, event.y);
        return false; // Alapértelmezés szerint nem kezeli a kattintást, de a származtatott osztályok felülírhatják
    }

    /**
     * @brief Segédfüggvény a képernyőkezelő ellenőrzésére, hogy van-e aktív dialog
     * @return true, ha van aktív dialog a jelenlegi képernyőn
     */
    bool iscurrentScreenDialogActive() {
        // Segédfüggvény a dialog állapot ellenőrzéséhez
        if (::iScreenManager == nullptr || *::iScreenManager == nullptr) {
            return false; // Ha nincs képernyőkezelő, akkor nincs aktív dialog
        }
        return (*::iScreenManager)->isCurrentScreenDialogActive();
    }

  public:
    /**
     * @brief Konstruktor
     * @param bounds A komponens határai (Rect)
     * @param colors Színpaletta a komponenshez
     * @details A komponens alapértelmezett színpalettát használ, ha nem adunk meg másikat.
     * @note A bounds szélessége és magassága 0 esetén az alapértelmezett méreteket használja (DEFAULT_COMPONENT_WIDTH és DEFAULT_COMPONENT_HEIGHT).
     */
    UIComponent(const Rect &bounds, const ColorScheme &colors = ColorScheme::defaultScheme()) : bounds(bounds), colors(colors) {}

    /**
     * Destruktor
     */
    virtual ~UIComponent() = default;

    /**
     * @brief Touch esemény kezelése
     * @param event A touch esemény, amely tartalmazza a koordinátákat és a lenyomás állapotát
     * @return true, ha a touch eseményt a komponens kezelte, false egyébként
     * @note Alapértelmezés szerint kezeli a touch eseményeket, de leszármazott osztályok felülírhatják.
     */
    virtual bool handleTouch(const TouchEvent &event) {

        // Ha le van tiltva a komponens, akkor nem kezeljük az eseményt
        if (disabled) {
            return false;
        }

        bool inside = isPointInside(event.x, event.y);
        if (!inside) {
            return false; // Ha a touch esemény nem a komponens határain belül van, akkor nem kezeljük
        }

        bool touchHandled = false;

        bool previousPressedState = this->pressed; // Korábbi 'pressed' állapot mentése
        if (event.pressed && !pressed) {
            this->pressed = true;
            this->touchDownTime = millis(); // Tagváltozó használata
            onTouchDown(event);

            // Ha pressed állapot változott, újrarajzolás szükséges
            if (allowsVisualPressedFeedback() && previousPressedState != this->pressed) {
                markForRedraw();
            }

            touchHandled = true;

        } else if (!event.pressed && pressed) {

            this->pressed = false;
            uint32_t touchDuration = millis() - this->touchDownTime; // Tagváltozó használata

            // Kiterjesztett tolerancia a release eseményhez - ha a touch területen belül volt a lenyomás
            constexpr int16_t RELEASE_TOLERANCE = 8; // 8 pixel tolerancia a felengedéshez
            bool releaseInside =
                (event.x >= bounds.x - RELEASE_TOLERANCE && event.x <= bounds.x + bounds.width + RELEASE_TOLERANCE && event.y >= bounds.y - RELEASE_TOLERANCE && event.y <= bounds.y + bounds.height + RELEASE_TOLERANCE);

            onTouchUp(event);

            // Debounce logika: csak akkor kezeljük a kattintást, ha a touchDuration érvényes és nem túl gyors
            if (releaseInside && touchDuration >= 30 && touchDuration <= 2000) { // Érvényes touch duration feltételek
                if (millis() - lastClickTime > getDebounceDelay()) {
                    lastClickTime = millis(); // Frissítjük az utolsó kattintás idejét
                    touchHandled = onClick(event);
                    // DEBUG("UIComponent: Valid click detected (debounced): duration=%dms\n", touchDuration);
                } else {
                    // DEBUG("UIComponent: Click debounced (too fast): duration=%dms\n", touchDuration);
                }

            } else { // Nem volt érvényes kattintás (pl. túl rövid, túl hosszú, vagy kicsúszott)

                onTouchCancel(event);
                // DEBUG("UIComponent: Touch cancelled: releaseInside=%s duration=%dms\n", releaseInside ? "true" : "false", touchDuration);
            }

            // Ha pressed állapot változott, újrarajzolás szükséges
            if (allowsVisualPressedFeedback() && previousPressedState != this->pressed) {
                markForRedraw();
            }
        }

        return touchHandled;
    }

    // Rajzolás
    virtual void draw() = 0;

    // Loop hívás - ezt minden komponens megkapja, alapértelmezés szerint üres
    virtual void loop() {}

    // ================================
    // Getters/Setters
    // ================================

    // Bounds getter/setter
    virtual void setBounds(const Rect &newBounds) {
        bounds = newBounds;
        markForRedraw();
    }
    inline const Rect &getBounds() const { return bounds; }

    // Színséma getter/setter
    void setColorScheme(const ColorScheme &newColors) {
        colors = newColors;
        markForRedraw();
    }
    const ColorScheme &getColorScheme() const { return colors; }

    // Tiltott állapot getter/setter
    inline bool isDisabled() const { return disabled; }
    inline void setDisabled(bool disabled) { this->disabled = disabled; } // Újrarajzolás getter/setter
    virtual void markForRedraw(bool markChildren = false) { needsRedraw = true; }
    virtual bool isRedrawNeeded() const { return needsRedraw; }
};

#endif // __UI_COMPONENT_H