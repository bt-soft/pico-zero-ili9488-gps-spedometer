#include "UIScreen.h"

// ================================
// Konstruktorok és inicializálás
// ================================
/**
 * @brief UIScreen konstruktor névvel
 * @param tft TFT display referencia
 * @param name Képernyő egyedi neve
 * * Automatikusan teljes képernyő méretet használ (0,0 SCREEN_W, SCREEN_H).
 * Az UIContainerComponent konstruktor hívása után a név beállítása történik.
 */
UIScreen::UIScreen(const char *name) : UIContainerComponent({0, 0, SCREEN_W, SCREEN_H}), name(name) {}

// ================================
// UIComponent Override Methods - Event Handling és Rendering
// ================================

/**
 * @brief Újrarajzolás szükségességének ellenőrzése
 * @return true ha újrarajzolás szükséges, false egyébként
 *
 * A metódus kompozit ellenőrzést végez:
 * 1. Szülő UIContainerComponent újrarajzolási igényének ellenőrzése
 * 2. Aktív dialógus újrarajzolási igényének ellenőrzése (ha van)
 *
 * Ez biztosítja, hogy mind az alapképernyő, mind a dialógusok
 * változásai megfelelően detektálódjanak.
 */
bool UIScreen::isRedrawNeeded() const {

    // Aktív dialógus újrarajzolási igény ellenőrzése
    if (isDialogActive()) {
        for (const auto &weakDialog : dialogStack) {
            auto dialog = weakDialog.lock();
            if (dialog && dialog->isRedrawNeeded()) {
                return true;
            }
        }
    }

    // Alapképernyő újrarajzolási igény ellenőrzése
    if (UIContainerComponent::isRedrawNeeded()) {
        return true;
    }

    // Ha egyik sem igényel újrarajzolást, akkor false-t adunk vissza
    return false;
}

/**
 * @brief Dialógus védelem - megakadályozza a háttér tartalom rajzolását aktív dialógus alatt
 *
 * Ez a centralizált védelem minden UIScreen leszármazott számára automatikusan működik.
 * Ha van aktív dialógus, akkor a képernyő saját tartalma nem rajzolódik ki,
 * ezzel megakadályozva a háttér tartalom villanását dialógus átmenetek során.
 *
 * A leszármazott képernyők a drawContent() metódusban implementálhatják a saját rajzolási logikájukat,
 * amely automatikusan védve lesz ez által a mechanizmus által.
 */
void UIScreen::drawSelf() {

    // Ne rajzoljuk ki a képernyő tartalmát, ha van aktív dialógus
    if (isDialogActive()) {
        return;
    }

    // Ha nincs aktív dialógus, hívjuk meg a leszármazott implementációt
    drawContent();
}

/**
 * @brief Képernyő és rétegzett dialógusok kirajzolása
 *
 * A draw metódus implementálja a layered dialog rendszer vizuális megjelenítését.
 *
 * Rajzolási sorrend (alulról felfelé):
 * 1. **Alapképernyő komponensek**: UIContainerComponent::draw() - gombok, szövegek, stb.
 * 2. **Rétegzett dialógusok**: Összes aktív dialógus a stack sorrendjében
 *
 * A rétegzési logika:
 * - A stack első eleme (index 0) = legalsó réteg
 * - A stack utolsó eleme (back()) = legfelső réteg
 * - Minden dialógus kirajzolódik
 *
 */
void UIScreen::draw() {

    // ===============================
    // 1. Alapképernyő komponensek rajzolása (alsó réteg)
    // ===============================
    UIContainerComponent::draw(); // Gombok, szövegek, egyéb UI elemek

    // ===============================
    // 2. Rétegzett dialógusok rajzolása (felső rétegek)
    // ===============================
    if (!dialogStack.empty()) {

        // Összes látható dialógus kirajzolása stack sorrendjében (alulról felfelé)
        int dialogCount = 0;
        for (auto &weakDialog : dialogStack) {
            auto dialog = weakDialog.lock();
            if (dialog) {
                dialog->draw();
                dialogCount++;
            }
        }
    }
}

/**
 * @brief Touch esemény kezelése és routing
 * @param event Touch esemény adatok (pozíció, típus, stb.)
 * @return true ha az esemény kezelve lett, false egyébként
 *
 * Event routing stratégia:
 * 1. **Dialog Priority**: Ha van aktív dialógus, az kapja meg az eseményt
 * 2. **Screen Fallback**: Ha nincs dialógus, az alapképernyő komponensek kapják
 *
 * Ez biztosítja a helyes event handling hierarchiát:
 * - A felső réteg (dialógus) mindig prioritást élvez
 * - Az alsó réteg (képernyő) csak akkor kap eseményt, ha a felső nem kezeli
 *
 * @note A visszatérési érték jelzi, hogy az esemény feldolgozásra került-e
 */
bool UIScreen::handleTouch(const TouchEvent &event) {

    if (isDialogActive()) {
        auto topDialog = dialogStack.back().lock();
        if (topDialog) {
            return topDialog->handleTouch(event);
        }
    }

    return UIContainerComponent::handleTouch(event);
}

/**
 * @brief Folyamatos loop kezelése és optimalizáció
 *
 * Loop routing stratégia:
 * - **Dialog Mode**: Ha van aktív dialógus, csak a dialógus loop-ja fut
 * - **Screen Mode**: Ha nincs dialógus, az alapképernyő komponensek loop-ja fut
 *
 * Teljesítmény optimalizáció:
 * Ez a megközelítés csökkenti a CPU terhelést azáltal, hogy egyszerre
 * csak a szükséges komponensek loop metódusai futnak.
 *
 * Használat:
 * - Animációk frissítése
 * - Időzített események kezelése
 * - Státusz frissítések
 * - Szenzorok olvasása
 */
void UIScreen::loop() {

    if (isDialogActive()) {
        auto topDialog = dialogStack.back().lock();
        if (topDialog) {
            topDialog->loop();
            // Process any deferred closes from the dialog
            topDialog->processDeferredClose();
        }

        // Ha van aktív dialógus, csak annak loop-ja fut
        return;
    }

    // Ha nincs aktív dialógus, akkor az alap képernyő komponensek loop-ja fut
    UIContainerComponent::loop();
}

// ================================
// Layered Dialog System Implementation
// ================================

/**
 * @brief Dialógus megjelenítése a layered dialog rendszerben
 * @param dialog Megjelenítendő dialógus shared_ptr
 *
 * A showDialog metódus implementálja a dual stack dialog rendszer magját:
 *
 * 1. Előző dialógus inaktiválása (ha van)
 * 2. Új dialógus hozzáadása mindkét stack-hez:
 *    - _dialogStack: weak_ptr (gyors elérés)
 *    - _dialogSharedStack: shared_ptr (memória védelem)
 * 3. Dialógus aktiválása és megjelenítése
 * 4. Képernyő újrarajzolási flag beállítása
 *
 * A dual stack rendszer biztosítja, hogy:
 * - A dialógusok ne kerüljenek korai destrukcióra
 * - Gyors elérés biztosított legyen a weak_ptr-eken keresztül
 * - Automatikus cleanup történjen a shared_ptr-ek segítségével
 *
 * @note Ha dialog nullptr, akkor a metódus nem csinál semmit
 */
void UIScreen::showDialog(std::shared_ptr<UIDialogBase> dialog) {

    if (dialog) {
        // 1. Előző dialógus inaktiválása (de látható marad)
        //    és a topDialog flagjének false-ra állítása
        if (!dialogStack.empty()) {
            auto previousTopDialog = dialogStack.back().lock();
            if (previousTopDialog) {
                previousTopDialog->setTopDialog(false);
            }
        }

        // 2. Dual stack tárolás - a layered dialog rendszer magja
        dialogStack.push_back(dialog);       // weak_ptr automatikus konverzió
        dialogSharedStack.push_back(dialog); // shared_ptr tárolás (életben tartás)

        // 3. Dialógus aktiválása és megjelenítése
        //    Az új dialógus lesz a legfelső.
        dialog->setTopDialog(true);
        dialog->show();

        // currentDialog frissítése az új aktív dialógusra
        // (Bár a dialogStack.back() is használható a legfelső elérésére)
        currentDialog = dialog;

        // 4. Képernyő újrarajzolás triggering
        this->markForRedraw();
    }
}

/**
 * @brief Dialógus bezárásának kezelése - Layered navigation magja
 * @param closedDialog A bezárt dialógus pointer
 *
 * Ez a metódus implementálja a layered dialog rendszer navigációs logikáját.
 * Automatikusan meghívódik amikor egy dialógus bezáródik (OK/Cancel gomb).
 *
 * Fő funkciók:
 * 1. **Dialog Stack Cleanup**: A bezárt dialógus eltávolítása mindkét stack-ből
 * 2. **Navigation Logic**: Visszatérés az előző dialógushoz vagy a főképernyőhöz
 * 3. **Memory Management**: Automatikus shared_ptr cleanup
 * 4. **Visual Refresh**: Teljes képernyő újrarajzolás a tiszta megjelenéshez
 *
 * Navigation logika:
 * - Ha ez volt az utolsó dialógus → visszatérés a főképernyőhöz
 * - Ha vannak még dialógusok → előző dialógus aktiválása
 *
 * @note A metódus robusztus - kezeli a középső dialógusok bezárását is
 */
void UIScreen::onDialogClosed(UIDialogBase *closedDialog) {

    // ===============================
    // 1. Aktuális dialógus referencia cleanup
    // ===============================
    if (currentDialog.get() == closedDialog) {
        currentDialog.reset();
    }

    // ===============================
    // 2. Dialog Stack Cleanup - Weak Pointer Stack
    // ===============================
    bool dialogRemoved = false;

    // Optimalizáció: Top dialog ellenőrzése először (leggyakoribb eset)
    if (!dialogStack.empty()) {
        auto topDialog = dialogStack.back().lock();
        if (topDialog && topDialog.get() == closedDialog) {
            dialogStack.pop_back();
            dialogRemoved = true;
        }
    }

    // Fallback: Keresés a stack középső elemei között (ritkább eset)
    if (!dialogRemoved) {
        for (auto it = dialogStack.begin(); it != dialogStack.end(); ++it) {
            auto dialog = it->lock();
            if (dialog && dialog.get() == closedDialog) {
                dialogStack.erase(it);
                dialogRemoved = true;
                break;
            }
        }
    }

    // ===============================
    // 3. Shared Pointer Stack Cleanup - Memory Protection
    // ===============================
    for (auto it = dialogSharedStack.begin(); it != dialogSharedStack.end(); ++it) {
        if (it->get() == closedDialog) {
            dialogSharedStack.erase(it);
            break;
        }
    }

    // ===============================
    // 4. Navigation Logic - Visszatérés főképernyőhöz vagy előző dialógushoz
    // ===============================

    if (dialogStack.empty()) {
        // ===========================================
        // 4A. UTOLSÓ DIALÓGUS BEZÁRVA - Visszatérés főképernyőhöz
        // ===========================================

        // Teljes képernyő törlése - tiszta újrakezdés
        tft.fillScreen(TFT_BLACK);

        // Saját újrarajzolási flag beállítása
        // A `true` paraméter biztosítja, hogy a képernyő összes gyerek komponense is újra legyen rajzolva.
        markForRedraw(true);

        // Azonnali teljes újrarajzolás
        draw();

    } else {
        // ===========================================
        // 4B. VAN MÉG DIALÓGUS - Visszanavigálás előző dialógushoz
        // ===========================================
        auto topDialog = dialogStack.back().lock();
        if (topDialog) {

            // Teljes képernyő törlése - tiszta rétegzett újrakezdés
            tft.fillScreen(TFT_BLACK);

            // Előző dialógus reaktiválása
            topDialog->setTopDialog(true);
            topDialog->resetVeilDrawnFlag(); // Fátyol újrarajzolásának engedélyezése
            topDialog->markForRedraw(true);  // A `true` paraméter a gyerekeket is megjelöli (gombok, stb.)
            currentDialog = topDialog;

            // Alapképernyő komponenseinek újrarajzolási flag beállítása
            markForRedraw(true);

            // Összes maradék dialógus újrarajzolási flag beállítása
            for (auto &weakDialog : dialogStack) {
                auto dialog = weakDialog.lock();
                if (dialog) {
                    dialog->markForRedraw(true); // A `true` paraméter a gyerekeket is megjelöli (gombok, stb.)
                }
            }

            // Teljes rétegzett újrarajzolás - alap képernyő + összes dialógus
            draw(); // Ez rajzolja az alap képernyőt + összes dialógust
        } else {
            DEBUG("UIScreen::onDialogClosed() - ERROR: Previous dialog pointer is null!\n");
        }
    }
}

/**
 * @brief Dialógus cleanup végrehajtása rajzolás nélkül
 * @param closedDialog A bezárt dialógus pointer
 * @details Lemásolja az onDialogClosed logikáját, de kihagyja a draw() hívásokat.
 * Hasznos olyan esetekben, amikor a leszármazott osztály egyedi rajzolási logikát szeretne.
 */
void UIScreen::performDialogCleanupWithoutDraw(UIDialogBase *closedDialog) {

    // ===============================
    // 1. Aktuális dialógus referencia cleanup
    // ===============================
    if (currentDialog.get() == closedDialog) {
        currentDialog.reset();
    }

    // ===============================
    // 2. Dialog Stack Cleanup - Weak Pointer Stack
    // ===============================
    bool dialogRemoved = false;

    // Optimalizáció: Top dialog ellenőrzése először (leggyakoribb eset)
    if (!dialogStack.empty()) {
        auto topDialog = dialogStack.back().lock();
        if (topDialog && topDialog.get() == closedDialog) {
            dialogStack.pop_back();
            dialogRemoved = true;
        }
    }

    // Fallback: Keresés a stack középső elemei között (ritkább eset)
    if (!dialogRemoved) {
        for (auto it = dialogStack.begin(); it != dialogStack.end(); ++it) {
            auto dialog = it->lock();
            if (dialog && dialog.get() == closedDialog) {
                dialogStack.erase(it);
                dialogRemoved = true;
                break;
            }
        }
    }

    // ===============================
    // 3. Shared Pointer Stack Cleanup - Memory Protection
    // ===============================
    for (auto it = dialogSharedStack.begin(); it != dialogSharedStack.end(); ++it) {
        if (it->get() == closedDialog) {
            dialogSharedStack.erase(it);
            break;
        }
    }

    // ===============================
    // 4. Screen cleanup - készítjük elő a rajzoláshoz
    // ===============================

    // Teljes képernyő törlése - tiszta újrakezdés
    tft.fillScreen(TFT_BLACK);

    // Saját újrarajzolási flag beállítása
    markForRedraw();

    // Összes gyerek komponens újrarajzolási flag beállítása
    for (auto &child : children) {
        if (child) {
            auto uiComponent = std::static_pointer_cast<UIComponent>(child);
            uiComponent->markForRedraw();
        }
    }

    // FONTOS: Itt NEM hívjuk a draw()-t, azt a hívó osztály fogja megtenni
}
