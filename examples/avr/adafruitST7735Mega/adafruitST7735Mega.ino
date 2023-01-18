/**
 * This example shows the use of an ST7735 and MCP23017 IO expander with a MEGA2560. It is configured out of the box
 * to work with a MEGA2560, ST7735 display and a rotary encoder connected on an MCP23017 IO expander. Other than this
 * it is a relatively simple demonstration of features.
 *
 * It uses the display buffering to reduce flicker, but this does use a little extra of the MEGA's RAM - ~1600 bytes.
 *
 * For the MCP23017:
 *   - device interrupt pin wired to Arduino on pin 2.
 *   - A and B are wired to the 23017 on pins 6 and 7.
 *   - Select is wired to the 23017 on 5.
 * Display:
 *   - Display CS is on 6 and RS on 7.
 *   - Don't forget the display is not 5V tolerant, all lines must be shifted.
 */
#include "adafruitST7735Mega_menu.h"

void setup() {
    IOLOG_START_SERIAL

    Wire.begin();
    setupMenu();
}

void loop() {
    taskManager.runLoop();
}
