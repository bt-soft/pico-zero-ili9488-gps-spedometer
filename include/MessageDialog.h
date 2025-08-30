#pragma once

#include "ButtonsGroupManager.h" // Hozzáadva
#include "UIDialogBase.h"

class MessageDialog : public UIDialogBase, public ButtonsGroupManager<MessageDialog> {
  public:
    enum class ButtonsType { Ok, OkCancel, YesNo, YesNoCancel, UserDefined };

  protected:
    const char *message;
    ButtonsType buttonsType;
    bool _okClosesDialog; // Új flag: az OK/Yes gomb bezárja-e a dialógust

    std::vector<std::shared_ptr<UIButton>> _buttonsList; // Megmarad a létrehozott gombok tárolására és eltávolítására
    std::vector<ButtonGroupDefinition> _buttonDefs;      // Gombdefiníciók tárolására

    // For UserDefined buttons
    const char *const *_userOptions = nullptr;
    uint8_t _numUserOptions = 0;
    int _clickedUserButtonIndex = -1;              // A _userOptions tömbben a kattintott gomb indexe
    const char *_clickedUserButtonLabel = nullptr; // A kattintott gomb felirata
    DialogCallback _userDialogCallback = nullptr;  // Felhasználó által megadott DialogCallback    // A createDialogContent most csak a _buttonDefs-et készíti elő
    virtual void createDialogContent() override;
    virtual void layoutDialogContent() override;

    // Felülírjuk a drawSelf-et az üzenet kirajzolásához
    virtual void drawSelf() override;

  public:
    /**
     * @brief Visszaadja az összes dialógus gombot (kivéve a bezáró X gombot).
     * @return A gombok listája shared_ptr-ekben.
     * @details Felülírja a UIDialogBase virtuális metódusát, hogy visszaadja
     * az összes MessageDialog gombját a _buttonsList-ből.
     */
    virtual std::vector<std::shared_ptr<UIButton>> getButtonsList() const override { return _buttonsList; }

    /**
     * @brief MessageDialog konstruktor alapértelmezett gombokkal (OK, OK/Cancel, Yes/No, Yes/No/Cancel).
     *
     * @param parentScreen A szülő UIScreen.
     * @param title A dialógus címe.
     * @param message Az üzenet szövege.
     * @param buttonsType A gombok típusa (alapértelmezett: Ok).
     * @param bounds A dialógus határai (pozíció és méret). Opcionális, alapértelmezett: automatikus méret és középre igazítás.
     * @param cs Színséma. Opcionális, alapértelmezett: ColorScheme::defaultScheme().
     * @param okClosesDialog Meghatározza, hogy az "OK" típusú gombok bezárják-e a dialógust. Opcionális, alapértelmezett: true.
     */
    MessageDialog(UIScreen *parentScreen, const char *title, const char *message, ButtonsType buttonsType = ButtonsType::Ok, const Rect &bounds = {-1, -1, 0, 0}, const ColorScheme &cs = ColorScheme::defaultScheme(),
                  bool okClosesDialog = true);

    /**
     * @brief MessageDialog konstruktor felhasználó által definiált gombokkal.
     *
     * @param parentScreen A szülő UIScreen.
     * @param title A dialógus címe.
     * @param message Az üzenet szövege.
     * @param options Gombok feliratainak tömbje (const char* []).
     * @param numOptions A gombok száma (options tömb mérete).
     * @param userDialogCb Dialógus lezárásakor hívandó callback (OK/Cancel után). Opcionális.
     * @param bounds A dialógus határai (pozíció és méret). Opcionális, alapértelmezett: automatikus méret és középre igazítás.
     * @param cs Színséma. Opcionális, alapértelmezett: ColorScheme::defaultScheme().
     * @param okClosesDialog Meghatározza, hogy az "OK" típusú gombok bezárják-e a dialógust. UserDefined esetén kevésbé releváns, mert minden gombválasztás bezárja. Opcionális,
     * alapértelmezett: true.
     */
    MessageDialog(UIScreen *parentScreen, const char *title, const char *message, const char *const *options, uint8_t numOptions, DialogCallback userDialogCb = nullptr, const Rect &bounds = {-1, -1, 0, 0},
                  const ColorScheme &cs = ColorScheme::defaultScheme(), bool okClosesDialog = true);
    virtual ~MessageDialog() override = default;

    int getClickedUserButtonIndex() const { return _clickedUserButtonIndex; }
    const char *getClickedUserButtonLabel() const { return _clickedUserButtonLabel; }

    /**
     * @brief Visszaadja az OK/Yes/első gombot, ha létezik.
     * @return Az OK/Yes/első gomb shared_ptr-je, vagy nullptr ha nincs gomb.
     * @details Helper metódus az első (általában OK/Yes) gomb egyszerű eléréséhez.
     */
    std::shared_ptr<UIButton> getOkButton() const { return (_buttonsList.empty()) ? nullptr : _buttonsList[0]; }
};
