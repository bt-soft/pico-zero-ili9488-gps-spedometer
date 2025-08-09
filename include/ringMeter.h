#ifndef __RINGMETER_H__
#define __RINGMETER_H__

#include "commons.h"

/**
 * 16 bites szivárvány szín visszaadása.
 *
 * A value értéke 0-127 között várható, 0 = kék, 127 = piros.
 *
 * @param value Szín spektrum index (0-127)
 * @return 16 bites RGB565 színkód
 */
unsigned int rainbow(byte value) {
    // Az érték 0-127 között várható
    // Az érték spektrum színné konvertálódik: 0 = kék, 127 = piros
    byte red = 0;   // A piros a felső 5 bit
    byte green = 0; // A zöld a középső 6 bit
    byte blue = 0;  // A kék az alsó 5 bit
    byte quadrant = value / 32;
    if (quadrant == 0) {
        blue = 31;
        green = 2 * (value % 32);
        red = 0;
    }
    if (quadrant == 1) {
        blue = 31 - (value % 32);
        green = 63;
        red = 0;
    }
    if (quadrant == 2) {
        blue = 0;
        green = 63;
        red = value % 32;
    }
    if (quadrant == 3) {
        blue = 0;
        green = 63 - 2 * (value % 32);
        red = 31;
    }
    return (red << 11) + (green << 5) + blue;
}

/**
 * Szinusz hullám érték visszaadása -1 és +1 között, adott fázisszögre (fokban).
 *
 * @param phase Fázisszög (fok)
 * @return Számított szinusz érték
 */
float sineWave(int phase) { return sin(phase * 0.0174532925); }

// A kirajzolás gyorsításához 64 pixeles puffer használata
#define BUFF_SIZE 64

/**
 * Körmérő (ring meter) kirajzolása a kijelzőre.
 *
 * @param tft TFT_eSPI kijelző objektum
 * @param value Aktuális érték
 * @param vmin Minimum érték
 * @param vmax Maximum érték
 * @param x Bal felső sarok X koordináta
 * @param y Bal felső sarok Y koordináta
 * @param r Gyűrű sugara
 * @param angle Teljes szög (fokban)
 * @param coloredValue Érték színezése (true: színes, false: fehér)
 * @param units Mértékegység szöveg
 * @param scheme Színséma
 * @return A jobb oldali x koordináta
 */
int ringMeter(TFT_eSPI *tft, int value, int vmin, int vmax, int x, int y, int r, int angle, bool coloredValue, const char *units, byte scheme) {
    // Minimum value of r is about 52 before value text intrudes on ring
    // drawing the text first is an option

    x += r;
    y += r; // Calculate coords of centre of ring

    int w = r / 3;                                         // Width of outer ring is 1/4 of radius
    int halfAngle = angle / 2;                             // Half the sweep angle of meter (300 degrees)
    int v = map(value, vmin, vmax, -halfAngle, halfAngle); // Map the value to an angle v

    byte seg = 3; // Segments are 3 degrees wide = 100 segments for 300 degrees
    byte inc = 6; // Draw segments every 3 degrees, increase to 6 for segmented ring

    // Variable to save "value" text colour from scheme and set default
    int colour = RINGMETER_LIGHTBLUE;

    // Draw colour blocks every inc degrees
    for (int i = -halfAngle + inc / 2; i < halfAngle - inc / 2; i += inc) {

        // Kicsi méret esetén ritkítunk
        if (r < 50) {
            if (i % 4) {
                continue;
            }
        }

        // Calculate pair of coordinates for segment start
        float sx = cos((i - 90) * 0.0174532925);
        float sy = sin((i - 90) * 0.0174532925);
        uint16_t x0 = sx * (r - w) + x;
        uint16_t y0 = sy * (r - w) + y;
        uint16_t x1 = sx * r + x;
        uint16_t y1 = sy * r + y;

        // Calculate pair of coordinates for segment end
        float sx2 = cos((i + seg - 90) * 0.0174532925);
        float sy2 = sin((i + seg - 90) * 0.0174532925);
        int x2 = sx2 * (r - w) + x;
        int y2 = sy2 * (r - w) + y;
        int x3 = sx2 * r + x;
        int y3 = sy2 * r + y;

        if (i < v) { // Fill in coloured segments with 2 triangles
            switch (scheme) {

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
                    colour = rainbow(map(i, -halfAngle, halfAngle, 0, 127));
                    break; // Full spectrum blue to red

                case GREEN2RED:
                    colour = rainbow(map(i, -halfAngle, halfAngle, 70, 127));
                    break; // Green to red (high temperature etc.)

                case RED2GREEN:
                    colour = rainbow(map(i, -halfAngle, halfAngle, 127, 63));
                    break; // Red to green (low battery etc.)

                default:
                    colour = RINGMETER_LIGHTBLUE;
                    break; // Fixed colour
            }
            tft->fillTriangle(x0, y0, x1, y1, x2, y2, colour);
            tft->fillTriangle(x1, y1, x2, y2, x3, y3, colour);

        } else { // Fill in blank segments

            tft->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_DARKGREY);
            tft->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_DARKGREY);
        }
    }

    // Convert value to a string
    char buf[10];
    byte len = 3;
    if (value > 999) {
        len = 5;
    }
    dtostrf(value, len, 0, buf);
    buf[len] = ' ';
    buf[len + 1] = 0; // Add blanking space and terminator, helps to centre text too!

    // Érték kiírása
    tft->setTextSize(1);

    // Érték text színe
    tft->setTextColor(coloredValue ? colour : TFT_WHITE, TFT_BLACK); // színesedik a ring-el együtt vagy fehér
    tft->setTextDatum(MC_DATUM);                                     // középre

    // Print value, if the meter is large then use big font 8, othewise use 4
    if (r > 84) {
        tft->setTextPadding(3 * 58);   // Allow for 3 digits each 55 pixels wide + 3 pixel az 1-es piszkának törléséhez
        tft->drawString(buf, x, y, 8); // Value in middle
    } else {
        tft->setTextPadding(3 * 14);   // Allow for 3 digits each 14 pixels wide
        tft->drawString(buf, x, y, 4); // Value in middle
    }

    // Print units, if the meter is large then use big font 4, othewise use 2
    tft->setTextPadding(0);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    if (r > 84) {
        tft->drawString(units, x, y + 60, 4); // Units display
    } else {
        tft->drawString(units, x, y + 15, 2); // Units display
    }

    // Calculate and return right hand side x coordinate
    return x + r;
}

#endif