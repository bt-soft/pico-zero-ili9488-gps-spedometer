#ifndef BUTTON_H
#define BUTTON_H

#include <TFT_eSPI.h>
#include <functional> // Required for std::function

class Button {
  public:
    Button(TFT_eSPI &tft, int16_t x, int16_t y, int16_t w, int16_t h, const char *label, uint16_t bgColor, uint16_t textColor, uint16_t borderColor, uint8_t font);

    void draw();
    void drawWithTextAtBottom(); // Special draw method for grid buttons
    void drawPressed();          // Special draw method for pressed state with smaller font
    bool contains(int16_t x, int16_t y);
    void press();
    void setCallback(std::function<void()> callback);
    void setText(const String &newLabel);
    void setBorderColor(uint16_t color);

  private:
    TFT_eSPI &_tft;
    int16_t _x, _y, _w, _h;
    String _label;
    uint16_t _bgColor, _textColor, _borderColor;
    uint8_t _font;
    std::function<void()> _callback;
};

#endif // BUTTON_H
