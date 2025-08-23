#include "UIDialogBase.h"
#include "UIScreen.h"

/**
 * @brief UIDialogBase konstruktor
 * @param parentScreen A szülő UIScreen, amely megjeleníti ezt a dialógust
 * @param title A dialógus címe (nullptr, ha nincs cím)
 * @param initialBounds A dialógus kezdeti határai (pozíció és méret)
 * @param cs A dialógus színpalettája (alapértelmezett ColorScheme)
 *
 */
UIDialogBase::UIDialogBase(UIScreen *parentScreen, const char *title, const Rect &initialBounds, const ColorScheme &cs) : UIContainerComponent(initialBounds, cs), parentScreen(parentScreen), title(title) {

    Rect finalBounds = initialBounds; // Szélesség beállítása: ha 0, akkor alapértelmezett, egyébként a megadott
    if (finalBounds.width == 0) {
        finalBounds.width = SCREEN_W * 0.8f; // Alapértelmezett szélesség
    }
    // Magasság beállítása: ha 0, akkor alapértelmezett, egyébként a megadott
    if (finalBounds.height == 0) {
        finalBounds.height = SCREEN_H * 0.6f; // Alapértelmezett magasság (ezt a MessageDialog felülírhatja)
    }

    // Középre igazítás, ha x vagy y -1
    if (finalBounds.x == -1) {
        finalBounds.x = (SCREEN_W - finalBounds.width) / 2;
    }
    if (finalBounds.y == -1) {
        finalBounds.y = (SCREEN_H - finalBounds.height) / 2;
    }
    UIComponent::setBounds(finalBounds);

    // Bezáró gomb létrehozása, ha van cím
    if (title != nullptr) {
        createCloseButton();
    }

    // A createDialogContent() és layoutDialogContent() a leszármazottban hívódik meg.
}

/**
 * @brief Megjeleníti a dialógust.
 * @details A dialógus megjelenítésekor a fátyol rajzolása csak egyszer történik meg,
 */
void UIDialogBase::show() {
    veilDrawn = false; // Reset a flag-et új megjelenéskor
    // A _parentScreen->markForRedraw() implicit módon megtörténik,
    // amikor a dialógus láthatóvá válik és a UIScreen::draw() lefut.
}

/**
 * @brief Bezárja a dialógust és meghívja a callback-et.
 * @param result A dialógus eredménye (Accepted, Rejected, Dismissed)
 */
void UIDialogBase::close(DialogResult result) {
    veilDrawn = false; // Reset bezáráskor

    // Callback meghívása ELŐBB - így a callback-ben lehet új dialógust megnyitni
    if (callback) {
        callback(this, result);
    }

    // Szülő képernyő értesítése UTÁNA - dialógus eltávolítása a stackből
    if (parentScreen) {
        parentScreen->onDialogClosed(this);
    }
}

/**
 * @brief Rajzolja a dialógust.
 * @details A dialógus hátterét és fejlécét rajzolja, valamint a gyerek komponenseket.
 */
void UIDialogBase::draw() {

    if (!veilDrawn) {
        drawVeil();
        veilDrawn = true;
    }

    // Dialógus keret és gyerekek rajzolása
    UIContainerComponent::draw();
}

/**
 * @brief Rajzolja a dialógus hátterét és fejlécét.
 * @details A fejléc kék háttérrel és egyenes sarkokkal rendelkezik.
 */
void UIDialogBase::drawSelf() {
    // Árnyék effekt rajzolása (eltolva jobbra és lefelé)
    const int shadowOffset = 4;
    const uint16_t shadowColor = TFT_COLOR(64, 64, 64); // Sötétszürke árnyék
    tft.fillRect(bounds.x + shadowOffset, bounds.y + shadowOffset, bounds.width, bounds.height, shadowColor);

    // Dialógus háttér rajzolása (az árnyék fölé)
    tft.fillRect(bounds.x, bounds.y, bounds.width, bounds.height, colors.background); // Vastagabb, világosabb keret több rétegben
    const uint16_t borderColor = TFT_WHITE;                                           // Fehér keret a jobb láthatóságért

    // 2 pixeles vastagságú keret rajzolása
    for (int i = 0; i < 2; i++) {
        tft.drawRect(bounds.x + i, bounds.y + i, bounds.width - 2 * i, bounds.height - 2 * i, borderColor);
    }

    // Fejléc kék háttér - egyenes sarkokkal (a keret után)
    uint16_t headerColor = UIColorPalette::DIALOG_HEADER_BACKGROUND;
    tft.fillRect(bounds.x + 2, bounds.y + 2, bounds.width - 4, HEADER_HEIGHT, headerColor);

    // Fejléc elválasztó vonal
    tft.drawLine(bounds.x + 2, bounds.y + HEADER_HEIGHT + 2, bounds.x + bounds.width - 3, bounds.y + HEADER_HEIGHT + 2, borderColor); // Cím kirajzolása
    if (title != nullptr) {
        tft.setTextColor(UIColorPalette::DIALOG_HEADER_TEXT, headerColor);
        tft.setFreeFont(&FreeSansBold9pt7b); // Nagyobb, vastagabb font a címnek
        tft.setTextSize(1);                  // A FreeSansBold9pt7b natív mérete
        tft.setTextDatum(ML_DATUM);
        int16_t titleX = bounds.x + PADDING + 6;             // 2 pixel keret + 4 padding
        int16_t titleY = bounds.y + 2 + (HEADER_HEIGHT / 2); // 2 pixel keret eltolás
        tft.drawString(title, titleX, titleY);
    }
}

/**
 * @brief Kezeli a touch eseményeket a dialóguson belül.
 * @param event A touch esemény, amely tartalmazza a koordinátákat és a lenyomás állapotát
 */
bool UIDialogBase::handleTouch(const TouchEvent &event) {

    // Csak akkor kezeljük, ha a toppon van és nem tiltott a dialógus
    if (!topDialog || disabled) {
        return false;
    }

    // Gyerek komponensek kezelik az eseményt (beleértve a bezáró gombot is)
    for (auto it = children.rbegin(); it != children.rend(); ++it) {
        if ((*it)->handleTouch(event)) {
            return true; // Egy gyerek komponens kezelte az eseményt
        }
    }

    // Ha a dialógus területén belül történt az érintés, elnyeljük
    if (bounds.contains(event.x, event.y)) {
        return true; // Az eseményt a dialógus kezelte (elnyelés)
    }

    // Ha kívül történt, nem kezeljük
    return false;
}

/**
 * @brief Jelzi, hogy a dialógust újra kell rajzolni.
 * @details Felülírja az UIComponent::markForRedraw metódusát, hogy biztosítsa
 * a dialógus-specifikus újrarajzolási igények kezelését.
 */
void UIDialogBase::markForRedraw(bool markChildren) {
    UIContainerComponent::markForRedraw(markChildren); // Meghívja az ősosztály implementációját
}

/**
 * @brief Fátyolt kirajzolása a dialógus körül.
 * @details A fátyol csak a dialógus területén kívül rajzolódik ki, hogy a dialógus kiemelkedjen.
 * A fátyol pixel mérete 3, és a színe a UIColorPalette::DIALOG_VEIL_COLOR.
 */
void UIDialogBase::drawVeil() {            // CSAK a dialógus területén KÍVÜL rajzoljuk a fátyolt!
    constexpr uint8_t VEIL_PIXEL_SIZE = 3; // Fátyol pixel mérete
    for (int16_t y = 0; y < SCREEN_H; y += VEIL_PIXEL_SIZE) {

        // Ne rajzoljunk fátyolt a dialógus területére!
        for (int16_t x = (y % VEIL_PIXEL_SIZE); x < SCREEN_W; x += VEIL_PIXEL_SIZE) {
            if (!bounds.contains(x, y)) {
                tft.drawPixel(x, y, UIColorPalette::DIALOG_VEIL_COLOR);
            }
        }
    }
}

void UIDialogBase::createCloseButton() {
    // Bezáró gomb mérete és pozíciója
    constexpr int16_t CLOSE_BTN_SIZE = 20;
    constexpr int16_t CLOSE_BTN_MARGIN = 4;

    int16_t closeBtnX = bounds.x + bounds.width - CLOSE_BTN_SIZE - CLOSE_BTN_MARGIN;
    int16_t closeBtnY = bounds.y + CLOSE_BTN_MARGIN;
    Rect closeBtnBounds(closeBtnX, closeBtnY, CLOSE_BTN_SIZE, CLOSE_BTN_SIZE);                    // Bezáró gomb létrehozása központi színpalettával
    closeButton = std::make_shared<UIButton>(DIALOG_DEFAULT_CLOSE_BUTTON_ID, closeBtnBounds, "X", //
                                             [this](const UIButton::ButtonEvent &event) {
                                                 if (event.state == UIButton::EventButtonState::Clicked) {
                                                     this->close(DialogResult::Dismissed);
                                                 }
                                             });

    addChild(closeButton);
}

/**
 * @brief Elhalasztott bezárást ütemez, hogy elkerülje a láncolt callback-eket
 * @param result A bezáráskor használandó dialógus eredmény
 */
void UIDialogBase::deferClose(DialogResult result) {
    deferredClose.pending = true;
    deferredClose.result = result;
    deferredClose.chainCloseDialog = nullptr; // No chain close
    DEBUG("UIDialogBase::deferClose() - Elhalasztott bezárás ütemezve, eredmény: %d\n", static_cast<int>(result));
}

/**
 * @brief Elhalasztott bezárást ütemez, amely egy másik dialógus láncolt bezárását is végrehajtja
 * @param result A bezáráskor használandó dialógus eredmény
 * @param chainDialog További dialógus, amelyet ez után kell bezárni
 */
void UIDialogBase::deferChainClose(DialogResult result, std::shared_ptr<UIDialogBase> chainDialog) {
    deferredClose.pending = true;
    deferredClose.result = result;
    deferredClose.chainCloseDialog = chainDialog;
    DEBUG("UIDialogBase::deferChainClose() - Elhalasztott láncolt bezárás ütemezve, eredmény: %d\n", static_cast<int>(result));
}

/**
 * @brief Feldolgozza a függőben lévő elhalasztott bezárási műveletet
 * Ezt a fő ciklusból vagy callback láncok lefutása után
 */
void UIDialogBase::processDeferredClose() {
    if (deferredClose.pending) {
        deferredClose.pending = false;
        DialogResult result = deferredClose.result;
        auto chainDialog = deferredClose.chainCloseDialog;
        deferredClose.chainCloseDialog = nullptr; // Clear chain reference

        DEBUG("UIDialogBase::processDeferredClose() - Elhalasztott bezárás feldolgozása, eredmény: %d\n", static_cast<int>(result));
        close(result);

        // If there's a chain dialog, close it too immediately
        if (chainDialog) {
            DEBUG("UIDialogBase::processDeferredClose() - Láncolt dialógus bezárása\n");
            chainDialog->close(result);
        }
    }
}