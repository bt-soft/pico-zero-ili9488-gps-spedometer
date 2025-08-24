#include "ScreenManager.h"

#include "ScreenDebugSetup.h"
#include "ScreenGPSSetup.h"
#include "ScreenInfo.h"
#include "ScreenMain.h"
#include "ScreenSats.h"
#include "ScreenSetup.h"
#include "ScreenSystemSetup.h"
#include "ScreenTFTSetup.h"
#include "ScreenTest.h"

/**
 * @brief Képernyőkezelő osztály konstruktor
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

    // Teszt képernyők regisztrálása
    registerScreenFactory(SCREEN_NAME_TEST, []() { return std::make_shared<ScreenTest>(); });
    // registerScreenFactory(SCREEN_NAME_EMPTY, []() { return std::make_shared<ScreenEmpty>(); });
}
