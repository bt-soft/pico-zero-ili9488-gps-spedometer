#ifndef __I_SCREEN_MANAGER_H
#define __I_SCREEN_MANAGER_H

class IScreenManager {

  public:
    virtual bool switchToScreen(const char *screenName, void *params = nullptr) = 0;
    virtual bool goBack() = 0;
    virtual bool isCurrentScreenDialogActive() = 0;
};

#endif
