#include "esp32Lcd_menu.h"

void setup() {
    Serial.begin(115200);
    Wire.begin(4, 15);
    setupMenu();

}

void loop() {
    taskManager.runLoop();

}


// see tcMenu list documentation on thecoderscorner.com
int CALLBACK_FUNCTION fnMyListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
   switch(mode) {
    case RENDERFN_INVOKE:
        // TODO - your code to invoke goes here - row is the index of the item
        return true;
    case RENDERFN_NAME:
        // TODO - each row has it's own name - 0xff is the parent item
        ltoaClrBuff(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_VALUE:
        // TODO - each row can has its own value - 0xff is the parent item
        buffer[0] = 'V'; buffer[1]=0;
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_EEPROM_POS: return 0xffff; // lists are generally not saved to EEPROM
    default: return false;
    }
}
