#pragma once

#include "ButtonsGroupManager.h"
#include "MessageDialog.h"
#include "UIScreen.h"
#include "ValueChangeDialog.h"

/**
 * @file ScreenTest.h
 * @brief Test képernyő osztály, amely használja a ButtonsGroupManager-t
 */

class ScreenTest : public UIScreen, public ButtonsGroupManager<ScreenTest> {

  public:
    /**
     * @brief TestScreen konstruktor
     * Inicializálja a teszt értékeket.
     * @param tft TFT display referencia
     */
    ScreenTest() : UIScreen(SCREEN_NAME_TEST), _testBool(false), _testInt(50), _testFloat(12.5f) {

        DEBUG("ScreenTest: Constructor called\n");
        layoutComponents();
    }
    virtual ~ScreenTest() = default;

    /**
     * @brief Loop hívás felülírása
     * animációs vagy egyéb saját logika végrehajtására
     * @note Ez a metódus nem hívja meg a gyerek komponensek loop-ját, csak saját logikát tartalmaz.
     */
    virtual void handleOwnLoop() override {}

    /**
     * @brief Kirajzolja a képernyő saját tartalmát
     */
    virtual void drawContent() override {
        // Szöveg középre igazítása
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_COLOR_BACKGROUND);
        tft.setFreeFont(); // Alapértelmezett (kisebb) font
        tft.setTextSize(3);

        // Képernyő cím kirajzolása
        tft.drawString(SCREEN_NAME_TEST, ::SCREEN_W / 2, ::SCREEN_H / 2 - 20);

        // Információs szöveg
        tft.setTextSize(1);
        tft.drawString("ScreenTest for debugging", ::SCREEN_W / 2, ::SCREEN_H / 2 + 20);
    }

  private:
    // Tagváltozók a ValueChangeDialog teszteléséhez
    bool _testBool;
    int _testInt;
    float _testFloat;

    /**
     * @brief UI komponensek létrehozása és elhelyezése
     */
    void layoutComponents() {
        // Előre definiált feliratok a vízszintes gombokhoz (Back gomb hozzáadva az elejére)
        const char *horizontalLabels[] = {"Back", "Msg Ok", "MsgOkCancel", "NestedDlg", "Bool Dlg", "Int Dlg", "Float Dlg"};
        constexpr size_t numHorizontalButtons = ARRAY_ITEM_COUNT(horizontalLabels);

        // Vízszintes gombok tesztelése (alsó sor)
        // Meglévő gombok + új dialógus indító gombok
        std::vector<ButtonGroupDefinition> horizontalButtonDefs;
        for (size_t i = 0; i < numHorizontalButtons; ++i) {
            horizontalButtonDefs.push_back(
                {static_cast<uint8_t>(i + 1), // ID
                 horizontalLabels[i],         // Felirat a tömbből
                 UIButton::ButtonType::Pushable,
                 [this, id = i + 1](const UIButton::ButtonEvent &event) {
                     if (event.state == UIButton::EventButtonState::Clicked) {
                         DEBUG("ScreenTest: Horizontal Button %d ('%s') clicked\n", id, event.label);

                         // Back gomb kezelése
                         if (STREQ(event.label, "Back")) {
                             DEBUG("ScreenTest: Back button clicked, going back to previous screen\n");
                             if (getScreenManager()) {
                                 getScreenManager()->goBack();
                             } else {
                                 DEBUG("ScreenTest: Warning - No screen manager available for back navigation\n");
                             }
                             return;
                         }

                         // Dialógusok indítása az új gombokkal
                         MessageDialog::ButtonsType dialogType = MessageDialog::ButtonsType::Ok;
                         const char *dialogMessage = "Default message";
                         bool showSpecificDialog = true;

                         if (STREQ(event.label, "Msg Ok")) {
                             dialogType = MessageDialog::ButtonsType::Ok;
                             dialogMessage = "This is an OK dialog.";

                         } else if (STREQ(event.label, "MsgOkCancel")) {
                             dialogType = MessageDialog::ButtonsType::OkCancel;
                             dialogMessage = "This is an OK/Cancel dialog.";

                         } else if (STREQ(event.label, "NestedDlg")) {
                             // Nested Dialog Chain
                             DEBUG("Starting nested dialog chain...\n");
                             // Dialog 3 (innermost)
                             // Ez a dialógus bezárul az OK-ra
                             auto showDialog3 = [this]() {
                                 Rect dlg3Bounds(90, 110, 200, 0);
                                 auto dialog3 =
                                     std::make_shared<MessageDialog>(this, "Dialog 3/3", "Final. Click OK.", MessageDialog::ButtonsType::Ok, dlg3Bounds, ColorScheme::defaultScheme(), true /*okClosesDialog=true*/);
                                 dialog3->setDialogCallback([this](UIDialogBase *sender, MessageDialog::DialogResult result) {
                                     if (result == MessageDialog::DialogResult::Accepted) {
                                         DEBUG("Dialog 3 OK. Closing Dialog 3.\n");
                                     }
                                 });
                                 this->showDialog(dialog3);
                             };
                             // Dialog 2
                             // Ennek az "OK" (Next) gombja nem zárja be, csak a callback-et hívja
                             auto showDialog2 = [this, showDialog3]() {
                                 Rect dlg2Bounds(60, 70, 200, 0);
                                 auto dialog2 =
                                     std::make_shared<MessageDialog>(this, "Dialog 2/3", "Next or Cancel.", MessageDialog::ButtonsType::OkCancel, dlg2Bounds, ColorScheme::defaultScheme(), false /*okClosesDialog=false*/);
                                 dialog2->setDialogCallback([this, showDialog3](UIDialogBase *sender, MessageDialog::DialogResult result) {
                                     if (result == MessageDialog::DialogResult::Accepted) {
                                         DEBUG("Dialog 2 Next. Showing Dialog 3.\n");
                                         showDialog3();
                                     } else if (result == MessageDialog::DialogResult::Rejected) {
                                         DEBUG("Dialog 2 Cancel. Closing Dialog 2.\n");
                                         // A MessageDialog "Cancel" gombja automatikusan hívja a close()-t,
                                         // ami az onDialogClosed-ot triggereli.
                                     }
                                 });
                                 this->showDialog(dialog2);
                             };
                             // Dialog 1 (outermost)
                             // Ennek az "OK" (Next) gombja nem zárja be, csak a callback-et hívja
                             Rect dlg1Bounds(30, 30, 200, 0);
                             auto dialog1 =
                                 std::make_shared<MessageDialog>(this, "Dialog 1/3", "Next or Cancel.", MessageDialog::ButtonsType::OkCancel, dlg1Bounds, ColorScheme::defaultScheme(), false /*okClosesDialog=false*/);
                             dialog1->setDialogCallback([this, showDialog2](UIDialogBase *sender, MessageDialog::DialogResult result) {
                                 if (result == MessageDialog::DialogResult::Accepted) {
                                     DEBUG("Dialog 1 Next. Showing Dialog 2.\n");
                                     showDialog2();
                                 } else if (result == MessageDialog::DialogResult::Rejected) {
                                     DEBUG("Dialog 1 Cancel. Closing Dialog 1.\n");
                                 }
                             });
                             this->showDialog(dialog1);
                             return; // Kilépés a showSpecificDialog logikából

                         } else if (STREQ(event.label, "Bool Dlg")) {
                             Rect dlgBounds(-1, -1, 280, 0); // Auto-magasság
                             auto boolDialog = std::make_shared<ValueChangeDialog>(
                                 this, "Boolean Test", "Change boolean value:", &_testBool,
                                 [this](const std::variant<int, float, bool> &newValue) {
                                     if (std::holds_alternative<bool>(newValue)) {
                                         DEBUG("ScreenTest: Boolean value changed to: %s\n", std::get<bool>(newValue) ? "true" : "false");
                                     }
                                 },       // ValueChangeCallback
                                 nullptr, // Explicitly passing nullptr for userDialogCb
                                 dlgBounds);
                             this->showDialog(boolDialog);
                             return;

                         } else if (STREQ(event.label, "Int Dlg")) {
                             Rect dlgBounds(-1, -1, 280, 0); // Auto-magasság
                             auto intDialog = std::make_shared<ValueChangeDialog>(
                                 this, "Integer Test", "Change integer value:", &_testInt, 0, 100, 5,
                                 [this](const std::variant<int, float, bool> &newValue) {
                                     if (std::holds_alternative<int>(newValue)) {
                                         DEBUG("ScreenTest: Integer value changed to: %d\n", std::get<int>(newValue));
                                     }
                                 },       // ValueChangeCallback
                                 nullptr, // Explicitly passing nullptr for userDialogCb
                                 dlgBounds);
                             this->showDialog(intDialog);
                             return;

                         } else if (STREQ(event.label, "Float Dlg")) {
                             Rect dlgBounds(-1, -1, 280, 0); // Auto-magasság
                             auto floatDialog = std::make_shared<ValueChangeDialog>(
                                 this, "Float Test", "Change float value:", &_testFloat, 0.0f, 50.0f, 0.5f,
                                 [this](const std::variant<int, float, bool> &newValue) {
                                     if (std::holds_alternative<float>(newValue)) {
                                         char tempBuffer[16];
                                         Utils::floatToString(std::get<float>(newValue), 2, tempBuffer, sizeof(tempBuffer));
                                         DEBUG("ScreenTest: Float value changed to: %s\n", tempBuffer);
                                     } // ValueChangeCallback
                                 },       // ValueChangeCallback
                                 nullptr, // Explicitly passing nullptr for userDialogCb
                                 dlgBounds);
                             this->showDialog(floatDialog);
                             return; // Kilépés a showSpecificDialog logikából

                         } else {
                             showSpecificDialog = false; // Nem dialógus indító gomb
                         }

                         if (showSpecificDialog) {
                             Rect dialogBounds(-1, -1, 300, 0); // Centered X, auto Y, width 180, auto-height
                             auto dialog = std::make_shared<MessageDialog>(this, "Test Dialog", dialogMessage, dialogType, dialogBounds, ColorScheme::defaultScheme(), true /*okClosesDialog=true alapértelmezetten*/);
                             dialog->setDialogCallback([this, label = event.label](UIDialogBase *sender, MessageDialog::DialogResult result) {
                                 const char *resultStr = "Unknown";
                                 if (result == MessageDialog::DialogResult::Accepted)
                                     resultStr = "Accepted";
                                 else if (result == MessageDialog::DialogResult::Rejected)
                                     resultStr = "Rejected";
                                 else if (result == MessageDialog::DialogResult::Dismissed)
                                     resultStr = "Dismissed";
                                 DEBUG("ScreenTest: Dialog from '%s' closed with: %s\n", label, resultStr);
                             });
                             this->showDialog(dialog);
                         }
                     }
                 },
                 UIButton::ButtonState::Off});
        }

        // A layoutHorizontalButtonGroup a ButtonsGroupManager-ből öröklődik
        layoutHorizontalButtonGroup(horizontalButtonDefs);

        // Előre definiált feliratok a függőleges gombokhoz
        const char *verticalLabels[] = {"VBtn1", "VBtn2", "VBtn3", "VBtn4", "VBtn5", "VBtn6", "VBtn7" /*, "VBtn8", "VBtn9", "VBtn10", "VBtn11", "VBtn12"*/};
        constexpr size_t numVerticalButtons = ARRAY_ITEM_COUNT(verticalLabels);

        // Függőleges gombok tesztelése (jobb oldali oszlop)
        std::vector<ButtonGroupDefinition> verticalButtonDefs;
        for (size_t i = 0; i < numVerticalButtons; ++i) {
            verticalButtonDefs.push_back({
                static_cast<uint8_t>(100 + i + 1), // Eltérő ID tartomány
                verticalLabels[i],                 // Felirat a tömbből
                UIButton::ButtonType::Toggleable,
                [this, id = 100 + i + 1](const UIButton::ButtonEvent &event) {
                    if (event.state == UIButton::EventButtonState::On || event.state == UIButton::EventButtonState::Off) {
                        DEBUG("ScreenTest: Vertical Button %d toggled to %s\n", id, event.state == UIButton::EventButtonState::On ? "ON" : "OFF");
                    }
                },
                (i % 2 == 0) ? UIButton::ButtonState::Off : UIButton::ButtonState::On, // Váltakozó kezdeti állapot
            });
        }

        // Alsó margó növelése a függőleges gombokhoz, hogy elférjenek a vízszintes gombok (akár 3 sorban is)
        int16_t horizontal_margin_bottom_for_group = 5; // A layoutHorizontalButtonGroup alapértelmezett marginBottom-ja
        int16_t horizontal_row_gap = 3;                 // A layoutHorizontalButtonGroup alapértelmezett buttonGap-je (sorok között is ezt használhatja)
        int16_t marginBottomForVerticalLayout = horizontal_margin_bottom_for_group + 3 * UIButton::DEFAULT_BUTTON_HEIGHT + 2 * horizontal_row_gap;

        layoutVerticalButtonGroup(verticalButtonDefs, nullptr, 5, 5, marginBottomForVerticalLayout);
    }
};
