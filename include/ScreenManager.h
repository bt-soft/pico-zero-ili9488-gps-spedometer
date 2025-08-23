#ifndef __SCREEN_MANAGER_H
#define __SCREEN_MANAGER_H

#include <functional>
#include <map>
#include <queue>
#include <vector> // Navigációs stack-hez

#include "Config.h"
#include "IScreenManager.h"
#include "UIScreen.h"

// Deferred action struktúra - biztonságos képernyőváltáshoz
struct DeferredAction {
    enum Type { SwitchScreen, GoBack };

    Type type;
    const char *screenName;
    void *params;

    DeferredAction(Type t, const char *name = nullptr, void *p = nullptr) : type(t), screenName(name), params(p) {}
};

// Képernyő factory típus
using ScreenFactory = std::function<std::shared_ptr<UIScreen>()>;

// Képernyőkezelő
class ScreenManager : public IScreenManager {

  private:
    std::map<String, ScreenFactory> screenFactories;
    std::shared_ptr<UIScreen> currentScreen;
    const char *previousScreenName;

    // Navigációs stack - többszintű back navigációhoz
    std::vector<String> navigationStack;

    // Deferred action queue - biztonságos képernyőváltáshoz
    std::queue<DeferredAction> deferredActions;
    bool processingEvents = false;

    void registerDefaultScreenFactories();

  public:
    ScreenManager() : previousScreenName(nullptr) { registerDefaultScreenFactories(); }

    // Aktuális képernyő lekérdezése
    std::shared_ptr<UIScreen> getCurrentScreen() const { return currentScreen; }

    // Előző képernyő neve
    String getPreviousScreenName() const { return previousScreenName; }

    // Képernyő factory regisztrálása
    void registerScreenFactory(const char *screenName, ScreenFactory factory) { screenFactories[screenName] = factory; }

    // Deferred képernyő váltás - biztonságos váltás eseménykezelés közben
    void deferSwitchToScreen(const char *screenName, void *params = nullptr) {
        DEBUG("ScreenManager: Deferring switch to screen '%s'\n", screenName);
        deferredActions.push(DeferredAction(DeferredAction::SwitchScreen, screenName, params));
    }

    // Deferred vissza váltás
    void deferGoBack() {
        DEBUG("ScreenManager: Deferring go back\n");
        deferredActions.push(DeferredAction(DeferredAction::GoBack));
    }

    // Deferred actions feldolgozása - a main loop-ban hívandó
    void processDeferredActions() {

        while (!deferredActions.empty()) {
            const DeferredAction &action = deferredActions.front();

            DEBUG("ScreenManager: Processing deferred action type=%d\n", static_cast<int>(action.type));

            if (action.type == DeferredAction::SwitchScreen) {
                immediateSwitch(action.screenName, action.params);
            } else if (action.type == DeferredAction::GoBack) {
                immediateGoBack();
            }

            deferredActions.pop();
        }
    }

    // Képernyő váltás név alapján - biztonságos verzió - IScreenManager
    bool switchToScreen(const char *screenName, void *params = nullptr) override {
        if (processingEvents) {
            // Eseménykezelés közben - halasztott váltás
            deferSwitchToScreen(screenName, params);
            return true;
        } else {
            // Biztonságos - azonnali váltás
            return immediateSwitch(screenName, params);
        }
    }

    // Azonnali képernyő váltás - csak biztonságos kontextusban hívható
    bool immediateSwitch(const char *screenName, void *params = nullptr, bool isBackNavigation = false) {

        // Ha már ez a képernyő aktív, nem csinálunk semmit
        if (currentScreen && STREQ(currentScreen->getName(), screenName)) {
            return true;
        }

        // Factory keresése
        auto it = screenFactories.find(screenName);
        if (it == screenFactories.end()) {
            DEBUG("ScreenManager: Screen factory not found for '%s'\n", screenName);
            return false;
        }

        // Navigációs stack kezelése KÉPERNYŐVÁLTÁS ELŐTT - csak forward navigációnál
        if (currentScreen && !isBackNavigation) {
            const char *currentName = currentScreen->getName();

            // Normál forward navigáció - jelenlegi képernyő hozzáadása a stackhez
            // (de csak ha nem screensaver-ről váltunk)
            navigationStack.push_back(String(currentName));
            DEBUG("ScreenManager: Added '%s' to navigation stack (size: %d)\n", currentName, navigationStack.size());

        } else if (isBackNavigation) {
            DEBUG("ScreenManager: Back navigation - not adding to stack\n");
        }

        // Jelenlegi képernyő törlése
        if (currentScreen) {
            const char *currentName = currentScreen->getName();

            previousScreenName = currentName;

            currentScreen->deactivate();
            currentScreen.reset(); // Memória felszabadítása
            DEBUG("ScreenManager: Destroyed screen '%s'\n", currentName);
        }

        // TFT display törlése a képernyőváltás előtt
        ::tft.fillScreen(TFT_BLACK);
        DEBUG("ScreenManager: Display cleared for screen switch\n");

        // Új képernyő létrehozása
        currentScreen = it->second();
        if (currentScreen) {
            currentScreen->setScreenManager(this);
            if (params) {
                currentScreen->setParameters(params);
            }
            currentScreen->activate();
            DEBUG("ScreenManager: Created and activated screen '%s'\n", screenName);
            return true;
        } else {
            DEBUG("ScreenManager: Failed to create screen '%s'\n", screenName);
        }
        return false;
    }

    // Vissza az előző képernyőre - biztonságos verzió - IScreenManager
    bool goBack() override {
        if (processingEvents) {
            // Eseménykezelés közben - halasztott váltás
            deferGoBack();
            return true;
        } else {
            // Biztonságos - azonnali váltás
            return immediateGoBack();
        }
    }

    // Azonnali visszaváltás - csak biztonságos kontextusban hívható
    bool immediateGoBack() {

        // Navigációs stack használata a többszintű back navigációhoz
        if (!navigationStack.empty()) {
            String previousScreen = navigationStack.back();
            navigationStack.pop_back();
            DEBUG("ScreenManager: Going back to '%s' from stack (remaining: %d)\n", previousScreen.c_str(), navigationStack.size());
            return immediateSwitch(previousScreen.c_str(), nullptr, true); // isBackNavigation = true
        }

        // Fallback - régi egyszintű viselkedés
        if (previousScreenName != nullptr) {
            DEBUG("ScreenManager: Fallback to old previousScreenName: '%s'\n", previousScreenName);
            return immediateSwitch(previousScreenName, nullptr, true); // isBackNavigation = true
        }

        DEBUG("ScreenManager: No screen to go back to\n");
        return false;
    }

    // Touch esemény kezelése
    bool handleTouch(const TouchEvent &event) {
        if (currentScreen) {
            processingEvents = true;
            bool result = currentScreen->handleTouch(event);
            processingEvents = false;
            return result;
        }
        return false;
    }

    // Loop hívás
    void loop() {

        // Először a halasztott műveletek feldolgozása
        processDeferredActions();

        if (currentScreen) {

            // Csak akkor rajzolunk, ha valóban szükséges
            if (currentScreen->isRedrawNeeded()) {
                currentScreen->draw();
            }

            currentScreen->loop();
        }
    }

    /**
     * segédfüggvény a dialog állapot ellenőrzéséhez
     */
    bool isCurrentScreenDialogActive() override {

        auto currentScreen = this->getCurrentScreen();
        if (currentScreen == nullptr) {
            return false;
        }

        return currentScreen->isDialogActive();
    }
};

#endif // __SCREEN_MANAGER_H