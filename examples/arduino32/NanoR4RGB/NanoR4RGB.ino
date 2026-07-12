/*
 * A very simple TcMenu app that was built using https://designer.thecoderscorner.com/
 * It is setup out of designer to run an R4Nano with SSD1351 OLED and a rotary encoder.
 *
 * It is almost unchanged from what came out of designer itself.
 *
 * Pin configuration:
 *    Encoder: A and B are on 6 and 7, select is on 5
 *    Display: CS=10, DC=9, RS=8. Hardware SPI.
 */
#include "NanoR4RGB_menu.h"

void buildMenu(TcMenuBuilder& builder) {
    builder.usingDynamicEEPROMStorage()
        .analogBuilder(MENU_COUNT_ID, "Count", DONT_SAVE, MenuFlags().readOnly(), 0, nullptr)
            .offset(0).divisor(1).step(1).maxValue(255).unit("tms").endItem()
        .actionItem(MENU_ADD_ID, "Add", NoMenuFlags, tcAdded_onAddToCount);
    
    setTitlePressedCallback([](int id) {
        showVersionDialog(&applicationInfo);
    });
}

void setup() {
    setupMenu();
}

void loop() {
    taskManager.runLoop();

}

void CALLBACK_FUNCTION tcAdded_onAddToCount(int id) {
    getMenuCount().setCurrentValue(getMenuCount().getCurrentValue() + 1); // Added by generator for the count example
}
