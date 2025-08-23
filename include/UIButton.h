#ifndef __UI_BUTTON_H
#define __UI_BUTTON_H

#include <functional>

#include "UIComponent.h"

/**
 * @brief UI Button komponens
 *
 * Egyszerű gomb komponens, amely kezeli a lenyomást és a megjelenítést.
 */
class UIButton : public UIComponent {

  public:
    // Alapértelmezett gomb méretek
    static constexpr uint16_t DEFAULT_BUTTON_WIDTH = 72;       // Alapértelmezett gomb szélesség
    static constexpr uint16_t DEFAULT_BUTTON_HEIGHT = 35;      // Alapértelmezett gomb magasság
    static constexpr uint16_t HORIZONTAL_TEXT_PADDING = 2 * 8; // 8px padding a szöveg mindkét oldalán
    static constexpr uint16_t BUTTON_TOUCH_MARGIN = 6;         // Gomb érintési érzékenység margója

    // Gomb típusok
    enum class ButtonType {
        Pushable,  // Egyszerű nyomógomb
        Toggleable // Váltógomb (on/off)
    };

    // Gomb állapotok (eseményekhez és külső lekérdezéshez, a belső logikai állapotot a LogicalButtonState tárolja)
    enum class EventButtonState {
        Off = 0,
        On,
        Disabled,
        CurrentActive, // Jelenleg aktív mód jelzése
        Clicked,       // Lenyomás történt (Pushable gomboknál)
        LongPressed    // Hosszú nyomás
    };

    // Gomb esemény struktúra
    struct ButtonEvent {
        uint8_t id;
        const char *label;
        EventButtonState state;
        ButtonEvent(uint8_t id, const char *label, EventButtonState state) : id(id), label(label), state(state) {}
    };

    enum class ButtonState { Off, On, Disabled, CurrentActive };

  private:
    static constexpr uint8_t CORNER_RADIUS = 5;
    static constexpr uint32_t LONG_PRESS_THRESHOLD = 1000; // ms

    uint8_t buttonId;
    const char *label;
    ButtonType buttonType = ButtonType::Pushable;
    ButtonState currentState = ButtonState::Off;
    bool autoSizeToText = false; // Új tagváltozó az automatikus méretezéshez
    bool useMiniFont = false;
    uint32_t pressStartTime = 0;
    bool longPressThresholdMet = false; // Jelzi, ha a hosszú lenyomás küszöbét elértük
    bool longPressEventFired = false;   // Jelzi, ha a hosszú lenyomás esemény már aktiválódott

    ButtonColorScheme currentButtonScheme;                  // Gomb-specifikus színséma
    std::function<void(const ButtonEvent &)> eventCallback; // Az UIComponent::colors továbbra is létezik és az alap sémát tárolja
    std::function<void()> clickCallback;                    // Backward compatibility

    // Gomb állapot színek
    struct StateColors {
        uint16_t background;
        uint16_t border;
        uint16_t text;
        uint16_t led;
    };

    /**
     * @brief Lekéri a gomb aktuális állapotának megfelelő színeket
     * @return StateColors struktúra, amely tartalmazza a háttér, keret, szöveg és LED színeket
     */
    StateColors getStateColors() const {
        StateColors resultColors;
        // A currentButtonScheme tagot használjuk, ami ButtonColorScheme típusú

        if (currentState == ButtonState::Disabled) {
            resultColors.background = this->currentButtonScheme.disabledBackground;
            resultColors.border = this->currentButtonScheme.disabledBorder;
            resultColors.text = this->currentButtonScheme.disabledForeground;
            resultColors.led = this->currentButtonScheme.ledOffColor;
            // TODO: A disabled állapot LED-jének láthatóságát még átvezetni a kódon

        } else if (this->pressed) {
            resultColors.background = this->currentButtonScheme.pressedBackground; // Gradiens alapja
            resultColors.border = this->currentButtonScheme.pressedBorder;
            resultColors.text = this->currentButtonScheme.pressedForeground;

            // LED színe lenyomott állapotban
            if (buttonType == ButtonType::Toggleable) {
                // Toggleable esetén a LED a logikai állapotot mutatja lenyomáskor is
                resultColors.led = (currentState == ButtonState::On) ? this->currentButtonScheme.ledOnColor : this->currentButtonScheme.ledOffColor;
            } else { // Pushable
                resultColors.led = TFT_BLACK;
            }

        } else {
            // Ha On állapotban van, az a konstruktor és a setButtonState miatt csak Toggleable lehet.
            if (currentState == ButtonState::On) {
                resultColors.background = this->currentButtonScheme.activeBackground; // Vagy .background, ha az ON állapot háttere azonos
                resultColors.border = this->currentButtonScheme.ledOnColor;           // Keret színe a LED "On" színével
                resultColors.text = this->currentButtonScheme.activeForeground;       // Vagy .foreground
                resultColors.led = this->currentButtonScheme.ledOnColor;

            } else if (currentState == ButtonState::CurrentActive) {
                resultColors.background = this->currentButtonScheme.activeBackground; // Vagy this->colors.activeBackground, ha más, mint az ON
                resultColors.border = TFT_BLUE;                                       // Speciális eset, vagy this->colors.activeBorder
                resultColors.text = this->currentButtonScheme.activeForeground;       // Vagy this->colors.activeForeground

                // CurrentActive állapotban a LED jellemzően nem releváns, a kiemelés a háttér/keret színeivel történik.
                resultColors.led = TFT_BLACK;
            } else {
                // ButtonState::Off (Pushable és Toggleable esetén is)
                resultColors.background = this->currentButtonScheme.background;
                resultColors.border = this->currentButtonScheme.border;
                resultColors.text = this->currentButtonScheme.foreground;
                resultColors.led = (buttonType == ButtonType::Toggleable) ? this->currentButtonScheme.ledOffColor : TFT_BLACK;
            }
        }
        return resultColors;
    }

    /**
     * @brief Sötétíti a megadott színt egy adott mértékben
     * @param color A szín, amelyet sötétíteni szeretnénk (16 bites RGB formátumban)
     * @param amount A sötétítés mértéke (0-255 között)
     * @return A sötétített szín (16 bites RGB formátumban)
     */
    uint16_t darkenColor(uint16_t color, uint8_t amount) const {
        // Kivonjuk a piros, zöld és kék színösszetevőket
        uint8_t r = (color & 0xF800) >> 11;
        uint8_t g = (color & 0x07E0) >> 5;
        uint8_t b = (color & 0x001F);

        // Finomítjuk a csökkentési mértéket, figyelembe véve a színösszetevők közötti eltéréseket
        uint8_t darkenAmount = amount > 0 ? (amount >> 3) : 0;

        // A csökkentésnél biztosítjuk, hogy ne menjenek 0 alá az értékek
        r = (r > darkenAmount) ? r - darkenAmount : 0;
        g = (g > darkenAmount) ? g - darkenAmount : 0;
        b = (b > darkenAmount) ? b - darkenAmount : 0;

        // Visszaalakítjuk a színt 16 bites RGB formátumba
        return (r << 11) | (g << 5) | b;
    }

    /**
     * @brief Rajzol egy gradiens effektet a gomb lenyomott állapotában
     * @param baseColorForEffect A gradiens alap színe, amelyet sötétítünk
     */
    void drawPressedEffect(uint16_t baseColorForEffect) {
        const uint8_t steps = 6; // TFT_BUTTON_DARKEN_COLORS_STEPS
        uint8_t stepWidth = bounds.width / steps;
        uint8_t stepHeight = bounds.height / steps;

        uint16_t baseColor = baseColorForEffect;

        for (uint8_t i = 0; i < steps; i++) {
            uint16_t fadedColor = darkenColor(baseColor, i * 30); // Erősebb sötétítés
            tft.fillRoundRect(bounds.x + i * stepWidth / 2, bounds.y + i * stepHeight / 2, bounds.width - i * stepWidth, bounds.height - i * stepHeight, CORNER_RADIUS, fadedColor);
        }
    }

    /**
     * @brief Frissíti a gomb szélességét a felirat, szövegméret és padding alapján.
     * Csak akkor fut le, ha az autoSizeToText engedélyezve van.
     */
    void updateWidthToFitText() {

        // Üres label esetén is lehet alapértelmezett szélesség
        if (!autoSizeToText || label == nullptr) {
            if (!autoSizeToText)
                return;
        }

        // TFT szövegbeállítások mentése és visszaállítása a számításhoz
        uint8_t prevDatum = tft.getTextDatum();
        uint8_t currentTftTextSize = tft.textsize; // A tft.textsize tartalmazza az aktuális méretet

        // Betűtípus beállítása a felirathoz
        tft.setTextSize(1);
        if (useMiniFont) {
            tft.setFreeFont(); // Alapértelmezett (kisebb) font
        } else {
            tft.setFreeFont(&FreeSansBold9pt7b);
        }

        uint16_t textW = (label && strlen(label) > 0) ? tft.textWidth(label) : 0;
        uint16_t newWidth = textW + HORIZONTAL_TEXT_PADDING; // HORIZONTAL_TEXT_PADDING mindkét oldali paddingot tartalmazza

        // Minimális szélesség alkalmazása (pl. ne legyen keskenyebb a magasságánál, vagy egy abszolút minimumnál)
        newWidth = std::max(newWidth, static_cast<uint16_t>(bounds.height > 0 ? bounds.height : DEFAULT_BUTTON_HEIGHT));
        newWidth = std::max(newWidth, static_cast<uint16_t>(DEFAULT_BUTTON_WIDTH / 2));

        if (bounds.width != newWidth) {
            bounds.width = newWidth;
            markForRedraw();
        }

        // Visszaállítás az eredeti TFT szövegméretre
        tft.setTextSize(currentTftTextSize);
        tft.setTextDatum(prevDatum);
    }

  public:
    /**
     * @brief Kiszámítja a gomb szélességét a szöveg, betűméret és aktuális gombmagasság alapján.
     * @param tftRef TFT_eSPI referencia
     * @param text A gomb felirata
     * @param btnUseMiniFont Ha true, akkor a kisebb betűtípust használja
     * @param currentButtonHeight Az aktuális gombmagasság, amelyet figyelembe veszünk a minimális szélesség meghatározásakor
     * @return A gomb szélessége, amely figyelembe veszi a szöveg hosszát és a minimális méreteket
     * @note A gomb szélessége legalább a minimális gombmagasság vagy a DEFAULT_BUTTON_WIDTH / 2, ha a szöveg nem elég széles.
     */
    static uint16_t calculateWidthForText(const char *text, bool btnUseMiniFont, uint16_t currentButtonHeight) {
        if (text == nullptr)
            text = ""; // Kezeljük a nullptr-t üres stringként

        uint8_t prevDatum = ::tft.getTextDatum();
        uint8_t currentTftTextSize = ::tft.textsize;

        // Betűtípus beállítása a felirathoz
        ::tft.setTextSize(1);
        if (btnUseMiniFont) {
            ::tft.setFreeFont(); // Alapértelmezett (kisebb) font
        } else {
            ::tft.setFreeFont(&FreeSansBold9pt7b);
        }

        uint16_t textW = strlen(text) > 0 ? ::tft.textWidth(text) : 0;
        uint16_t calculatedWidth = textW + HORIZONTAL_TEXT_PADDING;

        // Minimális szélesség
        uint16_t minHeight = (currentButtonHeight > 0) ? currentButtonHeight : DEFAULT_BUTTON_HEIGHT;
        calculatedWidth = std::max(calculatedWidth, minHeight);
        calculatedWidth = std::max(calculatedWidth, static_cast<uint16_t>(DEFAULT_BUTTON_WIDTH / 2));

        ::tft.setTextSize(currentTftTextSize);
        ::tft.setTextDatum(prevDatum);

        return calculatedWidth;
    }

    /**
     * @brief Gomb komponens konstruktora
     * @param id Gomb azonosítója
     * @param bounds A gomb határai (Rect)
     * @param label A gomb felirata
     * @param type A gomb típusa (Pushable vagy Toggleable)
     * @param state A gomb kezdeti állapota (Off, On, Disabled)
     * @param callback Eseménykezelő függvény, amely a gomb eseményeit kezeli
     * @param colors Színpaletta a gombhoz
     * @param autoSizeToText Ha true, akkor a gomb szélessége automatikusan igazodik a szöveghez
     * @note A bounds szélessége és magassága 0 esetén az alapértelmezett méreteket használja (DEFAULT_BUTTON_WIDTH és DEFAULT_BUTTON_HEIGHT).
     * @details A gomb alapértelmezett színpalettát használ, ha nem adunk meg másikat.
     */
    UIButton(uint8_t id,                                                                    // ID
             const Rect &bounds,                                                            // rect
             const char *label,                                                             // label
             ButtonType type = ButtonType::Pushable,                                        // type
             ButtonState state = ButtonState::Off,                                          // initial state
             std::function<void(const ButtonEvent &)> callback = nullptr,                   // callback
             const ButtonColorScheme &scheme = UIColorPalette::createDefaultButtonScheme(), // scheme
             bool autoSizeToText = false                                                    // Automatikus méretezés flag
             )
        : UIComponent(Rect(bounds.x, bounds.y,
                           (bounds.width == 0 && !autoSizeToText ? DEFAULT_BUTTON_WIDTH : bounds.width), // Szélesség beállítása
                           (bounds.height == 0 ? DEFAULT_BUTTON_HEIGHT : bounds.height)),                // Magasság beállítása
                      scheme),                                                                           // UIComponent konstruktora az alap ColorScheme résszel inicializálódik
          buttonId(id), label(label), buttonType(type), currentButtonScheme(scheme),                     // Teljes ButtonColorScheme mentése
          eventCallback(callback), autoSizeToText(autoSizeToText) {

        if (autoSizeToText) {
            updateWidthToFitText(); // Méret frissítése a szöveghez, ha kérték
        }

        this->currentState = state;
        if (buttonType == ButtonType::Pushable && this->currentState == ButtonState::On) {
            this->currentState = ButtonState::Off;
        }

        // Ha a gombot eleve letiltott állapottal hozzuk létre,
        // akkor az ősosztály 'disabled' flag-jét is be kell állítani.
        if (this->currentState == ButtonState::Disabled) {
            UIComponent::disabled = true;
        }
    }

    /**
     * @brief Gomb komponens konstruktora
     * @param id Gomb azonosítója
     * @param bounds A gomb határai (Rect)
     * @param label A gomb felirata
     * @param type A gomb típusa (Pushable vagy Toggleable)
     * @param callback Eseménykezelő függvény, amely a gomb eseményeit kezeli
     * @param colors Színpaletta a gombhoz
     * @param autoSizeToText Ha true, akkor a gomb szélessége automatikusan igazodik a szöveghez
     * @note A bounds szélessége és magassága 0 esetén az alapértelmezett méreteket használja (DEFAULT_BUTTON_WIDTH és DEFAULT_BUTTON_HEIGHT).
     */
    // Második konstruktor overload az autoSizeToText paraméterrel
    UIButton(uint8_t id,                                                                    // ID
             const Rect &bounds,                                                            // rect
             const char *label,                                                             // label
             ButtonType type,                                                               // Nincs alapértelmezett
             std::function<void(const ButtonEvent &)> callback,                             // Nincs alapértelmezett
             const ButtonColorScheme &scheme = UIColorPalette::createDefaultButtonScheme(), // scheme
             bool autoSizeToText = false                                                    // Automatikus méretezés flag
             )
        : UIComponent(Rect(bounds.x, bounds.y, (bounds.width == 0 && !autoSizeToText ? DEFAULT_BUTTON_WIDTH : bounds.width), (bounds.height == 0 ? DEFAULT_BUTTON_HEIGHT : bounds.height)),
                      scheme),                                                       // UIComponent konstruktora az alap ColorScheme résszel inicializálódik
          buttonId(id), label(label), buttonType(type), currentButtonScheme(scheme), // Teljes ButtonColorScheme mentése
          eventCallback(callback), autoSizeToText(autoSizeToText) {
        this->currentState = ButtonState::Off; // Alapértelmezett állapot ennél a konstruktornál
        if (autoSizeToText) {
            updateWidthToFitText();
        }
    }

    /**
     * @brief Sima pushButton
     * @param id Gomb azonosítója
     * @param bounds A gomb határai (Rect)
     * @param label A gomb felirata
     * @param callback Eseménykezelő függvény, amely a gomb eseményeit kezeli
     * @param autoSizeToText Ha true, akkor a gomb szélessége automatikusan igazodik a szöveghez
     * @note Ez a konstruktor csak pushable gombokat hoz létre, a gomb típusa mindig Pushable lesz.
     */
    // Harmadik konstruktor overload az autoSizeToText paraméterrel
    UIButton(uint8_t id,                                                  // ID
             const Rect &bounds,                                          // rect
             const char *label,                                           // label
             std::function<void(const ButtonEvent &)> callback = nullptr, // callback
             bool autoSizeToText = false                                  // Automatikus méretezés flag
             )
        : UIComponent(Rect(bounds.x, bounds.y, (bounds.width == 0 && !autoSizeToText ? DEFAULT_BUTTON_WIDTH : bounds.width), (bounds.height == 0 ? DEFAULT_BUTTON_HEIGHT : bounds.height)),
                      UIColorPalette::createDefaultButtonScheme()),                                                                                 // UIComponent konstruktora az alap ColorScheme résszel inicializálódik
          buttonId(id), label(label), buttonType(UIButton::ButtonType::Pushable), currentButtonScheme(UIColorPalette::createDefaultButtonScheme()), // Teljes ButtonColorScheme mentése
          currentState(UIButton::ButtonState::Off), eventCallback(callback), autoSizeToText(autoSizeToText) {
        if (autoSizeToText) {
            updateWidthToFitText();
        }
    }

    // /**
    //  * @brief Gomb állapotának szöveges megjelenítése
    //  * @param state Az állapot, amelyet szövegesen szeretnénk megjeleníteni
    //  */
    // static const char *eventButtonStateToString(EventButtonState state) {
    //     switch (state) {
    //     case EventButtonState::Off:
    //         return "Off";
    //     case EventButtonState::On:
    //         return "On";
    //     case EventButtonState::Disabled:
    //         return "Disabled";
    //     case EventButtonState::CurrentActive:
    //         return "CurrentActive";
    //     case EventButtonState::Clicked:
    //         return "Clicked";
    //     case EventButtonState::LongPressed:
    //         return "LongPressed";
    //     default:
    //         return "Unknown";
    //     }
    // }

    // // Segédfüggvény a ButtonType szöveges megjelenítéséhez (ha később kellene)
    // static const char *buttonTypeToString(ButtonType type) {
    //     switch (type) {
    //     case ButtonType::Pushable:
    //         return "Pushable";
    //     case ButtonType::Toggleable:
    //         return "Toggleable";
    //     default:
    //         return "Unknown";
    //     }
    // }

    // ================================
    // Getters/Setters
    // ================================

    // Getters & Setters
    uint8_t getId() const { return buttonId; }
    void setId(uint8_t id) { buttonId = id; }

    ButtonType getButtonType() const { return buttonType; }
    void setButtonType(ButtonType type) {
        if (buttonType != type) {
            buttonType = type;
            markForRedraw();
        }
    }

    /**
     * @brief Gomb aktuális állapotának lekérdezése
     * @return A gomb aktuális állapota (ButtonState)
     * @details Ez az állapot a logikai állapotot tükrözi, amelyet a gomb kezelése során használunk.
     */
    ButtonState getButtonState() const { return currentState; }

    /**
     * @brief Gomb állapotának beállítása
     * @param newState Az új állapot, amelyet be szeretnénk állítani
     */
    void setButtonState(ButtonState newState) {
        // Pushable gomb nem lehet On állapotú
        if (buttonType == ButtonType::Pushable && newState == ButtonState::On) {
            newState = ButtonState::Off;
        }

        if (currentState == newState) {
            // Ha az állapot már ugyanaz, ellenőrizzük és javítjuk a konzisztenciát UIComponent::enabled-del
            if (newState == ButtonState::Disabled) {
                if (UIComponent::isDisabled())
                    UIComponent::setDisabled(true);

            } else { // newState egy engedélyezett állapot
                if (!UIComponent::isDisabled())
                    UIComponent::setDisabled(false);
            }
            // Ha csak a konzisztencia javítása történt, és az enabled állapot nem változott, nincs szükség további markForRedraw-ra innen.
            return;
        }

        ButtonState oldState = currentState;
        currentState = newState;

        if (newState == ButtonState::Disabled) {
            UIComponent::setDisabled(true);             // Az ősosztály setDisabled metódusa kezeli a markForRedraw-t, ha az 'enabled' változik.
        } else if (oldState == ButtonState::Disabled) { // Tiltottról engedélyezett állapotra váltás
            UIComponent::setDisabled(false);            // Az ősosztály setDisabled metódusa kezeli a markForRedraw-t, ha az 'enabled' változik.
        }

        // Mindig újrarajzolást kérünk, ha a logikai állapot változott (pl. On -> Off),
        // mert a gomb kinézete ettől függhet, még ha az UIComponent::enabled nem is változott.
        markForRedraw();
    }

    /**
     * @brief Gomb engedélyezése vagy letiltása
     * @param enable true ha engedélyezni szeretnénk, false ha letiltani
     */
    void setEnabled(bool enable) {
        if (enable) {
            // Engedélyezés
            if (currentState == ButtonState::Disabled) { // Módosítva
                // Ha logikailag le volt tiltva, állítsuk vissza alapértelmezett engedélyezett állapotra (pl. Off).
                // A setButtonState gondoskodik az UIComponent::setEnabled(true) hívásáról.
                setButtonState(ButtonState::Off);
            } else {
                // Nem volt logikailag letiltva, de biztosítsuk, hogy az ősosztály komponens is engedélyezve legyen.
                UIComponent::setDisabled(false);
            }
        } else {
            // Letiltás
            if (currentState != ButtonState::Disabled) { // Módosítva
                // Ha nem volt logikailag letiltva, állítsuk letiltott állapotra.
                // A setButtonState gondoskodik az UIComponent::setEnabled(false) hívásáról.
                setButtonState(ButtonState::Disabled);
            } else {
                // Már logikailag le volt tiltva, csak biztosítsuk, hogy az ősosztály komponens is le legyen tiltva.
                UIComponent::setDisabled(true);
            }
        }
    }

    /**
     * @brief Automatikus méretezés bekapcsolása vagy kikapcsolása
     * @param enable true ha engedélyezni szeretnénk az automatikus méretezést, false ha kikapcsolni
     */
    void setAutoSizeToText(bool enable) {
        if (autoSizeToText != enable) {
            autoSizeToText = enable;
            if (autoSizeToText) {
                updateWidthToFitText(); // Azonnal frissítjük a méretet, ha bekapcsoljuk
            }
            // Ha kikapcsoljuk, a gomb megtartja az aktuális méretét,
            // vagy itt visszaállíthatnánk egy alapértelmezettre, ha az a kívánalom.
            // Jelenleg megtartja.
            else {
                markForRedraw(); // Lehet, hogy a stílus változik, de a méret nem feltétlenül
            }
        }
    }
    /**
     * @brief Lekérdezi, hogy a gomb automatikusan méreteződik-e a szöveghez
     * @return true ha az automatikus méretezés engedélyezve van, false ha nem
     */
    bool getAutoSizeToText() const { return autoSizeToText; }

    /**
     * @brief Gomb feliratának beállítása
     * @param newLabel Az új felirat, amelyet be szeretnénk állítani
     * @details Ha a felirat megváltozik, frissíti a gomb szélességét, ha az automatikus méretezés engedélyezve van.
     */
    void setLabel(const char *newLabel) {
        label = newLabel;

        if (autoSizeToText) {
            updateWidthToFitText(); // Ez már tartalmazza a markForRedraw-t, ha a szélesség változik
        } else {
            markForRedraw(); // Csak újrarajzolás, méret nem változik
        }
    }

    /**
     * @brief Gomb feliratának lekérdezése
     * @return A gomb felirata (const char*)
     */
    const char *getLabel() const { return label; }

    // Mini font használata
    void setUseMiniFont(bool mini) {
        if (useMiniFont != mini) {
            useMiniFont = mini;
            markForRedraw();
        }
    }

    /**
     * @brief Lekérdezi, hogy a gomb mini fontot használ-e
     * @return true ha a mini font engedélyezve van, false ha nem
     * @details A mini font kisebb méretű betűtípust jelent, amelyet a gomb szövegének megjelenítésére használunk.
     */
    bool isUseMiniFont() const { return useMiniFont; }

    /**
     * @brief Esemény callback beállítása
     * @param callback A callback függvény, amely a gomb eseményeit kezeli
     * @details Ez a callback függvény akkor hívódik meg, amikor a gomb esemény történik (pl. lenyomás, hosszú nyomás).
     * @note A callback függvénynek egy ButtonEvent struktúrát kell fogadnia, amely tartalmazza a gomb azonosítóját, feliratát és állapotát.
     */
    void setEventCallback(std::function<void(const ButtonEvent &)> callback) { eventCallback = callback; }

    /**
     * @brief Visszahívási függvény beállítása a gomb lenyomására
     * @param callback A visszahívási függvény, amely a gomb lenyomásakor hívódik meg
     * @details Ez a visszahívási függvény akkor hívódik meg, amikor a gombot lenyomják, és a gomb eseménykezelője nem lett beállítva.
     * @note Ez a funkció visszafelé kompatibilis, és a gomb lenyomására reagál, de nem kezeli a hosszú nyomást vagy más eseményeket.
     */
    void setClickCallback(std::function<void()> callback) { clickCallback = callback; }

    /**
     * @brief Beállítja a gombot "default choice" gombként
     * @details Ez a gombot letiltja és alkalmazza a "default choice" színsémát, amelyet a MultiButtonDialog használ.
     * A gomb nem lesz interaktív, és a felhasználó nem tudja megnyomni.
     */
    void setAsDefaultChoiceButton() {
        setEnabled(false); // Letiltjuk a gombot
        // A createDefaultChoiceButtonScheme ColorScheme-et ad vissza.
        // Ha ButtonColorScheme-et adna vissza, akkor a setButtonColorScheme-et hívnánk.
        // Jelenlegi implementáció szerint a UIComponent::setColorScheme hívódik meg.
        // Ahhoz, hogy a ButtonColorScheme specifikus részei is beállítódjanak, vagy
        // a createDefaultChoiceButtonScheme-nek ButtonColorScheme-et kell visszaadnia,
        // vagy itt manuálisan kell létrehozni egy ButtonColorScheme-et.
        // Tegyük fel, hogy createDefaultChoiceButtonScheme ButtonColorScheme-et ad vissza a jövőben:
        // setButtonColorScheme(UIColorPalette::createDefaultChoiceButtonScheme());
        // Addig is, ha ColorScheme-et ad vissza:
        ColorScheme baseScheme = UIColorPalette::createDefaultChoiceButtonScheme();
        ButtonColorScheme btnScheme(baseScheme, baseScheme.activeBorder, TFT_DARKGREY); // Próbáljuk meg kitölteni a LED színeket
        setButtonColorScheme(btnScheme);
    }

    /**
     * @brief Gomb megjelenítése
     * @details Ez a metódus rajzolja ki a gombot a TFT képernyőre.
     * Ha a gomb lenyomva van, akkor a lenyomott effektust rajzolja ki.
     */
    virtual void draw() override {
        if (!needsRedraw)
            return;

        StateColors currentDrawColors = getStateColors();

        if (this->pressed) {                                 // Ha lenyomva van, rajzoljuk a pressed effektet
            drawPressedEffect(currentDrawColors.background); // Gradiens effekt rajzolása lenyomott állapotban
        } else {
            tft.fillRoundRect(bounds.x, bounds.y, bounds.width, bounds.height, CORNER_RADIUS, currentDrawColors.background);
        }

        // Keret rajzolása
        tft.drawRoundRect(bounds.x, bounds.y, bounds.width, bounds.height, CORNER_RADIUS, currentDrawColors.border);

        // Szöveg rajzolása
        if (label != nullptr) {
            tft.setTextSize(1);
            if (useMiniFont) {
                tft.setFreeFont(); // Alapértelmezett (kisebb) font
            } else {
                tft.setFreeFont(&FreeSansBold9pt7b);
            }

            tft.setTextColor(currentDrawColors.text);
            tft.setTextDatum(MC_DATUM); // Middle Center

            // Ellenőrizzük, hogy lesz-e LED
            bool willHaveLed = (buttonType == ButtonType::Toggleable && !useMiniFont && currentDrawColors.led != TFT_BLACK);

            // Szöveg pozíció finomhangolása
            int16_t textY = bounds.centerY();
            if (useMiniFont) {
                textY += 1; // Mini font esetén kicsit lejjebb
            }

            // Ha van LED, akkor a szöveget feljebb toljuk, hogy 3 pixel gap legyen
            if (willHaveLed) {
                constexpr uint8_t LED_HEIGHT = 5;
                constexpr uint8_t LED_GAP = 3;                               // Kívánt gap a szöveg és LED között
                int16_t ledTopY = bounds.y + bounds.height - LED_HEIGHT - 3; // LED teteje
                int16_t desiredTextBottomY = ledTopY - LED_GAP;              // Szöveg alja a gap-pel

                // A szöveg magasságának becslése (font függő)
                int16_t textHeight = useMiniFont ? 8 : 12;                     // Becsült szövegmagasság
                int16_t adjustedTextY = desiredTextBottomY - (textHeight / 2); // Middle center pozíció

                // Csak akkor módosítjuk, ha feljebb van, mint az eredeti középpozíció
                if (adjustedTextY < textY) {
                    textY = adjustedTextY;
                }
            }

            tft.drawString(label, bounds.centerX(), textY);
        }

        // LED csík rajzolása (csak toggleable gomboknál, ha nem mini font és van LED szín)
        if (buttonType == ButtonType::Toggleable && !useMiniFont && currentDrawColors.led != TFT_BLACK) {
            constexpr uint8_t LED_HEIGHT = 5;
            constexpr uint8_t LED_MARGIN = 10;
            tft.fillRect(bounds.x + LED_MARGIN, bounds.y + bounds.height - LED_HEIGHT - 3, bounds.width - 2 * LED_MARGIN, LED_HEIGHT, currentDrawColors.led);
        }

        needsRedraw = false;
    }

    /**
     * @brief Beállítja a gomb színsémáját.
     * @param newScheme Az új ButtonColorScheme.
     */
    void setButtonColorScheme(const ButtonColorScheme &newScheme) {
        this->currentButtonScheme = newScheme;
        // Az ősosztálynak is átadjuk az alap ColorScheme részt, hogy konzisztens maradjon
        UIComponent::setColorScheme(newScheme); // Implicit konverzió ButtonColorScheme -> ColorScheme
        markForRedraw();
    }

  protected:
    /**
     * @brief Gomb lenyomása esemény kezelése
     * @param event A touch esemény, amely tartalmazza a koordinátákat és a lenyomás állapotát
     * @details Ez a metódus kezeli a gomb lenyomását, és beállítja a hosszú lenyomás eseményeket.
     * A hosszú lenyomás esemény akkor aktiválódik, ha a gombot legalább a longPressThreshold időn keresztül lenyomva tartják.
     */
    virtual void onTouchDown(const TouchEvent &event) override {

        UIComponent::onTouchDown(event); // Alap implementáció (pressed = true, markForRedraw)

        // A UIComponent::handleTouch már ellenrőrzi a 'disabled' állapotot, így ez a vizsgálat elvileg redundáns.
        if (currentState == ButtonState::Disabled) {
            return;
        }

        longPressThresholdMet = false;
        longPressEventFired = false;
        pressStartTime = millis();
    }

    /**
     * @brief Gomb felengedése esemény kezelése
     * @param event A touch esemény, amely tartalmazza a koordinátákat és a lenyomás állapotát
     * @details Ez a metódus kezeli a gomb felengedését, és ellenőrzi, hogy hosszú lenyomás történt-e.
     */
    virtual void onTouchUp(const TouchEvent &event) override {

        // Alap osztály hívása (jelenleg csak DEBUG üzenet)
        UIComponent::onTouchUp(event);

        if (currentState == ButtonState::Disabled) {
            // Biztonsági reset, bár a disabled állapotnak már korábban meg kellett volna akadályoznia az interakciót
            pressStartTime = 0;
            longPressThresholdMet = false;
            longPressEventFired = false;
            return;
        }

        // Ellenőrizzük, hogy a felengedés a gomb tényleges (nem margóval növelt) határain belül történt-e
        bool releaseInsideActualBounds = bounds.contains(event.x, event.y);

        if (longPressThresholdMet && releaseInsideActualBounds) {
            if (eventCallback) {
                DEBUG("UIButton: Long press event fired for button %d (%s)\n", buttonId, label);
                eventCallback(ButtonEvent(buttonId, label, EventButtonState::LongPressed));
            }
            longPressEventFired = true; // Jelöljük, hogy a hosszú lenyomás eseményt kezeltük
            markForRedraw();            // Újrarajzolás szükséges lehet
        }

        // A pressStartTime és longPressThresholdMet flag-eket az onClick vagy onTouchCancel fogja véglegesen resetelni,
        // miután a UIComponent::handleTouch meghozta a döntést.
        // A longPressEventFired itt beállításra kerül, hogy az onClick tudjon róla.
    }

    /**
     * @brief Gomb kattintás esemény kezelése
     * @param event A touch esemény, amely tartalmazza a koordinátákat és a lenyomás állapotát
     * @details Ez a metódus kezeli a gomb kattintását, és végrehajtja a megfelelő eseményeket.
     * Ha a gomb toggleable, akkor az állapotot váltja, ha pushable, akkor csak eseményt küld.
     */
    virtual bool onClick(const TouchEvent &event) override {

        UIComponent::onClick(event); // Alap implementáció

        if (currentState == ButtonState::Disabled) {
            return false; // Ha a gomb le van tiltva, nem csinálunk semmit
        }

        if (longPressEventFired) {
            // Ha az onTouchUp már kezelt egy hosszú lenyomás eseményt,
            // akkor itt már nem csinálunk semmit, csak resetelünk.
            pressStartTime = 0;
            longPressThresholdMet = false; // longPressEventFired már true, következő onTouchDown reseteli
            return false;                  // Nem kezeljük újra a kattintást, mert már kezeltük a hosszú nyomást
        }

        if (buttonType == ButtonType::Toggleable) {
            currentState = (currentState == ButtonState::Off || currentState == ButtonState::CurrentActive) ? ButtonState::On : ButtonState::Off; // Módosítva
            if (eventCallback) {
                eventCallback(ButtonEvent(buttonId, label, (currentState == ButtonState::On) ? EventButtonState::On : EventButtonState::Off)); // Módosítva
            }
        } else { // Pushable
            if (eventCallback) {
                eventCallback(ButtonEvent(buttonId, label, EventButtonState::Clicked));
            }
        }

        if (clickCallback) { // Backward compatibility
            clickCallback();
        }
        markForRedraw(); // Logikai állapot változott

        // Reset a következő interakcióhoz (ha nem long press volt)
        pressStartTime = 0;
        longPressThresholdMet = false;
        // longPressEventFired már false, vagy ha true volt, fentebb kiléptünk

        return true; // Visszatérünk, hogy a kattintás kezelve lett
    }

    /**
     * @brief Gomb lenyomás megszakítása esemény kezelése
     * @param event A touch esemény, amely tartalmazza a koordinátákat és a lenyomás állapotát
     * @details Ez a metódus kezeli a gomb lenyomásának megszakítását, például ha az ujj lecsúszik a gomb határain kívülre.
     * A hosszú lenyomás esemény nem aktiválódik, és a gomb állapota visszaáll az alapértelmezett lenyomott állapotba.
     */
    virtual void onTouchCancel(const TouchEvent &event) override {

        UIComponent::onTouchCancel(event); // Alap implementáció (pressed = false, markForRedraw)

        if (currentState == ButtonState::Disabled)
            return;

        // Ha a lenyomás megszakadt (pl. ujj lecsúszott), töröljük a flag-eket.
        // Hosszú nyomás esemény nem aktiválódik.
        pressStartTime = 0;
        longPressThresholdMet = false;
        longPressEventFired = false;
        // Az UIComponent::onTouchCancel már gondoskodik a markForRedraw-ról.
    }

  public:
    /**
     * @brief Ellenőrzi, hogy a gomb engedélyezi-e a vizuális lenyomott visszajelzést
     * @return true, ha a gomb engedélyezi a vizuális lenyomott visszajelzést, false egyébként
     */
    virtual bool allowsVisualPressedFeedback() const override { return true; }

    /**
     * @brief Gomb komponens loop függvénye
     * Ez a metódus folyamatosan ellenőrzi a gomb állapotát és kezeli a hosszú lenyomás eseményeket.
     * Ha a gomb lenyomva van, és a hosszú lenyomás küszöbértéke elérve van, akkor beállítja a hosszú lenyomás eseményt.
     */
    virtual void loop() override {

        // Alap osztály loop-ja (ha van)
        UIComponent::loop();

        if (currentState == ButtonState::Disabled || !this->pressed) // `this->pressed` a UIComponent-ből jön
            return;

        if (!longPressThresholdMet && pressStartTime > 0) {
            if (millis() - pressStartTime >= LONG_PRESS_THRESHOLD) {
                longPressThresholdMet = true;
                // Opcionális: markForRedraw(); ha vizuális visszajelzést szeretnénk arról, hogy a hosszú lenyomás "élesítve" van.
                // Hosszú nyomásnak lehet saját vizuális állapota, vagy csak eseményt vált ki
                // Ha a logikai állapot változik, itt kell beállítani és markForRedraw()
                // Pl. setLogicalButtonState(LogicalButtonState::LongPressedState); (ha lenne ilyen) // Eredeti
                // Pl. setButtonState(ButtonState::LongPressedState); (ha lenne ilyen) // Módosítva
                markForRedraw(); // Akár csak az esemény miatt is lehet újrarajzolás
            }
        }
    }

    /**
     * @brief Lekérdezi a gomb érintési érzékenységét
     * @return Az érintési érzékenység margója (BUTTON_TOUCH_MARGIN pixel)
     * @details Ez a margó határozza meg, hogy mennyire érzékeny a gomb az érintésekre.
     * A gomb akkor is reagál, ha az érintés a gomb határain kívül van, de a margón belül.
     */
    virtual int16_t getTouchMargin() const override { return BUTTON_TOUCH_MARGIN; }
};

#endif // __UI_BUTTON_H