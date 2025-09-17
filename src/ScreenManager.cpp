#include "ScreenManager.h"
#include "GpsManager.h"

#include "ScreenDebugSetup.h"
#include "ScreenGPSSetup.h"
#include "ScreenInfo.h"
#include "ScreenMain.h"
#include "ScreenSats.h"
#include "ScreenScreenSaver.h"
#include "ScreenSetup.h"
#include "ScreenSystemSetup.h"
#include "ScreenTFTSetup.h"
#include "ScreenTest.h"

extern GpsManager *gpsManager;

/**
 * @brief Képernyőkezelő osztály konstruktor
 */
ScreenManager::ScreenManager() {
    // Feliratkozás a config változásokra
    configCallbackId = config.registerChangeCallback([this]() { this->onConfigChanged(); });

    lastActivityTime = millis(); // Inicializáljuk az aktivitás időt
    registerDefaultScreenFactories();

    // Kezdeti értékek beállítása
    onConfigChanged();
}

/**
 * @brief Képernyőkezelő osztály destruktor
 */
ScreenManager::~ScreenManager() { config.unregisterCallback(configCallbackId); }

/**
 * @brief Callback függvény, amit a Config hív meg változás esetén
 */
void ScreenManager::onConfigChanged() {
    // DEBUG("ScreenManager::onConfigChanged() - Képernyővédő időtúllépés újraszámolása.\n");
    screenSaverTimeoutMs = config.data.screenSaverTimeout * 60 * 1000;
}

/**
 * @brief Regisztrálja az alapértelmezett képernyőgyárakat
 */
void ScreenManager::registerDefaultScreenFactories() {
    // Fő képernyő regisztrálása
    registerScreenFactory(SCREEN_NAME_MAIN, []() { return std::make_shared<ScreenMain>(); });

    // Információs képernyők regisztrálása
    registerScreenFactory(SCREEN_NAME_INFO, []() { return std::make_shared<ScreenInfo>(); });
    registerScreenFactory(SCREEN_NAME_SATS, []() { return std::make_shared<ScreenSats>(); });

    // Setup képernyők regisztrálása
    registerScreenFactory(SCREEN_NAME_SETUP, []() { return std::make_shared<ScreenSetup>(); });
    registerScreenFactory(SCREEN_NAME_TFT_SETUP, []() { return std::make_shared<ScreenTFTSetup>(); });
    registerScreenFactory(SCREEN_NAME_SYSTEM_SETUP, []() { return std::make_shared<ScreenSystemSetup>(); });
    registerScreenFactory(SCREEN_NAME_GPS_SETUP, []() { return std::make_shared<ScreenGPSSetup>(); });
    registerScreenFactory(SCREEN_NAME_DEBUG_SETUP, []() { return std::make_shared<ScreenDebugSetup>(); });

    // ScrenSaver képernyő regisztrálása
    registerScreenFactory(SCREEN_NAME_SCREENSAVER, []() { return std::make_shared<ScreenScreenSaver>(); });

    // Teszt képernyők regisztrálása
    registerScreenFactory(SCREEN_NAME_TEST, []() { return std::make_shared<ScreenTest>(); });
    // registerScreenFactory(SCREEN_NAME_EMPTY, []() { return std::make_shared<ScreenEmpty>(); });
}

/**
 * @brief Aktuális képernyő lekérése
 */
std::shared_ptr<UIScreen> ScreenManager::getCurrentScreen() const { return currentScreen; }

/**
 * @brief Előző képernyő nevének lekérése
 */
String ScreenManager::getPreviousScreenName() const { return previousScreenName; }

/**
 *  @brief Képernyőgyár regisztrálása egyedi képernyőkhöz
 */
void ScreenManager::registerScreenFactory(const char *screenName, ScreenFactory factory) { screenFactories[screenName] = factory; }

/**
 * @brief Képernyőkezelő fő loop függvénye
 */
void ScreenManager::deferSwitchToScreen(const char *screenName, void *params) { deferredActions.push(DeferredAction(DeferredAction::SwitchScreen, screenName, params)); }

void ScreenManager::deferGoBack() { deferredActions.push(DeferredAction(DeferredAction::GoBack)); }

/**
 * @brief Függvény a Deferred Action Queue feldolgozására (képernyőváltások biztonságos kezelése)
 */
void ScreenManager::processDeferredActions() {
    while (!deferredActions.empty()) {
        const DeferredAction &action = deferredActions.front();
        if (action.type == DeferredAction::SwitchScreen) {
            immediateSwitch(action.screenName, action.params);
        } else if (action.type == DeferredAction::GoBack) {
            immediateGoBack();
        }
        deferredActions.pop();
    }
}

/**
 *  @brief Képernyőváltás egy adott képernyőre
 */
bool ScreenManager::switchToScreen(const char *screenName, void *params) {
    if (processingEvents) {
        deferSwitchToScreen(screenName, params);
        return true;
    } else {
        return immediateSwitch(screenName, params);
    }
}

/**
 * @brief Azonnali képernyőváltás
 */
bool ScreenManager::immediateSwitch(const char *screenName, void *params, bool isBackNavigation) {

    if (currentScreen && STREQ(currentScreen->getName(), screenName)) {
        return true;
    }

    auto it = screenFactories.find(screenName);
    if (it == screenFactories.end()) {
        DEBUG("ScreenManager: Screen factory nem található '%s'\n", screenName);
        return false;
    }

    if (currentScreen && !isBackNavigation) {
        const char *currentName = currentScreen->getName();
        if (STREQ(screenName, SCREEN_NAME_SCREENSAVER)) {
            screenBeforeScreenSaver = String(currentName);
        } else if (!STREQ(currentName, SCREEN_NAME_SCREENSAVER)) {
            navigationStack.push_back(String(currentName));
        }
    }

    if (currentScreen) {
        const char *currentName = currentScreen->getName();
        if (!STREQ(screenName, SCREEN_NAME_SCREENSAVER)) {
            previousScreenName = currentName;
        }
        currentScreen->deactivate();
        currentScreen.reset();
    }

    ::tft.fillScreen(TFT_BLACK);
    currentScreen = it->second();
    if (currentScreen) {
        currentScreen->setScreenManager(this);
        if (params) {
            currentScreen->setParameters(params);
        }
        // Aktivitás idő frissítése minden képernyőváltásnál (kivéve screensaver aktiválás)
        if (!STREQ(screenName, SCREEN_NAME_SCREENSAVER)) {
            lastActivityTime = millis();
        }
        currentScreen->activate();
        return true;
    } else {
        DEBUG("ScreenManager: Képernyő létrehozása nem sikerült '%s'\n", screenName);
    }

    return false;
}

/**
 * @brief Visszalépés az előző képernyőre
 */
bool ScreenManager::goBack() {
    if (processingEvents) {
        deferGoBack();
        return true;
    } else {
        return immediateGoBack();
    }
}

/**
 * @brief Visszalépés az előző képernyőre
 */
bool ScreenManager::immediateGoBack() {

    if (currentScreen && STREQ(currentScreen->getName(), SCREEN_NAME_SCREENSAVER)) {
        if (!screenBeforeScreenSaver.isEmpty()) {
            String targetScreen = screenBeforeScreenSaver;
            screenBeforeScreenSaver = String();
            // Screensaver-ből való kilépéskor frissítjük az aktivitás időt
            lastActivityTime = millis();
            return immediateSwitch(targetScreen.c_str(), nullptr, true);
        }
    }

    if (!navigationStack.empty()) {
        String previousScreen = navigationStack.back();
        navigationStack.pop_back();
        return immediateSwitch(previousScreen.c_str(), nullptr, true);
    }

    if (previousScreenName != nullptr) {
        return immediateSwitch(previousScreenName, nullptr, true);
    }
    return false;
}

/**
 * @brief Touch esemény kezelése
 */
bool ScreenManager::handleTouch(const TouchEvent &event) {
    if (currentScreen) {
        if (!STREQ(currentScreen->getName(), SCREEN_NAME_SCREENSAVER)) {
            lastActivityTime = millis();
        }
        processingEvents = true;
        bool result = currentScreen->handleTouch(event);
        processingEvents = false;
        return result;
    }
    return false;
}

/**
 * @brief Képernyőkezelő fő loop függvénye
 */
void ScreenManager::loop() {

    // Feldolgozzuk a függőben lévő képernyőváltásokat
    processDeferredActions();

    if (currentScreen) {
        // GPS sebesség ellenőrzése a screensaver logikához
        float currentSpeed = 0.0f;
        bool hasValidSpeed = false;

        if (gpsManager && gpsManager->getSpeed().isValid()) {
            currentSpeed = gpsManager->getSpeed().kmph();
            hasValidSpeed = true;
        }

        // Ha aktív a screensaver és mozog a jármű (3km/h), deaktiváljuk
        if (isCurrentScreenScreensaver() && hasValidSpeed && currentSpeed > 3.0f) {
            goBack(); // Visszatérés az előző képernyőre
        } else if (screenSaverTimeoutMs > 0 && !isCurrentScreenScreensaver()) {
            // Screensaver aktiválás csak akkor, ha a jármű áll

            // Ha mozog a jármű, reseteljük az aktivitás időt (ne aktiválódjon a screensaver)
            if (hasValidSpeed && currentSpeed > 0.1f) {
                lastActivityTime = millis();
            } else if (millis() - lastActivityTime > screenSaverTimeoutMs) { // Csak álló helyzetben aktiválódjon a screensaver

                switchToScreen(SCREEN_NAME_SCREENSAVER);
            }
        }

        if (currentScreen->isRedrawNeeded()) {
            currentScreen->draw();
        }
        currentScreen->loop();
    }
}

/**
 * @brief Ellenőrzi, hogy van-e aktív dialógus az aktuális képernyőn
 */
bool ScreenManager::isCurrentScreenDialogActive() {
    auto currentScreen = this->getCurrentScreen();
    if (currentScreen == nullptr) {
        return false;
    }
    return currentScreen->isDialogActive();
}

/**
 * @brief Megmondja, hogy az aktuális képernyő a screensaver-e
 */
bool ScreenManager::isCurrentScreenScreensaver() const {
    //
    return currentScreen && STREQ(currentScreen->getName(), SCREEN_NAME_SCREENSAVER);
}