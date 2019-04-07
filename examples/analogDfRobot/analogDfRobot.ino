#include "analogDfRobot_menu.h"

void setup() {
    setupMenu();
    pinMode(LED_BUILTIN, OUTPUT);
    menuValueA0.setReadOnly(true);
    taskManager.scheduleFixedRate(200, [] {
        menuValueA0.setCurrentValue(analogRead(A0));
    });
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
