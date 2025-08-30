#pragma once

class IScreenManager {

  public:
    virtual bool switchToScreen(const char *screenName, void *params = nullptr) = 0;
    virtual bool goBack() = 0;
    virtual bool isCurrentScreenDialogActive() = 0;
};
