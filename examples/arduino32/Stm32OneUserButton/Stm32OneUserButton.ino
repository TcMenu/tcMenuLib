// A TcMenu example that demonstrates how to use the one button support with an STM32 based
// board, but it is simple enough that it should be easy to move to any other board.
//
// You configure the single pin in the code generator, and away you go. For this example I've
// used the USER button on the STM board itself, needing no extra wiring.
//
// This demonstrates just about the simplest possible menu with remote control.
//
// To build your own menu: https://designer.thecoderscorner.com/
// Getting started: https://www.thecoderscorner.com/products/apps/tcmenu-designer/
// Documentation: https://www.thecoderscorner.com/products/arduino-libraries/
//

#include "Stm32OneUserButton_menu.h"
#include <SPI.h>


// Declaring any arrays used by enum/list items
const char* strSettingsEnumPropEnumEntries[] = { "Item1", "Item2", "Item3" };

void buildMenu(TcMenuBuilder& builder) {
    builder.usingDynamicEEPROMStorage()
        .actionItem(MENU_PRESS_ME_ID, "Press Me", NoMenuFlags, onPressMe)
        .analogBuilder(MENU_TEMP_ID, "Temp", DONT_SAVE, NoMenuFlags, 0, nullptr)
            .offset(0).divisor(1).step(1).maxValue(100).unit("%").endItem()
        .subMenu(MENU_SETTINGS_ID, "Settings", NoMenuFlags, nullptr)
            .boolItem(MENU_SETTINGS_OPTION_ID, "Option", ROM_SAVE, NAMING_TRUE_FALSE, NoMenuFlags, false, nullptr)
            .analogBuilder(MENU_SETTINGS_INT_PROP_ID, "IntProp", ROM_SAVE, NoMenuFlags, 0, nullptr)
                .offset(0).divisor(1).step(1).maxValue(10).unit("A").endItem()
            .enumItem(MENU_SETTINGS_ENUM_PROP_ID, "EnumProp", ROM_SAVE, strSettingsEnumPropEnumEntries, 3, NoMenuFlags, 0, nullptr)
            .endSub();
}

void setup() {
    // Start the serial port so that we can use the remote connectivity
    Serial.begin(115200);

    // Prepare the correct SPI, your pins may differ
    SPI.setMISO(PB4);
    SPI.setMOSI(PB5);
    SPI.setSCLK(PB3);

    // and fire up the menu
    setupMenu();

    // That's it, you've got a menu with serial remote control!
}

void loop() {
    taskManager.runLoop();

}

void CALLBACK_FUNCTION onPressMe(int id) {
    serlogF2(SER_DEBUG, "Pressed ", id);
}
