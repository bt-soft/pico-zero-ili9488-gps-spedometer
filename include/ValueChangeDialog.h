#pragma once

#include "MessageDialog.h"
#include <functional>
#include <variant>

/**
 * @class ValueChangeDialog
 * @brief Univerzális érték módosító dialógus
 * Ez a dialógus képes különböző típusú értékek (int, float, bool) módosítására
 * egy egységes interfészen keresztül. Az értékek közvetlenül a megadott
 * pointeren vagy referencián keresztül frissülnek.
 */
class ValueChangeDialog : public MessageDialog {

  public:
    /**
     * @brief Támogatott érték típusok
     */
    enum class ValueType {
        Integer, ///< Egész szám
        Float,   ///< Lebegőpontos szám
        Boolean, ///< Logikai érték
        UInt8    ///< Előjel nélküli 8 bites egész
    };

    /**
     * @brief Érték módosítási callback típus
     * @param newValue Az új érték (variant típusként)
     */
    using ValueChangeCallback = std::function<void(const std::variant<int, float, bool> &)>;

  protected:
    // A _message tagváltozót a MessageDialog ősosztály már tartalmazza és kezeli.
    ValueType _valueType; ///< Az érték típusa

    // Érték pointerek (csak az egyik használatos a típus alapján)
    int *_intPtr = nullptr;
    float *_floatPtr = nullptr;
    bool *_boolPtr = nullptr;
    uint8_t *_uint8Ptr = nullptr;

    // Eredeti értékek a Cancel funkcióhoz
    int _originalIntValue = 0;
    float _originalFloatValue = 0.0f;
    bool _originalBoolValue = false;
    uint8_t _originalUint8Value = 0;

    // Beállítások
    int _minInt = 0, _maxInt = 100, _stepInt = 1;
    float _minFloat = 0.0f, _maxFloat = 100.0f, _stepFloat = 1.0f;
    uint8_t _minUint8 = 0, _maxUint8 = 255, _stepUint8 = 1;

    // Callbacks
    ValueChangeCallback _valueCallback = nullptr;
    DialogCallback _userDialogCallback = nullptr; // Új: Felhasználó által megadott DialogCallback

    // Optimalizálás: egyetlen buffer az értékek szöveges megjelenítéséhez
    mutable char _valueStringBuffer[32]; // Elég 32 karakter az összes típushoz

    // UI komponensek
    // Az _okButton és _cancelButton a MessageDialog által biztosított.
    std::shared_ptr<UIButton> _decreaseButton; ///< Érték csökkentő gomb
    std::shared_ptr<UIButton> _increaseButton; ///< Érték növelő gomb

    // Gomb méretek és elrendezés
    static constexpr uint16_t BUTTON_WIDTH = 40;  // 60;
    static constexpr uint16_t BUTTON_HEIGHT = 40; // 30;
    // static constexpr uint16_t SMALL_BUTTON_WIDTH = 40;
    static constexpr uint16_t BUTTON_SPACING = 8;
    static constexpr uint16_t VALUE_DISPLAY_HEIGHT = 40;
    static constexpr uint16_t FOOTER_AREA_HEIGHT = BUTTON_HEIGHT + 2 * PADDING;
    static constexpr uint16_t VERTICAL_OFFSET_FOR_VALUE_AREA = 50;
    static constexpr uint8_t VALUE_TEXT_FONT_SIZE = 2; ///< Az érték kijelzésének betűmérete (setTextSize)

    /**
     * @brief Dialógus tartalom létrehozása
     */
    virtual void createDialogContent() override;

    /**
     * @brief Dialógus tartalom elrendezése
     */
    virtual void layoutDialogContent() override;

    /**
     * @brief Saját tartalom rajzolása (üzenet + aktuális érték)
     */
    virtual void drawSelf() override;

  private:
    /**
     * @brief Aktuális érték lekérése char bufferbe
     * @param buffer A kimeneti buffer
     * @param bufferSize A buffer mérete
     * @return A buffer pointere
     */
    char *getCurrentValueAsString(char *buffer, size_t bufferSize) const;

    /**
     * @brief Érték növelése
     */
    void incrementValue();

    /**
     * @brief Érték csökkentése
     */
    void decrementValue();

    /**
     * @brief Eredeti érték visszaállítása (Cancel esetén)
     */
    void restoreOriginalValue();

    /**
     * @brief Érték boundary ellenőrzés és korrekció
     */
    void validateAndClampValue();

    /**
     * @brief Callback hívása ha van
     */
    void notifyValueChange();

    /**
     * @brief Csak az érték terület újrarajzolása (optimalizálás)
     */
    void redrawValueArea();

    /**
     * @brief Ellenőrzi, hogy az aktuális érték megegyezik-e az eredetivel
     * @return true ha az aktuális érték = eredeti érték
     */
    bool isCurrentValueOriginal() const;

    /**
     * @brief Csak az érték szöveg újrarajzolása (Boolean gombokhoz optimalizált)
     */
    void redrawValueTextOnly();

    /**
     * @brief Ellenőrzi, hogy növelhető-e az érték
     * @return true ha növelhető, false ha elérte a maximumot
     */
    bool canIncrement() const;

    /**
     * @brief Ellenőrzi, hogy csökkenthető-e az érték
     * @return true ha csökkenthető, false ha elérte a minimumot
     */
    bool canDecrement() const;

  public:
    /**
     * @brief Konstruktor integer értékhez
     * @param parentScreen Szülő képernyő
     * @param title Dialógus címe
     * @param message Érték magyarázó szöveg
     * @param valuePtr Módosítandó integer pointer
     * @param minValue Minimum érték
     * @param maxValue Maximum érték
     * @param stepValue Lépésköz
     * @param callback Értékváltozás callback
     * @param userDialogCb Dialógus lezárásakor hívandó callback (OK/Cancel után)
     * @param bounds Dialógus mérete és pozíciója
     * @param cs Színséma
     */
    ValueChangeDialog(UIScreen *parentScreen, const char *title, const char *message, int *valuePtr, int minValue, int maxValue, int stepValue = 1, ValueChangeCallback callback = nullptr,
                      DialogCallback userDialogCb = nullptr, const Rect &bounds = {-1, -1, 0, 0}, const ColorScheme &cs = ColorScheme::defaultScheme());

    /**
     * @brief Konstruktor float értékhez
     * @param parentScreen Szülő képernyő
     * @param title Dialógus címe
     * @param message Érték magyarázó szöveg
     * @param valuePtr Módosítandó float pointer
     * @param minValue Minimum érték
     * @param maxValue Maximum érték
     * @param stepValue Lépésköz
     * @param callback Értékváltozás callback
     * @param userDialogCb Dialógus lezárásakor hívandó callback (OK/Cancel után)
     * @param bounds Dialógus mérete és pozíciója
     * @param cs Színséma
     */
    ValueChangeDialog(UIScreen *parentScreen, const char *title, const char *message, float *valuePtr, float minValue, float maxValue, float stepValue = 1.0f, ValueChangeCallback callback = nullptr,
                      DialogCallback userDialogCb = nullptr, const Rect &bounds = {-1, -1, 0, 0}, const ColorScheme &cs = ColorScheme::defaultScheme());

    /**
     * @brief Konstruktor boolean értékhez
     * @param parentScreen Szülő képernyő
     * @param title Dialógus címe
     * @param message Érték magyarázó szöveg
     * @param valuePtr Módosítandó boolean pointer
     * @param callback Értékváltozás callback
     * @param userDialogCb Dialógus lezárásakor hívandó callback (OK/Cancel után)
     * @param bounds Dialógus mérete és pozíciója
     * @param cs Színséma
     */
    ValueChangeDialog(UIScreen *parentScreen, const char *title, const char *message, bool *valuePtr, ValueChangeCallback callback = nullptr, DialogCallback userDialogCb = nullptr, const Rect &bounds = {-1, -1, 0, 0},
                      const ColorScheme &cs = ColorScheme::defaultScheme());

    /**
     * @brief Konstruktor uint8_t értékhez
     * @param parentScreen Szülő képernyő
     * @param title Dialógus címe
     * @param message Érték magyarázó szöveg
     * @param valuePtr Módosítandó uint8_t pointer
     * @param minValue Minimum érték
     * @param maxValue Maximum érték
     * @param stepValue Lépésköz
     * @param callback Értékváltozás callback
     * @param userDialogCb Dialógus lezárásakor hívandó callback (OK/Cancel után)
     * @param bounds Dialógus mérete és pozíciója
     * @param cs Színséma
     */
    ValueChangeDialog(UIScreen *parentScreen, const char *title, const char *message, uint8_t *valuePtr, uint8_t minValue, uint8_t maxValue, uint8_t stepValue = 1, ValueChangeCallback callback = nullptr,
                      DialogCallback userDialogCb = nullptr, const Rect &bounds = {-1, -1, 0, 0}, const ColorScheme &cs = ColorScheme::defaultScheme());
    /**
     * @brief Destruktor
     */
    virtual ~ValueChangeDialog() = default;
};
