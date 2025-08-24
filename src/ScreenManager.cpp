#include "ScreenManager.h"

#include "ScreenInfo.h"
#include "ScreenMain.h"
#include "ScreenSats.h"
#include "ScreenSetup.h"
#include "ScreenTest.h"

/**
 * @brief Képernyőkezelő osztály konstruktor
 */
void ScreenManager::registerDefaultScreenFactories() {
    // Fő képernyő regisztrálása
    registerScreenFactory(SCREEN_NAME_MAIN, []() { return std::make_shared<ScreenMain>(); });

    // Info és Setup képernyők regisztrálása
    registerScreenFactory(SCREEN_NAME_INFO, []() { return std::make_shared<ScreenInfo>(); });
    registerScreenFactory(SCREEN_NAME_SETUP, []() { return std::make_shared<ScreenSetup>(); });
    registerScreenFactory(SCREEN_NAME_SATS, []() { return std::make_shared<ScreenSats>(); });

    // Teszt képernyők regisztrálása
    registerScreenFactory(SCREEN_NAME_TEST, []() { return std::make_shared<ScreenTest>(); });
    // registerScreenFactory(SCREEN_NAME_EMPTY, []() { return std::make_shared<ScreenEmpty>(); });
}
