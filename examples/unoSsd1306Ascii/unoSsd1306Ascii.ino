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


// see tcMenu list documentation on thecoderscorner.com
int CALLBACK_FUNCTION fnListItemRtCall(RuntimeMenuItem * item, uint8_t row, RenderFnMode mode, char * buffer, int bufferSize) {
   switch(mode) {
    case RENDERFN_INVOKE:
        // TODO - your code to invoke goes here - row is the index of the item
        return true;
    case RENDERFN_NAME:
        // TODO - each row has it's own name - 0xff is the parent item
        if(row == 0xff) {
            strcpy(buffer, "My List");
        }
        else {
            ltoaClrBuff(buffer, row, 3, NOT_PADDED, bufferSize);
        }
        return true;
    case RENDERFN_VALUE:
        // TODO - each row can has its own value - 0xff is the parent item
        buffer[0] = 'V'; buffer[1]=0;
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_EEPROM_POS: return 0xFFFF; // lists are generally not saved to EEPROM
    default: return false;
    }
}
