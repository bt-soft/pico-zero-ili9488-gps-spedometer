#ifndef __UI_COLOR_PALETTE_H
#define __UI_COLOR_PALETTE_H

#include <TFT_eSPI.h>

//--- TFT colors ---
#define TFT_COLOR(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
#define TFT_COLOR_BACKGROUND TFT_BLACK

// Alap színséma
struct ColorScheme {
    uint16_t screenBackground;   // Képernyő háttere
    uint16_t background;         // komponens háttere
    uint16_t foreground;         // komponens előtér színe (szöveg, ikonok, stb.)
    uint16_t border;             // komponens kerete
    uint16_t disabledBackground; // Disabled komponens háttere
    uint16_t disabledForeground; // Disabled komponens előtér színe (szöveg, ikonok, stb.)
    uint16_t disabledBorder;     // Disabled komponens kerete
    uint16_t activeBackground;   // Aktív komponens háttere (pl. gomb ON állapot)
    uint16_t activeForeground;   // Aktív komponens előtér színe (pl. gomb ON állapot szövege)
    uint16_t activeBorder;       // Aktív komponens kerete (pl. gomb ON állapot)

    static ColorScheme defaultScheme() {
        return {
            TFT_COLOR_BACKGROUND, // screenBackground
            TFT_DARKGREY,         // background (pl. dialógok háttere)
            TFT_WHITE,            // foreground
            TFT_LIGHTGREY,        // border
            TFT_BLACK,            // disabledBackground
            TFT_DARKGREY,         // disabledForeground (korábban ledColor értékével egyezett meg a defaultScheme-ben)
            TFT_DARKGREY,         // disabledBorder (korábban disabledLedColor értékével egyezett meg)
            TFT_GREEN,            // activeBackground (általános aktív, vagy gomb ON háttér)
            TFT_WHITE,            // activeForeground
            TFT_GREEN             // activeBorder (általános aktív, vagy gomb ON keret)
        };
    }
};

/**
 * Gomb színséma, amely a ColorScheme-t kiterjeszti LED színekkel
 * Ez a struktúra tartalmazza a gomb LED színeit ON és OFF állapotban,
 */
struct ButtonColorScheme : public ColorScheme {

    uint16_t pressedBackground = TFT_BLUE;  // lenyomott gomb háttér színe
    uint16_t pressedForeground = TFT_WHITE; // lenyomott gomb előtér színe (pl. szöveg)
    uint16_t pressedBorder = TFT_WHITE;     // lenyomott gomb kerete

    uint16_t ledOnColor = TFT_GREEN;     // Gomb LED színe ON állapotban (alapértelmezett: TFT_GREEN)
    uint16_t ledOffColor = TFT_DARKGREY; // Gomb LED színe OFF állapotban (alapértelmezett: TFT_DARKGREY)

    // TODO: A disabled állapot LED-jének láthatóságát még átvezetni a kódon
    uint16_t disabledLedOnColor = TFT_DARKGREEN; // Letiltott gomb LED színe ON állapotban (ha szükséges, de alapértelmezett a ledOnColor)
    uint16_t disabledLedOffColor = TFT_BLACK;    // Letiltott gomb LED színe (ha szükséges, de alapértelmezett a ledOffColor)

    // Alapértelmezett konstruktor gombokhoz
    ButtonColorScheme() : ColorScheme(ColorScheme::defaultScheme()) {}

    // Konstruktor az alap séma és LED színek megadásával
    ButtonColorScheme(const ColorScheme &baseColorScheme, uint16_t colorOn, uint16_t colorOff) : ColorScheme(baseColorScheme), ledOnColor(colorOn), ledOffColor(colorOff) {}
};

/**
 * Központi színpaletta a UI komponensekhez
 * Itt egy helyen lehet módosítani az alkalmazás színeit
 */
class UIColorPalette {
  public:
    // === DIALÓGUS SZÍNEK ===

    // Dialógus fejléc színek
    static constexpr uint16_t DIALOG_HEADER_BACKGROUND = TFT_NAVY; // Fejléc háttér
    static constexpr uint16_t DIALOG_HEADER_TEXT = TFT_WHITE;      // Fejléc szöveg

    // Dialógus bezáró gomb színek
    static constexpr uint16_t DIALOG_CLOSE_BUTTON_BACKGROUND = TFT_NAVY;  // Bezáró gomb háttér (fejléccel egyező)
    static constexpr uint16_t DIALOG_CLOSE_BUTTON_BORDER = TFT_NAVY;      // Bezáró gomb keret
    static constexpr uint16_t DIALOG_CLOSE_BUTTON_TEXT = TFT_WHITE;       // Bezáró gomb szöveg
    static constexpr uint16_t DIALOG_CLOSE_BUTTON_PRESSED = TFT_DARKGREY; // Bezáró gomb lenyomott állapot

    // Dialógus veil (fátyol) színek
    static constexpr uint16_t DIALOG_VEIL_COLOR = TFT_COLOR(190, 190, 190); // Fátyol szín

    // === GOMB SZÍNEK ===

    // Alapértelmezett gomb színek
    static constexpr uint16_t BUTTON_DEFAULT_BACKGROUND = TFT_COLOR(65, 65, 114); // Kék-szürke
    static constexpr uint16_t BUTTON_DEFAULT_TEXT = TFT_WHITE;
    static constexpr uint16_t BUTTON_DEFAULT_BORDER = TFT_WHITE;
    static constexpr uint16_t BUTTON_DEFAULT_PRESSED = TFT_BLUE;
    static constexpr uint16_t BUTTON_DEFAULT_PRESSED_BORDER = TFT_WHITE;

    // Speciális gomb színek
    static constexpr uint16_t BUTTON_OK_BACKGROUND = TFT_DARKGREEN;
    static constexpr uint16_t BUTTON_OK_TEXT = TFT_WHITE;
    static constexpr uint16_t BUTTON_OK_BORDER = TFT_DARKGREEN;
    static constexpr uint16_t BUTTON_CANCEL_BACKGROUND = TFT_MAROON;
    static constexpr uint16_t BUTTON_CANCEL_TEXT = TFT_WHITE;
    static constexpr uint16_t BUTTON_CANCEL_BORDER = TFT_MAROON;                  // Letiltott gomb színek - jól látható disabled állapot
    static constexpr uint16_t BUTTON_DISABLED_BACKGROUND = TFT_COLOR(64, 64, 64); // Eredeti sötétebb szürke háttér
    static constexpr uint16_t BUTTON_DISABLED_TEXT = TFT_COLOR(128, 128, 128);    // Közepes szürke szöveg
    static constexpr uint16_t BUTTON_DISABLED_BORDER = TFT_COLOR(96, 96, 96);     // Közepes szürke keret

    // === KÉPERNYŐ SZÍNEK ===

    // Alapértelmezett képernyő színek
    static constexpr uint16_t SCREEN_BACKGROUND = TFT_BLACK;
    static constexpr uint16_t SCREEN_TEXT = TFT_WHITE;
    static constexpr uint16_t SCREEN_BORDER = TFT_WHITE;

    // === Batterry szimbólum színek ===
    static constexpr uint16_t TFT_COLOR_DRAINED_BATTERY = TFT_COLOR(248, 252, 0);
    static constexpr uint16_t TFT_COLOR_SUBMERSIBLE_BATTERY = TFT_ORANGE; // === FREKVENCIA KIJELZŐ SZÍNEK ===

    // Normál (SSB/CW) mód színei
    static constexpr uint16_t FREQ_NORMAL_ACTIVE = TFT_GOLD;                // Aktív számjegyek színe
    static constexpr uint16_t FREQ_NORMAL_INACTIVE = TFT_COLOR(50, 50, 50); // Inaktív (háttér) számjegyek színe
    static constexpr uint16_t FREQ_NORMAL_INDICATOR = TFT_YELLOW;           // Indikátor elemek színe (egységek, aláhúzás)

    // BFO mód színei
    static constexpr uint16_t FREQ_BFO_ACTIVE = TFT_ORANGE;    // Aktív számjegyek színe BFO módban
    static constexpr uint16_t FREQ_BFO_INACTIVE = TFT_BROWN;   // Inaktív számjegyek színe BFO módban
    static constexpr uint16_t FREQ_BFO_INDICATOR = TFT_ORANGE; // Indikátor elemek színe BFO módban    // BFO címke háttere
    static constexpr uint16_t FREQ_BFO_LABEL_TEXT = TFT_BLACK; // "BFO" címke szövegszíne

    // ScreenSaver mód színei (kék színséma)
    static constexpr uint16_t FREQ_SCREENSAVER_ACTIVE = TFT_CYAN;                    // Aktív számjegyek színe képernyővédő módban
    static constexpr uint16_t FREQ_SCREENSAVER_INACTIVE = TFT_COLOR(0, 0, 80);       // Inaktív számjegyek színe (sötét kék)
    static constexpr uint16_t FREQ_SCREENSAVER_INDICATOR = TFT_COLOR(173, 216, 230); // Indikátor elemek színe (világos kék)    // === SEGÉD METÓDUSOK ===

    /**
     * Alapértelmezett gomb ColorScheme létrehozása
     */
    static ButtonColorScheme createDefaultButtonScheme() { // Visszatérési érték ButtonColorScheme
        ButtonColorScheme scheme;                          // Alapértelmezett ButtonColorScheme konstruktor
        scheme.background = BUTTON_DEFAULT_BACKGROUND;
        scheme.foreground = BUTTON_DEFAULT_TEXT;
        scheme.border = BUTTON_DEFAULT_BORDER;
        scheme.pressedBackground = BUTTON_DEFAULT_PRESSED;
        scheme.pressedForeground = BUTTON_DEFAULT_TEXT; // Lenyomáskor a szövegszín maradhat
        scheme.pressedBorder = BUTTON_DEFAULT_PRESSED_BORDER;
        scheme.disabledBackground = BUTTON_DISABLED_BACKGROUND;
        scheme.disabledForeground = BUTTON_DISABLED_TEXT;
        scheme.disabledBorder = BUTTON_DISABLED_BORDER;

        // Az "active" állapot (pl. toggle gomb ON) színei:
        // A UIButton logikája szerint az ON állapot háttere azonos a normál háttérrel, a keret pedig a ledOnColor.
        scheme.activeBackground = BUTTON_DEFAULT_BACKGROUND; // Vagy egyedi ON háttér
        scheme.activeForeground = BUTTON_DEFAULT_TEXT;       // Vagy egyedi ON szövegszín
        scheme.activeBorder = TFT_GREEN;                     // Ezt a UIButton felülírhatja a ledOnColor-ral
        scheme.ledOnColor = TFT_GREEN;
        scheme.ledOffColor = TFT_DARKGREY; // Jobban látható, mint a TFT_DARKGREEN fekete alapon

        return scheme;
    }

    /**
     * Default choice gomb ColorScheme létrehozása (MultiButtonDialog-ban használatos)
     */
    static ColorScheme createDefaultChoiceButtonScheme() {
        ColorScheme colors = createDefaultButtonScheme();
        // Mivel ez a függvény ColorScheme-et ad vissza, a ButtonColorScheme részeket nem tudja beállítani.
        // Ha ez is ButtonColorScheme-et adna vissza:
        // ButtonColorScheme scheme = createDefaultButtonScheme();
        colors.background = TFT_DARKGREEN;
        colors.foreground = TFT_NAVY;
        colors.border = TFT_DARKGREEN;
        //---Letiltott default állapot        colors.disabledBackground = TFT_DARKGREEN;
        colors.disabledForeground = TFT_BROWN;
        colors.disabledBorder = TFT_GREENYELLOW;
        return colors;
    }
};

#endif // __UI_COLOR_PALETTE_H