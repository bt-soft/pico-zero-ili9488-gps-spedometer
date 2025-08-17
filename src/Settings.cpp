#include "Settings.h"
#include "Config.h"               // A teljes fejléc implementációhoz való beillesztése
#include "TftBackLightAdjuster.h" // Teljes definíció beillesztése
#include "TrafipaxManager.h"
#include "Utils.h" // Csipogáshoz és egyéb segédprogramokhoz
#include "defines.h"
#include <math.h> // Trigonometrikus függvényekhez

// Címsor magasságának meghatározása
const int TITLE_BAR_HEIGHT = 80; // Megnövelt magasság a jobb vizuális megjelenésért

// Konstansok a gombelrendezés optimalizálásához
static const int BUTTON_MARGIN = 40;
static const int BUTTON_HEIGHT = 60; // Megnövelt magasság a jobb érintésérzékelésért
static const int BUTTON_SPACING = 75;
static const int CONTROL_BUTTON_WIDTH = 120; // Szélesebb a jobb érintésérzékelésért
static const int BACK_BUTTON_WIDTH = 160;

// Vizuális fejlesztési konstansok
static const int BUTTON_CORNER_RADIUS = 15;
static const int TITLE_SHADOW_OFFSET = 3;

// Színséma modern megjelenéshez
static const uint16_t HEADER_BG_COLOR = RGB565(45, 45, 75); // Sötét kékes-szürke
static const uint16_t HEADER_TEXT_COLOR = TFT_WHITE;
static const uint16_t BUTTON_BG_COLOR = RGB565(60, 60, 90); // Kissé világosabb
static const uint16_t BUTTON_TEXT_COLOR = TFT_WHITE;
static const uint16_t BUTTON_BORDER_ACTIVE = RGB565(100, 150, 255); // Szép kék
static const uint16_t BUTTON_BORDER_DISABLED = RGB565(100, 100, 100);
static const uint16_t ACCENT_GREEN = RGB565(50, 200, 100);
static const uint16_t ACCENT_RED = RGB565(255, 80, 80);
static const uint16_t ACCENT_ORANGE = RGB565(255, 165, 0);
static const uint16_t VALUE_DISPLAY_BG = RGB565(30, 30, 50);

/**
 *  A beállítások menü inicializálása
 */
Settings::Settings(TFT_eSPI &tft, Config &config, TftBackLightAdjuster &tftBackLightAdjuster, TrafipaxManager &trafipaxManager)
    : _tft(tft), _config(config), _active(false), _currentState(ScreenState::MAIN), _needsRedraw(true), _tftBackLightAdjuster(tftBackLightAdjuster), _trafipaxManager(trafipaxManager) {}

/**
 *  A beállítások menü inicializálása
 */
void Settings::init() {
    // Memória foglalása az újraallokációk elkerülése érdekében
    _mainButtons.reserve(4); // 3 rács gomb + 1 vissza gomb
    _brightnessButtons.reserve(4);
    _alarmButtons.reserve(4);
    _informationButtons.reserve(1);

    initMainButtons();
    initBrightnessButtons();
    initAlarmButtons();
    initInformationButtons();
}

void Settings::initMainButtons() {
    int screenWidth = _tft.width();
    int screenHeight = _tft.height();

    // Rács elrendezés a fő gombokhoz - 3 gomb egy sorban
    int gridButtonWidth = 130; // Szélesebb négyzetes gombok
    int gridButtonHeight = 80; // Magasabb a jobb érintésérzékelésért
    int gridSpacing = 15;      // Térköz a rács gombok között

    // Teljes rács szélességének kiszámítása és középre igazítása
    int totalGridWidth = 3 * gridButtonWidth + 2 * gridSpacing;
    int gridStartX = (screenWidth - totalGridWidth) / 2;
    int gridY = 140; // Függőlegesen középre igazítás a fejléc és az alsó gombok között

    // Képernyő gomb (balra) ikonnal
    createButton(_mainButtons, gridStartX, gridY, gridButtonWidth, gridButtonHeight, "Screen", BUTTON_BORDER_ACTIVE, [this]() {
        _currentState = ScreenState::BRIGHTNESS;
        _needsRedraw = true;
        Utils::beepTick();
    });

    // Riasztás gomb (középen) ikonnal
    createButton(_mainButtons, gridStartX + gridButtonWidth + gridSpacing, gridY, gridButtonWidth, gridButtonHeight, "Alarm", BUTTON_BORDER_ACTIVE, [this]() {
        _currentState = ScreenState::ALARM;
        _needsRedraw = true;
        Utils::beepTick();
    });

    // Infó gomb (jobbra) ikonnal
    createButton(_mainButtons, gridStartX + 2 * (gridButtonWidth + gridSpacing), gridY, gridButtonWidth, gridButtonHeight, "Info", BUTTON_BORDER_ACTIVE, [this]() {
        _currentState = ScreenState::INFORMATION;
        _needsRedraw = true;
        Utils::beepTick();
    });

    // Vissza gomb (alul középen) - menti a konfigurációt és kilép
    createButton(_mainButtons, screenWidth / 2 - BACK_BUTTON_WIDTH / 2, screenHeight - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Back", ACCENT_ORANGE, [this]() {
        _config.checkSave();
        Utils::beepTick();
        delay(100);
        exit();
    });
}

void Settings::initInformationButtons() {
    // Vissza gomb
    createButton(_informationButtons, _tft.width() - BACK_BUTTON_WIDTH, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Back", ACCENT_ORANGE, [this]() {
        _currentState = ScreenState::MAIN;
        _needsRedraw = true;
        Utils::beepTick();
    });
}

void Settings::initBrightnessButtons() {
    int screenWidth = _tft.width();
    int buttonWidth = screenWidth - 2 * BUTTON_MARGIN;

    // Automatikus fényerő kapcsoló gomb
    createButton(_brightnessButtons, BUTTON_MARGIN, 100, buttonWidth, BUTTON_HEIGHT, "", BUTTON_BORDER_ACTIVE, [this]() {
        _config.data.tftAutoBrightnessActive = !_config.data.tftAutoBrightnessActive;
        _tftBackLightAdjuster.setAutoBrightnessActive(_config.data.tftAutoBrightnessActive);
        _needsRedraw = true;
        Utils::beepTick();
    });

    // Kézi fényerő csökkentő gomb
    createButton(_brightnessButtons, BUTTON_MARGIN, 190, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "-", ACCENT_RED, [this]() {
        if (!_config.data.tftAutoBrightnessActive) {
            uint8_t &val = _config.data.tftManualBrightnessValue;
            val = (val > 5) ? val - 5 : NIGHTLY_BRIGHTNESS;
            _tftBackLightAdjuster.setBacklightLevel(val);
            _config.data.tftManualBrightnessValue = _tftBackLightAdjuster.getBacklightLevel();
            updateBrightnessValueDisplay();
            Utils::beepTick();
        }
    });

    // Kézi fényerő növelő gomb
    createButton(_brightnessButtons, screenWidth - BUTTON_MARGIN - CONTROL_BUTTON_WIDTH, 190, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "+", ACCENT_GREEN, [this]() {
        if (!_config.data.tftAutoBrightnessActive) {
            uint8_t &val = _config.data.tftManualBrightnessValue;
            val = (val < TFT_BACKGROUND_LED_MAX_BRIGHTNESS - 5) ? val + 5 : TFT_BACKGROUND_LED_MAX_BRIGHTNESS;
            _tftBackLightAdjuster.setBacklightLevel(val);
            _config.data.tftManualBrightnessValue = _tftBackLightAdjuster.getBacklightLevel();
            updateBrightnessValueDisplay();
            Utils::beepTick();
        }
    });

    // Vissza gomb
    createButton(_brightnessButtons, screenWidth - BACK_BUTTON_WIDTH, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Back", ACCENT_ORANGE, [this]() {
        _currentState = ScreenState::MAIN;
        _needsRedraw = true;
        Utils::beepTick();
    });
}

void Settings::initAlarmButtons() {
    int screenWidth = _tft.width();
    int buttonWidth = screenWidth - 2 * BUTTON_MARGIN;

    // GPS Trafi riasztás engedélyezése kapcsoló gomb
    createButton(_alarmButtons, BUTTON_MARGIN, 100, buttonWidth, BUTTON_HEIGHT, "", BUTTON_BORDER_ACTIVE, [this]() {
        _config.data.gpsTrafiAlarmEnabled = !_config.data.gpsTrafiAlarmEnabled;
        _needsRedraw = true;
        Utils::beepTick();
    });

    // GPS Trafi riasztás távolság csökkentő gomb
    createButton(_alarmButtons, BUTTON_MARGIN, 190, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "-", ACCENT_RED, [this]() {
        if (_config.data.gpsTrafiAlarmEnabled) {
            uint16_t &val = _config.data.gpsTrafiAlarmDistance;
            val = (val > 400) ? val - 100 : 400;
            updateAlarmValueDisplay();
            Utils::beepTick();
        }
    });

    // GPS Trafi riasztás távolság növelő gomb
    createButton(_alarmButtons, screenWidth - BUTTON_MARGIN - CONTROL_BUTTON_WIDTH, 190, CONTROL_BUTTON_WIDTH, BUTTON_HEIGHT, "+", ACCENT_GREEN, [this]() {
        if (_config.data.gpsTrafiAlarmEnabled) {
            uint16_t &val = _config.data.gpsTrafiAlarmDistance;
            val = (val < 2000) ? val + 100 : 2000;
            updateAlarmValueDisplay();
            Utils::beepTick();
        }
    });

    // Vissza gomb
    createButton(_alarmButtons, screenWidth - BACK_BUTTON_WIDTH, _tft.height() - BUTTON_HEIGHT, BACK_BUTTON_WIDTH, BUTTON_HEIGHT, "Back", ACCENT_ORANGE, [this]() {
        _currentState = ScreenState::MAIN;
        _needsRedraw = true;
        Utils::beepTick();
    });
}

bool Settings::isActive() { return _active; }

void Settings::enter() {
    _active = true;
    _currentState = ScreenState::MAIN;
    _needsRedraw = true;
    Utils::beepTick();
    delay(200);
}

void Settings::exit() {
    _active = false;
    Utils::beepTick();
}

void Settings::draw() {
    _tft.fillScreen(RGB565(25, 25, 35)); // Sötét modern háttér
    switch (_currentState) {
        case ScreenState::MAIN:
            drawMainScreen();
            break;
        case ScreenState::BRIGHTNESS:
            drawBrightnessScreen();
            break;
        case ScreenState::ALARM:
            drawAlarmScreen();
            break;
        case ScreenState::INFORMATION:
            drawInformationScreen();
            break;
    }
}

/**
 * Képernyő címének megjelenítése modern dizájnnal
 */
void Settings::drawScreenTitle(const char *title) {
    // Modern, gradiens-szerű fejléc háttér
    _tft.fillRect(0, 0, _tft.width(), TITLE_BAR_HEIGHT, HEADER_BG_COLOR);

    // Finom szegély hozzáadása alulra
    _tft.fillRect(0, TITLE_BAR_HEIGHT - 3, _tft.width(), 3, BUTTON_BORDER_ACTIVE);

    // Árnyék effektus a szöveghez
    _tft.setTextFont(4);
    _tft.setTextSize(2);
    _tft.setTextDatum(MC_DATUM);

    // Árnyék rajzolása
    _tft.setTextColor(RGB565(20, 20, 40)); // Sötét árnyék
    _tft.drawString(title, _tft.width() / 2 + TITLE_SHADOW_OFFSET, TITLE_BAR_HEIGHT / 2 + TITLE_SHADOW_OFFSET);

    // Fő szöveg rajzolása
    _tft.setTextColor(HEADER_TEXT_COLOR);
    _tft.drawString(title, _tft.width() / 2, TITLE_BAR_HEIGHT / 2);

    // Dekoratív sarokelemek hozzáadása
    _tft.fillCircle(20, 20, 8, BUTTON_BORDER_ACTIVE);
    _tft.fillCircle(_tft.width() - 20, 20, 8, BUTTON_BORDER_ACTIVE);

    _tft.setTextSize(1); // Alapértelmezett visszaállítása
}

/**
 * Fő képernyő megjelenítése modern grid layout-tal
 */
void Settings::drawMainScreen() {
    // Képernyő címének megjelenítése
    drawScreenTitle("Settings");

    // Árnyékok rajzolása a három fő rács gombhoz először
    int screenWidth = _tft.width();
    int gridButtonWidth = 130;
    int gridButtonHeight = 80;
    int gridSpacing = 15;
    int totalGridWidth = 3 * gridButtonWidth + 2 * gridSpacing;
    int gridStartX = (screenWidth - totalGridWidth) / 2;
    int gridY = 140;
    int shadowOffset = 4;

    // Árnyékok rajzolása a rács gombokhoz
    uint16_t shadowColor = RGB565(15, 15, 25); // Nagyon sötét árnyék
    for (int i = 0; i < 3; i++) {
        int buttonX = gridStartX + i * (gridButtonWidth + gridSpacing);
        _tft.fillRoundRect(buttonX + shadowOffset, gridY + shadowOffset, gridButtonWidth, gridButtonHeight, 15, shadowColor);
    }

    // Az első 3 gomb (rács gombok) rajzolása speciális elrendezéssel
    for (int i = 0; i < 3; i++) {
        _mainButtons[i].drawWithTextAtBottom();
    }

    // A fennmaradó gomb (Vissza) normál rajzolása
    for (int i = 3; i < _mainButtons.size(); i++) {
        _mainButtons[i].draw();
    }

    // Ikonok hozzáadása a rács gombokhoz az új ikon funkcióval
    int iconY = gridY + 25; // Ikonok pozíciója

    // Képernyő ikon (nap szimbólum)
    int screenButtonCenterX = gridStartX + gridButtonWidth / 2;
    drawButtonIcon(screenButtonCenterX, iconY, "sun", TFT_YELLOW);

    // Riasztás ikon (figyelmeztető háromszög)
    int alarmButtonCenterX = gridStartX + gridButtonWidth + gridSpacing + gridButtonWidth / 2;
    drawButtonIcon(alarmButtonCenterX, iconY, "warning", ACCENT_RED);

    // Infó ikon (i körben)
    int infoButtonCenterX = gridStartX + 2 * (gridButtonWidth + gridSpacing) + gridButtonWidth / 2;
    drawButtonIcon(infoButtonCenterX, iconY, "info", BUTTON_BORDER_ACTIVE);

    // Ikon hozzáadása a Vissza gombhoz (alul középen)
    int backButtonCenterX = screenWidth / 2;
    int backButtonCenterY = _tft.height() - BUTTON_HEIGHT / 2;
    drawButtonIcon(backButtonCenterX - 50, backButtonCenterY, "back", ACCENT_ORANGE); // Eltolás balra a szöveg átfedésének elkerülése érdekében

    // Finom cím hozzáadása a rács területéhez
    _tft.setTextFont(2);
    _tft.setTextSize(2);
    _tft.setTextColor(TFT_LIGHTGREY);
    _tft.setTextDatum(MC_DATUM);
    _tft.drawString("Select Category", screenWidth / 2, 110);
}

/**
 * Képernyő világosítása
 */
void Settings::drawBrightnessScreen() {
    // Képernyő címének megjelenítése
    drawScreenTitle("Brightness");

    // Automatikus fényerő gomb szövegének frissítése
    char autoText[20];
    sprintf(autoText, "Auto: %s", _config.data.tftAutoBrightnessActive ? "On" : "Off");
    _brightnessButtons[0].setText(autoText);

    // Gomb állapotok frissítése az automatikus fényerő mód alapján
    uint16_t disabledColor = _config.data.tftAutoBrightnessActive ? BUTTON_BORDER_DISABLED : ACCENT_RED;
    uint16_t enabledColor = _config.data.tftAutoBrightnessActive ? BUTTON_BORDER_DISABLED : ACCENT_GREEN;

    _brightnessButtons[1].setBorderColor(disabledColor);
    _brightnessButtons[2].setBorderColor(enabledColor);

    // Összes gomb rajzolása
    for (auto &button : _brightnessButtons) {
        button.draw();
    }

    // Ikonok hozzáadása a gombokhoz
    int screenWidth = _tft.width();

    // Kapcsoló ikon az automatikus fényerőhöz
    int toggleX = BUTTON_MARGIN + (screenWidth - 2 * BUTTON_MARGIN) / 2;
    int toggleY = 100 + BUTTON_HEIGHT / 2;
    drawButtonIcon(toggleX - 100, toggleY, "toggle", _config.data.tftAutoBrightnessActive ? ACCENT_GREEN : BUTTON_BORDER_DISABLED);

    // Mínusz ikon
    int minusX = BUTTON_MARGIN + CONTROL_BUTTON_WIDTH / 2;
    int minusY = 190 + BUTTON_HEIGHT / 2;
    drawButtonIcon(minusX, minusY, "minus", disabledColor);

    // Plusz ikon
    int plusX = screenWidth - BUTTON_MARGIN - CONTROL_BUTTON_WIDTH / 2;
    int plusY = 190 + BUTTON_HEIGHT / 2;
    drawButtonIcon(plusX, plusY, "plus", enabledColor);

    // Vissza nyíl ikon - pozícionálás a szövegtől balra
    int backX = screenWidth - BACK_BUTTON_WIDTH + 30; // Gomb bal oldala + margó
    int backY = _tft.height() - BUTTON_HEIGHT / 2;
    drawButtonIcon(backX, backY, "back", ACCENT_ORANGE);

    // Érték kijelző háttér rajzolása lekerekített sarkokkal
    int centerX = _tft.width() / 2;
    int centerY = 220;
    int boxWidth = 100;
    int boxHeight = 50;

    _tft.fillRoundRect(centerX - boxWidth / 2, centerY - boxHeight / 2, boxWidth, boxHeight, 10, VALUE_DISPLAY_BG);
    _tft.drawRoundRect(centerX - boxWidth / 2, centerY - boxHeight / 2, boxWidth, boxHeight, 10, BUTTON_BORDER_ACTIVE);

    // Kézi fényerő értékének megjelenítése
    updateBrightnessValueDisplay();
}

/**
 * Riasztási képernyő megjelenítése
 */
void Settings::drawAlarmScreen() {
    // Képernyő címének megjelenítése
    drawScreenTitle("Alarm Settings");

    // GPS Trafi riasztás engedélyezése gomb szövegének frissítése
    char enabledText[20];
    sprintf(enabledText, "Trafi Alarm: %s", _config.data.gpsTrafiAlarmEnabled ? "On" : "Off");
    _alarmButtons[0].setText(enabledText);

    // Gomb állapotok frissítése a riasztás engedélyezési mód alapján
    uint16_t disabledColor = !_config.data.gpsTrafiAlarmEnabled ? BUTTON_BORDER_DISABLED : ACCENT_RED;
    uint16_t enabledColor = !_config.data.gpsTrafiAlarmEnabled ? BUTTON_BORDER_DISABLED : ACCENT_GREEN;

    _alarmButtons[1].setBorderColor(disabledColor);
    _alarmButtons[2].setBorderColor(enabledColor);

    // Összes gomb rajzolása
    for (auto &button : _alarmButtons) {
        button.draw();
    }

    // Ikonok hozzáadása a gombokhoz
    int screenWidth = _tft.width();

    // Kapcsoló ikon a riasztás engedélyezéséhez
    int toggleX = BUTTON_MARGIN + (screenWidth - 2 * BUTTON_MARGIN) / 2;
    int toggleY = 100 + BUTTON_HEIGHT / 2;
    drawButtonIcon(toggleX - 100, toggleY, "toggle", _config.data.gpsTrafiAlarmEnabled ? ACCENT_GREEN : BUTTON_BORDER_DISABLED);

    // Mínusz ikon
    int minusX = BUTTON_MARGIN + CONTROL_BUTTON_WIDTH / 2;
    int minusY = 190 + BUTTON_HEIGHT / 2;
    drawButtonIcon(minusX, minusY, "minus", disabledColor);

    // Plusz ikon
    int plusX = screenWidth - BUTTON_MARGIN - CONTROL_BUTTON_WIDTH / 2;
    int plusY = 190 + BUTTON_HEIGHT / 2;
    drawButtonIcon(plusX, plusY, "plus", enabledColor);

    // Vissza nyíl ikon - pozícionálás a szövegtől balra
    int backX = screenWidth - BACK_BUTTON_WIDTH + 30; // Gomb bal oldala + margó
    int backY = _tft.height() - BUTTON_HEIGHT / 2;
    drawButtonIcon(backX, backY, "back", ACCENT_ORANGE);

    // Érték kijelző háttér rajzolása lekerekített sarkokkal
    int centerX = _tft.width() / 2;
    int centerY = 220;
    int boxWidth = 120;
    int boxHeight = 50;

    _tft.fillRoundRect(centerX - boxWidth / 2, centerY - boxHeight / 2, boxWidth, boxHeight, 10, VALUE_DISPLAY_BG);
    _tft.drawRoundRect(centerX - boxWidth / 2, centerY - boxHeight / 2, boxWidth, boxHeight, 10, BUTTON_BORDER_ACTIVE);

    // Riasztási távolság értékének megjelenítése
    updateAlarmValueDisplay();

    // Egység szöveg hozzáadása
    if (_config.data.gpsTrafiAlarmEnabled) {
        _tft.setTextFont(2);
        _tft.setTextColor(TFT_LIGHTGREY);
        _tft.setTextDatum(MC_DATUM);
        _tft.drawString("meters", centerX, centerY + 35);
    }
}

/**
 * Információs képernyő megjelenítése
 */
void Settings::drawInformationScreen() {
    // Képernyő címének megjelenítése
    drawScreenTitle("Information");

    char buf[64];
    int yPos = 120;
    int lineSpacing = 45;

    // Infókártya háttér létrehozása
    int cardX = 30;
    int cardY = 110;
    int cardWidth = _tft.width() - 60;
    int cardHeight = 160;

    _tft.fillRoundRect(cardX, cardY, cardWidth, cardHeight, 15, VALUE_DISPLAY_BG);
    _tft.drawRoundRect(cardX, cardY, cardWidth, cardHeight, 15, BUTTON_BORDER_ACTIVE);

    _tft.setTextDatum(TL_DATUM);
    _tft.setTextFont(2);

    // Program verzió ikon-szerű ponttal
    _tft.fillCircle(50, yPos + 10, 5, ACCENT_GREEN);
    _tft.setTextColor(TFT_WHITE);
    sprintf(buf, "Version: V%s", APP_VERSION);
    _tft.drawString(buf, 70, yPos);

    yPos += lineSpacing;
    // Fordítási idő
    _tft.fillCircle(50, yPos + 10, 5, BUTTON_BORDER_ACTIVE);
    _tft.setTextColor(TFT_LIGHTGREY);
    sprintf(buf, "Build: %s %s", __DATE__, __TIME__);
    _tft.drawString(buf, 70, yPos);

    yPos += lineSpacing;
    // Trafipaxok száma
    _tft.fillCircle(50, yPos + 10, 5, ACCENT_ORANGE);
    _tft.setTextColor(TFT_YELLOW);
    sprintf(buf, "Trafipax Count: %d", _trafipaxManager.count());
    _tft.drawString(buf, 70, yPos);

    // Összes gomb rajzolása
    for (auto &button : _informationButtons) {
        button.draw();
    }

    // Vissza nyíl ikon hozzáadása a Vissza gombhoz - pozícionálás a szövegtől balra
    int backButtonCenterX = _tft.width() - BACK_BUTTON_WIDTH + 30; // Gomb bal oldala + margó
    int backButtonCenterY = _tft.height() - BUTTON_HEIGHT / 2;
    drawButtonIcon(backButtonCenterX, backButtonCenterY, "back", TFT_WHITE);
}

/**
 * Képernyő világosítása
 */
void Settings::updateBrightnessValueDisplay() { updateValueDisplay(_config.data.tftManualBrightnessValue, "-", !_config.data.tftAutoBrightnessActive); }

void Settings::updateAlarmValueDisplay() { updateValueDisplay(_config.data.gpsTrafiAlarmDistance, "-", _config.data.gpsTrafiAlarmEnabled); }

void Settings::handleTouch() {
    uint16_t x, y;
    if (_tft.getTouch(&x, &y)) {
        switch (_currentState) {
            case ScreenState::MAIN:
                handleTouchForButtonList(_mainButtons, x, y);
                break;
            case ScreenState::BRIGHTNESS:
                handleTouchForButtonList(_brightnessButtons, x, y);
                break;
            case ScreenState::ALARM:
                handleTouchForButtonList(_alarmButtons, x, y);
                break;
            case ScreenState::INFORMATION:
                handleTouchForButtonList(_informationButtons, x, y);
                break;
        }
        delay(200); // Gombnyomás késleltetés
    }
}

// Régebbi érintéskezelők kompatibilitási okokból megtartva, de egyszerűsítve
void Settings::handleMainTouch(uint16_t x, uint16_t y) { handleTouchForButtonList(_mainButtons, x, y); }

void Settings::handleBrightnessTouch(uint16_t x, uint16_t y) { handleTouchForButtonList(_brightnessButtons, x, y); }

void Settings::handleAlarmTouch(uint16_t x, uint16_t y) { handleTouchForButtonList(_alarmButtons, x, y); }

void Settings::handleInformationTouch(uint16_t x, uint16_t y) { handleTouchForButtonList(_informationButtons, x, y); }

void Settings::loop() {
    if (!_active) {
        return;
    }

    if (_needsRedraw) {
        draw();
        _needsRedraw = false;
    }

    handleTouch();
}

// Segédfüggvény implementációk
void Settings::createButton(std::vector<Button> &buttonList, int16_t x, int16_t y, int16_t w, int16_t h, const char *label, uint16_t borderColor, std::function<void()> callback) {
    buttonList.emplace_back(_tft, x, y, w, h, label, BUTTON_BG_COLOR, BUTTON_TEXT_COLOR, borderColor, 4);
    buttonList.back().setCallback(callback);
}

void Settings::handleTouchForButtonList(std::vector<Button> &buttonList, uint16_t x, uint16_t y) {
    for (auto &button : buttonList) {
        if (button.contains(x, y)) {
            button.press();
            return;
        }
    }
}

void Settings::updateValueDisplay(int value, const char *disabledText, bool isEnabled) {
    // Először törölje az érték kijelző területét
    int centerX = _tft.width() / 2;
    int centerY = 220;
    int clearWidth = 120;
    int clearHeight = 50;

    _tft.fillRoundRect(centerX - clearWidth / 2, centerY - clearHeight / 2, clearWidth, clearHeight, 10, VALUE_DISPLAY_BG);
    _tft.drawRoundRect(centerX - clearWidth / 2, centerY - clearHeight / 2, clearWidth, clearHeight, 10, BUTTON_BORDER_ACTIVE);

    // Új érték megjelenítése függőlegesen és vízszintesen középre igazítva
    _tft.setTextFont(4);
    _tft.setTextSize(2);
    _tft.setTextDatum(MC_DATUM);

    if (isEnabled) {
        _tft.setTextColor(TFT_WHITE);
        _tft.drawString(String(value), centerX, centerY);
    } else {
        _tft.setTextColor(BUTTON_BORDER_DISABLED);
        _tft.drawString(disabledText, centerX, centerY);
    }

    _tft.setTextFont(1); // Alapértelmezett betűtípus visszaállítása
    _tft.setTextSize(1); // Alapértelmezett szövegméret visszaállítása
}

void Settings::drawButtonIcon(int16_t centerX, int16_t centerY, const char *iconType, uint16_t color) {
    _tft.setTextFont(2);
    _tft.setTextColor(TFT_WHITE);
    _tft.setTextDatum(MC_DATUM);

    if (strcmp(iconType, "sun") == 0) {
        // Fényerő/Képernyő ikon (nap)
        _tft.fillCircle(centerX, centerY, 8, TFT_YELLOW);
        // Egyszerű sugarak
        _tft.drawLine(centerX, centerY - 12, centerX, centerY - 16, TFT_YELLOW);
        _tft.drawLine(centerX, centerY + 12, centerX, centerY + 16, TFT_YELLOW);
        _tft.drawLine(centerX - 12, centerY, centerX - 16, centerY, TFT_YELLOW);
        _tft.drawLine(centerX + 12, centerY, centerX + 16, centerY, TFT_YELLOW);
    } else if (strcmp(iconType, "warning") == 0) {
        // Riasztás ikon (figyelmeztető háromszög)
        _tft.fillTriangle(centerX, centerY - 10, centerX - 10, centerY + 6, centerX + 10, centerY + 6, ACCENT_RED);
        _tft.setTextColor(TFT_WHITE);
        _tft.drawString("!", centerX, centerY - 1);
    } else if (strcmp(iconType, "info") == 0) {
        // Infó ikon (i körben)
        _tft.fillCircle(centerX, centerY, 10, BUTTON_BORDER_ACTIVE);
        _tft.setTextColor(TFT_WHITE);
        _tft.drawString("i", centerX, centerY);
    } else if (strcmp(iconType, "minus") == 0) {
        // Mínusz ikon
        _tft.fillRoundRect(centerX - 8, centerY - 2, 16, 4, 2, color);
    } else if (strcmp(iconType, "plus") == 0) {
        // Plusz ikon
        _tft.fillRoundRect(centerX - 8, centerY - 2, 16, 4, 2, color);
        _tft.fillRoundRect(centerX - 2, centerY - 8, 4, 16, 2, color);
    } else if (strcmp(iconType, "back") == 0) {
        // Vissza nyíl ikon
        _tft.fillTriangle(centerX - 8, centerY, centerX + 2, centerY - 6, centerX + 2, centerY + 6, color);
        _tft.fillRoundRect(centerX + 2, centerY - 2, 8, 4, 2, color);
    } else if (strcmp(iconType, "exit") == 0) {
        // Kilépés ikon (X)
        _tft.drawLine(centerX - 6, centerY - 6, centerX + 6, centerY + 6, color);
        _tft.drawLine(centerX + 6, centerY - 6, centerX - 6, centerY + 6, color);
        _tft.drawLine(centerX - 5, centerY - 6, centerX + 7, centerY + 6, color);
        _tft.drawLine(centerX + 5, centerY - 6, centerX - 7, centerY + 6, color);
    } else if (strcmp(iconType, "save") == 0) {
        // Mentés ikon (pipa)
        _tft.drawLine(centerX - 6, centerY, centerX - 2, centerY + 4, color);
        _tft.drawLine(centerX - 2, centerY + 4, centerX + 6, centerY - 4, color);
        _tft.drawLine(centerX - 6, centerY + 1, centerX - 2, centerY + 5, color);
        _tft.drawLine(centerX - 2, centerY + 5, centerX + 6, centerY - 3, color);
    } else if (strcmp(iconType, "toggle") == 0) {
        // Kapcsoló ikon
        _tft.fillRoundRect(centerX - 10, centerY - 4, 20, 8, 4, BUTTON_BORDER_DISABLED);
        _tft.fillCircle(centerX + 6, centerY, 5, color);
    }
}
