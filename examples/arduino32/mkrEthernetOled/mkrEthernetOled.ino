/*
 * Simple demo project that runs on our MKR1300 boards with Ethernet module
 * and OLED screen. It uses the fluent menu builder and all-in-one plugin
 * support added in 4.5.
 *
 * Setup:
 *
 * OLED: configured on standard I2C GPIOs using an SH1106 driver.
 * Ethernet: We have the regular MKR ethernet shield.
 * Menu, a simple menu structure defined in TcMenu Designer Turbo
 *
 * To build your own menu: https://designer.thecoderscorner.com/
 * Getting started: https://www.thecoderscorner.com/products/apps/tcmenu-designer/
 * Documentation: https://tcmenu.github.io/documentation/
 */

#include "MkrEthernetOLED_menu.h"

// This variable is the RAM data for scroll choice item Foods
char ScrollRam[] = "1\0        2\0        3\0        4\0        5\0        ~";


void setup() {
    // On SAMD/MKR it's best to wait for the serial port to start before proceeding.
    while (!Serial && millis() < 15000) {}

    // Before we do anything else, we start things up.
    Serial.begin(115200);
    Wire.begin();

    serEnableLevel(SER_TCMENU_DEBUG, true);

    // in TcMenu we set up the menu using this method.
    setupMenu();
    serlogF(SER_TCMENU_DEBUG, "Menu setup complete");

    // here we use task manager to schedule some tasks to happen ever 500 millis
    // https://tcmenu.github.io/documentation/arduino-libraries/taskmanager-io/task-manager-scheduling-guide/
    taskManager.schedule(repeatMillis(500), [] {
        // Here we adjust some menu items at runtime, every 500 millis.
        //https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/
        getAnalogItemById(MENU_KITCHEN_ID).setCurrentValue(random(100));
        getAnalogItemById(MENU_LOUNGE_ID).setCurrentValue(random(100));
        //    menuKitchen.setCurrentValue(random(100));
        //    menuLounge.setCurrentValue(random(100));
    });
}

// All TcMenu projects use taskmanager
void loop() {
    taskManager.runLoop();
}
