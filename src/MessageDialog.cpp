
#include <vector>

#include "MessageDialog.h"

/**
 * @brief MessageDialog konstruktor
 * @param parentScreen A szülő UIScreen, amely megjeleníti ezt a dialógust
 * @param title A dialógus címe (nullptr, ha nincs cím)
 * @param message Az üzenet szövege, amely megjelenik a dialógusban
 * @param buttonsType A gombok típusa (Ok, OkCancel, YesNo, YesNoCancel)
 * @param bounds A dialógus kezdeti határai (pozíció és méret)
 * @param cs A dialógus színpalettája (alapértelmezett ColorScheme)
 * @details A dialógus automatikusan méreteződik, ha a bounds.height 0.
 *          A gombok típusa meghatározza, hogy milyen gombok jelennek meg a dialógusban.
 */
MessageDialog::MessageDialog(UIScreen *parentScreen, const char *title, const char *message, ButtonsType buttonsType, const Rect &initialInputBounds, const ColorScheme &cs, bool okClosesDialog)
    : UIDialogBase(parentScreen, title, initialInputBounds, cs), message(message), buttonsType(buttonsType), _okClosesDialog(okClosesDialog) {

    // Dialógus tartalmának létrehozása és elrendezése
    createDialogContent(); // Előkészíti a _buttonDefs-et

    Rect currentBoundsAfterBase = this->bounds; // A bounds, amit az UIDialogBase konstruktora beállított
    Rect finalDialogBounds = currentBoundsAfterBase;
    bool boundsHaveChanged = false;

    // Automatikus magasság számítása, ha bounds.height == 0 volt az inputon
    // A UIDialogBase konstruktora már adhatott neki egy alapértelmezett magasságot.
    // Itt finomítjuk az üzenet és a gombok alapján.
    if (initialInputBounds.height == 0) {          // Az eredeti kérés volt auto-magasság
        tft.setFreeFont(&FreeSansBold9pt7b);       // Üzenet fontja a magasság kalkulációhoz
        tft.setTextSize(1);                        // Üzenet szövegmérete a magasság kalkulációhoz
        int16_t textHeight = tft.fontHeight() * 2; // Durva becslés 2 sorra (állítható)
        // TODO: Pontosabb szövegmagasság számítás a layoutDialogContent-ben
        uint16_t requiredHeight = getHeaderHeight() + PADDING + textHeight + PADDING + UIButton::DEFAULT_BUTTON_HEIGHT + (2 * PADDING); // Megduplázott alsó PADDING
        if (finalDialogBounds.height < requiredHeight) {
            finalDialogBounds.height = requiredHeight;
            boundsHaveChanged = true;
        }

        tft.setFreeFont(); // Visszaállítás alapértelmezett fontra
    }

    // Középre igazítás, ha x vagy y -1 volt az inputon
    if (initialInputBounds.x == -1) {
        finalDialogBounds.x = (SCREEN_W - finalDialogBounds.width) / 2;
        boundsHaveChanged = true;
    }
    if (initialInputBounds.y == -1) {
        finalDialogBounds.y = (SCREEN_H - finalDialogBounds.height) / 2;
        boundsHaveChanged = true;
    }

    if (boundsHaveChanged) {
        // UIComponent::setBounds-et hívjuk, hogy a this->bounds frissüljön, és a dialógus újrarajzolásra legyen jelölve.
        // Ez nem hívja meg automatikusan a layoutDialogContent-et.
        UIComponent::setBounds(finalDialogBounds);
    }

    // A dialógus tartalmát (gombokat) mindig elrendezzük a véglegesített határok alapján.
    layoutDialogContent(); // Ez létrehozza és hozzáadja a gombokat, és markForRedraw-t hív.
    // Az UIComponent::setBounds már beállította a markForRedraw-t a dialógusra, ha a boundsHaveChanged igaz volt.
    // Ha nem, akkor a layoutDialogContent() hívja. Dupla hívás nem probléma.

    // Belső DialogCallback beállítása, ami meghívja a _userDialogCallback-et, ha van.
    // Ez a UIDialogBase::callback-et állítja be.
    UIDialogBase::setDialogCallback([this](UIDialogBase *sender, DialogResult result) {
        if (_userDialogCallback) {
            _userDialogCallback(sender, result);
        }
    });
}

/**
 * @brief MessageDialog konstruktor felhasználó által definiált gombokkal.
 *
 * @param parentScreen A szülő UIScreen.
 * @param title A dialógus címe.
 * @param message Az üzenet szövege.
 * @param options Gombok feliratainak tömbje (const char* []).
 * @param numOptions A gombok száma (options tömb mérete).
 * @param userDialogCb Dialógus lezárásakor hívandó callback.
 * @param ctorInputBounds A dialógus határai (pozíció és méret).
 * @param cs Színséma.
 * @param okClosesDialog Meghatározza, hogy az "OK" típusú gombok bezárják-e a dialógust.
 */
MessageDialog::MessageDialog(UIScreen *parentScreen, const char *title, const char *message, const char *const *options, uint8_t numOptions, DialogCallback userDialogCb, const Rect &ctorInputBounds,
                             const ColorScheme &cs, bool okClosesDialog)
    : UIDialogBase(parentScreen, title, ctorInputBounds, cs), // UIDialogBase kezeli a kezdeti x,y,w,h alapértelmezéseket
      message(message), buttonsType(ButtonsType::UserDefined), _okClosesDialog(okClosesDialog), _userOptions(options), _numUserOptions(numOptions), _userDialogCallback(userDialogCb) {

    // Dialógus tartalmának létrehozása és elrendezése
    createDialogContent(); // Előkészíti a _buttonDefs-et

    // A UIDialogBase beállította a this->bounds értékét. Most, ha a ctorInputBounds.height 0 volt,
    // finomítjuk a magasságot a MessageDialog tartalma alapján.
    if (ctorInputBounds.height == 0) {
        Rect refinedBounds = this->bounds; // Azzal kezdünk, amit a UIDialogBase kalkulált

        tft.setFreeFont(&FreeSansBold9pt7b);
        tft.setTextSize(1);
        int16_t textHeight = tft.fontHeight() * 2; // Becslés az üzenet területére

        uint16_t buttonAreaHeight = 0;
        if (_numUserOptions > 0) {                                                      // Csak akkor vesszük figyelembe a gombok magasságát, ha vannak gombok
            buttonAreaHeight = UIButton::DEFAULT_BUTTON_HEIGHT + UIDialogBase::PADDING; // Egy sor gomb magassága + padding
        }

        uint16_t contentHeight = UIDialogBase::PADDING + textHeight + UIDialogBase::PADDING + buttonAreaHeight;
        uint16_t requiredTotalHeight = getHeaderHeight() + contentHeight + UIDialogBase::PADDING;

        if (refinedBounds.height < requiredTotalHeight) {
            refinedBounds.height = requiredTotalHeight;
            // Ha a magasság változott, és az eredeti Y középre igazítást kért, újra középre igazítjuk az Y-t.
            if (ctorInputBounds.y == -1) {
                refinedBounds.y = (SCREEN_H - refinedBounds.height) / 2;
            }
            UIComponent::setBounds(refinedBounds); // Frissítjük a határokat és újrarajzolásra jelöljük
        }
        tft.setFreeFont(); // Betűtípus visszaállítása
    }

    layoutDialogContent();

    // Belső DialogCallback beállítása, ami meghívja a _userDialogCallback-et, ha van.
    // Ez a UIDialogBase::callback-et állítja be.
    UIDialogBase::setDialogCallback([this](UIDialogBase *sender, DialogResult result) {
        if (_userDialogCallback) {
            _userDialogCallback(sender, result);
        }
    });
}

/**
 * @brief Létrehozza a dialógus tartalmát, beleértve a gombokat.
 * @details A gombok típusa alapján hozza létre a megfelelő gombokat és azok eseménykezelőit.
 */
void MessageDialog::createDialogContent() {
    _buttonDefs.clear();
    uint8_t buttonIdCounter = 1;

    switch (buttonsType) {
        case ButtonsType::Ok:
            _buttonDefs.push_back({buttonIdCounter++, "OK", UIButton::ButtonType::Pushable,
                                   [this](const UIButton::ButtonEvent &event) {
                                       if (event.state == UIButton::EventButtonState::Clicked) {
                                           if (_okClosesDialog) {
                                               close(DialogResult::Accepted);
                                           } else {
                                               if (this->callback) { // UIDialogBase::callback
                                                   this->callback(this, DialogResult::Accepted);
                                               }
                                           }
                                       }
                                   },
                                   UIButton::ButtonState::Off, 0, UIButton::DEFAULT_BUTTON_HEIGHT});
            break;
        case ButtonsType::OkCancel:
            _buttonDefs.push_back({buttonIdCounter++, "OK", UIButton::ButtonType::Pushable,
                                   [this](const UIButton::ButtonEvent &event) {
                                       if (event.state == UIButton::EventButtonState::Clicked) {
                                           if (_okClosesDialog) {
                                               close(DialogResult::Accepted);
                                           } else {
                                               if (this->callback) {
                                                   this->callback(this, DialogResult::Accepted);
                                               }
                                           }
                                       }
                                   },
                                   UIButton::ButtonState::Off, 0, UIButton::DEFAULT_BUTTON_HEIGHT});
            _buttonDefs.push_back({buttonIdCounter++, "Cancel", UIButton::ButtonType::Pushable,
                                   [this](const UIButton::ButtonEvent &event) {
                                       if (event.state == UIButton::EventButtonState::Clicked)
                                           close(DialogResult::Rejected);
                                   },
                                   UIButton::ButtonState::Off, 0, UIButton::DEFAULT_BUTTON_HEIGHT});
            break;
        case ButtonsType::YesNo:
            _buttonDefs.push_back({buttonIdCounter++, "Yes", UIButton::ButtonType::Pushable,
                                   [this](const UIButton::ButtonEvent &event) {
                                       if (event.state == UIButton::EventButtonState::Clicked) {
                                           if (_okClosesDialog) {
                                               close(DialogResult::Accepted);
                                           } else {
                                               if (this->callback) {
                                                   this->callback(this, DialogResult::Accepted);
                                               }
                                           }
                                       }
                                   },
                                   UIButton::ButtonState::Off, 0, UIButton::DEFAULT_BUTTON_HEIGHT});
            _buttonDefs.push_back({buttonIdCounter++, "No", UIButton::ButtonType::Pushable,
                                   [this](const UIButton::ButtonEvent &event) {
                                       if (event.state == UIButton::EventButtonState::Clicked)
                                           close(DialogResult::Rejected);
                                   },
                                   UIButton::ButtonState::Off, 0, UIButton::DEFAULT_BUTTON_HEIGHT});
            break;
        case ButtonsType::YesNoCancel:
            _buttonDefs.push_back({buttonIdCounter++, "Yes", UIButton::ButtonType::Pushable,
                                   [this](const UIButton::ButtonEvent &event) {
                                       if (event.state == UIButton::EventButtonState::Clicked) {
                                           if (_okClosesDialog) {
                                               close(DialogResult::Accepted);
                                           } else {
                                               if (this->callback) {
                                                   this->callback(this, DialogResult::Accepted);
                                               }
                                           }
                                       }
                                   },
                                   UIButton::ButtonState::Off, 0, UIButton::DEFAULT_BUTTON_HEIGHT});
            _buttonDefs.push_back({buttonIdCounter++, "No", UIButton::ButtonType::Pushable,
                                   [this](const UIButton::ButtonEvent &event) {
                                       if (event.state == UIButton::EventButtonState::Clicked)
                                           close(DialogResult::Rejected);
                                   },
                                   UIButton::ButtonState::Off, 0, UIButton::DEFAULT_BUTTON_HEIGHT});
            _buttonDefs.push_back({buttonIdCounter++, "Cancel", UIButton::ButtonType::Pushable,
                                   [this](const UIButton::ButtonEvent &event) {
                                       if (event.state == UIButton::EventButtonState::Clicked)
                                           close(DialogResult::Dismissed);
                                   },
                                   UIButton::ButtonState::Off, 0, UIButton::DEFAULT_BUTTON_HEIGHT});
            break;

        case ButtonsType::UserDefined:
            if (_userOptions && _numUserOptions > 0) {
                for (uint8_t i = 0; i < _numUserOptions; ++i) {
                    _buttonDefs.push_back({static_cast<uint8_t>(buttonIdCounter + i), _userOptions[i], UIButton::ButtonType::Pushable,
                                           [this, index = i, label = _userOptions[i]](const UIButton::ButtonEvent &event) {
                                               if (event.state == UIButton::EventButtonState::Clicked) {
                                                   _clickedUserButtonIndex = index;
                                                   _clickedUserButtonLabel = label;
                                                   // UserDefined gomb mindig bezárja a dialógust Accepted eredménnyel
                                                   // A close() metódus fogja meghívni a UIDialogBase::callback-et,
                                                   // ami pedig a mi belsőleg beállított lambdánkon keresztül
                                                   // meghívja a _userDialogCallback-et, ha az létezik.
                                                   close(DialogResult::Accepted);
                                               }
                                           },
                                           UIButton::ButtonState::Off, 0, UIButton::DEFAULT_BUTTON_HEIGHT});
                }
            }
            break;
    }
}

/**
 * @brief Elrendezi a dialógus gombjait a megadott elrendezési szabályok szerint.
 */
void MessageDialog::layoutDialogContent() {
    // Korábbi gombok eltávolítása
    for (const auto &btn : _buttonsList) {
        removeChild(btn);
    }
    _buttonsList.clear();

    if (_buttonDefs.empty()) {
        markForRedraw();
        return;
    }

    // uint16_t numButtons = _buttonDefs.size(); // Erre már nincs szükség
    uint16_t buttonHeight = UIButton::DEFAULT_BUTTON_HEIGHT; // Gomb magassága

    // Frissítjük a _buttonDefs-ben a szélességeket
    for (auto &def : _buttonDefs) {
        // def.width = 0; // A createDialogContent-ben már 0-ra van állítva,
        // jelezve az auto-méretezést a ButtonsGroupManager számára.
        def.height = buttonHeight; // Biztosítjuk a magasságot is
    }

    // Margók kiszámítása a ButtonsGroupManager számára (képernyő-relatív)
    // A gombok a dialógus alján lesznek.
    int16_t manager_marginLeft = bounds.x + UIDialogBase::PADDING;
    int16_t manager_marginRight = SCREEN_W - (bounds.x + bounds.width - UIDialogBase::PADDING);
    // A marginBottom azt jelenti, hogy a gombok alja milyen messze van a képernyő aljától.
    int16_t manager_marginBottom = SCREEN_H - (bounds.y + bounds.height - (2 * UIDialogBase::PADDING)); // Megduplázott PADDING itt is

    // Gombok elrendezése a ButtonsGroupManager segítségével
    layoutHorizontalButtonGroup(_buttonDefs, &_buttonsList, manager_marginLeft, manager_marginRight, manager_marginBottom,
                                UIButton::DEFAULT_BUTTON_WIDTH, // defaultButtonWidthRef (ha def.width=0 lenne)
                                buttonHeight,                   // defaultButtonHeightRef
                                UIDialogBase::PADDING,          // rowGap (egy sor esetén nem releváns)
                                UIDialogBase::BUTTONS_GAP,      // buttonGap (gombok közötti rés)
                                true                            // centerHorizontally = true
    );

    markForRedraw();
}

/**
 * @brief Rajzolja a dialógus hátterét, fejlécét és az üzenetet.
 * @details A dialógus háttere és kerete rajzolódik, majd az üzenet szövege jelenik meg a középső területen.
 */
void MessageDialog::drawSelf() {
    UIDialogBase::drawSelf(); // Alap dialógus keret és fejléc rajzolása

    if (message) {
        tft.setTextSize(1);                                     // A FreeSansBold9pt7b natív mérete
        tft.setFreeFont(&FreeSansBold9pt7b);                    // Nagyobb, vastagabb font az üzenetnek
        tft.setTextColor(colors.foreground, colors.background); // Dialógus színeit használva

        uint16_t headerH = getHeaderHeight();
        // Hozzávetőleges magasság a gomb területének
        Rect textArea;
        textArea.x = bounds.x + UIDialogBase::PADDING + 2; // Kis extra margó
        textArea.y = bounds.y + headerH + UIDialogBase::PADDING;
        textArea.width = bounds.width - (2 * (UIDialogBase::PADDING + 2));                                         // Szélességben is figyelembe vesszük a 2px extra margót
        textArea.height = bounds.height - headerH - UIButton::DEFAULT_BUTTON_HEIGHT - (4 * UIDialogBase::PADDING); // Header, PADDING_alatta, text, PADDING_alatta, gombok, 2*PADDING_alatta

        if (textArea.width > 0 && textArea.height > 0) {
            // Többsoros szöveg kezelése - sorok felosztása \n karakterek alapján
            // Egyszerű char* feldolgozás String helyett
            std::vector<String> lines;

            // Másolatot készítünk a message-ről, hogy módosíthatóvá tegyük
            size_t messageLen = strlen(message);
            char *messageCopy = new char[messageLen + 1];
            strcpy(messageCopy, message);

            // Sorok felosztása \n karakterek alapján
            char *lineStart = messageCopy;
            char *lineEnd = strchr(lineStart, '\n');

            while (lineEnd != nullptr) {
                *lineEnd = '\0'; // Null terminátor a sor végére
                lines.push_back(String(lineStart));
                lineStart = lineEnd + 1;
                lineEnd = strchr(lineStart, '\n');
            }

            // Utolsó sor hozzáadása (vagy az egyetlen sor, ha nincs \n)
            if (*lineStart != '\0') {
                lines.push_back(String(lineStart));
            }

            delete[] messageCopy;

            // Sorok renderelése
            int16_t lineHeight = tft.fontHeight();
            int16_t totalTextHeight = lines.size() * lineHeight;
            int16_t startY = textArea.y + (textArea.height - totalTextHeight) / 2;

            // Ha a szöveg túl magas, kezdjük a terület tetejéről
            if (totalTextHeight > textArea.height) {
                startY = textArea.y;
            }

            tft.setTextDatum(TC_DATUM); // Top-Center
            for (size_t i = 0; i < lines.size(); i++) {
                int16_t lineY = startY + (i * lineHeight);
                // Csak akkor rajzoljuk ki a sort, ha még a látható területen belül van
                if (lineY + lineHeight <= textArea.y + textArea.height) {
                    tft.drawString(lines[i], textArea.x + textArea.width / 2, lineY);
                }
            }
        }
    }
}