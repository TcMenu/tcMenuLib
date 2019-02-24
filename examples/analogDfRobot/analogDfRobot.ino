#include "analogDfRobot_menu.h"

void setup() {
    setupMenu();

}

void loop() {
    taskManager.runLoop();

}

void CALLBACK_FUNCTION onLed1(int id) {
    // TODO - your menu change code
}

void CALLBACK_FUNCTION onLed2(int id) {
    // TODO - your menu change code
}
