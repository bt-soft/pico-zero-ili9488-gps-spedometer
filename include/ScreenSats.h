#pragma once

#include "SatelliteDb.h"
#include "UIButton.h"
#include "UIScreen.h"
#include "defines.h"
#include <TFT_eSPI.h>

/**
 * @brief Műholdak képernyő
 *
 * Megjeleníti a GPS műholdak részletes információit táblázatosan és vizuálisan.
 * Optimalizált frissítéssel sprites használatával.
 */
class ScreenSats : public UIScreen {
  public:
    /**
     * @brief Konstruktor
     */
    ScreenSats();

    /**
     * @brief Destruktor
     */
    ~ScreenSats();

  protected:
    /**
     * @brief Kirajzolja a képernyő saját tartalmát
     */
    void drawContent() override;

    /**
     * @brief Kezeli a képernyő saját ciklusát
     */
    void handleOwnLoop() override;

  private:
    // Sprite-ok a gyors frissítéshez
    TFT_eSprite *tableSprite;
    TFT_eSprite *circleSprite;

    // Korábbi adatok tárolása a változások detektálásához
    std::vector<SatelliteDb::SatelliteData> lastSatellites;
    uint8_t lastSatCount;
    bool firstDraw;

    /**
     * @brief UI komponensek elhelyezése
     */
    void layoutComponents();

    /**
     * @brief Címsor rajzolása
     */
    void drawTitle();

    /**
     * @brief Műholdak táblázatának rajzolása a bal oldalon
     */
    void drawSatelliteTable();

    /**
     * @brief Műholdak vizuális megjelenítése koncentrikus körben
     */
    void drawSatelliteCircle();

    /**
     * @brief Egy műhold rajzolása a körön
     */
    void drawSatelliteOnCircle(TFT_eSprite *sprite, int16_t centerX, int16_t centerY, int16_t maxRadius, const SatelliteDb::SatelliteData &sat);

    /**
     * @brief Ellenőrzi, hogy változtak-e a műhold adatok
     */
    bool satelliteDataChanged(const std::vector<SatelliteDb::SatelliteData> &currentSats, uint8_t currentSatCount);

    /**
     * @brief Sprite-ok inicializálása
     */
    void initSprites();

    /**
     * @brief Sprite-ok felszabadítása
     */
    void freeSprites();

    /**
     * @brief Visszaadja a SNR értékhez tartozó színt
     */
    uint32_t getColorBySnr(uint8_t snr);
};
