#ifndef __LINEARMETER_H__
#define __LINEARMETER_H__

#include "commons.h"

/***************************************************************************************
** Function name:           rainbowColor
** Description:             Return a 16 bit rainbow colour
***************************************************************************************/
// If 'spectrum' is in the range 0-159 it is converted to a spectrum colour
// from 0 = red through to 127 = blue to 159 = violet
// Extending the range to 0-191 adds a further violet to red band

uint16_t rainbowColor(uint8_t spectrum) {
    spectrum = spectrum % 192;

    uint8_t red = 0;   // Red is the top 5 bits of a 16 bit colour spectrum
    uint8_t green = 0; // Green is the middle 6 bits, but only top 5 bits used here
    uint8_t blue = 0;  // Blue is the bottom 5 bits

    uint8_t sector = spectrum >> 5;
    uint8_t amplit = spectrum & 0x1F;

    switch (sector) {
    case 0:
        red = 0x1F;
        green = amplit; // Green ramps up
        blue = 0;
        break;
    case 1:
        red = 0x1F - amplit; // Red ramps down
        green = 0x1F;
        blue = 0;
        break;
    case 2:
        red = 0;
        green = 0x1F;
        blue = amplit; // Blue ramps up
        break;
    case 3:
        red = 0;
        green = 0x1F - amplit; // Green ramps down
        blue = 0x1F;
        break;
    case 4:
        red = amplit; // Red ramps up
        green = 0;
        blue = 0x1F;
        break;
    case 5:
        red = 0x1F;
        green = 0;
        blue = 0x1F - amplit; // Blue ramps down
        break;
    }

    return red << 11 | green << 6 | blue;
}
// #########################################################################
//  Draw the linear meter
// #########################################################################
// val =  reading to show (range is 0 to n)
// x, y = position of top left corner
// w, h = width and height of a single bar
// g    = pixel gap to next bar (can be 0)
// n    = number of segments
// s    = colour scheme
void linearBar(TFT_eSPI *tft, int val, int x, int y, int w, int h, int g, int n, byte s) {

    // Variable to save "value" text colour from scheme and set default
    int colour = TFT_BLUE;

    // Draw n colour blocks
    for (int b = 1; b <= n; b++) {
        if (val > 0 && b <= val) { // Fill in coloured blocks
            switch (s) {

            case RED2RED:
                colour = TFT_RED;
                break; // Fixed colour

            case GREEN2GREEN:
                colour = TFT_GREEN;
                break; // Fixed colour

            case BLUE2BLUE:
                colour = TFT_BLUE;
                break; // Fixed colour

            case BLUE2RED:
                colour = rainbowColor(map(b, 0, n, 127, 0));
                break; // Blue to red

            case GREEN2RED:
                colour = rainbowColor(map(b, 0, n, 63, 0));
                break; // Green to red

            case RED2GREEN:
                colour = rainbowColor(map(b, 0, n, 0, 63));
                break; // Red to green

            case RED2VIOLET:
                colour = rainbowColor(map(b, 0, n, 0, 159));
                break; // Rainbow (red to violet)
            }

            tft->fillRect(x + b * (w + g), y, w, h, colour);

        } else { // Fill in blank segments

            tft->fillRect(x + b * (w + g), y, w, h, TFT_DARKGREY);
        }
    }
}

/**
 *
 */
void verticalLinearMeter(TFT_eSPI *tft, const char *category, float val, float minVal, float maxVal, int x, int y, int w, int h, int g, int n, byte s, boolean mirrored = false) {

    // Header szöveg kiírása
    tft->setTextSize(1);
    int16_t textWidthHeader = tft->textWidth(category, 2);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->drawString(category, x + (mirrored ? 0 : textWidthHeader / 2), y - (n * (h + g)) - h + 1, 2);

    char buf[10]; // Karaktertömb a szöveg tárolásához

    // Vertical bar-ok kirajzolása
    tft->setTextSize(1);
    int barVal = map(val, minVal, maxVal, 1, n);
    int colour = TFT_DARKGREY;

    for (int b = 1; b <= n; b++) {

        colour = TFT_DARKGREY;

        if (b <= barVal) { // Fill in coloured blocks
            switch (s) {

            case RED2RED:
                colour = TFT_RED;
                break; // Fixed colour

            case GREEN2GREEN:
                colour = TFT_GREEN;
                break; // Fixed colour

            case BLUE2BLUE:
                colour = TFT_BLUE;
                break; // Fixed colour

            case BLUE2RED:
                colour = rainbowColor(map(b, 0, n, 127, 0));
                break; // Blue to red

            case GREEN2RED:
                colour = rainbowColor(map(b, 0, n, 63, 0));
                break; // Green to red

            case RED2GREEN:
                colour = rainbowColor(map(b, 0, n, 0, 63));
                break; // Red to green

            case RED2VIOLET:
                colour = rainbowColor(map(b, 0, n, 0, 159));
                break; // Rainbow (red to violet)
            }
        }

        // Bar kirajzolása
        tft->fillRect(x, y - b * (h + g), w, h, colour);

        // Korábbi bar melletti felirat/érték törlése
        int tickX = x + (mirrored ? -3 * 6 : w + 15);

        tft->fillRect(tickX + (mirrored ? -15 : -10), // x
                      y - b * (h + g),                // y
                      5 * 6,                          // w
                      h,                              // h
                      TFT_BLACK);                     // color

        // Koordináta érték megjelenítése
        // A min/max szélsőérték megjelenítése
        if (b == 1 || b == n) {
            sprintf(buf, "%.1f", b == 1 ? minVal : maxVal); // Szám értékének formázása és karaktertömbbe írása
        } else if (b <= barVal && b + 1 > barVal) {
            sprintf(buf, "%.1f", val); // Aktuális érték formázása és karaktertömbbe írása
        } else {
            memset(buf, '\0', sizeof(buf));
        }

        int16_t textWidth = tft->textWidth(buf, 1); // Szöveg szélességének kiszámítása
        tft->setTextPadding(textWidth);             // Szélesség beállítása a paddinghez
        tft->setTextColor(TFT_WHITE, TFT_BLACK);    // Fehér szöveg fekete háttérrel

        // koordináta szöveg kiírása
        // tft->drawString(buffer, x + (w / 2) - (textWidth / 2), y - (b * (h + g) / 2), 1); // Szöveg rajzolása bele a bar közepébe
        tft->drawString(buf, tickX, y - (b * (h + g)) + h / 2, 1);
    }

    // Érték kiírása a bar-ok alá
    sprintf(buf, "%.2f", val);            // Aktuális érték formázása és karaktertömbbe írása
    tft->setTextPadding(4 * FONT2_WIDTH); // Szélesség beállítása a paddinghez
    tft->setTextSize(2);
    tft->drawString(buf, x + (mirrored ? 0 : 30), y + 10, 1);
}

#endif