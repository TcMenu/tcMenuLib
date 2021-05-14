/**
 * BETA early access, the frame buffer is not yet fully working. Only for evaluation.
 */

#include <mbed.h>
#include "stm32f429FrameBuffer_menu.h"

BufferedSerial console(USBTX, USBRX);
MBedLogger LoggingPort(console);

void setup() {
    setupMenu();

}

int main() {
    console.set_baud(115200);
    setup();
    while(1) {
        taskManager.runLoop();
    }
}



void CALLBACK_FUNCTION onTargetChanged(int id) {
    // TODO - your menu change code
}
