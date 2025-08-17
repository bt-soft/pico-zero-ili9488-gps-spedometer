#include "Button.h"

/**
 *  A gomb konstruktora
 */
Button::Button(TFT_eSPI &tft, int16_t x, int16_t y, int16_t w, int16_t h, const char *label, uint16_t bgColor, uint16_t textColor, uint16_t borderColor, uint8_t font)
    : _tft(tft), _x(x), _y(y), _w(w), _h(h), _label(label), _bgColor(bgColor), _textColor(textColor), _borderColor(borderColor), _font(font) {}

/**
 *  A gomb kirajzolása a képernyőre
 */
void Button::draw() {
    // Draw the button with a border
    _tft.fillRoundRect(_x, _y, _w, _h, 10, _bgColor);
    _tft.drawRoundRect(_x, _y, _w, _h, 10, _borderColor);

    // Draw the label centered in the button, ensuring transparent background
    _tft.setTextColor(_textColor);
    _tft.setTextDatum(MC_DATUM);

    // Select the font, draw the string, then restore the original font
    _tft.setTextFont(_font);
    _tft.drawString(_label, _x + _w / 2, _y + _h / 2);
    _tft.setTextFont(1); // Restore a default font
}

/**
 *  Ellenőrzi, hogy a megadott koordináták a gomb területén belül vannak-e
 */
bool Button::contains(int16_t x, int16_t y) { return (x >= _x && x <= (_x + _w) && y >= _y && y <= (_y + _h)); }

/**
 *  Beállítja a gomb visszahívási függvényét
 */
void Button::setCallback(std::function<void()> callback) { _callback = callback; }

/**
 *  A gomb megnyomása
 */
void Button::press() {
    // Execute the callback function if it's set
    if (_callback) {
        _callback();
    }
}
