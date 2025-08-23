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
    // Draw the button with a border using more rounded corners
    _tft.fillRoundRect(_x, _y, _w, _h, 15, _bgColor);
    _tft.drawRoundRect(_x, _y, _w, _h, 15, _borderColor);

    // Draw the label centered in the button, ensuring transparent background
    _tft.setTextColor(_textColor);
    _tft.setTextDatum(MC_DATUM);

    // Select the font, draw the string, then restore the original font
    _tft.setTextFont(_font);
    _tft.drawString(_label, _x + _w / 2, _y + _h / 2);
    _tft.setTextFont(1); // Restore a default font
}

/**
 *  A gomb kirajzolása a képernyőre szöveggel az alján (grid buttonokhoz)
 */
void Button::drawWithTextAtBottom() {
    // Draw the button with a border using more rounded corners
    _tft.fillRoundRect(_x, _y, _w, _h, 15, _bgColor);
    _tft.drawRoundRect(_x, _y, _w, _h, 15, _borderColor);

    // Draw the label at the bottom of the button
    _tft.setTextColor(_textColor);
    _tft.setTextDatum(MC_DATUM);

    // Select the font, draw the string at bottom, then restore the original font
    _tft.setTextFont(_font);
    _tft.drawString(_label, _x + _w / 2, _y + _h - 15); // 15px from bottom
    _tft.setTextFont(1);                                // Restore a default font
}

/**
 *  A gomb kirajzolása megnyomott állapotban kisebb fonttal
 */
void Button::drawPressed() {
    // Draw the button with a border using more rounded corners
    _tft.fillRoundRect(_x, _y, _w, _h, 15, _bgColor);
    _tft.drawRoundRect(_x, _y, _w, _h, 15, _borderColor);

    // Draw the label centered in the button with smaller font for pressed state
    _tft.setTextColor(_textColor);
    _tft.setTextDatum(MC_DATUM);

    // Lenyomott állpotban a gomb felirata
    _tft.setTextFont(4);
    _tft.setTextSize(1);
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
    // Visual feedback: briefly show pressed state
    uint16_t originalBg = _bgColor;
    uint16_t originalBorder = _borderColor;

    // Darken the button for press effect
    _bgColor = originalBg >> 1; // Simple way to darken
    _borderColor = 0xFFFF;      // White border for press

    // Use smaller font for grid buttons (Screen/Alarm/Info) when pressed
    // Check if this is a grid button by checking the label
    if (_label == "Screen" || _label == "Alarm" || _label == "Info") {
        drawPressed(); // Use smaller font for grid buttons
    } else {
        draw(); // Use normal font for other buttons
    }
    delay(100); // Brief delay for visual feedback

    // Restore original colors
    _bgColor = originalBg;
    _borderColor = originalBorder;

    // Redraw with normal appearance
    if (_label == "Screen" || _label == "Alarm" || _label == "Info") {
        drawWithTextAtBottom(); // Restore grid button appearance
    } else {
        draw(); // Restore normal button appearance
    }

    // Execute the callback function if it's set
    if (_callback) {
        _callback();
    }
}

/**
 *  Beállítja a gomb feliratát
 */
void Button::setText(const String &newLabel) { _label = newLabel; }

/**
 *  Beállítja a gomb keret színét
 */
void Button::setBorderColor(uint16_t color) { _borderColor = color; }
