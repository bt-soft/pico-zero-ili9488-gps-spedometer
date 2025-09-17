#pragma once

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
    uint32_t lastActivityTime;
    uint32_t screenSaverTimeoutMs; // Kiszámolt képernyővédő időtúllépés ms-ben

    // Navigációs stack - többszintű back navigációhoz
    std::vector<String> navigationStack;

    // Screensaver előtti képernyő neve - screensaver visszatéréshez
    String screenBeforeScreenSaver;

    // Deferred action queue - biztonságos képernyőváltáshoz
    std::queue<DeferredAction> deferredActions;
    bool processingEvents = false;

    // Config callback token az automatikus leiratkozáshoz
    size_t configCallbackId;

    void registerDefaultScreenFactories();

  public:
    ScreenManager();
    ~ScreenManager();
    std::shared_ptr<UIScreen> getCurrentScreen() const;
    String getPreviousScreenName() const;
    void registerScreenFactory(const char *screenName, ScreenFactory factory);
    void deferSwitchToScreen(const char *screenName, void *params = nullptr);
    void deferGoBack();
    void processDeferredActions();
    bool switchToScreen(const char *screenName, void *params = nullptr) override;
    bool immediateSwitch(const char *screenName, void *params = nullptr, bool isBackNavigation = false);
    bool goBack() override;
    bool immediateGoBack();
    bool handleTouch(const TouchEvent &event);
    void loop();
    bool isCurrentScreenDialogActive() override;

    /**
     * @brief Callback függvény, amit a Config hív meg változás esetén
     */
    void onConfigChanged();

    /**
     * @brief Megmondja, hogy az aktuális képernyő a screensaver-e
     */
    bool isCurrentScreenScreensaver() const;
};