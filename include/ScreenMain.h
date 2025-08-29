#ifndef __SCREEN_MAIN_H
#define __SCREEN_MAIN_H

#include "ButtonsGroupManager.h"
#include "GpsManager.h"
#include "Large_Font.h"
#include "MessageDialog.h"
#include "SensorUtils.h"
#include "UIScreen.h"
#include "ValueChangeDialog.h"

// Globális GPS manager
extern GpsManager *gpsManager;

/**
 * @file ScreenMain.h
 * @brief Test képernyő osztály, amely használja a ButtonsGroupManager-t
 */

class ScreenMain : public UIScreen, public ButtonsGroupManager<ScreenMain> {

  public:
    /**
     * @brief Adatok struktúrája a képernyő megjelenítéséhez
     */
    struct DisplayData {
        // Műhold adatok
        uint8_t satelliteCount = 0;
        bool satelliteValid = false;
        String gpsMode = "";

        // Dátum és idő
        String dateString = "";
        String timeString = "";
        bool dateTimeValid = false;

        // Magasság
        double altitude = 0.0;
        bool altitudeValid = false;

        // GPS pontosság
        double hdop = 0.0;
        bool hdopValid = false;

        // Sebesség
        double currentSpeed = 0.0;
        bool speedValid = false;

        // Szenzorok
        float batteryVoltage = 0.0;
        float temperature = 0.0;
        String temperatureLabel = "";
    };

    /**
     * @brief ScreenMain konstruktor
     * @param tft TFT display referencia
     */
    ScreenMain() : UIScreen(SCREEN_NAME_MAIN) {

        DEBUG("ScreenMain: Constructor called\n");
        layoutComponents();
    }
    virtual ~ScreenMain() = default;

    /**
     * @brief Loop hívás felülírása
     * animációs vagy egyéb saját logika végrehajtására
     * @note Ez a metódus nem hívja meg a gyerek komponensek loop-ját, csak saját logikát tartalmaz.
     */
    virtual void handleOwnLoop() override;

    /**
     * @brief Képernyő aktiválása - Reset statikus változók
     *
     * Meghívódik amikor a képernyő aktívvá válik (pl. visszatérés Info/Setup képernyőről)
     * Reseteli a statikus változókat hogy kényszerítse az újrarajzolást
     */
    virtual void activate() override;

    /**
     * @brief Kirajzolja a képernyő saját tartalmát
     */
    virtual void drawContent() override;

    /**
     * @brief Touch esemény kezelése
     */
    virtual bool handleTouch(const TouchEvent &event) override;

  private:
    /**
     * @brief UI komponensek létrehozása és elhelyezése
     */
    void layoutComponents();

    /**
     * @brief Műhold ikon rajzolása
     */
    void drawSatelliteIcon(int16_t x, int16_t y);

    /**
     * @brief Magasság ikon rajzolása
     */
    void drawAltitudeIcon(int16_t x, int16_t y);

    /**
     * @brief Naptár ikon rajzolása
     */
    void drawCalendarIcon(int16_t x, int16_t y);

    /**
     * @brief GPS pontosság ikon rajzolása
     */
    void drawGpsAccuracyIcon(int16_t x, int16_t y);

    /**
     * @brief Speedometer ikon rajzolása
     */
    void drawSpeedometerIcon(int16_t x, int16_t y);

    /**
     * @brief Normál módú adatok legyűjtése
     * @return DisplayData struktúra a valós adatokkal
     */
    DisplayData collectRealData();

    /**
     * @brief Demó módú adatok generálása
     * @return DisplayData struktúra a szimulált adatokkal
     */
    DisplayData collectDemoData();

  private:
    /**
     * @brief Hőmérsékleti mód: true = külső hőmérséklet, false = CPU hőmérséklet
     */
    bool externalTemperatureMode = true; // true = external, false = CPU

    /**
     * @brief Kényszerített újrarajzolás flag - amikor visszatérünk más képernyőről
     */
    bool forceRedraw = false;

    /**
     * @brief Sprite frissítés időzítő
     */
    uint32_t lastSpriteUpdate = 0;
};

#endif // __SCREEN_MAIN_H