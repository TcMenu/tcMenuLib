#include <mbed.h>
#include "stm32EncoderLcdI2c_menu.h"

// Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/

//
// Set up the logging support in IoAbstraction. For mbed we rely on an MBedLogger being created.
//
BufferedSerial serPort(USBTX, USBRX);
MBedLogger LoggingPort(serPort);

I2C i2cDisplay(PB_9, PB_8);

void setup() {
    serPort.set_baud(115200);
    serdebugF("LCD mbed is starting");
    setupMenu();
}

int main() {
    setup();
    while(1) {
        taskManager.runLoop();
    }
}
