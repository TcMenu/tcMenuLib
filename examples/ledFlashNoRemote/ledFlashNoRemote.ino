#include "ledFlashNoRemote.h"
#include <EepromAbstraction.h>

AvrEeprom eeprom;

void updateAnalogMenuItem() {
    menuA0Volts.setCurrentValue(analogRead(A0));
}

void setup() {
    setupMenu();
    pinMode(LED_BUILTIN, OUTPUT);
    menuMgr.load(eeprom);
    taskManager.scheduleFixedRate(200, updateAnalogMenuItem, TIME_MILLIS);
}

void loop() {
    taskManager.runLoop();
}

void CALLBACK_FUNCTION onLedChange(int id) {
    digitalWrite(LED_BUILTIN, menuBuiltInLED.getBoolean());
}

void CALLBACK_FUNCTION onSaveState(int id) {
    menuMgr.save(eeprom);
}
