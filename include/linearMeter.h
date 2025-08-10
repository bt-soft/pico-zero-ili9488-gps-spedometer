#ifndef __LINEARMETER_H__
#define __LINEARMETER_H__

#include "commons.h"

/**
 * Szivárvány szín visszaadása 16 bites RGB565 formátumban.
 *
 * Ha a 'spectrum' 0-159 közötti, akkor a szín a pirosból indul, kéken át ibolyáig tart.
 * 0 = piros, 127 = kék, 159 = ibolya, 191-ig tovább megy vissza pirosba.
 *
 * @param spectrum Szín spektrum index (0-191)
 * @return 16 bites RGB565 színkód
 */
uint16_t rainbowColor(uint8_t spectrum) {
    spectrum = spectrum % 192;
    uint8_t red = 0;   // A piros a felső 5 bit
    uint8_t green = 0; // A zöld a középső 6 bit, de csak a felső 5 bitet használjuk
    uint8_t blue = 0;  // A kék az alsó 5 bit
    uint8_t sector = spectrum >> 5;
    uint8_t amplit = spectrum & 0x1F;
    switch (sector) {
        case 0:
            red = 0x1F;
            green = amplit; // Zöld felfut
            blue = 0;
            break;
        case 1:
            red = 0x1F - amplit; // Piros lecseng
            green = 0x1F;
            blue = 0;
            break;
        case 2:
            red = 0;
            green = 0x1F;
            blue = amplit; // Kék felfut
            break;
        case 3:
            red = 0;
            green = 0x1F - amplit; // Zöld lecseng
            blue = 0x1F;
            break;
        case 4:
            red = amplit; // Piros felfut
            green = 0;
            blue = 0x1F;
            break;
        case 5:
            red = 0x1F;
            green = 0;
            blue = 0x1F - amplit; // Kék lecseng
            break;
    }
    return red << 11 | green << 6 | blue;
}

/**
 * Vízszintes sávos mérő kirajzolása a kijelzőre.
 * @param tft TFT_eSPI kijelző objektum
 * @param val Megjelenítendő érték (0-tól n-ig)
 * @param x Bal felső sarok X koordináta
 * @param y Bal felső sarok Y koordináta
 * @param w Egy sáv szélessége
 * @param h Egy sáv magassága
 * @param g Sávok közötti távolság
 * @param n Sávok száma
 * @param s Színséma
 */
void linearBar(TFT_eSPI *tft, int val, int x, int y, int w, int h, int g, int n, byte s) {
    // Változó a sáv színének tárolására, alapértelmezett: kék
    int colour = TFT_BLUE;
    // n darab színes blokk kirajzolása
    for (int b = 1; b <= n; b++) {
        if (val > 0 && b <= val) { // Kitöltött színes blokkok
            switch (s) {
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
                    colour = rainbowColor(map(b, 0, n, 127, 0));
                    break; // Kékből pirosba
                case GREEN2RED:
                    colour = rainbowColor(map(b, 0, n, 63, 0));
                    break; // Zöldből pirosba
                case RED2GREEN:
                    colour = rainbowColor(map(b, 0, n, 0, 63));
                    break; // Pirosból zöldbe
                case RED2VIOLET:
                    colour = rainbowColor(map(b, 0, n, 0, 159));
                    break; // Szivárvány (pirosból ibolyába)
            }
            tft->fillRect(x + b * (w + g), y, w, h, colour);
        } else { // Üres (szürke) blokkok
            tft->fillRect(x + b * (w + g), y, w, h, TFT_DARKGREY);
        }
    }
}

/**
 * Függőleges sávos mérő kirajzolása a kijelzőre.
 * @param tft TFT_eSPI kijelző objektum
 * @param category A mérő címe (pl. "Temp")
 * @param val Az aktuális érték
 * @param minVal Minimum érték
 * @param maxVal Maximum érték
 * @param x Bal alsó sarok X koordináta
 * @param y Bal alsó sarok Y koordináta
 * @param w Egy sáv szélessége
 * @param h Egy sáv magassága
 * @param g Sávok közötti távolság
 * @param n Sávok száma
 * @param s Színséma
 * @param mirrored Szövegek pozíciója (false=jobbra, true=balra)
 */
void verticalLinearMeter(TFT_eSprite *sprite, int meterHeight, int meterWidth, const char *category, float val, float minVal, float maxVal, int x, int y, int w, int h, int g, int n, byte s, boolean mirrored = false) {
    char buf[20];

    sprite->fillSprite(TFT_BLACK);

    // 1. Cím kiírása a sprite tetejére
    int titleY = 0;
    sprite->setTextSize(1);
    sprite->setTextColor(TFT_YELLOW, TFT_BLACK);
    sprite->setTextDatum(mirrored ? TR_DATUM : TL_DATUM);
    sprite->drawString(category, mirrored ? meterWidth : 0, titleY, 2);
    sprite->setTextDatum(TL_DATUM);

    // 2. Aktuális érték sávszámra váltása (1-től n-ig)
    int barVal = map(val, minVal, maxVal, 1, n);
    if (barVal < 1) {
        barVal = 1;
    }
    if (barVal > n) {
        barVal = n;
    }

    // 3. Sávok és feliratok rajzolása
    for (int b = 1; b <= n; b++) {
        // Sáv Y koordinátája
        int barY = 20 + (n - b) * (h + g);
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

        // Bar téglalap rajzolása
        sprite->fillRect(mirrored ? meterWidth - w : 0, barY, w, h, barColor);

        // Korábbi bar szöveg értékek törlése
        int bgX = mirrored ? meterWidth - w - 50 : w + 5;
        sprite->fillRect(bgX, barY - 2, 45, h + 4, TFT_BLACK);

        // Bar szöveg meghatározása
        buf[0] = '\0';
        if (b == 1) {
            dtostrf(minVal, 0, 1, buf);
        } else if (b == n) {
            dtostrf(maxVal, 0, 1, buf);
        } else if (b == barVal) {
            dtostrf(val, 0, 1, buf);
        }

        // Bar szöveg kiírása
        if (buf[0] != '\0') {
            sprite->setTextSize(1);
            sprite->setTextColor(TFT_WHITE, TFT_BLACK);
            sprite->setTextDatum(mirrored ? TR_DATUM : TL_DATUM);

            // Bar szöveg pozíciója
            int textX = mirrored ? meterWidth - w - 8 : w + 8;
            int textY = barY + h / 2 - 4;
            sprite->drawString(buf, textX, textY, 1);
        }
    }

    // 4. Aktuális érték kiírása a bar aljára
    dtostrf(val, 0, 2, buf);
    sprite->setTextSize(2);
    sprite->setTextDatum(BL_DATUM);
    sprite->setTextColor(TFT_ORANGE, TFT_BLACK);
    sprite->drawString(buf, mirrored ? 10 : 0, meterHeight, 1);

    // Sprite kirajzolása a kijelzőre
    int drawX = x;
    int drawY = y - meterHeight;

    sprite->pushSprite(drawX, drawY);
}

#endif