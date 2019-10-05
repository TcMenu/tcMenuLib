/**
 * ESP8266 example of the simplest way possible to construct a u8g2 menu.
 * 
 * This example shows about the most basic example possible and should be useful for anyone
 * trying to get started with either adafruit graphics or u8g2. It is missing title widgets,
 * remote capabilities, EEPROM storage and many other things but makes for the simplest
 * possible starting point for a graphical build.
 * 
 * The circuit uses a PCF8574 with both the input via a rotary encoder, which is common with
 * this device.
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

void setup() {
    Wire.begin();
    Serial.begin(115200);

    // start up the display.
    gfx.begin();

    setupMenu();
}

void loop() {
    taskManager.runLoop();
}

// this is the callback function that we declared in the designer
void CALLBACK_FUNCTION onStartToasting(int id) {
    // TODO - your menu change code
}
