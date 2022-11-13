// Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/

#include "esp32s3TftEncoder_menu.h"

void setup() {
    setupMenu();

}

void loop() {
    taskManager.runLoop();

}
