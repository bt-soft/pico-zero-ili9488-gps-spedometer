#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "UIComponent.h"

class UIContainerComponent : public UIComponent {

  protected:
    std::vector<std::shared_ptr<UIComponent>> children;

  public:
    /**
     * @brief UIContainerComponent konstruktor
     * @param bounds A konténer határai (alapértelmezett: {0, 0, 0, 0})
     * @param colors A konténer színpalettája (alapértelmezett: ColorScheme::defaultScheme())
     */
    UIContainerComponent(const Rect &bounds = {0, 0, 0, 0}, const ColorScheme &colors = ColorScheme::defaultScheme()) : UIComponent(bounds, colors) {}

    /**
     *  @brief UIContainerComponent destruktor
     *  @details A destruktor felszabadítja a gyerek komponenseket, így azok nem maradnak "ott ragadt" elemek a képernyőváltáskor.
     *  @note A gyerek komponensek shared_ptr-ek, így automatikusan felszabadulnak, ha már nincs rájuk szükség.
     */
    virtual ~UIContainerComponent() override {
        // Töröljük a gyerek komponenseket, hogy felszabadítsuk a memóriát
        children.clear(); // Eltávolítja az összes gyerek komponenst
    };

    /**
     * @brief Gyerek komponens hozzáadása a konténerhez.
     * @param child A hozzáadandó gyerek komponens.
     */
    void addChild(std::shared_ptr<UIComponent> child) { children.push_back(child); }

    /**
     * @brief Gyerek komponens eltávolítása a konténerből.
     * @param child A eltávolítandó gyerek komponens.
     */
    void removeChild(std::shared_ptr<UIComponent> child) { children.erase(std::remove(children.begin(), children.end(), child), children.end()); }

    /**
     * @brief Gyerek komponensek listájának lekérése
     * @return A gyerek komponensek konstans referenciája
     */
    const std::vector<std::shared_ptr<UIComponent>> &getChildren() const { return children; }

    /**
     * @brief Touch esemény kezelése, amely először a gyerek komponenseken próbálkozik,
     * majd ha egyik sem, akkor maga a UIContainerComponent kezeli.
     * @param event A touch esemény, amely tartalmazza a koordinátákat és a lenyomás állapotát.
     * @return true, ha a touch eseményt egy gyerek komponens kezelte, vagy maga a UIContainerComponent.
     */
    virtual bool handleTouch(const TouchEvent &event) override {

        // Ha tiltott a UIContainerComponent, akkor nem hívjuk meg a touch-ot sem magára, sem a gyerekekre
        if (UIComponent::disabled) {
            return false;
        }

        // 1. Gyerekek kezelik először (a legfelső kapja meg először - fordított iteráció)
        for (auto it = children.rbegin(); it != children.rend(); ++it) {

            // Csak az aktív gyerekeknek adjuk tovább
            if (!(*it)->isDisabled() && (*it)->handleTouch(event)) {
                return true; // Ha egy gyerek már feldolgozta -> nem megyünk tovább
            }
        }

        // 2. Ha egyik gyerek sem kezelte, akkor a UIContainerComponent maga (mint UIComponent) próbálja meg
        // Az UIComponent::handleTouch ellenőrzi a 'bounds'-ot és hívja az onTouchDown/onClick stb. metódusokat.
        return UIComponent::handleTouch(event);
    }

    /**
     * @brief Jelzi, hogy a konténert újra kell rajzolni.
     * @param markChildren Ha true, akkor a gyerekeket is megjelöli (alapértelmezett: false)
     */
    virtual void markForRedraw(bool markChildren = false) override {

        UIComponent::markForRedraw(); // A konténert újra kell rajzolni

        // Mindegy gyereket is megjelölünk újrarajzolásra, ha kell
        if (markChildren) {
            for (auto &child : children) {
                if (child)
                    child->markForRedraw(true);
            }
        }
    }

    /**
     * @brief Loop metódus, amely először saját loop logikáját kezeli, majd a gyerek komponensek loop-ját hívja meg.
     */
    virtual void loop() override {

        // Ha tiltott a UIContainerComponent, akkor nem hívjuk meg a loop-ot sem magára, sem a gyerekekre
        if (UIComponent::isDisabled()) {
            return;
        }

        // Először saját loop logika (UIComponent::loop() + UIContainerComponent specifikus)
        UIComponent::loop(); // Hívja az UIComponent alap loopját (ami jelenleg üres, de lehetne benne logika)
        handleOwnLoop();     // Hívja a UIContainerComponent specifikus loopját (ha van)

        // Majd minden aktív gyerek megkapja (megszakítás nélkül)
        for (auto &child : children) {
            if (!child->isDisabled()) {
                child->loop();
            }
        }
    }

    /**
     * @brief Rajzolás metódus, amely először saját maga rajzolását kezeli, majd a gyerek komponenseket.
     */
    virtual void draw() override {

        // 1. Saját maga rajzolása (mint UIComponent), ha szükséges
        // Az UIComponent::needsRedraw flag-et az UIComponent maga kezeli.
        // Ha a UIContainerComponent-nek van saját vizuális megjelenése (pl. háttér),
        // azt a drawSelf()-ben kell implementálni.
        if (UIComponent::isRedrawNeeded()) {  // Ellenőrzi a UIComponent::needsRedraw flag-et
            drawSelf();                       // Leszármazott implementálja, ha van mit rajzolnia (pl. háttér)
            UIComponent::needsRedraw = false; // Fontos: töröljük a flag-et, miután a "saját" rajzolás megtörtént
        }

        // 2. Gyerekek rajzolása (csak ha szükséges újrarajzolás)
        for (auto &child : children) {
            if (child->isRedrawNeeded()) {
                child->draw();
            }
        }
    }

    /**
     * @brief Ellenőrzi, hogy szükséges-e újrarajzolás.
     * @return true, ha a konténer vagy bármelyik gyerek komponens igényel újrarajzolást.
     */
    virtual bool isRedrawNeeded() const override {

        if (UIComponent::isRedrawNeeded()) { // Ellenőrzi a konténer a saját needsRedraw flag-jét
            return true;
        }

        // Ellenőrizzük a gyerek komponenseket is
        for (const auto &child : children) {
            if (child->isRedrawNeeded()) {
                return true;
            }
        }
        return false; // Sem a konténer, sem egyik gyereke sem igényel újrarajzolást
    }

  protected:
    // Ezeket a leszármazott osztályok implementálhatják specifikus logikához
    virtual void handleOwnLoop() {} // Pl. animációkhoz a konténeren belül
    virtual void drawSelf() {}      // Pl. háttér vagy keret rajzolása a konténeren belül
};
