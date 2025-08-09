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

    // A r minimális értéke kb. 52, különben az érték szöveg belelóg a gyűrűbe

    x += r;
    y += r; // A gyűrű középpontjának koordinátái

    int w = r / 3;                                         // A külső gyűrű szélessége a sugár 1/4-e
    int halfAngle = angle / 2;                             // A mérő fél szöge (pl. 300 foknál 150)
    int v = map(value, vmin, vmax, -halfAngle, halfAngle); // Az érték leképezése szögre

    byte seg = 3; // Egy szegmens 3 fok széles = 100 szegmens 300 foknál
    byte inc = 6; // 6 fokonként rajzolunk szegmenst, növelve szegmentáltabb lesz

    // Változó az érték szöveg színének tárolására, alapértelmezett világoskék
    int colour = RINGMETER_LIGHTBLUE;

    // Színes blokkok rajzolása minden inc foknál
    for (int i = -halfAngle + inc / 2; i < halfAngle - inc / 2; i += inc) {

        // Kis méret esetén ritkítunk
        if (r < 50) {
            if (i % 4) {
                continue;
            }
        }

        // A szegmens kezdőpontjainak koordinátái
        float sx = cos((i - 90) * 0.0174532925);
        float sy = sin((i - 90) * 0.0174532925);
        uint16_t x0 = sx * (r - w) + x;
        uint16_t y0 = sy * (r - w) + y;
        uint16_t x1 = sx * r + x;
        uint16_t y1 = sy * r + y;

        // A szegmens végpontjainak koordinátái
        float sx2 = cos((i + seg - 90) * 0.0174532925);
        float sy2 = sin((i + seg - 90) * 0.0174532925);
        int x2 = sx2 * (r - w) + x;
        int y2 = sy2 * (r - w) + y;
        int x3 = sx2 * r + x;
        int y3 = sy2 * r + y;

        if (i < v) { // Kitöltött színes szegmensek (2 háromszöggel)
            switch (scheme) {
                case RED2RED:
                    colour = TFT_RED;
                    break; // Fix szín
                case GREEN2GREEN:
                    colour = TFT_GREEN;
                    break; // Fix szín
                case BLUE2BLUE:
                    colour = TFT_BLUE;
                    break; // Fix szín
                case BLUE2RED:
                    colour = rainbow(map(i, -halfAngle, halfAngle, 0, 127));
                    break; // Teljes spektrum kékből pirosba
                case GREEN2RED:
                    colour = rainbow(map(i, -halfAngle, halfAngle, 70, 127));
                    break; // Zöldből pirosba (pl. hőmérséklet)
                case RED2GREEN:
                    colour = rainbow(map(i, -halfAngle, halfAngle, 127, 63));
                    break; // Pirosból zöldbe (pl. akku)
                default:
                    colour = RINGMETER_LIGHTBLUE;
                    break; // Fix szín
            }
            tft->fillTriangle(x0, y0, x1, y1, x2, y2, colour);
            tft->fillTriangle(x1, y1, x2, y2, x3, y3, colour);

        } else { // Üres (szürke) szegmensek
            tft->fillTriangle(x0, y0, x1, y1, x2, y2, TFT_DARKGREY);
            tft->fillTriangle(x1, y1, x2, y2, x3, y3, TFT_DARKGREY);
        }
    }

    // Nagy kör?
    bool bigRing = r > 84;

    // Mértékegység kiírása, nagy gyűrűnél nagyobb betűtípus
    int unitY = bigRing ? y - 60 : y - 18; // Mértékegység Y koordináta 
    int unitFontSize = bigRing ? 4 : 2;    // Mértékegység betűméret
    tft->setTextSize(1);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextPadding(0);
    tft->setTextDatum(MC_DATUM);                          // Középre igazítás
    tft->drawCentreString(units, x, unitY, unitFontSize); // Mértékegység

    // Érték karakterlánccá alakítása
    char buf[10];
    byte len = 3;
    if (value > 999) {
        len = 5;
    }
    dtostrf(value, 0, 0, buf); // Nincs padding, csak a szükséges karakterek
    // Érték kiírása vízszintesen középre igazítva
    int valueY = bigRing ? y - 20 : y; // Mértékegység Y koordináta 
    int valueFontSize = bigRing ? 8 : 4;
    tft->setTextSize(1);
    tft->setTextColor(coloredValue ? colour : TFT_WHITE, TFT_BLACK); // Színesedik a gyűrűvel vagy fehér
    tft->setTextDatum(MC_DATUM);                                     // Vízszintes és függőleges középre igazítás
    tft->setTextPadding(0);                                          // Nincs padding, így a szöveg pontosan középre kerül
    tft->drawCentreString(buf, x, valueY, valueFontSize);                 // Érték pontosan középen a gyűrű közepén

    // Visszaadja a jobb oldali x koordinátát
    return x + r;
}

#endif