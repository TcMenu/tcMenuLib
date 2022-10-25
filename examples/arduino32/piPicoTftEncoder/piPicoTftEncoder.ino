#include "piPicoTftEncoder_menu.h"
#include <PlatformDetermination.h>
#include <IoLogging.h>
#include <TaskManagerIO.h>
#include <BaseDialog.h>
#include <tcMenuVersion.h>

// TFT_eSPI setup is "Setup60_RP2040_ILI9341.h".


const char* fileNames[] = {
        "Automatic.dat",
        "System.cpp",
        "Driver123.h",
        "SuperCode.java",
        "TurboGame.o",
};

#define FILE_NAME_SIZE 5

void onTitlePressed(int);

void setup() {
    // Initialise serial for logging
    // If you're using a pico probe, set build flag LoggingPort=Serial1 and initialise serial port 1.
    //Serial.begin(115200);
    Serial1.begin(115200);

    // This is added by the code generator, it initialises the menu
    setupMenu();

    // Add a callback to show the build version when the title is pressed
    setTitlePressedCallback(onTitlePressed);

    // Add another button that directly controls the mute menu item
    switches.addSwitch(22, [] (pinid_t key, bool held) {
        if(!held) {
            menuMute.setBoolean(!menuMute.getBoolean());
        }
    }, NO_REPEAT);

    // and initialise the list menu item with the number of rows, see the list callback function below
    menuRootList.setNumberOfRows(FILE_NAME_SIZE);
}

void loop() {
    taskManager.runLoop();
}


void CALLBACK_FUNCTION onRestart(int /*id*/) {
    serdebugF("Restart selected");
}


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
    case RENDERFN_INVOKE:
        serlogF2(SER_DEBUG, "List invoke: ", row);
        menuMgr.resetMenu(false); // drop back one level dismissing the list
        reinterpret_cast<ListRuntimeMenuItem*>(item)->asParent();
        return true;
    case RENDERFN_NAME:
        strncpy(buffer, "Choose File", bufferSize);
        return true;
    case RENDERFN_VALUE:
        if(row < FILE_NAME_SIZE) {
            strncpy(buffer, fileNames[row], bufferSize);
        }
        return true;
    case RENDERFN_EEPROM_POS: return 0xffff; // lists are generally not saved to EEPROM
    default: return false;
    }
}

void onTitlePressed(int) {
    withMenuDialogIfAvailable([](MenuBasedDialog *dlg) {
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
        dlg->show("TcMenu PI Pico example", false);
        char sz[20];
        tccore::copyTcMenuVersion(sz, sizeof sz);
        dlg->copyIntoBuffer(sz);
    });
}
