
#include "ValueChangeDialog.h"
#include "UIColorPalette.h"
#include "UIScreen.h"
#include "defines.h"

/**
 * @brief Konstruktor integer értékhez
 * @param parentScreen Szülő képernyő referencia
 * @param title Dialógus címe
 * @param message Megjelenítendő üzenet
 * @param valuePtr Pointer az integer értékre
 * @param minValue Minimális érték
 * @param maxValue Maximális érték
 * @param stepValue Lépésköz
 * @param callback Callback függvény az érték változáskor
 * @param userDialogCb Dialógus lezárásakor hívandó callback (OK/Cancel után)
 * @param bounds Dialógus pozíciója és mérete
 * @param cs Színséma
 * @note Ez a konstruktor integer típusú értékekhez készült, amely egész számokat kezel.
 */
ValueChangeDialog::ValueChangeDialog(UIScreen *parentScreen, const char *title, const char *message, int *valuePtr, int minValue, int maxValue, int stepValue, ValueChangeCallback callback, DialogCallback userDialogCb,
                                     const Rect &bounds, const ColorScheme &cs)
    : MessageDialog(parentScreen, title, message, MessageDialog::ButtonsType::OkCancel, bounds, cs, true /*okClosesDialog*/), _valueType(ValueType::Integer), _intPtr(valuePtr), _minInt(minValue), _maxInt(maxValue),
      _stepInt(stepValue), _valueCallback(callback), _userDialogCallback(userDialogCb) { // Eredeti érték mentése
    if (_intPtr) {
        _originalIntValue = *_intPtr;
    }
    createDialogContent();
    layoutDialogContent();

    // Callback beállítása az OK/Cancel események kezelésére
    setDialogCallback([this](UIDialogBase *sender, DialogResult result) {
        if (result == MessageDialog::DialogResult::Accepted) {
            // Hívjuk meg a _valueCallback-et a végleges, elfogadott értékkel.
            notifyValueChange();
            if (_userDialogCallback)
                _userDialogCallback(sender, result);
        } else if (result == MessageDialog::DialogResult::Rejected) {
            restoreOriginalValue(); // Ez most már tartalmazza a notifyValueChange() hívást is.
            if (_userDialogCallback)
                _userDialogCallback(sender, result);
        }
        // A MessageDialog maga kezeli a close() hívást az _okClosesDialog alapján
    });
}

/**
 * @brief Konstruktor float értékhez
 * @param parentScreen Szülő képernyő referencia
 * @param title Dialógus címe
 * @param message Megjelenítendő üzenet
 * @param valuePtr Pointer a float értékre
 * @param minValue Minimális érték
 * @param maxValue Maximális érték
 * @param stepValue Lépésköz
 * @param callback Callback függvény az érték változáskor
 * @param userDialogCb Dialógus lezárásakor hívandó callback (OK/Cancel után)
 * @param bounds Dialógus pozíciója és mérete
 * @param cs Színséma
 * @note Ez a konstruktor float típusú értékekhez készült, amely lebegőpontos számokat kezel.
 */
ValueChangeDialog::ValueChangeDialog(UIScreen *parentScreen, const char *title, const char *message, float *valuePtr, float minValue, float maxValue, float stepValue, ValueChangeCallback callback,
                                     DialogCallback userDialogCb, const Rect &bounds, const ColorScheme &cs)
    : MessageDialog(parentScreen, title, message, MessageDialog::ButtonsType::OkCancel, bounds, cs, true /*okClosesDialog*/), _valueType(ValueType::Float), _floatPtr(valuePtr), _minFloat(minValue), _maxFloat(maxValue),
      _stepFloat(stepValue), _valueCallback(callback), _userDialogCallback(userDialogCb) { // Eredeti érték mentése
    if (_floatPtr) {
        _originalFloatValue = *_floatPtr;
    }

    createDialogContent();
    layoutDialogContent();

    // Callback beállítása az OK/Cancel események kezelésére
    setDialogCallback([this](UIDialogBase *sender, DialogResult result) {
        if (result == MessageDialog::DialogResult::Accepted) {
            // Hívjuk meg a _valueCallback-et a végleges, elfogadott értékkel.
            notifyValueChange();
            if (_userDialogCallback)
                _userDialogCallback(sender, result);
        } else if (result == MessageDialog::DialogResult::Rejected) {
            restoreOriginalValue(); // Ez most már tartalmazza a notifyValueChange() hívást is.
            if (_userDialogCallback)
                _userDialogCallback(sender, result);
        }
    });
}

/**
 * @brief Konstruktor boolean értékhez
 * @param parentScreen Szülő képernyő referencia
 * @param title Dialógus címe
 * @param message Megjelenítendő üzenet
 * @param valuePtr Pointer a boolean értékre
 * @param callback Callback függvény az érték változáskor
 * @param userDialogCb Dialógus lezárásakor hívandó callback (OK/Cancel után)
 * @param bounds Dialógus pozíciója és mérete
 * @param cs Színséma
 * @note Ez a konstruktor boolean típusú értékekhez készült, amely TRUE/FALSE értékeket kezel.
 */
ValueChangeDialog::ValueChangeDialog(UIScreen *parentScreen, const char *title, const char *message, bool *valuePtr, ValueChangeCallback callback, DialogCallback userDialogCb, const Rect &bounds, const ColorScheme &cs)
    : MessageDialog(parentScreen, title, message, MessageDialog::ButtonsType::OkCancel, bounds, cs, true /*okClosesDialog*/), _valueType(ValueType::Boolean), _boolPtr(valuePtr), _valueCallback(callback),
      _userDialogCallback(userDialogCb) { // Eredeti érték mentése
    if (_boolPtr) {
        _originalBoolValue = *_boolPtr;
    }

    createDialogContent();
    layoutDialogContent();

    // Callback beállítása az OK/Cancel események kezelésére
    setDialogCallback([this](UIDialogBase *sender, DialogResult result) {
        if (result == MessageDialog::DialogResult::Accepted) {
            // Hívjuk meg a _valueCallback-et a végleges, elfogadott értékkel.
            notifyValueChange();
            if (_userDialogCallback)
                _userDialogCallback(sender, result);
        } else if (result == MessageDialog::DialogResult::Rejected) {
            restoreOriginalValue(); // Ez most már tartalmazza a notifyValueChange() hívást is.
            if (_userDialogCallback)
                _userDialogCallback(sender, result);
        }
    });
}

/**
 * @brief Konstruktor uint8_t értékhez
 * @param parentScreen Szülő képernyő referencia
 * @param title Dialógus címe
 * @param message Megjelenítendő üzenet
 * @param valuePtr Pointer az uint8_t értékre
 * @param minValue Minimális érték
 * @param maxValue Maximális érték
 * @param stepValue Lépésköz
 * @param callback Callback függvény az érték változáskor
 * @param userDialogCb Dialógus lezárásakor hívandó callback (OK/Cancel után)
 * @param bounds Dialógus pozíciója és mérete
 * @param cs Színséma
 * @note Ez a konstruktor uint8_t típusú értékekhez készült, amely 0-255 közötti értékeket kezel.
 */
ValueChangeDialog::ValueChangeDialog(UIScreen *parentScreen, const char *title, const char *message, uint8_t *valuePtr, uint8_t minValue, uint8_t maxValue, uint8_t stepValue, ValueChangeCallback callback,
                                     DialogCallback userDialogCb, const Rect &bounds, const ColorScheme &cs)
    : MessageDialog(parentScreen, title, message, MessageDialog::ButtonsType::OkCancel, bounds, cs, true /*okClosesDialog*/), _valueType(ValueType::UInt8), _uint8Ptr(valuePtr), _minUint8(minValue), _maxUint8(maxValue),
      _stepUint8(stepValue), _valueCallback(callback), _userDialogCallback(userDialogCb) {
    if (_uint8Ptr) {
        _originalUint8Value = *_uint8Ptr;
    }
    createDialogContent();
    layoutDialogContent();

    // Callback beállítása az OK/Cancel események kezelésére
    setDialogCallback([this](UIDialogBase *sender, DialogResult result) {
        if (result == MessageDialog::DialogResult::Accepted) {
            // Hívjuk meg a _valueCallback-et a végleges, elfogadott értékkel.
            // A _valueCallback int-et vár a variantban, ezért castolunk.
            notifyValueChange();
            if (_userDialogCallback)
                _userDialogCallback(sender, result);
        } else if (result == MessageDialog::DialogResult::Rejected) {
            restoreOriginalValue(); // Ez most már tartalmazza a notifyValueChange() hívást is.
            if (_userDialogCallback)
                _userDialogCallback(sender, result);
        }
        // A MessageDialog maga kezeli a close() hívást az _okClosesDialog alapján
    });
}

/**
 * @brief Érték módosító gombok létrehozása
 * Létrehozza a +/- gombokat integer és float típusokhoz, illetve TRUE/FALSE gombokat boolean típushoz.
 * A gombok eseménykezelői frissítik az értéket és újrarajzolják az érték területet.
 * A boolean típus esetén a gombok TRUE/FALSE értékeket állítanak be, és frissítik az állapotukat.
 */
void ValueChangeDialog::createDialogContent() {

    constexpr uint8_t BUTTON_DECREASE_ID = 3; // Csökkentő gomb ID
    constexpr uint8_t BUTTON_INCREASE_ID = 4; // Növelő gomb ID

    // Az OK és Cancel gombokat a MessageDialog ősosztály hozza létre.
    // Itt csak az érték-specifikus gombokat kell létrehozni.    // Érték módosító gombok (csak integer és float esetén)
    if (_valueType == ValueType::Integer || _valueType == ValueType::Float || _valueType == ValueType::UInt8) {
        // Csökkentő gomb (-)
        _decreaseButton = std::make_shared<UIButton>( //
            BUTTON_DECREASE_ID,                       //
            Rect(0, 0, BUTTON_WIDTH, BUTTON_HEIGHT),  //
            "-",                                      //
            UIButton::ButtonType::Pushable,           //
            [this](const UIButton::ButtonEvent &event) {
                if (event.state == UIButton::EventButtonState::Clicked) {
                    decrementValue();
                    // A decrementValue már hívja a validateAndClampValue-t és a notifyValueChange-t.
                    // Az érték terület újrarajzolása itt továbbra is szükséges.
                    redrawValueArea();
                }
            } //
        );
        //_decreaseButton->setUseMiniFont(true);
        addChild(_decreaseButton);

        // Növelő gomb (+)
        _increaseButton = std::make_shared<UIButton>( //
            BUTTON_INCREASE_ID,                       //
            Rect(0, 0, BUTTON_WIDTH, BUTTON_HEIGHT),  //
            "+",                                      //
            UIButton::ButtonType::Pushable,           //
            [this](const UIButton::ButtonEvent &event) {
                if (event.state == UIButton::EventButtonState::Clicked) {
                    incrementValue();
                    // Az incrementValue már hívja a validateAndClampValue-t és a notifyValueChange-t.
                    redrawValueArea();
                }
            } //
        );
        //_increaseButton->setUseMiniFont(true);
        addChild(_increaseButton);

    } else {
        // Boolean esetén FALSE/TRUE gombok létrehozása
        _decreaseButton = std::make_shared<UIButton>(     //
            BUTTON_DECREASE_ID,                           //
            Rect(0, 0, BUTTON_WIDTH + 10, BUTTON_HEIGHT), //
            "FALSE",                                      //
            UIButton::ButtonType::Pushable,               //
            [this](const UIButton::ButtonEvent &event) {
                if (event.state == UIButton::EventButtonState::Clicked) {
                    decrementValue(); // FALSE-ra állítás a decrementValue() függvényen keresztül
                    // A decrementValue már hívja a notifyValueChange-t.
                    redrawValueTextOnly();
                }
            } //
        );
        //_decreaseButton->setUseMiniFont(true);
        addChild(_decreaseButton);

        _increaseButton = std::make_shared<UIButton>(     //
            BUTTON_INCREASE_ID,                           //
            Rect(0, 0, BUTTON_WIDTH + 10, BUTTON_HEIGHT), //
            "TRUE",                                       //
            UIButton::ButtonType::Pushable,               //
            [this](const UIButton::ButtonEvent &event) {
                if (event.state == UIButton::EventButtonState::Clicked) {
                    incrementValue(); // TRUE-ra állítás a incrementValue() függvényen keresztül
                    // Az incrementValue már hívja a notifyValueChange-t.
                    redrawValueTextOnly();
                }
            } //
        );
        //_increaseButton->setUseMiniFont(true);
        addChild(_increaseButton);
    }

    // Gombállapotok beállítása közvetlenül a létrehozás után
    if (_decreaseButton) {
        _decreaseButton->setEnabled(canDecrement());
        _decreaseButton->markForRedraw();
    }
    if (_increaseButton) {
        _increaseButton->setEnabled(canIncrement());
        _increaseButton->markForRedraw();
    }
}

/**
 * @brief Dialógus tartalom elrendezése
 * Elrendezi a gombokat és az érték megjelenítését a dialógusban
 * A gombok a dialógus alján, az érték pedig a dialógus közepén jelenik meg.
 * A gombok elrendezése a MessageDialog ősosztály által kezelt ButtonsGroupManager segítségével történik.
 */
void ValueChangeDialog::layoutDialogContent() {
    const Rect contentBounds = bounds;
    const int16_t centerX = contentBounds.x + contentBounds.width / 2;

    // Az OK és Cancel gombokat a MessageDialog ősosztály rendezi el a ButtonsGroupManager segítségével, a dialógus aljára.
    // Itt csak az érték-specifikus gombokat kell elrendezni.

    const int16_t headerHeight = getHeaderHeight();
    const int16_t valueAreaY = contentBounds.y + headerHeight + PADDING + VERTICAL_OFFSET_FOR_VALUE_AREA;

    if (_valueType == ValueType::Integer || _valueType == ValueType::Float || _valueType == ValueType::UInt8) {
        // +/- gombok az érték körül - UGYANAZON A VONALON
        const int16_t valueBoxWidth = 100;
        const int16_t valueButtonSpacing = 10;
        const int16_t totalWidth = 2 * BUTTON_WIDTH + 2 * valueButtonSpacing + valueBoxWidth;
        const int16_t startX2 = centerX - totalWidth / 2;

        _decreaseButton->setBounds(Rect(startX2, valueAreaY, BUTTON_WIDTH, BUTTON_HEIGHT));
        _increaseButton->setBounds(Rect(startX2 + BUTTON_WIDTH + 2 * valueButtonSpacing + valueBoxWidth, valueAreaY, BUTTON_WIDTH, BUTTON_HEIGHT));
    } else {
        // Boolean TRUE/FALSE gombok az érték körül
        const int16_t valueBoxWidth = 100;
        const int16_t valueButtonSpacing = 10;
        const int16_t boolButtonWidth = BUTTON_WIDTH + 10;
        const int16_t totalWidth = 2 * boolButtonWidth + 2 * valueButtonSpacing + valueBoxWidth;
        const int16_t startX2 = centerX - totalWidth / 2;

        _decreaseButton->setBounds(Rect(startX2, valueAreaY, boolButtonWidth, BUTTON_HEIGHT));                                                            // FALSE
        _increaseButton->setBounds(Rect(startX2 + boolButtonWidth + 2 * valueButtonSpacing + valueBoxWidth, valueAreaY, boolButtonWidth, BUTTON_HEIGHT)); // TRUE
    }
}

/**
 * @brief Dialógus teljes kirajzolása
 * Kirajzolja a dialógus keretét, üzenetet és az aktuális értéket
 */
void ValueChangeDialog::drawSelf() {
    // 1. Az UIDialogBase kirajzolja a keretet és a fejlécet.
    UIDialogBase::drawSelf();

    const Rect contentBounds = bounds;
    const int16_t centerX = contentBounds.x + contentBounds.width / 2;
    const int16_t headerHeight = getHeaderHeight();

    // 2. Az üzenet szövegének kirajzolása (a MessageDialog-tól örökölt 'message' tagváltozó alapján)
    // Az üzenetet a fejléc alá, középre igazítva rajzoljuk.
    if (this->message) {                                                        // Ellenőrizzük, hogy van-e üzenet
        const int16_t messageY = contentBounds.y + headerHeight + PADDING + 10; // Üzenet teteje 10 pixellel a fejléc+padding alatt
        tft.setFreeFont(&FreeSansBold9pt7b);
        tft.setTextSize(1);
        tft.setTextColor(colors.foreground, colors.background); // A dialógus általános színeit használjuk
        tft.setTextDatum(TC_DATUM);                             // Top-Center igazítás
        // Szélesség korlátozása a szövegnek, hogy ne lógjon ki
        // uint16_t messageMaxWidth = contentBounds.width - (2 * (PADDING + 5)); // 5px extra margó mindkét oldalon
        // TODO: Szükség esetén szövegtördelés implementálása, ha a szöveg túl hosszú
        tft.drawString(this->message, centerX, messageY); // A font már be van állítva a setFreeFont hívással
    }

    // Aktuális érték megjelenítése - gombok szintjében
    const int16_t valueAreaY = contentBounds.y + headerHeight + PADDING + VERTICAL_OFFSET_FOR_VALUE_AREA; // Ugyanaz mint layoutban
    const int16_t valueY = valueAreaY + BUTTON_HEIGHT / 2;                                                // Gomb közepén

    // Érték háttér nélküli megjelenítés - csak nagy szöveg
    String valueStr = getCurrentValueAsString(); // Színválasztás: teal ha eredeti érték, különben fehér
    uint16_t textColor = UIColorPalette::SCREEN_TEXT;
    if (isCurrentValueOriginal()) {
        textColor = TFT_CYAN; // Teal színű az eredeti érték
    }

    tft.setFreeFont(&FreeSansBold9pt7b); // Biztosítjuk a helyes betűtípust az értékhez
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(textColor, colors.background); // A dialógus hátterét használjuk
    tft.setTextSize(VALUE_TEXT_FONT_SIZE);
    tft.drawString(valueStr, centerX, valueY);
}

/**
 * @brief Aktuális érték szöveges formában
 * @return Az aktuális érték string reprezentációja
 */
String ValueChangeDialog::getCurrentValueAsString() const {
    switch (_valueType) {
        case ValueType::Integer:
            return _intPtr ? String(*_intPtr) : "N/A";
        case ValueType::Float:
            if (_floatPtr) {
                // Intelligens formázás: ha step 0.1, akkor 1 tizedesjegy, ha 0.01, akkor 2, stb.
                int decimalPlaces = 1; // Default 1 tizedesjegy 0.1 lépéshez
                if (_stepFloat >= 1.0f) {
                    decimalPlaces = 0;
                } else if (_stepFloat >= 0.1f) {
                    decimalPlaces = 1;
                } else if (_stepFloat >= 0.01f) {
                    decimalPlaces = 2;
                } else {
                    decimalPlaces = 3;
                }
                return String(*_floatPtr, decimalPlaces);
            }
            return "N/A";
        case ValueType::Boolean:
            return _boolPtr ? (*_boolPtr ? "True" : "False") : "N/A";
        case ValueType::UInt8:
            return _uint8Ptr ? String(*_uint8Ptr) : "N/A";
        default:
            return "Error";
    }
}

/**
 * @brief Érték növelése a lépésközzel
 * Növeli az értéket a megadott lépésközzel, ha nem lépi túl a maximumot
 */
void ValueChangeDialog::incrementValue() {
    bool valueChanged = false;
    switch (_valueType) {
        case ValueType::Integer:
            if (_intPtr && *_intPtr < _maxInt) { // Szigorúan kisebb, hogy a stepInt ne vigye túl
                *_intPtr += _stepInt;
                valueChanged = true;
            }
            break;
        case ValueType::Float:
            if (_floatPtr && *_floatPtr < _maxFloat) {
                *_floatPtr += _stepFloat;
                valueChanged = true;
            }
            break;
        case ValueType::Boolean:
            if (_boolPtr && !(*_boolPtr)) {
                *_boolPtr = true;
                valueChanged = true;
            }
            break;
        case ValueType::UInt8:
            if (_uint8Ptr) {
                // Biztonságos aritmetika overflow ellenőrzéssel
                int tempValue = (int)*_uint8Ptr + (int)_stepUint8;
                if (tempValue <= (int)_maxUint8) {
                    *_uint8Ptr = (uint8_t)tempValue;
                    valueChanged = true;
                }
            }
            break;
    }
    if (valueChanged) {
        validateAndClampValue();
        notifyValueChange();
    }
    // A redrawValueArea() / redrawValueTextOnly() hívása a hívó oldalon történik (gombnyomás, rotary)
}

/**
 * @brief Érték csökkentése a lépésközzel
 * Csökkenti az értéket a megadott lépésközzel, ha nem megy a minimum alá
 */
void ValueChangeDialog::decrementValue() {
    bool valueChanged = false;
    switch (_valueType) {
        case ValueType::Integer:
            if (_intPtr && *_intPtr > _minInt) { // Szigorúan nagyobb
                *_intPtr -= _stepInt;
                valueChanged = true;
            }
            break;
        case ValueType::Float:
            if (_floatPtr && *_floatPtr > _minFloat) {
                *_floatPtr -= _stepFloat;
                valueChanged = true;
            }
            break;
        case ValueType::Boolean:
            if (_boolPtr && (*_boolPtr)) {
                *_boolPtr = false;
                valueChanged = true;
            }
            break;
        case ValueType::UInt8:
            if (_uint8Ptr) {
                // Biztonságos aritmetika underflow ellenőrzéssel
                int tempValue = (int)*_uint8Ptr - (int)_stepUint8;
                if (tempValue >= (int)_minUint8) {
                    *_uint8Ptr = (uint8_t)tempValue;
                    valueChanged = true;
                }
            }
            break;
    }
    if (valueChanged) {
        validateAndClampValue();
        notifyValueChange();
    }
    // A redrawValueArea() / redrawValueTextOnly() hívása a hívó oldalon történik (gombnyomás, rotary)
}

/**
 * @brief Eredeti érték visszaállítása (Cancel esetén)
 * Visszaállítja az értéket a dialógus megnyitásakori eredeti értékre
 */
void ValueChangeDialog::restoreOriginalValue() {
    switch (_valueType) {
        case ValueType::Integer:
            if (_intPtr) {
                *_intPtr = _originalIntValue;
            }
            break;
        case ValueType::Float:
            if (_floatPtr) {
                *_floatPtr = _originalFloatValue;
            }
            break;
        case ValueType::Boolean:
            if (_boolPtr) {
                *_boolPtr = _originalBoolValue;
            }
            break;
        case ValueType::UInt8:
            if (_uint8Ptr) {
                *_uint8Ptr = _originalUint8Value;
            }
            break;
    }
    // Miután visszaállt az érték, értesítjük a külső callback-et
    notifyValueChange();
    // És frissítjük a dialógusban megjelenő értéket is
    redrawValueArea(); // Vagy redrawValueTextOnly(), de az Area általánosabb
}

/**
 * @brief Érték validálása és határok közé szorítása
 * Ellenőrzi és korrigálja az értéket, ha kívül esik a megengedett tartományon
 */
void ValueChangeDialog::validateAndClampValue() {
    switch (_valueType) {
        case ValueType::Integer:
            if (_intPtr) {
                if (*_intPtr < _minInt)
                    *_intPtr = _minInt;
                if (*_intPtr > _maxInt)
                    *_intPtr = _maxInt;
            }
            break;
        case ValueType::Float:
            if (_floatPtr) {
                if (*_floatPtr < _minFloat)
                    *_floatPtr = _minFloat;
                if (*_floatPtr > _maxFloat)
                    *_floatPtr = _maxFloat;
            }
            break;
        case ValueType::Boolean:
            // Boolean esetén nincs validáció szükséges
            break;
        case ValueType::UInt8:
            if (_uint8Ptr) {
                if (*_uint8Ptr < _minUint8)
                    *_uint8Ptr = _minUint8;
                if (*_uint8Ptr > _maxUint8)
                    *_uint8Ptr = _maxUint8;
            }
            break;
    }
}

/**
 * @brief Értesítés küldése az érték változásáról
 * Meghívja a callback függvényt az aktuális értékkel
 */
void ValueChangeDialog::notifyValueChange() {
    if (_valueCallback) {
        switch (_valueType) {
            case ValueType::Integer:
                if (_intPtr) {
                    // A _valueCallback std::variant<int, float, bool>-t vár
                    _valueCallback(std::variant<int, float, bool>(static_cast<int>(*_intPtr)));
                }
                break;
            case ValueType::Float:
                if (_floatPtr) {
                    _valueCallback(std::variant<int, float, bool>(*_floatPtr));
                }
                break;
            case ValueType::Boolean:
                if (_boolPtr) {
                    _valueCallback(std::variant<int, float, bool>(*_boolPtr));
                }
                break;
            case ValueType::UInt8:
                if (_uint8Ptr) {
                    // A _valueCallback std::variant<int, float, bool>-t vár, ezért int-re castolunk
                    _valueCallback(std::variant<int, float, bool>(static_cast<int>(*_uint8Ptr)));
                }
                break;
        }
    }
}

/**
 * @brief Érték terület újrarajzolása
 * Újrarajzolja az érték szöveget és frissíti a gombok állapotát
 */
void ValueChangeDialog::redrawValueArea() {
    const Rect contentBounds = bounds;
    const int16_t centerX = contentBounds.x + contentBounds.width / 2;

    // Értékterület koordinátái
    const int16_t headerHeight = getHeaderHeight();
    const int16_t valueAreaY = contentBounds.y + headerHeight + PADDING + VERTICAL_OFFSET_FOR_VALUE_AREA;
    const int16_t valueY = valueAreaY + BUTTON_HEIGHT / 2; // Háttér törlése a régi érték helyén (csak az érték szöveg területe)
    const int16_t clearWidth = 120;                        // Kisebb, csak az érték szöveghez
    const int16_t clearHeight = 30;                        // Kisebb magasság
    const int16_t clearX = centerX - clearWidth / 2;
    const int16_t clearY = valueY - clearHeight / 2;

    tft.fillRect(clearX, clearY, clearWidth, clearHeight, colors.background);

    // Érték szöveg - nagyobb fonttal, színkóddal
    String valueStr = getCurrentValueAsString();

    // Színválasztás: teal ha eredeti érték, különben fehér
    uint16_t textColor = UIColorPalette::SCREEN_TEXT;
    tft.setFreeFont(&FreeSansBold9pt7b); // Biztosítjuk a helyes betűtípust
    if (isCurrentValueOriginal()) {
        textColor = TFT_CYAN; // Teal színű az eredeti érték
    }
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(textColor, colors.background);
    tft.setTextSize(VALUE_TEXT_FONT_SIZE);
    tft.drawString(valueStr, centerX, valueY);

    // Gombok állapotának frissítése típus alapján
    if (_decreaseButton) {
        _decreaseButton->setEnabled(canDecrement());
        _decreaseButton->markForRedraw(); // Újrarajzolás kényszerítése
    }

    if (_increaseButton) {
        _increaseButton->setEnabled(canIncrement());
        _increaseButton->markForRedraw(); // Újrarajzolás kényszerítése
    }
}

/**
 * @brief Ellenőrzi, hogy az aktuális érték megegyezik-e az eredetivel
 * @return true ha az aktuális érték megegyezik az eredetivel, false egyébként
 */
bool ValueChangeDialog::isCurrentValueOriginal() const {
    switch (_valueType) {
        case ValueType::Integer:
            return _intPtr && (*_intPtr == _originalIntValue);
        case ValueType::Float:
            return _floatPtr && (abs(*_floatPtr - _originalFloatValue) < 0.001f); // Float összehasonlítás toleranciával
        case ValueType::Boolean:
            return _boolPtr && (*_boolPtr == _originalBoolValue);
        case ValueType::UInt8:
            return _uint8Ptr && (*_uint8Ptr == _originalUint8Value);
        default:
            return false;
    }
}

/**
 * @brief Ellenőrzi, hogy növelhető-e az érték
 * @return true ha az érték növelhető (nem éri el a maximumot), false egyébként
 */
bool ValueChangeDialog::canIncrement() const {
    switch (_valueType) {
        case ValueType::Integer:
            if (_intPtr) {
                return (*_intPtr + _stepInt) <= _maxInt;
            }
            return false;
        case ValueType::Float:
            if (_floatPtr) {
                // Lebegőpontos összehasonlítás toleranciával
                float newValue = *_floatPtr + _stepFloat;
                return newValue <= (_maxFloat + 0.001f); // Kis tolerancia a kerekítési hibákhoz
            }
            return false;
        case ValueType::Boolean:
            if (_boolPtr) {
                return !(*_boolPtr);
            }
            return false;
        case ValueType::UInt8:
            if (_uint8Ptr) {
                // Ellenőrizzük az overflow-t és underflow-t
                int tempValue = (int)*_uint8Ptr + (int)_stepUint8;
                return tempValue <= (int)_maxUint8;
            }
            return false;
        default:
            return false;
    }
}

/**
 * @brief Ellenőrzi, hogy csökkenthető-e az érték
 * @return true ha az érték csökkenthető (nem éri el a minimumot), false egyébként
 */
bool ValueChangeDialog::canDecrement() const {
    switch (_valueType) {
        case ValueType::Integer:
            if (_intPtr) {
                return (*_intPtr - _stepInt) >= _minInt;
            }
            return false;
        case ValueType::Float:
            if (_floatPtr) {
                // Lebegőpontos összehasonlítás toleranciával
                float newValue = *_floatPtr - _stepFloat;
                return newValue >= (_minFloat - 0.001f); // Kis tolerancia a kerekítési hibákhoz
            }
            return false;
        case ValueType::Boolean:
            if (_boolPtr) {
                return (*_boolPtr);
            }
            return false;
        case ValueType::UInt8:
            if (_uint8Ptr) {
                // Ellenőrizzük az underflow-t
                int tempValue = (int)*_uint8Ptr - (int)_stepUint8;
                return tempValue >= (int)_minUint8;
            }
            return false;
        default:
            return false;
    }
}

/**
 * @brief Csak az érték szöveg újrarajzolása (optimalizált)
 * Újrarajzolja csak az érték szöveget anélkül, hogy a gombokat is frissítené
 */
void ValueChangeDialog::redrawValueTextOnly() {
    const Rect contentBounds = bounds;
    const int16_t centerX = contentBounds.x + contentBounds.width / 2;
    const int16_t headerHeight = getHeaderHeight();
    const int16_t valueAreaY = contentBounds.y + headerHeight + PADDING + VERTICAL_OFFSET_FOR_VALUE_AREA;
    const int16_t valueY = valueAreaY + BUTTON_HEIGHT / 2;

    // Csak az érték szöveg területének törlése és újrarajzolása
    const int16_t clearWidth = 120;
    const int16_t clearHeight = 30;
    const int16_t clearX = centerX - clearWidth / 2;
    const int16_t clearY = valueY - clearHeight / 2;
    tft.fillRect(clearX, clearY, clearWidth, clearHeight, colors.background);
    String valueStr = getCurrentValueAsString();
    uint16_t textColor = isCurrentValueOriginal() ? TFT_CYAN : UIColorPalette::SCREEN_TEXT;
    tft.setFreeFont(&FreeSansBold9pt7b); // Biztosítjuk a helyes betűtípust
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(textColor, colors.background);
    tft.setTextSize(VALUE_TEXT_FONT_SIZE);
    tft.drawString(valueStr, centerX, valueY);

    // Gombok állapotának frissítése
    if (_decreaseButton) {
        _decreaseButton->setEnabled(canDecrement());
        _decreaseButton->markForRedraw(); // Újrarajzolás kényszerítése
    }

    if (_increaseButton) {
        _increaseButton->setEnabled(canIncrement());
        _increaseButton->markForRedraw(); // Újrarajzolás kényszerítése
    }
}
