#pragma once

#include "UIButton.h"
#include "UIScreen.h"
#include "defines.h"

/**
 * @brief Műholdak képernyő
 *
 * Megjeleníti a GPS műholdak részletes információit.
 */
class ScreenSat : public UIScreen {
  public:
    /**
     * @brief Konstruktor
     */
    ScreenSat() : UIScreen(SCREEN_NAME_SAT) { layoutComponents(); }

  protected:
    /**
     * @brief Kirajzolja a képernyő saját tartalmát
     */
    void drawContent() override {
        // Címsor
        tft.setTextDatum(MC_DATUM);
        tft.setFreeFont();
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("GPS Satellites", ::SCREEN_W / 2, 30);

        // Műholdak száma
        uint8_t satCount = gpsManager->getSatellites().isValid() ? gpsManager->getSatellites().value() : 0;

        tft.setTextSize(1);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("Satellites in view: " + String(satCount), ::SCREEN_W / 2, 70);

        // HDOP érték
        double hdop = gpsManager->getHdop().isValid() ? gpsManager->getHdop().hdop() : 0.0;
        char hdopStr[10];
        dtostrf(hdop, 0, 2, hdopStr);
        tft.drawString("HDOP: " + String(hdopStr), ::SCREEN_W / 2, 100);

        // GPS státusz
        bool gpsValid = gpsManager->getLocation().isValid();
        tft.setTextColor(gpsValid ? TFT_GREEN : TFT_RED, TFT_BLACK);
        tft.drawString("GPS Status: " + String(gpsValid ? "Valid" : "Invalid"), ::SCREEN_W / 2, 130);

        // Koordináták (ha érvényesek)
        if (gpsValid) {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            double lat = gpsManager->getLocation().lat();
            double lng = gpsManager->getLocation().lng();

            char latStr[15], lngStr[15];
            dtostrf(lat, 0, 6, latStr);
            dtostrf(lng, 0, 6, lngStr);

            tft.drawString("Latitude: " + String(latStr), ::SCREEN_W / 2, 160);
            tft.drawString("Longitude: " + String(lngStr), ::SCREEN_W / 2, 190);
        }

        // Műhold ikon rajzolása középen
        drawSatelliteIcon(::SCREEN_W / 2 - 12, 220);
    }

    /**
     * @brief Kezeli a képernyő saját ciklusát
     */
    void handleOwnLoop() override {
        // 2 másodperces frissítés
        static long lastUpdate = 0;
        if (!Utils::timeHasPassed(lastUpdate, 2000)) {
            return;
        }
        lastUpdate = millis();

        // Teljes képernyő újrarajzolása
        markForRedraw(true);
    }

  private:
    /**
     * @brief UI komponensek elhelyezése
     */
    void layoutComponents() {
        // Back gomb jobb alsó sarokban
        auto backButton = std::make_shared<UIButton>(1, Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT),
                                                     "Back", UIButton::ButtonType::Pushable, [this](const UIButton::ButtonEvent &event) {
                                                         if (event.state == UIButton::EventButtonState::Clicked) {
                                                             DEBUG("ScreenSat: Back button clicked\n");
                                                             getScreenManager()->switchToScreen(SCREEN_NAME_MAIN);
                                                         }
                                                     });

        addChild(backButton);
    }

    /**
     * @brief Műhold ikon rajzolása
     */
    void drawSatelliteIcon(int16_t x, int16_t y) {
        // Központi műhold test (kör)
        tft.fillCircle(x + 12, y + 12, 8, TFT_LIGHTGREY);
        tft.drawCircle(x + 12, y + 12, 8, TFT_WHITE);

        // Felső napelem
        tft.fillRect(x + 8, y + 2, 8, 4, TFT_BLUE);
        tft.drawRect(x + 8, y + 2, 8, 4, TFT_WHITE);

        // Alsó napelem
        tft.fillRect(x + 8, y + 22, 8, 4, TFT_BLUE);
        tft.drawRect(x + 8, y + 22, 8, 4, TFT_WHITE);

        // Bal napelem
        tft.fillRect(x + 2, y + 8, 4, 8, TFT_BLUE);
        tft.drawRect(x + 2, y + 8, 4, 8, TFT_WHITE);

        // Jobb napelem
        tft.fillRect(x + 22, y + 8, 4, 8, TFT_BLUE);
        tft.drawRect(x + 22, y + 8, 4, 8, TFT_WHITE);

        // Összekötő vonalak
        tft.drawLine(x + 12, y + 4, x + 12, y + 6, TFT_WHITE);
        tft.drawLine(x + 12, y + 18, x + 12, y + 20, TFT_WHITE);
        tft.drawLine(x + 4, y + 12, x + 6, y + 12, TFT_WHITE);
        tft.drawLine(x + 18, y + 12, x + 20, y + 12, TFT_WHITE);

        // Központi jelzőfény
        tft.fillCircle(x + 12, y + 12, 3, TFT_RED);
    }
};
