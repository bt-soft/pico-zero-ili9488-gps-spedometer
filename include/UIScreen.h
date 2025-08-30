#pragma once

#include "IScreenManager.h"
#include "UIContainerComponent.h"
#include "UIDialogBase.h"

class UIScreen : public UIContainerComponent {

  private:
    /**
     * @brief A képernyő egyedi neve
     */
    const char *name;

    /**
     * @brief ScreenManager referencia a képernyő váltásokhoz
     */
    IScreenManager *screenManager = nullptr;

    /**
     * @brief Dialógus stack gyors eléréshez (weak_ptr)
     */
    std::vector<std::weak_ptr<UIDialogBase>> dialogStack;
    /**
     * @brief Dialógus stack memória védelemhez (shared_ptr) - Dual stack rendszer
     */
    std::vector<std::shared_ptr<UIDialogBase>> dialogSharedStack;

    /**
     * @brief Jelenleg aktív dialógus referencia
     */
    std::shared_ptr<UIDialogBase> currentDialog;

    // ===================================================================
    // UI komponensek factory metódusok
    // ===================================================================

    /**
     * @brief a képernyő üres részére klikk kiváltotta újrarajzolás elkerülése
     * @return false, mert a képernyők nem adnak vizuális "lenyomott" visszajelzést a hátterükön
     */
    virtual bool allowsVisualPressedFeedback() const override { return false; }

    /**
     * @brief Dialógus védelem - megakadályozza a háttér tartalom rajzolását aktív dialógus alatt
     *
     * Ez a metódus centralizált védelmet biztosít minden UIScreen leszármazott számára.
     * Ha van aktív dialógus, akkor a képernyő saját tartalma nem rajzolódik ki,
     * ezzel megakadályozva a háttér tartalom villanását dialógus átmenetek során.
     */
    virtual void drawSelf() override;

    /**
     * @brief Képernyő specifikus tartalom rajzolása
     *
     * A leszármazott képernyők ebben a metódusban implementálhatják a saját rajzolási logikájukat.
     * Ez a metódus automatikusan védve van a dialógus védelem által - csak akkor hívódik meg,
     * ha nincs aktív dialógus.
     *
     * Alapértelmezés szerint üres implementáció.
     */
    virtual void drawContent() {}
    virtual void layoutComponents() {}

  public:
    /**
     * @brief Konstruktor képernyő névvel
     * @param name A képernyő egyedi neve
     *
     * A képernyő teljes display méretet használja automatikusan.
     */
    UIScreen(const char *name);

    /**
     * @brief Virtuális destruktor
     *
     * Automatikusan felszabadítja az összes erőforrást.
     */
    virtual ~UIScreen() = default;

    /**
     * @brief Képernyő egyedi nevének elkérése
     * @return A képernyő neve
     */
    const char *getName() const { return name; }

    /**
     * @brief ScreenManager beállítása
     * @param mgr ScreenManager referencia
     */
    void setScreenManager(IScreenManager *mgr) { screenManager = mgr; }

    /**
     * @brief ScreenManager referencia lekérése
     * @return IScreenManager referencia
     */
    IScreenManager *getScreenManager() const { return screenManager; }

    /**
     * @brief Paraméterek beállítása
     * @param params Paraméter pointer (képernyő specifikus típus)
     *
     * Képernyők közötti adatátadáshoz használható.
     * Felülírható a specifikus paraméter kezelés implementálásához.
     */
    virtual void setParameters(void *params) {}

    // ================================
    // Screen Lifecycle Management
    // ================================
    /**
     * @brief Képernyő aktiválása
     *
     * Meghívódik amikor a képernyő aktívvá válik.
     * Felülírható a specifikus aktiválási logika implementálásához.
     */
    virtual void activate() {}

    /**
     * @brief Képernyő deaktiválása
     *
     * Meghívódik amikor a képernyő inaktívvá válik.
     * Felülírható a specifikus deaktiválási logika implementálásához.
     */
    virtual void deactivate() {}

    // ================================
    // UIComponent Override Methods - Event Handling és Rendering
    // ================================

    /**
     * @brief Újrarajzolás szükségességének ellenőrzése
     * @return true ha újrarajzolás szükséges, false egyébként
     *
     * Ellenőrzi mind a képernyő, mind az aktív dialógusok újrarajzolási igényét.
     * A szülő UICompositeComponent logikát kiegészíti dialógus ellenőrzéssel.
     */
    virtual bool isRedrawNeeded() const override;

    /**
     * @brief Képernyő és dialógusok kirajzolása
     *
     * Rajzolási sorrend:
     * 1. Alap képernyő komponensek (UICompositeComponent::draw())
     * 2. Összes aktív dialógus a stack sorrendjében (alulról felfelé)
     *
     * A layered dialog rendszer magja - minden látható dialógust kirajzol
     * a megfelelő rétegzési sorrendben.
     */
    virtual void draw() override;

    /**
     * @brief Touch esemény kezelése
     * @param event Touch esemény adatok
     * @return true ha az esemény kezelve lett, false egyébként
     *
     * Event routing:
     * 1. Ha van aktív dialógus → dialógusnak továbbítja
     * 2. Egyébként → alap képernyő komponenseknek (UICompositeComponent)
     *
     * Ez biztosítja, hogy a felső dialógus mindig megkapja az eseményeket.
     */
    virtual bool handleTouch(const TouchEvent &event) override;

    /**
     * @brief Folyamatos loop hívás kezelése
     *
     * Loop routing:
     * - Ha van aktív dialógus → csak a dialógus loop-ja fut
     * - Egyébként → alap képernyő komponensek loop-ja (UICompositeComponent)
     *
     * Ez optimalizálja a teljesítményt azáltal, hogy csak a szükséges
     * komponensek loop-ja fut.
     */
    virtual void loop() override;

    // ================================
    // Layered Dialog System
    // ================================

    /**
     * @brief Dialógus megjelenítése a layered dialog rendszerben
     * @param dialog Megjelenítendő dialógus (shared_ptr)
     *
     * A dialógus hozzáadódik a dual stack rendszerhez:
     * - _dialogStack: gyors elérés (weak_ptr)
     * - _dialogSharedStack: memória védelem (shared_ptr)
     *
     * A dialógus automatikusan megjelenik és aktívvá válik.
     * Az előző dialógus (ha van) inaktívvá válik, de látható marad.
     *
     * @see docs/LayeredDialogSystem.md#showDialog
     */
    virtual void showDialog(std::shared_ptr<UIDialogBase> dialog);

    /**
     * @brief Dialógus bezárásának kezelése
     * @param closedDialog A bezárt dialógus pointer
     *
     * Automatikusan meghívódik amikor egy dialógus bezáródik.
     * Kezeli a stack cleanup-ot és az előző dialógus visszaállítását.
     *
     * Funkciók:
     * - Dialógus eltávolítása mindkét stack-ből
     * - Előző dialógus aktiválása (ha van)
     * - Képernyő újrarajzolás (ha szükséges)
     * - Memória cleanup
     *
     * @see docs/LayeredDialogSystem.md#onDialogClosed
     */
    virtual void onDialogClosed(UIDialogBase *closedDialog);

    /**
     * @brief Dialógus aktivitás állapot ellenőrzése
     * @return true ha van aktív dialógus, false egyébként
     *
     * Használható új dialógusok megnyitása előtt a duplikáció elkerülésére.
     *
     * @example
     * if (!isDialogActive()) {
     *     showDialog(newDialog);
     * }
     */
    inline bool isDialogActive() const { return !dialogStack.empty(); }

  protected:
    // ===================================================================
    // Dialog cleanup helper methods
    // ===================================================================

    /**
     * @brief Dialógus cleanup végrehajtása rajzolás nélkül
     * @param closedDialog A bezárt dialógus pointer
     * @details Lemásolja az onDialogClosed logikáját, de kihagyja a draw() hívásokat.
     * Hasznos olyan esetekben, amikor a leszármazott osztály egyedi rajzolási logikát szeretne.
     */
    void performDialogCleanupWithoutDraw(UIDialogBase *closedDialog);
};
