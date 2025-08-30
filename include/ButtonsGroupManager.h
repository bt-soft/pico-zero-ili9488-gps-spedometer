#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "UIButton.h"
#include "UIContainerComponent.h"
#include "UIScreen.h"

// Struktúra a gombok definíciójához a csoportos elrendezéshez
// Ezt a struktúrát használják a layout metódusok.
struct ButtonGroupDefinition {
    uint8_t id;                                                      // Gomb egyedi azonosítója
    const char *label;                                               // Gomb felirata
    UIButton::ButtonType type;                                       // Gomb típusa (Pushable, Toggleable)
    std::function<void(const UIButton::ButtonEvent &)> callback;     // Visszahívási függvény eseménykezeléshez
    UIButton::ButtonState initialState = UIButton::ButtonState::Off; // Kezdeti állapot (alapértelmezetten Off)
    uint16_t width = 0;                                              // Egyedi szélesség (0 = auto-méretezés a szöveghez)
    uint16_t height = 0;                                             // Egyedi magasság (0 = UIButton::DEFAULT_BUTTON_HEIGHT)
};

template <typename DerivedContainer> // Curiously Recurring Template Pattern (CRTP): A DerivedContainer lesz a konkrét UIContainerComponent alapú osztály
class ButtonsGroupManager {
  protected:
    // A konstruktor és destruktor lehet default, mivel ez az osztály
    // elsősorban metódusokat biztosít.
    ButtonsGroupManager() = default;
    virtual ~ButtonsGroupManager() = default;

    /**
     * @brief Gombokat rendez el függőlegesen, a képernyő jobb széléhez igazítva.
     * A metódus a leszármazott képernyő (ami UIScreen-ből is származik) tft referenciáját és addChild metódusát használja.
     *
     * @param buttonDefs A gombok definíciói, amelyek tartalmazzák az ID-t, feliratot, típust, kezdeti állapotot és callback-et.
     * @param out_createdButtons Opciósan visszaadja a létrehozott gombokat, ha nem nullptr.
     * @param marginRight A jobb margó a képernyő szélétől (alapértelmezett 5 pixel).
     * @param marginTop A felső margó a képernyő tetejétől (alapértelmezett 5 pixel).
     * @param marginBottom Az alsó margó a képernyő aljától (alapértelmezett 5 pixel).
     * @param defaultButtonWidthRef Az alapértelmezett gomb szélessége, ha a gomb definícióban nincs megadva (alapértelmezett UIButton::DEFAULT_BUTTON_WIDTH).
     * @param defaultButtonHeightRef Az alapértelmezett gomb magassága, ha a gomb definícióban nincs megadva (alapértelmezett UIButton::DEFAULT_BUTTON_HEIGHT).
     * @param columnGap Az oszlopok közötti távolság (alapértelmezett 3 pixel).
     * @param buttonGap A gombok közötti függőleges távolság (alapértelmezett 3 pixel).
     *
     *
     * Ha kell a létrehozott gombok listája, akkor a `out_createdButtons` paramétert meg kell adni.
     * std::vector<std::shared_ptr<UIButton>> createdVerticalButtons
     * layoutVerticalButtonGroup(horizontalButtonDefs, &createdVerticalButtons);
     */
    void layoutVerticalButtonGroup(const std::vector<ButtonGroupDefinition> &buttonDefs, std::vector<std::shared_ptr<UIButton>> *out_createdButtons = nullptr,
                                   int16_t marginRight = 5,                                          //
                                   int16_t marginTop = 5,                                            //
                                   int16_t marginBottom = 5,                                         //
                                   int16_t defaultButtonWidthRef = UIButton::DEFAULT_BUTTON_WIDTH,   //
                                   int16_t defaultButtonHeightRef = UIButton::DEFAULT_BUTTON_HEIGHT, //
                                   int16_t columnGap = 3,                                            //
                                   int16_t buttonGap = 3) {

        DerivedContainer *self = static_cast<DerivedContainer *>(this);

        if (buttonDefs.empty()) {
            return;
        }

        // A 'self->tft' a DerivedContainer (pl. TestScreen, MessageDialog) UIComponent ősosztályából
        // (pontosabban UIComponent-ből) örökölt 'tft' referenciája lesz.
        // Az 'self->addChild' pedig az UIContainerComponent public metódusa.
        const int16_t screenHeight = ::SCREEN_H;
        const int16_t maxColumnHeight = screenHeight - marginTop - marginBottom;
        const int16_t screenWidth = ::SCREEN_W;

        if (out_createdButtons) {
            out_createdButtons->clear();
        }

        // --- Előfeldolgozási fázis: Oszlopok struktúrájának és méreteinek meghatározása ---
        std::vector<std::vector<ButtonGroupDefinition>> colsOfButtons;
        std::vector<int16_t> colMaxWidhtsList;
        std::vector<ButtonGroupDefinition> currentBuildingColButtons;
        int16_t currentY_build = marginTop;
        int16_t currentBuildingColMaxW = 0;

        // Szövegbeállítások mentése a TFT-n
        uint8_t prevDatum = ::tft.getTextDatum();

        for (const auto &def : buttonDefs) {
            int16_t btnW;
            if (def.width > 0) {
                btnW = def.width;
            } else { // def.width == 0, kérjük a UIButton-tól a kalkulált szélességet
                // A UIButton alapértelmezett szövegméretét (2) és a gomb magasságát használjuk a kalkulációhoz.
                // A UIButton-nak van egy textSize tagja, ami alapból 2.
                // A def.height vagy defaultButtonHeightRef adja a magasságot.
                uint16_t btnActualHeight = (def.height > 0) ? def.height : defaultButtonHeightRef;
                btnW = UIButton::calculateWidthForText(def.label, false /*default useMiniFont*/, btnActualHeight);
                if (btnW <= 0)
                    btnW = defaultButtonWidthRef; // Fallback
            }
            int16_t btnH = (def.height > 0) ? def.height : defaultButtonHeightRef;

            if (currentY_build == marginTop && btnH > maxColumnHeight) {
                // Ha az előző oszlopban voltak gombok
                if (!currentBuildingColButtons.empty()) {
                    colsOfButtons.push_back(currentBuildingColButtons);
                    colMaxWidhtsList.push_back(currentBuildingColMaxW);
                    currentBuildingColButtons.clear();
                    currentBuildingColMaxW = 0;
                }
                currentY_build = marginTop; // Marad a jelenlegi oszlop tetején a következő gombnak
                continue;
            }

            // Új oszlopot kell kezdeni
            if (currentY_build + btnH > maxColumnHeight && currentY_build != marginTop) {
                colsOfButtons.push_back(currentBuildingColButtons);
                colMaxWidhtsList.push_back(currentBuildingColMaxW);
                currentBuildingColButtons.clear();
                currentY_build = marginTop;
                currentBuildingColMaxW = 0;

                // Ellenőrizzük, hogy az új oszlopot kezdő gomb nem túl magas-e
                if (btnH > maxColumnHeight) {
                    continue;
                }
            }
            currentBuildingColButtons.push_back(def);
            currentBuildingColMaxW = std::max(currentBuildingColMaxW, btnW);
            currentY_build += btnH + buttonGap;
        }

        if (!currentBuildingColButtons.empty()) {
            colsOfButtons.push_back(currentBuildingColButtons);
            colMaxWidhtsList.push_back(currentBuildingColMaxW);
        }

        if (colsOfButtons.empty())
            return;

        uint8_t numCols = colMaxWidhtsList.size();
        int16_t totalColsEffectiveWidth = 0;
        for (int16_t w : colMaxWidhtsList) {
            totalColsEffectiveWidth += w;
        }
        int16_t totalColsWidthWithGaps = totalColsEffectiveWidth + (numCols > 1 ? (numCols - 1) * columnGap : 0);

        // Szövegbeállítások visszaállítása
        ::tft.setTextDatum(prevDatum);
        // A setTextSize visszaállítása nem szükséges, mert a UIButton::draw() beállítja a sajátját.

        // --- Elrendezési fázis ---
        int16_t startX = screenWidth - marginRight - totalColsWidthWithGaps;
        int16_t currentLayoutX = startX;

        for (size_t colIndex = 0; colIndex < colsOfButtons.size(); ++colIndex) {
            int16_t currentLayoutY = marginTop;
            const auto &currentCol = colsOfButtons[colIndex];
            for (const auto &def : currentCol) {
                int16_t btnWidth;
                bool autoSizeBtn = (def.width == 0); // Ha a definícióban 0 a szélesség, akkor auto-méretezést kérünk
                if (!autoSizeBtn) {                  // Explicit szélesség megadva
                    btnWidth = def.width;
                } else { // Automatikus méretezés kérése
                    // A bounds-hoz az előfeldolgozásban kalkulált értéket használjuk.
                    // A UIButton konstruktora ezt felülírhatja, ha autoSizeBtn true.
                    uint16_t btnActualHeight = (def.height > 0) ? def.height : defaultButtonHeightRef;
                    btnWidth = UIButton::calculateWidthForText(def.label, false, btnActualHeight);
                    if (btnWidth <= 0)
                        btnWidth = defaultButtonWidthRef;
                }
                int16_t btnHeight = (def.height > 0) ? def.height : defaultButtonHeightRef;

                Rect bounds(currentLayoutX, currentLayoutY, btnWidth, btnHeight);
                auto button = std::make_shared<UIButton>(def.id, bounds, def.label, def.type, def.initialState, def.callback, UIColorPalette::createDefaultButtonScheme(), autoSizeBtn);
                self->addChild(button);
                if (out_createdButtons) {
                    out_createdButtons->push_back(button);
                }
                currentLayoutY += btnHeight + buttonGap;
            }
            if (colIndex < colsOfButtons.size() - 1) {
                currentLayoutX += colMaxWidhtsList[colIndex] + columnGap;
            }
        }
    }

    /**
     * @brief Gombokat rendez el vízszintesen, a képernyő aljához igazítva.
     * A metódus a leszármazott képernyő (ami UIScreen-ből is származik) tft referenciáját és addChild metódusát használja.
     *
     * @param buttonDefs A gombok definíciói, amelyek tartalmazzák az ID-t, feliratot, típust, kezdeti állapotot és callback-et.
     * @param out_createdButtons Opcionális kimeneti paraméter, amelybe a létrehozott gombok pointereit tárolja.
     * @param marginLeft Bal oldali margó a gombok elhelyezéséhez (alapértelmezett 5 pixel).
     * @param marginRight Jobb oldali margó a gombok elhelyezéséhez (alapértelmezett 5 pixel).
     * @param marginBottom Alsó margó a gombok elhelyezéséhez (alapértelmezett 5 pixel).
     * @param defaultButtonWidthRef Alapértelmezett gomb szélesség referencia (alapértelmezett UIButton::DEFAULT_BUTTON_WIDTH).
     * @param defaultButtonHeightRef Alapértelmezett gomb magasság referencia (alapértelmezett UIButton::DEFAULT_BUTTON_HEIGHT).
     * @param rowGap Sorok közötti függőleges rés (alapértelmezett 3 pixel).
     * @param buttonGap Gombok közötti vízszintes rés (alapértelmezett 3 pixel).
     *
     * Ha kell a létrehozott gombok listája, akkor a `out_createdButtons` paramétert meg kell adni.
     * std::vector<std::shared_ptr<UIButton>> createdHorizontalButtons;
     * layoutHorizontalButtonGroup(horizontalButtonDefs, &createdHorizontalButtons);
     *
     */
    void layoutHorizontalButtonGroup(const std::vector<ButtonGroupDefinition> &buttonDefs, std::vector<std::shared_ptr<UIButton>> *out_createdButtons = nullptr,
                                     int16_t marginLeft = 5,                                           //
                                     int16_t marginRight = 5,                                          //
                                     int16_t marginBottom = 5,                                         //
                                     int16_t defaultButtonWidthRef = UIButton::DEFAULT_BUTTON_WIDTH,   //
                                     int16_t defaultButtonHeightRef = UIButton::DEFAULT_BUTTON_HEIGHT, //
                                     int16_t rowGap = 3,                                               //
                                     int16_t buttonGap = 3,                                            //
                                     bool centerHorizontally = false) {

        DerivedContainer *self = static_cast<DerivedContainer *>(this);

        if (buttonDefs.empty()) {
            return;
        }
        const int16_t maxRowWidth = ::SCREEN_W - marginLeft - marginRight;

        if (out_createdButtons) {
            out_createdButtons->clear();
        }

        // --- Előfeldolgozási fázis: Sorok struktúrájának és méreteinek meghatározása ---
        std::vector<std::vector<ButtonGroupDefinition>> rowsOfButtons;
        std::vector<int16_t> rowMaxHeightsList;
        std::vector<ButtonGroupDefinition> currentBuildingRowButtons;
        int16_t currentX_build = marginLeft;
        int16_t currentBuildingRowMaxH = 0;

        // Szövegbeállítások mentése és visszaállítása
        uint8_t prevDatum = ::tft.getTextDatum();

        for (const auto &def : buttonDefs) {
            int16_t btnWidth;
            if (def.width > 0) {
                btnWidth = def.width;
            } else { // def.width == 0, auto-size
                uint16_t btnActualHeight = (def.height > 0) ? def.height : defaultButtonHeightRef;
                btnWidth = UIButton::calculateWidthForText(def.label, false, btnActualHeight);
                if (btnWidth <= 0)
                    btnWidth = defaultButtonWidthRef;
            }

            // Gomb szélességének korlátozása a maximális sor szélességre
            if (btnWidth > maxRowWidth) {
                btnWidth = maxRowWidth;
            }
            int16_t btnHeight = (def.height > 0) ? def.height : defaultButtonHeightRef;

            if (currentX_build == marginLeft && btnWidth > maxRowWidth) {

                // Ha az előző sorban voltak gombok
                if (!currentBuildingRowButtons.empty()) {
                    rowsOfButtons.push_back(currentBuildingRowButtons);
                    rowMaxHeightsList.push_back(currentBuildingRowMaxH);
                    currentBuildingRowButtons.clear();
                    currentBuildingRowMaxH = 0;
                }
                currentX_build = marginLeft; // Marad a jelenlegi sor elején a következő gombnak
                continue;
            }

            // Új sort kell kezdeni
            if (currentX_build + btnWidth > maxRowWidth && currentX_build != marginLeft) {
                rowsOfButtons.push_back(currentBuildingRowButtons);
                rowMaxHeightsList.push_back(currentBuildingRowMaxH);
                currentBuildingRowButtons.clear();
                currentX_build = marginLeft;
                currentBuildingRowMaxH = 0;

                // Ellenőrizzük, hogy az új sort kezdő gomb nem túl széles-e
                if (btnWidth > maxRowWidth) {
                    continue;
                }
            }

            currentBuildingRowButtons.push_back(def);
            currentBuildingRowMaxH = std::max(currentBuildingRowMaxH, btnHeight);
            currentX_build += btnWidth + buttonGap;
        }

        if (!currentBuildingRowButtons.empty()) {
            rowsOfButtons.push_back(currentBuildingRowButtons);
            rowMaxHeightsList.push_back(currentBuildingRowMaxH);
        }

        if (rowsOfButtons.empty())
            return;

        uint8_t numRows = rowMaxHeightsList.size();
        int16_t totalRowsEffectiveHeight = 0;
        for (int16_t h : rowMaxHeightsList) {
            totalRowsEffectiveHeight += h;
        }
        int16_t totalRowsHeightWithGaps = totalRowsEffectiveHeight + (numRows > 1 ? (numRows - 1) * rowGap : 0);

        // Szövegbeállítások visszaállítása
        ::tft.setTextDatum(prevDatum);
        // A setTextSize visszaállítása nem szükséges, mert a UIButton::draw() beállítja a sajátját.

        // --- Elrendezési fázis ---
        int16_t startY = ::SCREEN_H - marginBottom - totalRowsHeightWithGaps;
        int16_t currentLayoutY = startY;

        for (size_t rowIndex = 0; rowIndex < rowsOfButtons.size(); ++rowIndex) {
            const auto &currentRow = rowsOfButtons[rowIndex];
            int16_t currentLayoutX;

            if (centerHorizontally) {
                // Teljes szélesség kiszámítása az aktuális sorban lévő gombokhoz a középre igazításhoz
                int16_t totalWidthOfButtonsInRow = 0;
                for (size_t i = 0; i < currentRow.size(); ++i) {
                    const auto &def = currentRow[i];
                    bool autoSizeBtn = (def.width == 0);
                    int16_t btnActualWidth = autoSizeBtn ? UIButton::calculateWidthForText(def.label, false, (def.height > 0) ? def.height : defaultButtonHeightRef) : def.width;
                    if (btnActualWidth <= 0 && autoSizeBtn)
                        btnActualWidth = defaultButtonWidthRef; // Fallback
                    if (btnActualWidth > maxRowWidth)
                        btnActualWidth = maxRowWidth; // Korlátozás
                    totalWidthOfButtonsInRow += btnActualWidth;
                    if (i < currentRow.size() - 1) {
                        totalWidthOfButtonsInRow += buttonGap;
                    }
                }
                currentLayoutX = marginLeft + (maxRowWidth - totalWidthOfButtonsInRow) / 2;
                // Biztosítjuk, hogy ne legyen negatív, vagy kisebb, mint a bal margó
                if (currentLayoutX < marginLeft)
                    currentLayoutX = marginLeft;
            } else {
                // Alapértelmezett balra igazítás
                currentLayoutX = marginLeft;
            }

            for (const auto &def : currentRow) { // Most már a currentRow-t használjuk
                int16_t btnWidth;
                bool autoSizeBtn = (def.width == 0);
                if (!autoSizeBtn) {
                    btnWidth = def.width;
                } else {
                    // Az előfeldolgozásban kalkulált szélességet használjuk a Rect-hez.
                    // A gomb maga fogja beállítani a végleges szélességét, ha autoSizeBtn true.
                    uint16_t btnActualHeight = (def.height > 0) ? def.height : defaultButtonHeightRef;
                    btnWidth = UIButton::calculateWidthForText(def.label, false, btnActualHeight);
                    if (btnWidth <= 0)
                        btnWidth = defaultButtonWidthRef;
                }
                int16_t btnHeight = (def.height > 0) ? def.height : defaultButtonHeightRef;
                // Gomb szélességének korlátozása a maximális sor szélességre (újra, mert a def const ref)
                if (btnWidth > maxRowWidth) {
                    btnWidth = maxRowWidth;
                }

                Rect bounds(currentLayoutX, currentLayoutY, btnWidth, btnHeight);
                auto button = std::make_shared<UIButton>(def.id, bounds, def.label, def.type, def.initialState, def.callback, UIColorPalette::createDefaultButtonScheme(), autoSizeBtn);
                self->addChild(button);
                if (out_createdButtons) {
                    out_createdButtons->push_back(button);
                }
                currentLayoutX += btnWidth + buttonGap;
            }
            if (rowIndex < rowsOfButtons.size() - 1) {
                currentLayoutY += rowMaxHeightsList[rowIndex] + rowGap;
            }
        }
    }
};
