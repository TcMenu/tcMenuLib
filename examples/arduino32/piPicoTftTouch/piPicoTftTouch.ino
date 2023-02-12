/**
 * An example that we test regularly on a Raspberry PI Pico (should work on either core), it has a screen based on
 * the TFT_eSPI library and also a rotary encoder. It demonstrates quite a few features of the library.
 *
 * Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/
 */
#include "piPicoTftTouch_menu.h"
#include <PlatformDetermination.h>
#include <IoLogging.h>
#include <TaskManagerIO.h>
#include <BaseDialog.h>
#include <tcUtil.h>

// TFT_eSPI setup is "Setup60_RP2040_ILI9341.h".


const char* fileNames[] = {
        "Automatic.dat",
        "System.cpp",
        "Driver123.h",
        "SuperCode.java",
        "TurboGame.o",
};

#define FILE_NAME_SIZE 5

void setup() {
    // This example logs using IoLogging, see the following guide to enable
    // https://www.thecoderscorner.com/products/arduino-libraries/io-abstraction/arduino-logging-with-io-logging/
    IOLOG_START_SERIAL

    // This is added by the code generator, it initialises the menu
    setupMenu();

    // check if the touch screen needs to be initialised, we provide a callback that will be called before and
    // after the calibration UI is shown, it should reset the screen and touch to native orientation on start, and
    // put it back to "menu" mode on finishing.
    touchCalibrator.initCalibration([](bool starting) {
        static iotouch::TouchOrientationSettings oldSettings = iotouch::TouchOrientationSettings(false, false, false);
        if(starting) {
            gfx.setRotation(0);
            oldSettings = touchScreen.changeOrientation(iotouch::TouchOrientationSettings(false, true, true));
        } else {
            gfx.setRotation(1);
            touchScreen.changeOrientation(oldSettings);
        }
    }, true);

    // Add a callback to show the build version when the title is pressed
    // this uses the standard function to show a version dialog from tcUtil.h
    setTitlePressedCallback([](int) {
        showVersionDialog(&applicationInfo);
    });

    // Add another button that directly controls the mute menu item
    switches.addSwitch(22, [] (pinid_t key, bool held) {
        if(!held) {
            menuMute.setBoolean(!menuMute.getBoolean());
        }
    }, NO_REPEAT);

    // and initialise the list menu item with the number of rows, see the list callback function below
    // we set the number of items to all the files plus the refresh item
    menuRootList.setNumberOfRows(FILE_NAME_SIZE + 1);
}

void loop() {
    taskManager.runLoop();
}

/**
 * An example callback that was registered in the designer UI and generated for us here.
 */
void CALLBACK_FUNCTION onVolumeChanged(int /*id*/) {
    serdebugF2("Volume changed ", menuVolume.getCurrentValue());
}

/**
 * Here we create a task that will be scheduled to update the "second" dialog that appears. It keeps updating the
 * count until the dialog is dismissed.
 */
class CountingTask : public Executable {
private:
    MenuBasedDialog *dialog = nullptr;
    int currentCount = 0;
    taskid_t taskid = TASKMGR_INVALIDID;
public:
    /**
     * Store the dialog and reset.
     * @param dlg the dialog manager
     * @param taskIdParam the task id that was registered
     */
    void init(MenuBasedDialog* dlg, taskid_t taskIdParam) {
        dialog = dlg;
        currentCount = 0;
        taskid = taskIdParam;
    }

    /**
     * Called upon the schedule to bump the count and update the dialog
     */
    void exec() override {
        if(!dialog || taskid == TASKMGR_INVALIDID) return;
        char sz[10];
        ltoaClrBuff(sz, ++currentCount, 4, NOT_PADDED, sizeof sz);
        dialog->copyIntoBuffer(sz);
    }

    /**
     * Call this to deregister with task manager.
     */
    void stop() {
        if(taskid == TASKMGR_INVALIDID) return;
        taskManager.cancelTask(taskid);
        taskid = TASKMGR_INVALIDID;
        dialog = nullptr;
    }
} countingTask;

/**
 * When the 2nd dialog is dismissed, we stop the counting task
 */
void onCountingDlgComplete(ButtonType, void*) {
    countingTask.stop();
}

/**
 * When the first dialog is dismissed we check the button type and if it is accepted, we show a 2nd dialog
 * @param btnPressed the button type pressed
 * @param data this is the data provided in dialog's setUserData.
 */
void onCompletedDialog(ButtonType btnPressed, void* data) {
    auto dlg = reinterpret_cast<MenuBasedDialog*>(data);
    if(btnPressed == BTNTYPE_ACCEPT) {
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE, 0);
        dlg->show("Pressed Accept", false, onCountingDlgComplete);
        dlg->copyIntoBuffer("");
        auto taskId = taskManager.scheduleFixedRate(200, &countingTask);
        countingTask.init(dlg, taskId);
    }
}

/**
 * Show the first dialog, which has an accept and cancel button, when the show dialog menu item is pressed.
 */
void CALLBACK_FUNCTION onShowDialogs(int) {
    withMenuDialogIfAvailable([](MenuBasedDialog *dlg) {
        dlg->setButtons(BTNTYPE_ACCEPT, BTNTYPE_CANCEL, 1);
        dlg->setUserData(dlg);
        dlg->show("Dialogs test", true, onCompletedDialog);
        dlg->copyIntoBuffer("Accept for more..");
    });

}

/**
 * Here we show a list callback that presents a list of "files", these simulated files are in a global array, but they
 * could just as easily be somewhat dynamic and read from a proper source.
 * See: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
 * @param item the actual list item
 * @param row the currently selected row
 * @param mode the action that is needed, eg copy value, invoke etc.
 * @param buffer the buffer if appropriate for the action
 * @param bufferSize the size of the buffer
 * @return depends on the action, see the docs
 */
int CALLBACK_FUNCTION fnRootListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
   switch(mode) {
    case RENDERFN_ACTIVATE:
        serlogF2(SER_DEBUG, "List activate ", row);
        return false;
    case RENDERFN_INVOKE:
        // we have a list of files and a refresh option at the end.
        if(row < FILE_NAME_SIZE) {
            // a file has been selected, dismiss list.
            serlogF2(SER_DEBUG, "List invoke: ", row);
            menuMgr.resetMenu(false); // drop back one level dismissing the list
            reinterpret_cast<ListRuntimeMenuItem *>(item)->asParent();
        } else {
            // refresh was selected, refresh and force recalc.
            item->setNumberOfRows(item->getNumberOfRows() + 1);
            menuMgr.recalculateListIfOnDisplay(item);
        }
        return true;
    case RENDERFN_NAME:
        strncpy(buffer, "Choose File", bufferSize);
        return true;
    case RENDERFN_VALUE:
        if(row == LIST_PARENT_ITEM_POS) {
            // no value on the parent item, IE when this list is displayed in a parent menu.
            buffer[0]=0;
        } else if(row < FILE_NAME_SIZE) {
            // copy the file name into the buffer
            strncpy(buffer, fileNames[row], bufferSize);
        } else if(row == FILE_NAME_SIZE) {
            strcpy(buffer, "Add more");
        } else {
            ltoaClrBuff(buffer, row, 4, '0', bufferSize);
        }
        return true;
    case RENDERFN_EEPROM_POS: return 0xffff; // lists are generally not saved to EEPROM
    default: return false;
    }
}

//
// For Item: menuStatusInfo
//
// Here we present how to customize a runtime menu item such as a text item, RGB or Large number. You can take full
// control of the callback, or just override a few of the features to customize it.
//
// In this case we just want to override the name at runtime so we can change the name to be a counter of the number of
// times it is drawn.
//
// Steps
// 1. In the Designer of a text item, select "edit" next to the function callback text field
// 2. Select the "Runtime RenderFn override implementation" option
// 3. Once code generation the function is generated (exactly as below)
//
int CALLBACK_FUNCTION infoRenderingRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    static int staticCounter = 0;
    // See https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/based-on-runtimemenuitem/

    switch(mode) {
    case RENDERFN_NAME:
        strncpy(buffer, "Count ", bufferSize);
        fastltoa(buffer, staticCounter++, 7, NOT_PADDED, bufferSize);
        return true; // override title by returning true
    }
    return textItemRenderFn(item, row, mode, buffer, bufferSize);
}

/**
 * How to change an item based on an Info block, such as Analog, Enum, Boolean, Float, Action and SubMenu.
 *
 * This is a standard callback function, it is attached in designer to the restart menu item. Here we show how to
 * rename a menu item that is based on a menu info block.
 */
void CALLBACK_FUNCTION onRestart(int /*id*/) {
    serdebugF("Restart selected");
    strncpy(minfoStatusRestart.name, "Restarting", NAME_SIZE_T);
    menuStatusRestart.setChanged(true);
}
