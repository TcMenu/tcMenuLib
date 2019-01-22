#include "takeOverDisplay.h"
#include <EepromAbstractionWire.h>

// We configured io23017 as a variable to handle the reading of inputs and writing of display
// we now create it as an MCP23017 expander on address 0x20 with interrupt pin connected to pin 2.
IoAbstractionRef io23017 = ioFrom23017(0x20, ACTIVE_LOW_OPEN, 2);
#define MCP23017_RESET_PIN 30

// if you don't have an i2c rom uncomment the avr variant and remove the i2c one.
// AvrEeprom eeprom; 
I2cAt24Eeprom eeprom(0x50, 64); // page size 64 for AT24-128 model

void setup() {
    // You must call wire.begin if you are using the wire library.
    Wire.begin();

    // when using an MCP23017 either the reset pin must be held high or you
    // must configure a pin that will hold it high unless you want to reset it.
    pinMode(MCP23017_RESET_PIN, OUTPUT);
    digitalWrite(MCP23017_RESET_PIN, LOW);
    delay(10);
    digitalWrite(MCP23017_RESET_PIN, HIGH);

    // this is put in by the menu designer and must be called (always ensure devices are setup first).
    setupMenu();

    menuMgr.load(eeprom);

}

void loop() {
    taskManager.runLoop();
}

void CALLBACK_FUNCTION onFoodChoice(int id) {
    // copy the enum text for the current value
    char enumStr[20];
    int enumVal = menuFood.getCurrentValue();
    menuFood.copyEnumStrToBuffer(enumStr, enumVal);
    
    // and put it into a text menu item
    menuText.setTextValue(enumStr);
}

int counter = 0;
// this is the function called by the renderer every 1/5 second once the display is
// taken over, we pass this function to takeOverDisplay below.
void myDisplayFunction(bool clicked) {
    if(clicked) {
        // when the button is clicked, we give back to the menu..
        renderer.giveBackDisplay();
    }
    else {
        // otherwise update the counter.
        lcd.setCursor(0, 1);
        lcd.print(++counter);
    }
}

void CALLBACK_FUNCTION onTakeOverDisplay(int id) {
    // in order to take over rendering onto the display we just request the display
    // at which point tcMenu will stop rendering until the display is "given back"
    lcd.clear();
    lcd.print("We have the display!");
    
    // always do this once the display has been prepared.
    renderer.takeOverDisplay(myDisplayFunction);
}

void CALLBACK_FUNCTION onSaveSettings(int id) {
    menuMgr.save(eeprom);
}
