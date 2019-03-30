#include "takeOverDisplay_menu.h"
#include <EepromAbstractionWire.h>
#include <IoAbstractionWire.h>

/**
 * This TcMenu example shows how to take over the display for your own purposes from a menu item.
 * 
 * For more detail see the README.md file
 */

// In the designer UI we configured io23017 as an IoExpander variable for both the input and display.
// We must now create it as an MCP23017 expander. Address is 0x20 with interrupt pin connected to pin 2.
// make sure you've arranged for !RESET pin to be held HIGH!!
IoAbstractionRef io23017 = ioFrom23017(0x20, ACTIVE_LOW_OPEN, 2);


// if you don't have an i2c rom uncomment the avr variant and remove the i2c one.
// AvrEeprom eeprom; 
I2cAt24Eeprom eeprom(0x50, 64); // page size 64 for AT24-128 model

void setup() {
    // You must call wire.begin if you are using the wire library. Importantly the library
    // or designer does not presently do this for you to make it compatible with the widest
    // range of possibilities.
    Wire.begin();

    // this is put in by the menu designer and must be called (always ensure devices are setup first).
    setupMenu();

    // here we use the EEPROM to load back the last set of values.
    menuMgr.load(eeprom);

}

//
// standard setup for all taskManager based sketches. Always call runLoop in the loop.
// Never do anything long running in here.
//
void loop() {
    taskManager.runLoop();
}

//
// When the food choice option is changed on the menu, this function is called, it takes
// the value from menuFood and renders it as text in the menuText text item.
//
void CALLBACK_FUNCTION onFoodChoice(int /*id*/) {
    // copy the enum text for the current value
    char enumStr[20];
    int enumVal = menuFood.getCurrentValue();
    menuFood.copyEnumStrToBuffer(enumStr, sizeof(enumStr), enumVal);
    
    // and put it into a text menu item
    menuText.setTextValue(enumStr);
}

// a counter that we use in the display function when we take over the display.
int counter = 0;

//
// this is the function called by the renderer every 1/5 second once the display is
// taken over, we pass this function to takeOverDisplay below.
//
void myDisplayFunction(unsigned int encoderValue, bool clicked) {
    // we initialise the display on the first call.
    if(counter == 0) {
        switches.changeEncoderPrecision(999, 50);
        lcd.clear();
        lcd.print("We have the display!");
    }

    // We are told when the button is pressed in by the boolean parameter.
    // When the button is clicked, we give back to the menu..
    if(clicked) {
        renderer.giveBackDisplay();
        counter = 0;
    }
    else {
        char buffer[5];
        // otherwise update the counter.
        lcd.setCursor(0, 1);
        ltoaClrBuff(buffer, ++counter, 4, ' ', sizeof(buffer));
        lcd.print(buffer);
        lcd.setCursor(12, 1);
        ltoaClrBuff(buffer, encoderValue, 4, '0', sizeof(buffer));
        lcd.print(buffer);
    }
}

//
// We have an option on the menu to take over the display, this function is called when that
// option is chosen.
//
void CALLBACK_FUNCTION onTakeOverDisplay(int /*id*/) {
    // in order to take over rendering onto the display we just request the display
    // at which point tcMenu will stop rendering until the display is "given back".
    // Don't forget that LiquidCrystalIO uses task manager and things can be happening
    // in the background. Always ensure all operations with the LCD occur on the rendering
    // call back.

    counter = 0;
    renderer.takeOverDisplay(myDisplayFunction);
}

//
// We have a save option on the menu to save the settings. In a real system we could instead
// look at using a power down detection circuit to do this. For more info see below link.
// https://www.thecoderscorner.com/electronics/microcontrollers/psu-control/detecting-power-loss-in-powersupply/
//
void CALLBACK_FUNCTION onSaveSettings(int /*id*/) {
    menuMgr.save(eeprom);
}
