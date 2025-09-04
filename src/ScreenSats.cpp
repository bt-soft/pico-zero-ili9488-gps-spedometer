#include <algorithm>

#include "ScreenSats.h"
#include "Utils.h"

#include "GpsManager.h"
extern GpsManager *gpsManager;

// A táblázatban Maximum megjelenítendő elemek száma
constexpr uint8_t MAX_SATS_TABLE_ITEMS = 9;

// Területek méretei
constexpr int16_t TABLE_X = 5;
constexpr int16_t TABLE_Y = 55;
constexpr int16_t TABLE_WIDTH = 160;
constexpr int16_t TABLE_HEIGHT = 220;

constexpr int16_t CIRCLE_AREA_X = TABLE_X + TABLE_WIDTH + 80;
constexpr int16_t CIRCLE_AREA_Y = TABLE_Y;
constexpr int16_t CIRCLE_WIDTH = 200;
constexpr int16_t CIRCLE_HEIGHT = 200;

/**
 * @brief Konstruktor
 */
ScreenSats::ScreenSats() : UIScreen(SCREEN_NAME_SATS), lastSatCount(0), firstDraw(true), currentSortType(SatelliteDb::BY_SNR), sortOrderChanged(false) { layoutComponents(); }

/**
 * @brief Destruktor
 */
ScreenSats::~ScreenSats() {}

/**
 * @brief Képernyő tartalmának rajzolása
 */
void ScreenSats::drawContent() {
    if (firstDraw || sortOrderChanged) {
        // Első rajzoláskor vagy rendezés változásakor mindent újra rajzolunk
        if (firstDraw) {
            tft.fillScreen(TFT_BLACK);
            drawTitle();
        }
        drawSatelliteTable();
        drawSatelliteCircle();

        // Adatok mentése
        auto satellites = gpsManager->getSatelliteSnapshotForUI(currentSortType);
        uint8_t satCount = gpsManager->getSatellites().isValid() ? gpsManager->getSatellites().value() : 0;
        lastSatellites = satellites;
        lastSatCount = satCount;
        sortOrderChanged = false;
        firstDraw = false;
        return;
    }

    // Műhold adatok lekérése
    auto satellites = gpsManager->getSatelliteSnapshotForUI(currentSortType);
    uint8_t satCount = gpsManager->getSatellites().isValid() ? gpsManager->getSatellites().value() : 0;

    // Csak akkor frissítünk, ha változtak az adatok
    if (satelliteDataChanged(satellites, satCount)) {
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

    markForRedraw();
}

/**
 * @brief UI komponensek elhelyezése
 */
void ScreenSats::layoutComponents() {
    // Back gomb jobb alsó sarokban
    auto backButton = std::make_shared<UIButton>(                                                                                                                         //
        1,                                                                                                                                                                //
        Rect(::SCREEN_W - UIButton::DEFAULT_BUTTON_WIDTH, ::SCREEN_H - UIButton::DEFAULT_BUTTON_HEIGHT, UIButton::DEFAULT_BUTTON_WIDTH, UIButton::DEFAULT_BUTTON_HEIGHT), //
        "Back",                                                                                                                                                           //
        UIButton::ButtonType::Pushable,                                                                                                                                   //
        [this](const UIButton::ButtonEvent &event) {
            if (event.state == UIButton::EventButtonState::Clicked) {
                getScreenManager()->goBack();
            }
        } //
    );

    addChild(backButton);
}

/**
 * @brief Címsor rajzolása
 */
void ScreenSats::drawTitle() {
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold18pt7b);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("GPS Satellites", ::SCREEN_W / 2, 20);
    tft.setFreeFont();
}

/**
 * @brief Műhold táblázat rajzolása közvetlenül a képernyőre
 */
void ScreenSats::drawSatelliteTable() {
    const int16_t lineHeight = 18;

    auto satellites = gpsManager->getSatelliteSnapshotForUI(currentSortType);
    uint8_t satCount = gpsManager->getSatellites().isValid() ? gpsManager->getSatellites().value() : 0;

    // Státusz információk
    tft.setTextDatum(TL_DATUM);
    tft.setFreeFont();
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // A háttér törlése a státusz szöveg előtt
    tft.fillRect(TABLE_X, TABLE_Y, TABLE_WIDTH, 24, TFT_BLACK);
    snprintf(valueBuffer, sizeof(valueBuffer), "In view: %u", satCount);
    tft.drawString(valueBuffer, TABLE_X, TABLE_Y);
    snprintf(valueBuffer, sizeof(valueBuffer), "In DB:   %zu", satellites.size());
    tft.drawString(valueBuffer, TABLE_X, TABLE_Y + 12);

    // Táblázat fejléc
    int16_t headerTextY = TABLE_Y + 28;
    if (firstDraw || sortOrderChanged) {
        // A fejléc területének törlése
        tft.fillRect(TABLE_X, headerTextY, TABLE_WIDTH, 12, TFT_BLACK);
        uint16_t prnColor = (currentSortType == SatelliteDb::BY_PRN) ? TFT_CYAN : TFT_YELLOW;
        tft.setTextColor(prnColor, TFT_BLACK);
        tft.drawString("PRN", TABLE_X, headerTextY);

        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("     Elv    Azm   ", TABLE_X + 18, headerTextY);

        uint16_t snrColor = (currentSortType == SatelliteDb::BY_SNR) ? TFT_CYAN : TFT_YELLOW;
        tft.setTextColor(snrColor, TFT_BLACK);
        tft.drawString("SNR", TABLE_X + 120, headerTextY);

        tft.drawFastHLine(TABLE_X, headerTextY + 10, TABLE_WIDTH, TFT_DARKGREY);
    }

    int16_t currentY = headerTextY + 10 + 12;

    // Műholdak listázása
    tft.setFreeFont(&FreeMono9pt7b);
    uint8_t itemCount = 0;

    for (const auto &sat : satellites) {
        if (itemCount >= MAX_SATS_TABLE_ITEMS)
            break;

        uint16_t color = getColorBySnr(sat.snr);
        tft.setTextColor(color, TFT_BLACK);

        char line[20];
        sprintf(line, "%2d %3d %3d %2d", sat.prn, sat.elevation, sat.azimuth, sat.snr);

        // A sor törlése a rajzolás előtt
        tft.fillRect(TABLE_X, currentY, TABLE_WIDTH, lineHeight, TFT_BLACK);
        tft.drawString(line, TABLE_X, currentY);

        currentY += lineHeight;
        itemCount++;
    }

    // Üres sorok törlése a lista végén
    if (itemCount < MAX_SATS_TABLE_ITEMS) {
        tft.fillRect(TABLE_X, currentY, TABLE_WIDTH, (MAX_SATS_TABLE_ITEMS - itemCount) * lineHeight, TFT_BLACK);
        currentY += (MAX_SATS_TABLE_ITEMS - itemCount) * lineHeight;
    }

    // Ha több műhold van, mint amit megjelenítünk, írjuk ki a végére, különben töröljük a felirat helyét
    currentY += 10;
    if (satellites.size() > MAX_SATS_TABLE_ITEMS) {
        uint8_t remaining = satellites.size() - MAX_SATS_TABLE_ITEMS;
        tft.setTextColor(TFT_BROWN, TFT_BLACK);
        tft.setTextDatum(TL_DATUM);
        tft.setFreeFont();
        tft.setTextSize(1);
        char msg[16];
        sprintf(msg, "and %u more...", remaining);
        tft.drawString(msg, TABLE_X, currentY);
    } else {
        // töröljük a felirat helyét
        tft.fillRect(TABLE_X, currentY, TABLE_WIDTH, 28, TFT_BLACK);
    }

    tft.setFreeFont();
}

/**
 * @brief Műhold kör rajzolása közvetlenül a képernyőre
 */
void ScreenSats::drawSatelliteCircle() {
    const int16_t centerX = CIRCLE_AREA_X + CIRCLE_WIDTH / 2;
    const int16_t centerY = CIRCLE_AREA_Y + CIRCLE_HEIGHT / 2;
    const int16_t maxRadius = 80;

    // Háttér törlése
    tft.fillRect(CIRCLE_AREA_X, CIRCLE_AREA_Y, CIRCLE_WIDTH, CIRCLE_HEIGHT, TFT_BLACK);

    // Koncentrikus körök és vonalak rajzolása
    tft.drawCircle(centerX, centerY, maxRadius, TFT_DARKGREY);
    tft.drawCircle(centerX, centerY, maxRadius * 2 / 3, TFT_DARKGREY);
    tft.drawCircle(centerX, centerY, maxRadius * 1 / 3, TFT_DARKGREY);
    tft.drawCircle(centerX, centerY, 5, TFT_DARKGREY);
    tft.drawLine(centerX, centerY - maxRadius, centerX, centerY + maxRadius, TFT_DARKGREY);
    tft.drawLine(centerX - maxRadius, centerY, centerX + maxRadius, centerY, TFT_DARKGREY);

    // Irány és eleváció feliratok
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    tft.setFreeFont();
    tft.setTextPadding(0);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("N", centerX, centerY - maxRadius - 12);
    tft.drawString("S", centerX, centerY + maxRadius + 8);
    tft.drawString("E", centerX + maxRadius + 8, centerY + 5);
    tft.drawString("W", centerX - maxRadius - 8, centerY + 5);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("90°", centerX + 8, centerY - 5);
    tft.drawString("60°", centerX + maxRadius * 1 / 3 + 8, centerY - 5);
    tft.drawString("30°", centerX + maxRadius * 2 / 3 + 8, centerY - 5);
    tft.drawString("0°", centerX + maxRadius + 8, centerY - 5);

    // Műholdak rajzolása
    auto satellites = gpsManager->getSatelliteSnapshotForUI();
    for (const auto &sat : satellites) {
        drawSatelliteOnCircle(centerX, centerY, maxRadius, sat);
    }
}

/**
 * @brief Egy műhold rajzolása a körön
 */
void ScreenSats::drawSatelliteOnCircle(int16_t centerX, int16_t centerY, int16_t maxRadius, const SatelliteDb::SatelliteData &sat) {
    float elevationRad = sat.elevation * PI / 180.0;
    float distance = maxRadius * (1.0 - elevationRad / (PI / 2));
    float azimuthRad = (sat.azimuth - 90) * PI / 180.0;

    int16_t satX = centerX + distance * cos(azimuthRad);
    int16_t satY = centerY + distance * sin(azimuthRad);

    uint16_t color = getColorBySnr(sat.snr);
    int radius = (sat.snr > 30) ? 4 : 3;

    tft.fillCircle(satX, satY, radius, color);      // Műhold kitöltése az SNR értéknek megfelelően
    tft.drawCircle(satX, satY, radius, TFT_PURPLE); // Műhold körvonalának rajzolása

    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
    tft.setTextPadding(0); // Ne töröljön bele semmibe
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    snprintf(valueBuffer, sizeof(valueBuffer), "%u", sat.prn);
    tft.drawString(valueBuffer, satX, satY + radius + 8);
}

/**
 * @brief Műholdak adatainak változása
 */
bool ScreenSats::satelliteDataChanged(const std::vector<SatelliteDb::SatelliteData> &currentSats, uint8_t currentSatCount) {
    if (currentSats.size() != lastSatellites.size() || currentSatCount != lastSatCount) {
        return true;
    }
    for (size_t i = 0; i < currentSats.size(); i++) {
        if (i >= lastSatellites.size() || currentSats[i].prn != lastSatellites[i].prn || currentSats[i].snr != lastSatellites[i].snr) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Visszaadja a SNR értékhez tartozó színt
 */
uint32_t ScreenSats::getColorBySnr(uint8_t snr) {
    if (snr > 30)
        return TFT_GREEN;
    if (snr > 20)
        return TFT_YELLOW;
    if (snr > 10)
        return TFT_GOLD;

    return TFT_BROWN;
}

/**
 * @brief Érintés kezelése
 */
bool ScreenSats::handleTouch(const TouchEvent &event) {

    if (UIScreen::handleTouch(event)) {
        return true;
    }

    if (!event.pressed) {
        int16_t x = event.x;
        int16_t y = event.y;

        // PRN touch?
        if (x >= TABLE_X && x <= TABLE_X + 40 && y >= TABLE_Y + 25 && y <= TABLE_Y + 45) {
            handlePrnHeaderClick();
            return true;

        } // SNR touch?
        else if (x >= TABLE_X + 120 && x <= TABLE_X + TABLE_WIDTH && y >= TABLE_Y + 25 && y <= TABLE_Y + 45) {
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

    // Pittyentünk, ha az engedélyezve van
    if (config.data.beeperEnabled) {
        Utils::beepTick();
    }
}

/**
 * @brief SNR oszlop fejléc kattintásának kezelése
 */
void ScreenSats::handleSnrHeaderClick() {
    currentSortType = SatelliteDb::BY_SNR;
    sortOrderChanged = true;

    // Pittyentünk, ha az engedélyezve van
    if (config.data.beeperEnabled) {
        Utils::beepTick();
    }
}
