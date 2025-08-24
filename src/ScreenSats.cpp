#include <algorithm>

#include "GpsManager.h"
#include "ScreenSats.h"
#include "Utils.h"

extern GpsManager *gpsManager;

// A táblázatban Maximum megjelenítendő elemek száma
constexpr uint8_t MAX_SATS_TABLE_ITEMS = 9;

// Területek méretei
constexpr int16_t TABLE_WIDTH = 160; // +20 pixel szélesebb
constexpr int16_t TABLE_HEIGHT = 220;
constexpr int16_t CIRCLE_WIDTH = 200;
constexpr int16_t CIRCLE_HEIGHT = 200;

/**
 * @brief Konstruktor
 */
ScreenSats::ScreenSats() : UIScreen(SCREEN_NAME_SATS), tableSprite(nullptr), circleSprite(nullptr), lastSatCount(0), firstDraw(true), currentSortType(SatelliteDb::BY_SNR), sortOrderChanged(false) {
    layoutComponents();
    initSprites();
}

/**
 * @brief Destruktor
 */
ScreenSats::~ScreenSats() { freeSprites(); }

/**
 * @brief Sprite-ok inicializálása
 */
void ScreenSats::initSprites() {

    // Táblázat sprite inicializálása
    tableSprite = new TFT_eSprite(&tft);
    if (tableSprite->createSprite(TABLE_WIDTH, TABLE_HEIGHT)) {
        tableSprite->fillSprite(TFT_BLACK);
    } else {
        delete tableSprite;
        tableSprite = nullptr;
        DEBUG("ScreenSats: Failed to create table sprite\n");
    }

    // Kör sprite inicializálása
    circleSprite = new TFT_eSprite(&tft);
    if (circleSprite->createSprite(CIRCLE_WIDTH, CIRCLE_HEIGHT)) {
        circleSprite->fillSprite(TFT_BLACK);
    } else {
        delete circleSprite;
        circleSprite = nullptr;
        DEBUG("ScreenSats: Failed to create circle sprite\n");
    }
}

/**
 * @brief Sprite-ok felszabadítása
 */
void ScreenSats::freeSprites() {
    if (tableSprite) {
        tableSprite->deleteSprite();
        delete tableSprite;
        tableSprite = nullptr;
    }

    if (circleSprite) {
        circleSprite->deleteSprite();
        delete circleSprite;
        circleSprite = nullptr;
    }
}

/**
 * @brief Képernyő tartalmának rajzolása
 */
void ScreenSats::drawContent() {
    if (firstDraw) {
        // Csak az első rajzoláskor töröljük a teljes képernyőt
        tft.fillScreen(TFT_BLACK);
        drawTitle();
        firstDraw = false;

        // Első rajzoláskor mindig rajzoljunk
        drawSatelliteTable();
        drawSatelliteCircle();

        // Adatok mentése
        auto satellites = gpsManager->getSatelliteSnapshotForUI(currentSortType);
        uint8_t satCount = gpsManager->getSatellites().isValid() ? gpsManager->getSatellites().value() : 0;
        lastSatellites = satellites;
        lastSatCount = satCount;
        sortOrderChanged = false;

        return;
    }

    // Műhold adatok lekérése
    auto satellites = gpsManager->getSatelliteSnapshotForUI(currentSortType);
    uint8_t satCount = gpsManager->getSatellites().isValid() ? gpsManager->getSatellites().value() : 0;

    // Csak akkor frissítünk, ha változtak az adatok vagy a rendezési mód
    if (satelliteDataChanged(satellites, satCount) || sortOrderChanged) {
        drawSatelliteTable();
        drawSatelliteCircle();

        // Adatok mentése a következő összehasonlításhoz
        lastSatellites = satellites;
        lastSatCount = satCount;
        sortOrderChanged = false;
    }
}

/**
 * @brief Kezeli a képernyő saját ciklusát
 */
void ScreenSats::handleOwnLoop() {
    // 2 másodperces frissítés
    static long lastUpdate = 0;
    if (!Utils::timeHasPassed(lastUpdate, 2000)) {
        return;
    }
    lastUpdate = millis();

    // Csak az adatok frissítése, nem a teljes képernyő újrarajzolása
    drawContent();
}

/**
 * @brief UI komponensek elhelyezése
 */
void ScreenSats::layoutComponents() {
    // Back gomb jobb alsó sarokban
    auto backButton = std::make_shared<UIButton>( //
        1, Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), "Back", UIButton::ButtonType::Pushable,
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->switchToScreen(SCREEN_NAME_MAIN);
            }
        });

    addChild(backButton);
}

/**
 * @brief Címsor rajzolása
 */
void ScreenSats::drawTitle() {
    // Címsor rajzolása
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("GPS Satellites", ::SCREEN_W / 2, 15);
    tft.setFreeFont();
}

/**
 * @brief Műhold táblázat rajzolása
 */
void ScreenSats::drawSatelliteTable() {

    if (!tableSprite) {
        // Fallback közvetlen rajzolásra, ha nincs sprite
        // drawSatelliteTableDirect();
        DEBUG("ScreenSats: No table sprite available - skipping\n");
        return;
    }

    // Sprite törlése
    tableSprite->fillSprite(TFT_BLACK);

    // Táblázat pozíciója
    const int16_t tableX = 0; // Sprite koordinátákban
    const int16_t tableY = 0;
    const int16_t lineHeight = 18; // Kicsit nagyobb sormagasság

    // Műholdak adatainak lekérése rendezéssel
    auto satellites = gpsManager->getSatelliteSnapshotForUI(currentSortType);

    // Műholdak száma
    uint8_t satCount = gpsManager->getSatellites().isValid() ? gpsManager->getSatellites().value() : 0;

    tableSprite->setTextDatum(TL_DATUM);
    tableSprite->setFreeFont();
    tableSprite->setTextSize(1);

    // Státusz információk
    tableSprite->setTextColor(TFT_WHITE, TFT_BLACK);
    tableSprite->drawString("In view: " + String(satCount), tableX, tableY);
    tableSprite->drawString("In DB: " + String(satellites.size()), tableX, tableY + 8);

    // Táblázat fejléc
    int16_t currentY = tableY + 25;

    // PRN oszlop szövege
    uint16_t prnColor = (currentSortType == SatelliteDb::BY_PRN) ? TFT_CYAN : TFT_YELLOW;
    tableSprite->setTextColor(prnColor, TFT_BLACK);
    tableSprite->drawString("PRN", tableX, currentY);

    // Középső oszlopok (mindig sárga)
    tableSprite->setTextColor(TFT_YELLOW, TFT_BLACK);
    tableSprite->drawString("     Elv    Azm   ", tableX + 18, currentY);

    // SNR oszlop szövege
    uint16_t snrColor = (currentSortType == SatelliteDb::BY_SNR) ? TFT_CYAN : TFT_YELLOW;
    tableSprite->setTextColor(snrColor, TFT_BLACK);
    tableSprite->drawString("SNR", tableX + 120, currentY);
    // Vonal a fejléc alatt
    currentY += 7;
    tableSprite->drawFastHLine(tableX, currentY, TABLE_WIDTH, TFT_DARKGREY);

    // Műholdak listázása
    currentY += 10;
    uint8_t itemCount = 0;

    tableSprite->setFreeFont(&FreeMono9pt7b);

    for (const auto &sat : satellites) {
        if (itemCount >= MAX_SATS_TABLE_ITEMS)
            break;

        // SNR alapján színkódolás
        uint16_t color = getColorBySnr(sat.snr);
        tableSprite->setTextColor(color, TFT_BLACK);

        // Adatok formázása
        char line[20];
        sprintf(line, "%2d %3d %3d %2d", sat.prn, sat.elevation, sat.azimuth, sat.snr);
        tableSprite->drawString(line, tableX, currentY);

        currentY += lineHeight;
        itemCount++;
    }

    // Ha több műhold van, mint amit meg tudunk jeleníteni
    if (satellites.size() > MAX_SATS_TABLE_ITEMS) {
        tableSprite->setTextColor(TFT_DARKGREY, TFT_BLACK);
        tableSprite->drawString("+" + String(satellites.size() - MAX_SATS_TABLE_ITEMS) + " more...", tableX, currentY);
    }

    // Sprite kirajzolása a képernyőre
    tableSprite->pushSprite(5, 55);

    // Font visszaállítása
    tableSprite->setFreeFont();
}

/**
 * @brief Műhold kör rajzolása
 */
void ScreenSats::drawSatelliteCircle() {

    if (!circleSprite) {
        // Fallback közvetlen rajzolásra, ha nincs sprite
        // drawSatelliteCircleDirect();
        DEBUG("ScreenSats: No circle sprite available - skipping\n");
        return;
    }

    // Sprite törlése
    circleSprite->fillSprite(TFT_BLACK);

    // Kör pozíciója és mérete (sprite koordinátákban)
    const int16_t centerX = CIRCLE_WIDTH / 2;
    const int16_t centerY = CIRCLE_HEIGHT / 2;
    const int16_t maxRadius = 80;

    // Koncentrikus körök rajzolása (elevation alapján)
    circleSprite->drawCircle(centerX, centerY, maxRadius, TFT_DARKGREY);         // 0° elevation
    circleSprite->drawCircle(centerX, centerY, maxRadius * 2 / 3, TFT_DARKGREY); // 30° elevation
    circleSprite->drawCircle(centerX, centerY, maxRadius * 1 / 3, TFT_DARKGREY); // 60° elevation
    circleSprite->drawCircle(centerX, centerY, 5, TFT_DARKGREY);                 // 90° elevation (zenit)

    // Azimuth vonalak (fő irányok)
    // Észak (0°)
    circleSprite->drawLine(centerX, centerY - maxRadius, centerX, centerY + maxRadius, TFT_DARKGREY);
    // Kelet (90°)
    circleSprite->drawLine(centerX - maxRadius, centerY, centerX + maxRadius, centerY, TFT_DARKGREY);

    // Irány feliratok
    circleSprite->setTextDatum(MC_DATUM);
    circleSprite->setTextSize(1);
    circleSprite->setFreeFont();
    circleSprite->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    circleSprite->drawString("N", centerX, centerY - maxRadius - 12);
    circleSprite->drawString("S", centerX, centerY + maxRadius + 8);
    circleSprite->drawString("E", centerX + maxRadius + 8, centerY + 5);
    circleSprite->drawString("W", centerX - maxRadius - 8, centerY + 5);

    // Elevation címkék
    circleSprite->setTextColor(TFT_DARKGREY, TFT_BLACK);
    circleSprite->drawString("90°", centerX + 8, centerY - 5);
    circleSprite->drawString("60°", centerX + maxRadius * 1 / 3 + 8, centerY - 5);
    circleSprite->drawString("30°", centerX + maxRadius * 2 / 3 + 8, centerY - 5);
    circleSprite->drawString("0°", centerX + maxRadius + 8, centerY - 5);

    // Műholdak rajzolása
    auto satellites = gpsManager->getSatelliteSnapshotForUI();

    for (const auto &sat : satellites) {
        drawSatelliteOnCircle(circleSprite, centerX, centerY, maxRadius, sat);
    }

    // Sprite kirajzolása a képernyőre (40px-el balrább)
    circleSprite->pushSprite(::SCREEN_W - CIRCLE_WIDTH - 40, 55);
}

/**
 * @brief Műhold rajzolása körön
 */
void ScreenSats::drawSatelliteOnCircle(TFT_eSprite *sprite, int16_t centerX, int16_t centerY, int16_t maxRadius, const SatelliteDb::SatelliteData &sat) {
    // Elevation alapján távolság számítása (90° = központ, 0° = külső kör)
    float elevationRad = sat.elevation * PI / 180.0;
    float distance = maxRadius * (1.0 - elevationRad / (PI / 2));

    // Azimuth alapján pozíció számítása (0° = észak, 90° = kelet)
    float azimuthRad = (sat.azimuth - 90) * PI / 180.0; // -90 hogy észak legyen felül

    int16_t satX = centerX + distance * cos(azimuthRad);
    int16_t satY = centerY + distance * sin(azimuthRad);

    // SNR alapján színkódolás és méret
    uint16_t color = getColorBySnr(sat.snr);
    int radius = 2;

    if (sat.snr > 30) {
        radius = 4;
    } else if (sat.snr > 20) {
        radius = 3;
    } else if (sat.snr > 10) {
        radius = 3;
    } else {
        radius = 2;
    }

    // Műhold rajzolása (sprite-ra vagy közvetlenül a képernyőre)
    if (sprite) {
        sprite->fillCircle(satX, satY, radius, color);
        sprite->drawCircle(satX, satY, radius, TFT_WHITE);

        // PRN szám megjelenítése
        sprite->setTextDatum(MC_DATUM);
        sprite->setTextSize(1);
        sprite->setTextColor(TFT_WHITE, TFT_BLACK);
        sprite->drawString(String(sat.prn), satX, satY + radius + 8);
    } else {
        tft.fillCircle(satX, satY, radius, color);
        tft.drawCircle(satX, satY, radius, TFT_WHITE);

        // PRN szám megjelenítése
        tft.setTextDatum(MC_DATUM);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString(String(sat.prn), satX, satY + radius + 8);
    }
}

/**
 * @brief Műholdak adatainak változása
 */
bool ScreenSats::satelliteDataChanged(const std::vector<SatelliteDb::SatelliteData> &currentSats, uint8_t currentSatCount) {
    // Ha különböző a méret vagy a műholdak száma
    if (currentSats.size() != lastSatellites.size() || currentSatCount != lastSatCount) {
        return true;
    }

    // Ha megváltozott valamelyik műhold adata
    for (size_t i = 0; i < currentSats.size(); i++) {
        if (i >= lastSatellites.size()) {
            return true;
        }

        const auto &current = currentSats[i];
        const auto &last = lastSatellites[i];

        if (current.prn != last.prn || current.elevation != last.elevation || current.azimuth != last.azimuth || current.snr != last.snr) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Visszaadja a SNR értékhez tartozó színt
 */
uint32_t ScreenSats::getColorBySnr(uint8_t snr) {
    if (snr > 30) {
        return TFT_GREEN;
    } else if (snr > 20) {
        return TFT_YELLOW;
    } else if (snr > 10) {
        return TFT_GOLD;
    } else {
        return TFT_ORANGE;
    }
}

/**
 * @brief Érintés kezelése
 */
bool ScreenSats::handleTouch(const TouchEvent &event) {
    // Először a szülő osztály touch kezelését hívjuk meg
    if (UIScreen::handleTouch(event)) {
        return true;
    }

    // Debug információ
    // DEBUG("ScreenSats touch: x=%d, y=%d, pressed=%d\n", event.x, event.y, event.pressed);

    // Érintés felengedéskor reagálunk (pressed=0)
    if (!event.pressed) {
        int16_t x = event.x;
        int16_t y = event.y;

        // Fejléc területének koordinátái (sprite pozíció + relatív koordináták)
        const int16_t tableX = 5;            // sprite X pozíciója
        const int16_t tableY = 55;           // sprite Y pozíciója
        const int16_t headerY = tableY + 25; // fejléc Y pozíciója

        //  DEBUG("ScreenSats header area: tableX=%d, tableY=%d, headerY=%d\n", tableX, tableY, headerY);

        // PRN oszlop fejléc területe (a logok alapján szélesebb és alacsonyabb)
        if (x >= 5 && x <= 40 && y >= 70 && y <= 90) {
            // DEBUG("ScreenSats: PRN header clicked\n");
            handlePrnHeaderClick();
            return true;
        }

        // SNR oszlop fejléc területe (a logok alapján szélesebb és alacsonyabb)
        if (x >= 120 && x <= 160 && y >= 70 && y <= 90) {
            // DEBUG("ScreenSats: SNR header clicked\n");
            handleSnrHeaderClick();
            return true;
        }
    }

    return false;
}

/**
 * @brief PRN oszlop fejléc kattintásának kezelése
 */
void ScreenSats::handlePrnHeaderClick() {
    currentSortType = SatelliteDb::BY_PRN;
    sortOrderChanged = true;
    // DEBUG("ScreenSats: Sorting by PRN\n");
}

/**
 * @brief SNR oszlop fejléc kattintásának kezelése
 */
void ScreenSats::handleSnrHeaderClick() {
    currentSortType = SatelliteDb::BY_SNR;
    sortOrderChanged = true;
    // DEBUG("ScreenSats: Sorting by SNR\n");
}