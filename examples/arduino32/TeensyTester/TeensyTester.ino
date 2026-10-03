// This example assumes you have a TFT configured using TFT_eSPI and a rotary encoder attached on
// pins A=3, B=4, OK=5.
//
// It presents a simple UI that demonstrates a working meny on Teensy.
//
// You can create you own menu (or load this EMF) in web designer:
// https://designer-next.thecoderscorner.com/

#include "TeensyTester_menu.h"

void buildMenu(TcMenuBuilder& builder) {
    builder.usingDynamicEEPROMStorage()
        .analogBuilder(MENU_COUNT_ID, "Count", DONT_SAVE, MenuFlags().readOnly(), 0, nullptr)
            .offset(0).divisor(1).step(1).maxValue(255).unit("tms").endItem()
        .actionItem(MENU_ADD_ID, "Add", NoMenuFlags, tcAdded_onAddToCount);
}

void setup() {
    setTitlePressedCallback([](int id) {
        showVersionDialog(&applicationInfo);
    });
    setupMenu();
}

void loop() {
    taskManager.runLoop();
}


void CALLBACK_FUNCTION tcAdded_onAddToCount(int id) {
    getMenuCount().setCurrentValue(getMenuCount().getCurrentValue() + 1); // Added by generator for the count example
}
