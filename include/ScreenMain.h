#pragma once

#include "ButtonsGroupManager.h"
#include "GpsManager.h"
#include "Large_Font.h"
#include "MessageDialog.h"
#include "SensorUtils.h"
#include "TraffipaxManager.h"
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

        // Pozíció
        double latitude = 0.0;
        double longitude = 0.0;
        bool positionValid = false;

        // Magasság
        double altitude = 0.0;
        bool altitudeValid = false;

        // GPS pontosság
        double hdop = 0.0;
        bool hdopValid = false;

        // Sebesség
        double currentSpeed = 0.0;
        double maxSpeed = 0.0;
        bool speedValid = false;

        // Szenzorok
        float voltage = 0.0;
        float temperature = 0.0;
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
     * @brief Hőmérsékleti mód: true = külső hőmérséklet, false = CPU hőmérséklet
     */
    bool externalTemperatureMode = true; // true = external, false = CPU

    /**
     * @brief Feszmérő mód: true = VBus, false = VSys
     */
    bool externalVoltageMode = true; // true = external, false = CPU

    // Optimalizált, duplikációmentes állapotváltozók
    double lastMaxSpeed = -1.0;
    double lastSpeed = -1.0;
    long lastUpdate = 0;
    unsigned long lastDemoEndTime = 0;
    bool wasDemoActive = false;
    uint8_t lastSatCount = 255;
    String lastGpsMode = "";
    String lastDateTime = "?";
    double lastAltitude = -9999.0;
    double lastHdop = -1.0;
    unsigned long lastVerticalLinearSpriteUpdate = 0;

    // String optimalizálás: egyetlen buffer a szöveges értékekhez
    char valueBuffer[64]; // Elég nagy az összes értékhez

    // Demó logika kiszervezése
    void handleDemoMode(DisplayData &data);

    /**
     * @brief Kényszerített újrarajzolás flag - amikor visszatérünk más képernyőről
     */
    bool forceRedraw = false;

    bool traffiAlarmActive = false;

    // ...existing code...

    // Intelligens traffipax figyelmeztető rendszer
    struct TraffipaxAlert {
        enum State { INACTIVE, APPROACHING, NEARBY_STOPPED, DEPARTING };

        State currentState = INACTIVE;
        const TraffipaxManager::TraffipaxRecord *activeTraffipax = nullptr;
        double currentDistance = 0.0;
        double lastDistance = 999999.0;
        unsigned long lastSirenTime = 0;
        unsigned long lastStateChange = 0;

        static constexpr unsigned long SIREN_INTERVAL = 10000; // 10 sec szirénázási intervallum
    };
    TraffipaxAlert traffipaxAlert;

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

    /**
     * @brief Intelligens traffipax figyelmeztetés feldolgozása
     */
    void processIntelligentTraffipaxAlert(double currentLat, double currentLon, bool positionValid);

    /**
     * Trafipax figyelmeztető sáv megjelenítése
     */
    void displayTraffipaxAlert(const TraffipaxManager::TraffipaxRecord *trafipax, double distance);

    /**
     * Trafipax figyelmeztető sáv törlése
     */
    void clearTraffipaxAlert();
};
