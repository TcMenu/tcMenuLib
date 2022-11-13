// An example showing how to setup and use the BSP framebuffer support for STM32 devices on mbed. Tested and working
// with STM32F4x9 DISC1
//
// Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/

#include <mbed.h>
#include "stm32f429FrameBuffer_menu.h"

BufferedSerial console(USBTX, USBRX);
MBedLogger LoggingPort(console);

void setup() {
    // setup LTDC display controller first.
    // by the time setupMenu is called, the display must be ready for use.
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(0, SDRAM_DEVICE_ADDR);

    setupMenu();

    taskManager.scheduleFixedRate(100, [] {
        menuACLine.setCurrentValue(2350 + (rand() % 100));
        menuConsumption.setCurrentValue(1900 + (rand() % 200));
    });
}

int main() {
    console.set_baud(115200);
    setup();
    while(1) {
        taskManager.runLoop();
    }
}



void CALLBACK_FUNCTION onTargetChanged(int id) {
    // TODO - your menu change code
}

void onFirstDialogCompleted(ButtonType btnType, void* data) {
    if(btnType != BTNTYPE_ACCEPT) return;

    auto dlg = reinterpret_cast<MenuBasedDialog*>(data);
    dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
    dlg->show("Second dialog", false);
    dlg->copyIntoBuffer("Extra data");
}

void CALLBACK_FUNCTION onPresentDialog(int id) {
    withMenuDialogIfAvailable([](MenuBasedDialog *dlg) {
        dlg->setButtons(BTNTYPE_ACCEPT, BTNTYPE_CANCEL, 1);
        dlg->setUserData(dlg);
        dlg->show("More dialogs?", true, onFirstDialogCompleted);
        dlg->copyIntoBuffer("Accept for more");
    });
}
