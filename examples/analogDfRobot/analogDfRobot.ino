#include "analogDfRobot_menu.h"

void setup() {
    // first we setup the menu
    setupMenu();

    // we are going to toggle the built in LED, so set it as output.
    pinMode(LED_BUILTIN, OUTPUT);

    // now we read the value of A0 every 200millis and set it onto a menu item
    menuValueA0.setReadOnly(true);
    taskManager.scheduleFixedRate(200, [] {
        menuValueA0.setCurrentValue(analogRead(A0));
    });

    // Finally we set up back (left) and next (right) functionality.
    menuMgr.setBackButton(DF_KEY_LEFT);
    menuMgr.setNextButton(DF_KEY_RIGHT);
}

void loop() {
    taskManager.runLoop();
}

void CALLBACK_FUNCTION onLed1(int id) {
    digitalWrite(LED_BUILTIN, menuLED1.getBoolean());
}

void CALLBACK_FUNCTION onLed2(int id) {
    // TODO: write your own second LED function..
    // Called whenever you change the LED2 menu item..
}

