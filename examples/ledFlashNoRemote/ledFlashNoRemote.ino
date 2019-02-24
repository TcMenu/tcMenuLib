#include "ledFlashNoRemote_menu.h"
#include <EepromAbstraction.h>

/**
 * This example controls the state of the built in LED using the menu. It also shows the
 * status of analog pin A0 on the display too.
 * 
 * For more detail see the README.md file.
 */

// This example saves it's state to EEPROM using the inbuilt eeprom support in tcMenu
// You could choose an At24C based I2C eeprom too. See the takeOverDisplay example.
AvrEeprom eeprom;

//
// We register this function with task manager to be called every 200 millis and update the
// menu item with the latest reading from pin A0.
//
void updateAnalogMenuItem() {
    menuA0Volts.setCurrentValue(analogRead(A0));
}

void setup() {
    // needed in all sketches and generally added by the designer.
    setupMenu();
    
    // We want to control the LED_BUILTIN so we must set to output first.
    pinMode(LED_BUILTIN, OUTPUT);

    // now load back the previous state from EEPROM.
    menuMgr.load(eeprom);

    // and set up the task that will read A0 and update the menu item.
    taskManager.scheduleFixedRate(200, updateAnalogMenuItem, TIME_MILLIS);
}

void loop() {
    // we must always call runLoop in loop, do nothing that takes a long time in loop.
    // this is generally added by designer UI.
    taskManager.runLoop();
}

//
// When the LED menu item is changed, this is the callback function. We just read the current
// state and set the built in LED pin accordingly.
//
void CALLBACK_FUNCTION onLedChange(int id) {
    digitalWrite(LED_BUILTIN, menuBuiltInLED.getBoolean());
}

//
// When the save menu item is pressed, this function is called, it updates the eeprom. In a real
// life system you may want to do this on power down. See the below article:
// https://www.thecoderscorner.com/electronics/microcontrollers/psu-control/detecting-power-loss-in-powersupply/
//
void CALLBACK_FUNCTION onSaveState(int id) {
    menuMgr.save(eeprom);
}
