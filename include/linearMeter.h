#ifndef __LINEARMETER_H__
#define __LINEARMETER_H__

#include "commons.h"

/******************************************************    // 1. Cím kiírása a mérő tetejére - pozíció javítva hogy ne lógjon ki
    tft->setTextSize(1);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    int titleY = y - (n * (h + g)) - 20;
    int titleX = mirrored ? x - w : x;  // mirrored esetén csak a sáv szélességével balra
    if (titleX < 0) titleX = 0;  // ne menjen negatívba
    tft->drawString(category, titleX, titleY, 2);***************************
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
 * Függőleges sávos mérő rajzolása
 * @param tft - TFT kijelző objektum
 * @param category - mérő címe (pl. "Temp")
 * @param val - aktuális érték
 * @param minVal - minimum érték
 * @param maxVal - maximum érték
 * @param x - bal alsó sarok X koordináta
 * @param y - bal alsó sarok Y koordináta
 * @param w - egy sáv szélessége
 * @param h - egy sáv magassága
 * @param g - sávok közötti távolság
 * @param n - sávok száma
 * @param s - színséma
 * @param mirrored - szövegek pozíciója (false=jobbra, true=balra)
 */
void verticalLinearMeter(TFT_eSPI *tft, const char *category, float val, float minVal, float maxVal, int x, int y, int w, int h, int g, int n, byte s, boolean mirrored = false) {

    char buf[20];

    // 1. Cím kiírása a mérő tetejére
    int titleY = y - (n * (h + g)) - 20;
    tft->setTextSize(1);
    tft->setTextColor(TFT_YELLOW, TFT_BLACK);
    tft->setTextDatum(mirrored ? TC_DATUM : TL_DATUM);
    tft->drawString(category, x, titleY, 2);
    tft->setTextDatum(TL_DATUM);

    // 2. Aktuális érték sávszámra váltása (1-től n-ig)
    int barVal = map(val, minVal, maxVal, 1, n);
    if (barVal < 1)
        barVal = 1;
    if (barVal > n)
        barVal = n;

    // 3. Sávok és feliratok rajzolása
    for (int b = 1; b <= n; b++) {

        // Sáv Y koordinátája
        int barY = y - (b * (h + g));

        // Sáv színének meghatározása
        uint16_t barColor = TFT_DARKGREY;
        if (b <= barVal) {
            switch (s) {
                case RED2RED:
                    barColor = TFT_RED;
                    break;
                case GREEN2GREEN:
                    barColor = TFT_GREEN;
                    break;
                case BLUE2BLUE:
                    barColor = TFT_BLUE;
                    break;
                case BLUE2RED:
                    barColor = rainbowColor(map(b, 1, n, 127, 0));
                    break;
                case GREEN2RED:
                    barColor = rainbowColor(map(b, 1, n, 63, 0));
                    break;
                case RED2GREEN:
                    barColor = rainbowColor(map(b, 1, n, 0, 63));
                    break;
                case RED2VIOLET:
                    barColor = rainbowColor(map(b, 1, n, 0, 159));
                    break;
                default:
                    barColor = TFT_BLUE;
                    break;
            }
        }

        // Sáv rajzolása
        tft->fillRect(x, barY, w, h, barColor);

        // Szöveg pozíciója
        int textX = mirrored ? x - 45 : x + w + 8;
        int textY = barY + h / 2 - 4;

        // Háttér törlése
        int bgX = mirrored ? x - 50 : x + w + 5;
        tft->fillRect(bgX, barY - 2, 45, h + 4, TFT_BLACK);

        // Szöveg meghatározása
        buf[0] = '\0';

        if (b == 1) {
            dtostrf(minVal, 0, 1, buf);
        } else if (b == n) {
            dtostrf(maxVal, 0, 1, buf);
        } else if (b == barVal) {
            dtostrf(val, 0, 1, buf);
        }

        // Szöveg kiírása
        if (buf[0] != '\0') {
            tft->setTextColor(TFT_WHITE, TFT_BLACK);
            tft->setTextSize(1);
            tft->drawString(buf, textX, textY, 1);
        }
    }

    // 4. Nagy aktuális érték kiírása a mérő aljára
    dtostrf(val, 0, 2, buf);
    tft->setTextSize(2);
    tft->setTextDatum(mirrored ? MC_DATUM : TL_DATUM);
    tft->setTextColor(TFT_ORANGE, TFT_BLACK);
    tft->drawString(buf, x, y + 10, 1);
}

#endif