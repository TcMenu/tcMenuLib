/**
 * ESP8266 example of the *simplest* possible menu on u8g2.
 * 
 * This example shows about the most basic example possible and should be useful for anyone
 * trying to get started with either adafruit graphics or u8g2. It is missing title widgets,
 * remote capabilities, EEPROM storage and many other things but makes for the simplest
 * possible starting point for a graphical build.
 * 
 * The circuit uses a PCF8574 for the input using a rotary encoder, a common configuration.
 */

#include "simpleU8g2_menu.h"
#include <Wire.h>
#include <IoAbstractionWire.h>

// the width and height of the attached OLED display.
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// Here we declare the variable using exactly the name that we used in the 
// designers code generator panel for the graphics variable. The name and
// type must match exactly
U8G2_SSD1306_128X64_NONAME_F_SW_I2C gfx(U8G2_R0, 5, 4);

// this is the interrupt pin connection from the PCF8574 back to the ESP8266 board.
#define IO_INTERRUPT_PIN 12

// as we've attached the rotary encoder to an I2C PCF8574 device we need to
// declare it here. We've told the designer that we would when we set the
// switch IO device.
IoAbstractionRef io8574 = ioFrom8574(0x20, IO_INTERRUPT_PIN);

//
// In a tcMenu application, before calling setupMenu it's your responsibility to ensure
// that the display you're going to use is ready for drawing. You also need to start
// wire if you have any I2C devices. Here I start serial for some printing in the callback.
//
void setup() {
    // If you use i2c devices, be sure to start wire.
    Wire.begin();

    Serial.begin(115200);

    // start up the display. Important, the rendering expects this has been done.
    gfx.begin();

    // This is added by tcMenu Designer automatically during the first setup.
    setupMenu();

    // Note:
    // during setup in a full menu application you'd probably load values
    // back from EEPROM and maybe initialise your remote code (see other examples)
}

//
// In any IoAbstraction based application you'll normally use tasks via taskManager
// instead of writing code in loop. You are free to write code here as long as it
// does not delay or block execution. Otherwise task manager will be blocked.
//
void loop() {
    taskManager.runLoop();
}

//
// this is the callback function that we declared in the designer for action
// "Start Toasting". This will be called when the action is performed. Notice
// instead of using callbacks for every toaster setting, we just get the value
// associated with the menu item directly.
//
void CALLBACK_FUNCTION onStartToasting(int id) {
    Serial.println("Let's start toasting");
    Serial.print("Power:  "); Serial.println(menuToasterPower.getCurrentValue());
    Serial.print("Type:   "); Serial.println(menuType.getCurrentValue());
    Serial.print("Frozen: "); Serial.println(menuFrozen.getCurrentValue());
}

