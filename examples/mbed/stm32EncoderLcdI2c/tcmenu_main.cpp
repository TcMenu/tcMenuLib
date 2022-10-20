#include <mbed.h>
#include "stm32EncoderLcdI2c_menu.h"

//
// Set up the logging support in IoAbstraction. For mbed we rely on an MBedLogger being created.
//
#ifdef BUILD_FOR_MBED_6
BufferedSerial serPort(USBTX, USBRX);
#else
Serial serPort(USBTX, USBRX);
#endif
MBedLogger LoggingPort(serPort);

I2C i2cDisplay(PF_0, PF_1);

void setup() {
    serdebugF("LCD mbed is starting");
    setupMenu();
}

int main() {
    setup();
    while(1) {
        taskManager.runLoop();
    }
}
