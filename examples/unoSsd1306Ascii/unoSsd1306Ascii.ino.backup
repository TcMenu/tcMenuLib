#include "src/unoSsd1306Ascii_menu.h"
#include <IoLogging.h>

SSD1306AsciiAvrI2c gfx;

#define I2C_ADDRESS 0x3C

void setup() {
    Serial.begin(115200);
    gfx.begin(&SH1106_128x64, I2C_ADDRESS);
    gfx.clear();
    setupMenu();
}

void loop() {
    taskManager.runLoop();
}

const char szDialogHeader[] PROGMEM = "You pressed me";

void CALLBACK_FUNCTION onActionPressed(int id) {
    BaseDialog* dlg = renderer.getDialog();
    char extraData[10];
    fastltoa(extraData, menuBrightness.getCurrentValue(), 3, '0', sizeof(extraData));
    dlg->copyIntoBuffer(extraData);
    dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
    dlg->show(szDialogHeader, true);
}
