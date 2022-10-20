#include "piPicoTftEncoder_menu.h"
#include <PlatformDetermination.h>
#include <IoLogging.h>
#include <TaskManagerIO.h>
#include <BaseDialog.h>

// TFT_eSPI setup is "Setup60_RP2040_ILI9341.h".

void setup() {
    Serial.begin(115200);
    while(!Serial);

    setupMenu();
}

void loop() {
    taskManager.runLoop();
}


void CALLBACK_FUNCTION onRestart(int /*id*/) {
    serdebugF("Restart selected");
}


void CALLBACK_FUNCTION onVolumeChanged(int id) {
    serdebugF2("Volume changed ", menuVolume.getCurrentValue());
}

class CountingTask : public Executable {
private:
    MenuBasedDialog *dialog;
    int currentCount;
    taskid_t taskid = TASKMGR_INVALIDID;
public:
    void init(MenuBasedDialog* dlg, taskid_t taskIdParam) {
        dialog = dlg;
        currentCount = 0;
        taskid = taskIdParam;
    }

    void exec() override {
        if(!dialog || taskid == TASKMGR_INVALIDID) return;
        char sz[10];
        ltoaClrBuff(sz, ++currentCount, 4, NOT_PADDED, sizeof sz);
        dialog->copyIntoBuffer(sz);
    }

    void stop() {
        if(taskid == TASKMGR_INVALIDID) return;
        taskManager.cancelTask(taskid);
        taskid = TASKMGR_INVALIDID;
        dialog = nullptr;
    }
} countingTask;

void onCountingDlgComplete(ButtonType, void*) {
    countingTask.stop();
}

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

void CALLBACK_FUNCTION onShowDialogs(int id) {
    withMenuDialogIfAvailable([](MenuBasedDialog *dlg) {
        dlg->setButtons(BTNTYPE_ACCEPT, BTNTYPE_CANCEL, 1);
        dlg->setUserData(dlg);
        dlg->show("Dialogs test", true, onCompletedDialog);
        dlg->copyIntoBuffer("Accept for more..");
    });

}
