#include "generated/allMenuItemProject_menu.h"

// This variable is the RAM data for scroll choice item %menu.16.name
char* myScrollChoices = "1\0        2\0        3\0        4\0        5\0        ~";


void setup() {
    setupMenu();

}

void loop() {
    taskManager.runLoop();

}



void CALLBACK_FUNCTION scrollRomChange(int id) {
    serdebugF("Scroll rom changed")
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

void CALLBACK_FUNCTION callback1(int id) {
    serdebugF("RGBR changed")
}

int CALLBACK_FUNCTION largeNumCustomRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    // See https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/based-on-runtimemenuitem/
    switch(mode) {
    case RENDERFN_NAME:
        return false; // use default
    }
    return largeNumItemRenderFn(item, row, mode, buffer, bufferSize);
}


void CALLBACK_FUNCTION actionItem(int id) {
    serdebugF("Action changed")
}


void CALLBACK_FUNCTION rgbWasChanged(int id) {
    serdebugF("RGBA changed")
}


void CALLBACK_FUNCTION scrollRamFn(int id) {
    serdebugF("Scroll Ram change")
}



void CALLBACK_FUNCTION listItemWasSelected(int id) {
    // TODO - your menu change code
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
    // TODO - your menu change code
}


void CALLBACK_FUNCTION listRamHasChanged(int id) {
    // TODO - your menu change code
}



void CALLBACK_FUNCTION listItemWasSelected(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION listRamHasChanged(int id) {
    // TODO - your menu change code
}



void CALLBACK_FUNCTION listRamHasChanged(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION listItemWasSelected(int id) {
    // TODO - your menu change code
}
