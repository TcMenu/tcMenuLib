#include <EEPROM.h>
#include "generated/allMenuItemProject_menu.h"

// This variable is the RAM data for scroll choice item %menu.16.name
char myScrollChoices[] = "Test\0     Case\0     Mem\0      Check\0    Project\0  ~";


void setup() {
    // before proceeding, we must start wire and serial, then call setup menu.
    Serial.begin(115200);
    serdebugF("Starting ESP32-S2 all menu test");
    Wire.begin();
    Wire.setClock(1000000);
    EEPROM.begin(512);

    Rgb32MenuItem::setRgbPrintMode(HEX_HTML);

    // using list RAM Array mode the designer creates an array of the size intial items for you and fills it with nullptr,
    // you can then simply "fill it in". This is easier than using the list in custom mode, but still gives you some
    // freedom. IMPORTANTLY you must never set the rows to be bigger than initial rows.
    enumStrSubRamListRam[0] = (char*)"Item123";
    enumStrSubRamListRam[1] = (char*)"Another32";
    enumStrSubRamListRam[2] = (char*)"Abcdef";
    enumStrSubRamListRam[3] = (char*)"Xyz";
    enumStrSubRamListRam[4] = (char*)"123";
    enumStrSubRamListRam[5] = (char*)"456";

    setupMenu();

    menuMgr.getEepromAbstraction()->writeArrayToRom(400, (const uint8_t*)"Rom12Test1Test2 ", 16);
}

void loop() {
    taskManager.runLoop();
}

// This callback needs to be implemented by you, see the below docs:
//  1. List Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
//  2. ScrollChoice Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
int CALLBACK_FUNCTION fnSubRamListCustomRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    switch(mode) {
    default:
        return defaultRtListCallback(item, row, mode, buffer, bufferSize);
    }
}

void CALLBACK_FUNCTION listCustomHasSelect(int id) {
    serdebugF("Custom list");
}



void CALLBACK_FUNCTION scrollRamFn(int id) {
    serdebugF("Scroll RAM");
}


void CALLBACK_FUNCTION callback1(int id) {
    serdebugF("callback1");
}

int CALLBACK_FUNCTION largeNumCustomRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    // See https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/based-on-runtimemenuitem/
    switch(mode) {
    case RENDERFN_NAME:
        return false; // use default
    }
    return largeNumItemRenderFn(item, row, mode, buffer, bufferSize);
}


void CALLBACK_FUNCTION listRamHasChanged(int id) {
    serdebugF("List RAM change");
}


void CALLBACK_FUNCTION listItemWasSelected(int id) {
    serdebugF("List Flash change");
}


void CALLBACK_FUNCTION actionItem(int id) {
    serdebugF("Action!");
}


void CALLBACK_FUNCTION scrollRomChange(int id) {
    serdebugF("Scroll ROM");
}


void CALLBACK_FUNCTION rgbWasChanged(int id) {
    serdebugF("RGB change")
}

// This callback needs to be implemented by you, see the below docs:
//  1. List Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
//  2. ScrollChoice Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
int CALLBACK_FUNCTION fnSubRamCustomScrollRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    switch(mode) {
    default:
        return defaultRtListCallback(item, row, mode, buffer, bufferSize);
    }
}

void CALLBACK_FUNCTION scrollCustomChanged(int id) {
    serdebugF("Custom Scrolll")
}
