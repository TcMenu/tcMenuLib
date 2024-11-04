/**
 * This example shows how to create scroll choices, lists, and dynamically add items to both dialogs and existing menus
 * at runtime.
 *
 * Test environment: Seeed MG126 - Matrix Keyboard, Adafruit 128x128 display, rotary encoder. However, it is easy to
 * apply these concepts to any other menu arrangements.
 *
 * Although it's set up to run on SAMD there is no reason it could not be easily reconfigured for any
 * other board. This is one of the biggest advantages of this framework, moving boards is easier.
 *
 * Getting started: https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/tcmenu-overview-quick-start/
 * Dialogs: https://tcmenu.github.io/documentation/arduino-libraries//tc-menu/rendering-with-tcmenu-lcd-tft-oled/
 * MenuManager: https://tcmenu.github.io/documentation/arduino-libraries//tc-menu/menumanager-and-iteration/
 */

#include "generated/dynamicMenuItems_menu.h"
#include <BaseDialog.h>
#include <IoAbstractionWire.h>
#include <graphics/RuntimeTitleMenuItem.h>
#include <tcMenuVersion.h>
#include <IoLogging.h>
#include <tcUtil.h>

//
// ScrollChoice menu items when using data in RAM mode reference a fixed width array in your code, these will be
// created by the code generator if needed. Genreally they are an array of char large enough to hold all items.
// In this case they are pizza toppings, each zero terminated.
// https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
//                       0123456789 0123456789 0123456789 0123456789 0123456789 0123456789
char pizzaToppings[] = {"Peperoni\0 Onions\0   Olives\0   Sweetcorn\0Mushrooms\0Peppers\0  "};

//
// We now create some menu items that we manually add to the oven menu during initialisation. We set the sub menu child
// to be the first of the two menu items, which are provided in a linked list. Notice that these are completely created
// RAM, and not program memory: https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/
//
TextMenuItem menuTextExtra(newAnyMenuInfo("Text", nextRandomId(), 0xffff, NO_CALLBACK), "hello", 8, nullptr, INFO_LOCATION_RAM);
BooleanMenuItem menuOvenFull(newBooleanMenuInfoP("Start Oven", nextRandomId(), 0xffff, NO_CALLBACK, NAMING_YES_NO), false, &menuTextExtra, INFO_LOCATION_RAM);
AnyMenuInfo minfoOvenPower = { "Start Oven", nextRandomId(), 0xffff, 255, NO_CALLBACK};
ActionMenuItem menuOvenPower(&minfoOvenPower, &menuOvenFull, INFO_LOCATION_RAM);
AnalogMenuItem menuOvenTempItem(newAnalogMenuInfo("Oven Temp", nextRandomId(), 0xffff, NO_CALLBACK, 255, 0, 1, "C"), 0, &menuOvenPower, INFO_LOCATION_RAM);

//
// Here's a few examples of how you can capture the state of various actions taking place in the menu manager, such as
// actionable menu items being activated, editing starting and ending on editable items, and changes in tree structure.
// We create an instance of this class and then register it with menuMgr further down.
// https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menumanager-and-iteration/
//
class MyMenuManagerObserver : public MenuManagerObserver {
private:
    bool allowDialogBack = false;
public:
    void setAllowDialogBackAction(bool allow) {
        allowDialogBack = allow;
    }

    // called when the structure of the menu has completely changed, IE menu items added or removed from the tree
    void structureHasChanged() override {
        serdebugF("Structure of menu tree has changed, IE item added or removed!");
    }

    // editing is about to start on an item, or a submenu is about to be shown, return true to show the menu, false
    // to prevent the item from being edited or submenu shown
    bool menuEditStarting(MenuItem *item) override {
        serdebugF2("Item about to be actioned/editing", item->getId());

        if(item->getId() == menuList.getId() && menuMgr.getCurrentMenu() == item) {
            // If we get here, list is on display, check if the back item is presented.
            auto* list = reinterpret_cast<ListRuntimeMenuItem*>(item);
            serdebugF2("List item press on ", list->getActiveIndex());
            return list->getActiveIndex() != 0;
        }

        auto backMenuId = reinterpret_cast<MenuBasedDialog*>(renderer.getDialog())->getBackMenuItemId();

        if(item->getId() == backMenuId || item->getId() == menuDialogsBlockedBool.getId() ||
                item->getId() == menuDialogsBlockedSub.getId() || item->getId() == menuDialogsBlockedAction.getId()) {
            // Here we block certain actions for the back menu item of the dialog, or a few chosen menu items that
            // we want to control using allowDialogBack variable.
            return allowDialogBack;
        }
        return true; // otherwise, default to allowing all actions.
    }

    // called when the menu has finished editing or a menu is coming off the display. Note that in the special case
    // of a submenu being shown, this will be after the new `menuEditStarted` because that could be prevented by
    // returning false.
    void menuEditEnded(MenuItem *item) override {
        serdebugF2("Edit has completed for ID ", item->getId());
    }

    // called whenever the active item changes
    void activeItemHasChanged(MenuItem* newActive) override {
        serdebugF2("The active item changed to ", newActive->getId())
    }
} myMgrObserver;

//
// these are used by the list rendering function further down, as the items to put into the list. The count constant is
// also used to set up the initial list size during setup.
//
const char* listElements[] = {
        "Beans",
        "Tomatoes",
        "Rice",
        "Pasta",
        "Coke",
        "Oranges",
        "Apples",
};
const int numListItems = 7;

//
// Here we add some dynamic items at runtime to the oven menu.
//
void prepareOvenMenuAtRuntime() {
    // as we declared these two items as not in progmem (last menu item parameter is false), we can adjust these items at runtime.
    minfoOvenPower.maxValue = 300;

    // here we add an entire child menu after the back option on the oven menu
    menuMgr.addMenuAfter(&menuBackOven, &menuOvenTempItem);
}

//
// We register a handler that will be called when the title menu item is selected, see setup where we set the callback
//
void onTitlePressed(int /*id*/) {
    withMenuDialogIfAvailable([] (MenuBasedDialog* dlg) {
        dlg->setButtons(BTNTYPE_CLOSE, BTNTYPE_NONE);
        dlg->showRam("Title clicked", false);
        char szVer[10];
        tccore::copyTcMenuVersion(szVer, sizeof szVer);
        dlg->copyIntoBuffer(szVer);
    });
}

void setup() {
    // This example logs using IoLogging, see the following guide to enable
    //https://tcmenu.github.io/documentation/arduino-libraries//io-abstraction/arduino-logging-with-io-logging/
    IOLOG_START_SERIAL
    serEnableLevel(SER_TCMENU_DEBUG, true);

    // start wire if using I2C
    Wire.begin();

    // now we turn off the reset support
    renderer.turnOffResetLogic();

    setupMenu();

    // Here we add a menu observer that is notified as events take place on the menu.
    menuMgr.addChangeNotification(&myMgrObserver);

    // Now we set up the runtime parameters of various menu items
    prepareOvenMenuAtRuntime();
    menuList.setNumberOfRows(numListItems);

    // and lastly we add the title callback that is notified of the title being selected
    appTitleMenuItem.setCallback(onTitlePressed);
}

void loop() {
    taskManager.runLoop();
}

//
// Helper function to print a menu items name and value
//
void serialPrintMenuItem(MenuItem* item) {
    char sz[32];
    copyMenuItemNameAndValue(item, sz, sizeof sz);
    Serial.println(sz);
}

//
// Called when the start cooking menu item is clicked. It prints all the associated menu items out
void CALLBACK_FUNCTION onStartCooking(int id) {
    Serial.println("We are making pizza!");
    serialPrintMenuItem(&menuPizzaMakerOvenTemp);
    serialPrintMenuItem(&menuPizzaMakerTopping1);
    serialPrintMenuItem(&menuPizzaMakerTopping2);
    serialPrintMenuItem(&menuPizzaMakerTopping3);
}

//
// here we create an extra menu item that allows us to have three lines of text in the info dialog, made up of the
// title, the buffer line, and also this line, then the buttons. It uses the showRam function which shows from
// a non constant / PGM string
// https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/rendering-with-tcmenu-lcd-tft-oled/#presenting-a-dialog-to-the-user
//
TextMenuItem secondItem(newAnyMenuInfo("Detail: ", nextRandomId(), 0xffff, NO_CALLBACK), "det", 12, nullptr, INFO_LOCATION_RAM);

void CALLBACK_FUNCTION onDialogInfo(int id) {
    // withMenuDialogIfAvailable checks if the dialog can be presented now, and if so will call the function
    // provided with the dialog as the parameter. you then just prepare the dialog to be shown.
    withMenuDialogIfAvailable([](MenuBasedDialog* dlg) {
        // we set it to have only one button, named close.
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);

        // and now we add an extra item to the dialog
        secondItem.setTextValue("More text!");
        dlg->insertMenuItem(&secondItem);

        // and we tell the rendering framework to draw it as a textual item, without it's name, left justified.
        GridPosition gridPlacement(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_LEFT_VALUE_ONLY, 2);
        renderer.getDisplayPropertiesFactory().addGridPosition(&secondItem, gridPlacement);

        // and lastly we now show the menu and set the first buffer line text too.
        dlg->showRam("Non Pgm Header", true);
        dlg->copyIntoBuffer("line 2");
    });
}

//
// Shows a question dialog that gets the title from const/program memory. Notice that in this case we have a completion
// handler that accepts the final value, so that we could do something with the result.
//
const char pgmQuestionHeader[] PROGMEM = {"Override the title?"};
const char pgmTitleOverride[] PROGMEM = {"Title Overridden"};
taskid_t questionTask = TASKMGR_INVALIDID;
void CALLBACK_FUNCTION onDialogQuestion(int id) {
    // the question action item is set to have static data in RAM, therefore we can change it at will. Here we show
    // how to adjust the menu info block at runtime. Be aware that the menu must be marked structurally changed after.
    strncpy(minfoDialogsQuestion.name, "Questioned", sizeof minfoDialogsQuestion.name);
    menuMgr.notifyStructureChanged();

    // withMenuDialogIfAvailable checks if the dialog can be presented now, and if so will call the function
    // provided with the dialog as the parameter. you then just prepare the dialog to be shown.
    withMenuDialogIfAvailable([] (MenuBasedDialog* dlg) {
        // present a dialog with OK and CANCEL buttons with the question message, it has a completion handler.
        dlg->setButtons(BTNTYPE_OK, BTNTYPE_CANCEL);
        dlg->show(pgmQuestionHeader, true, [](ButtonType btn, void* data) {
            // this is the completion task that runs when the dialog is dismissed.
            Serial.print("Question result was "); Serial.println(btn);
            if(btn == BTNTYPE_OK) {
                // here we override the main menu title to a new value
                appTitleMenuItem.setTitleOverridePgm(pgmTitleOverride);
            }
            else {
                // here we clear the title override, and put it back to default.
                appTitleMenuItem.clearTitleOverride();
            }

            // we must cancel the task that we started when the dialog was created
            taskManager.cancelTask(questionTask);
        });

        // we create a task that keeps updating the value in the dialog buffer item.
        dlg->copyIntoBuffer("...");
        questionTask = taskManager.scheduleFixedRate(250, [] {
            char sz[10];
            ltoaClrBuff(sz, rand() % 100, 4, '0', sizeof sz);
            renderer.getDialog()->copyIntoBuffer(sz);
        });
    });
}

//
// In this case we create a controller based dialog that increments and decrements a number that is stored within
// an Analog item. We add the analog item and also an extra button to the dialog.
//
class MyDialogController : public BaseDialogController {
private:
    // create an extra button (buttonNum = 2)
    LocalDialogButtonMenuItem menuExtraButton = LocalDialogButtonMenuItem(dialogButtonRenderFn, nextRandomId(), 2, nullptr);

    // create an extra analog item to add.
    AnalogMenuInfo minfoAnalogController = { "Current", nextRandomId(), 0xffff, 100, NO_CALLBACK, 0, 1,"" };
    AnalogMenuItem menuAnalogController = AnalogMenuItem(&minfoAnalogController, 0, nullptr, false);
public:
    void initialiseAndGetHeader(BaseDialog *dialog, char *buffer, size_t bufferSize) override {
        // here we are responsible for setting the dialog title by copying the title text into the provided buffer,
        // and we can also add any extra items that we need here too.
        auto* menuDlg = reinterpret_cast<MenuBasedDialog*>(dialog);
        strcpy(buffer, "My Title");
        menuDlg->insertMenuItem(&menuAnalogController);
        menuDlg->insertMenuItem(&menuExtraButton);
    }

    void dialogDismissed(ButtonType buttonType) override {
        // once the dialog is dismissed this method is called when you can act on the result.
        Serial.print("Dialog dismissed: ");
        Serial.println(buttonType);
    }

    bool dialogButtonPressed(int buttonNum) override {
        // when any dialog button is pressed, either of the two default ones, or any extra ones you add, then this
        // controller is called. The default action of a dialog is to close when a button is pressed, however you
        // can stop that here by returning false.

        Serial.println("Button pressed: ");
        Serial.println(buttonNum);

        int currentVal = (int)menuAnalogController.getCurrentValue();
        if(buttonNum == 1) currentVal++;
        if(buttonNum == 2) currentVal--;
        menuAnalogController.setCurrentValue(currentVal);

        // we return true to allow processing (IE close dialog), false to stop the dialog closing.
        return buttonNum == 0;
    }

    void copyCustomButtonText(int buttonNumber, char *buffer, size_t bufferSize) override {
        // if either, the button number is not 0 or 1, or the button type is one of the custom values
        // then this method is called to get the text.

        // position 1 is going to be called up and 2 will be doown
        if(buttonNumber == 1) {
            strcpy(buffer, "Up");
        }
        else if(buttonNumber == 2) {
            strcpy(buffer, "Down");
        }
    }
};

MyDialogController dialogController;

void CALLBACK_FUNCTION onDialogController(int id) {
    // withMenuDialogIfAvailable checks if the dialog can be presented now, and if so will call the function
    // provided with the dialog as the parameter. you then just prepare the dialog to be shown.
    withMenuDialogIfAvailable([](MenuBasedDialog* dlg) {
        dlg->setButtons(BTNTYPE_OK, BTNTYPE_CUSTOM0);
        dlg->showController(true, &dialogController);
        dlg->copyIntoBuffer("Press up/down");
    });
}

// This callback needs to be implemented by you, see the below docs:
//  1. List Docs - https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
//  2. ScrollChoice Docs - https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
//
// This is called back each time a list item needs to draw data. This is a list that is based on an array of string values.
//
int CALLBACK_FUNCTION fnListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    if(mode == RENDERFN_ACTIVATE) {
        Serial.print("Activated "); Serial.println(row);
        return true;
    } else if(mode == RENDERFN_INVOKE) {
        if(row != LIST_PARENT_ITEM_POS) {
            Serial.print("Selected ");
            Serial.println(row);
            // do something with the selected item.
            menuMgr.resetMenu(false);
        }
        return true;
    } else if(mode == RENDERFN_VALUE && row < numListItems) {
        // if the value is in range we copy the value from our array
        strncpy(buffer, listElements[row], bufferSize);
        return true; // we have copied.
    }
    return defaultRtListCallback(item, row, mode, buffer, bufferSize);
}

void CALLBACK_FUNCTION onDialogBack(int id) {
    myMgrObserver.setAllowDialogBackAction(menuDialogsDialogBack.getBoolean());
}


// Here we show how to implement a custom text control, in this case we implement it so that it can be used both by
// rotary encoder and by keyboards, it will allow the user to only enter values between 0 and 7, and hence is an
// octal text filter.
int CALLBACK_FUNCTION octalOnlyRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    // See https://tcmenu.github.io/documentation/arduino-libraries//tc-menu/menu-item-types/based-on-runtimemenuitem/
    auto textItem = reinterpret_cast<TextMenuItem*>(item);
    switch(mode) {
        case RENDERFN_NAME:
            strncpy(buffer, "Oct", bufferSize);
            return true; // to override name return true, to use default return false
        case RENDERFN_GETPART:
            // this is called to get the current value at a given position, in a way that is suitable for a rotary
            // encoder to edit. IE a zero based value between 0 and 7.
            if (row < 1) return 0;
            return textItem->getTextValue()[row - 1] - '0';
        case RENDERFN_SET_VALUE:
            // this is called when a rotary encoder or similar changes value, these are zero based so the value
            // will be between 0 and 7 and we then simply add '0'.
            if (row < 1) return false;
            textItem->setCharValue(row - 1, '0' + *buffer);
            return true;
        case RENDERFN_SET_TEXT_VALUE:
            // this is called by keyboards when a key is pressed in edit mode, the character entered on the
            // keyboard is provided in the buffer, and it is up to you to validate it, return true if successful
            // otherwise false
            if (row < 1 || *buffer < '0' || *buffer > '7') return false;
            textItem->setCharValue(row - 1, *buffer);
            return true;
        default:
            // when we are not handling something, we need to call through to the parent callback
            return textItemRenderFn(item, row, mode, buffer, bufferSize);
    }
}
