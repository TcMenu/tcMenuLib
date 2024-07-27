//
// A TcMenu example that demonstrates how to use the one button support with an STM32 based
// board, but it is sufficiently simple that it should be easy to move to any other board.
//
// You configure the single pin in code generator, and away you go.
//
// Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/
//

#include "generated/stm32DuinoOneButton_menu.h"
#include <SPI.h>

void setup() {
    // This example logs using IoLogging, see the following guide to enable
    // https://www.thecoderscorner.com/products/arduino-libraries/io-abstraction/arduino-logging-with-io-logging/
    IOLOG_START_SERIAL
    serEnableLevel(SER_NETWORK_DEBUG, true);

    // Start up serial and prepare the correct SPI, your pins may differ
    SPI.setMISO(PB4);
    SPI.setMOSI(PB5);
    SPI.setSCLK(PB3);

    setupMenu();

}

void loop() {
    taskManager.runLoop();
}


void CALLBACK_FUNCTION onPressMe(int id) {
    // the callback functions are added by designer when we provide a callback function name
    // in the menu item editor. It will be called when the item changes along with the item id
    serlogF2(SER_DEBUG, "Pressed ", id);
}
