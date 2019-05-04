/**
 * An example that uses a simple analog joystick Up/Down for input using the
 * JoystickRotaryEncoder support that is built into IoAbstraction.
 * 
 * Just wire a joystick to any available analog pin, and a switch to any other
 * pin. By default this assumes you have a DfRobot LCD, but using the designer
 * you can change it to use any display type.
 */

#include "simpleJoystickLcd_menu.h"

void setup() {
    setupMenu();

}

void loop() {
    taskManager.runLoop();
    
}

void CALLBACK_FUNCTION onTidalGate1(int id) {
    // TODO - your menu change code
}

void CALLBACK_FUNCTION onTidalGate2(int id) {
    // TODO - your menu change code
}

void CALLBACK_FUNCTION onTidalGate3(int id) {
    // TODO - your menu change code
}
