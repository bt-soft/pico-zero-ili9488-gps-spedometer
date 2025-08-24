#include "ScreenSats.h"
#include "GpsManager.h"
#include "Utils.h"
#include <algorithm>

extern GpsManager *gpsManager;

// Területek méretei
constexpr int16_t TABLE_WIDTH = 150; // +20 pixel szélesebb
constexpr int16_t TABLE_HEIGHT = 200;
constexpr int16_t CIRCLE_WIDTH = 200;
constexpr int16_t CIRCLE_HEIGHT = 200;

/**
 * @brief Konstruktor
 */
ScreenSats::ScreenSats() : UIScreen(SCREEN_NAME_SATS), tableSprite(nullptr), circleSprite(nullptr), lastSatCount(0), firstDraw(true) {
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
    DEBUG("ScreenSats: Initializing sprites...\n");

    // Táblázat sprite inicializálása
    tableSprite = new TFT_eSprite(&tft);
    if (tableSprite->createSprite(TABLE_WIDTH, TABLE_HEIGHT)) {
        tableSprite->fillSprite(TFT_BLACK);
        DEBUG("ScreenSats: Table sprite created successfully (%dx%d)\n", TABLE_WIDTH, TABLE_HEIGHT);
    } else {
        delete tableSprite;
        tableSprite = nullptr;
        DEBUG("ScreenSats: Failed to create table sprite\n");
    }

    // Kör sprite inicializálása
    circleSprite = new TFT_eSprite(&tft);
    if (circleSprite->createSprite(CIRCLE_WIDTH, CIRCLE_HEIGHT)) {
        circleSprite->fillSprite(TFT_BLACK);
        DEBUG("ScreenSats: Circle sprite created successfully (%dx%d)\n", CIRCLE_WIDTH, CIRCLE_HEIGHT);
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
        auto satellites = gpsManager->getSatelliteSnapshotForUI();
        uint8_t satCount = gpsManager->getSatellites().isValid() ? gpsManager->getSatellites().value() : 0;
        lastSatellites = satellites;
        lastSatCount = satCount;

        return;
    }

    // Műhold adatok lekérése
    auto satellites = gpsManager->getSatelliteSnapshotForUI();
    uint8_t satCount = gpsManager->getSatellites().isValid() ? gpsManager->getSatellites().value() : 0;

    // Csak akkor frissítünk, ha változtak az adatok
    if (satelliteDataChanged(satellites, satCount)) {
        DEBUG("ScreenSats: Data changed - updating display\n");
        drawSatelliteTable();
        drawSatelliteCircle();

        // Adatok mentése a következő összehasonlításhoz
        lastSatellites = satellites;
        lastSatCount = satCount;
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

    // Műholdak adatainak lekérése
    auto satellites = gpsManager->getSatelliteSnapshotForUI();

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
    tableSprite->setTextColor(TFT_YELLOW, TFT_BLACK);
    tableSprite->drawString("PRN     Elv    Azm   SNR", tableX, currentY);
    // Vonal a fejléc alatt
    currentY += 7;
    tableSprite->drawFastHLine(tableX, currentY, TABLE_WIDTH, TFT_DARKGREY);

    // Műholdak listázása
    currentY +=  10;
    uint8_t itemCount = 0;
    constexpr uint8_t maxItems = 12; // Maximum megjelenítendő elemek száma

    tableSprite->setFreeFont(&FreeMono9pt7b);

    for (const auto &sat : satellites) {
        if (itemCount >= maxItems)
            break;

        // SNR alapján színkódolás
        uint16_t color = TFT_WHITE;
        if (sat.snr > 30)
            color = TFT_GREEN;
        else if (sat.snr > 20)
            color = TFT_YELLOW;
        else if (sat.snr > 10)
            color = TFT_GOLD;
        else
            color = TFT_ORANGE;

        tableSprite->setTextColor(color, TFT_BLACK);

        // Adatok formázása
        char line[20];
        sprintf(line, "%2d %3d %3d %2d", sat.prn, sat.elevation, sat.azimuth, sat.snr);
        tableSprite->drawString(line, tableX, currentY);

        currentY += lineHeight;
        itemCount++;
    }

    // Ha több műhold van, mint amit meg tudunk jeleníteni
    if (satellites.size() > maxItems) {
        tableSprite->setTextColor(TFT_DARKGREY, TFT_BLACK);
        tableSprite->drawString("+" + String(satellites.size() - maxItems) + " more...", tableX, currentY);
    }

    // Sprite kirajzolása a képernyőre
    tableSprite->pushSprite(5, 55);

    // Font visszaállítása
    tableSprite->setFreeFont();
}

/**
 * @brief Műhold táblázat közvetlen rajzolása
 */
/*
void ScreenSats::drawSatelliteTableDirect() {
    DEBUG("ScreenSats: Using direct table drawing (fallback)\n");

    // Közvetlen rajzolás, ha nincs sprite (fallback)
    const int16_t tableX = 5;
    const int16_t tableY = 55;
    const int16_t lineHeight = 14; // Kicsit nagyobb sormagasság

    // Terület törlése
    tft.fillRect(tableX, tableY, TABLE_WIDTH, TABLE_HEIGHT, TFT_BLACK);

    // Fejléc
    tft.setTextDatum(TL_DATUM);
    tft.setTextSize(1); // Nagyobb font
    tableSprite->setFreeFont(&FreeMono9pt7b);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("PRN Elv Azm SNR", tableX, tableY);

    // Vonal a fejléc alatt
    tft.drawFastHLine(tableX, tableY + 10, 110, TFT_DARKGREY);

    // Műholdak adatainak lekérése
    auto satellites = gpsManager->getSatelliteSnapshotForUI();

    // Műholdak száma
    uint8_t satCount = gpsManager->getSatellites().isValid() ? gpsManager->getSatellites().value() : 0;

    // Státusz információk
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("In view: " + String(satCount), tableX, tableY + 15);
    tft.drawString("In DB: " + String(satellites.size()), tableX, tableY + 27);

    // Műholdak listázása
    int16_t currentY = tableY + 45;
    int itemCount = 0;
    const int maxItems = 12;

    for (const auto &sat : satellites) {
        if (itemCount >= maxItems)
            break;

        // SNR alapján színkódolás
        uint16_t color = TFT_WHITE;
        if (sat.snr > 30)
            color = TFT_GREEN;
        else if (sat.snr > 20)
            color = TFT_YELLOW;
        else if (sat.snr > 10)
            color = TFT_ORANGE;
        else
            color = TFT_RED;

        tft.setTextColor(color, TFT_BLACK);

        // Adatok formázása
        char line[20];
        sprintf(line, "%2d %3d %3d %2d", sat.prn, sat.elevation, sat.azimuth, sat.snr);
        tft.drawString(line, tableX, currentY);

        currentY += lineHeight;
        itemCount++;
    }

    // Ha több műhold van, mint amit meg tudunk jeleníteni
    if (satellites.size() > maxItems) {
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft.drawString("+" + String(satellites.size() - maxItems) + " more...", tableX, currentY);
    }
}
*/

/**
 * @brief Műhold kör rajzolása
 */
void ScreenSats::drawSatelliteCircle() {
    DEBUG("ScreenSats: Drawing satellite circle (sprite: %s)\n", circleSprite ? "available" : "NULL");

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
    DEBUG("ScreenSats: Drawing direction labels at centerX=%d, centerY=%d, maxRadius=%d\n", centerX, centerY, maxRadius);
    circleSprite->drawString("N", centerX, centerY - maxRadius - 12);
    circleSprite->drawString("S", centerX, centerY + maxRadius + 8);
    circleSprite->drawString("E", centerX + maxRadius + 8, centerY);
    circleSprite->drawString("W", centerX - maxRadius - 8, centerY);

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

    // Sprite kirajzolása a képernyőre
    circleSprite->pushSprite(::SCREEN_W - CIRCLE_WIDTH, 55);
}

/**
 * @brief Műhold kör rajzolása
 */
/*
void ScreenSats::drawSatelliteCircleDirect() {
    DEBUG("ScreenSats: Using direct circle drawing (fallback)\n");

    // Közvetlen rajzolás, ha nincs sprite (fallback)
    const int16_t centerX = ::SCREEN_W - 120;
    const int16_t centerY = 120 + 20; // 20 pixellel lejjebb
    const int16_t maxRadius = 80;

    // Terület törlése
    tft.fillRect(::SCREEN_W - CIRCLE_WIDTH, 55, CIRCLE_WIDTH, CIRCLE_HEIGHT, TFT_BLACK);

    // Koncentrikus körök rajzolása (elevation alapján)
    tft.drawCircle(centerX, centerY, maxRadius, TFT_DARKGREY);         // 0° elevation
    tft.drawCircle(centerX, centerY, maxRadius * 2 / 3, TFT_DARKGREY); // 30° elevation
    tft.drawCircle(centerX, centerY, maxRadius * 1 / 3, TFT_DARKGREY); // 60° elevation
    tft.drawCircle(centerX, centerY, 5, TFT_DARKGREY);                 // 90° elevation (zenit)

    // Azimuth vonalak (fő irányok)
    tft.drawLine(centerX, centerY - maxRadius, centerX, centerY + maxRadius, TFT_DARKGREY);
    tft.drawLine(centerX - maxRadius, centerY, centerX + maxRadius, centerY, TFT_DARKGREY);

    // Irány feliratok
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    tft.setFreeFont();
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("N", centerX, centerY - maxRadius - 12);
    tft.drawString("S", centerX, centerY + maxRadius + 8);
    tft.drawString("E", centerX + maxRadius + 8, centerY);
    tft.drawString("W", centerX - maxRadius - 8, centerY);

    // Elevation címkék
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("90°", centerX + 8, centerY - 5);
    tft.drawString("60°", centerX + maxRadius * 1 / 3 + 8, centerY - 5);
    tft.drawString("30°", centerX + maxRadius * 2 / 3 + 8, centerY - 5);
    tft.drawString("0°", centerX + maxRadius + 8, centerY - 5);

    // Műholdak rajzolása
    auto satellites = gpsManager->getSatelliteSnapshotForUI();

    for (const auto &sat : satellites) {
        drawSatelliteOnCircle(nullptr, centerX, centerY, maxRadius, sat);
    }
}
*/

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
    uint16_t color = TFT_WHITE;
    int radius = 2;

    if (sat.snr > 30) {
        color = TFT_GREEN;
        radius = 4;
    } else if (sat.snr > 20) {
        color = TFT_YELLOW;
        radius = 3;
    } else if (sat.snr > 10) {
        color = TFT_ORANGE;
        radius = 3;
    } else {
        color = TFT_RED;
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
