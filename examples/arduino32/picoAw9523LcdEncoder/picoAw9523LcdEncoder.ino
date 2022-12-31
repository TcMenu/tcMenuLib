/**
 * A very simple tcMenu example that shows how to configure the menu to work with an AW9523 for both LCD display and
 * rotary encoder input. We've chosen to use interrupt input here, but you could choose either mode you see fit.
 *
 * Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/
 *
 * For this example the wiring is as follows:
 * AW9523 connected to Pico - SDA 12, SCL 13, Device Interrupt - 15
 * AW9523 to rotary encoder - 0 - switch, 1,2 A and B
 * AW9523 to display - RS, RW, EN, Data lines, 0..6 in order.
 * AW9523 Backlight - controlled by the AW9523 LED controller on pin 15
 */
#include "picoAw9523LcdEncoder_menu.h"
#include <tcMenuVersion.h>

const int backlightPin = 15;

// We create an analog device for the LED controller in the AW9523, we can then use the device analog output functions.
// as the actual abstraction was created in the designer, we just use the reference to it from
AW9523AnalogAbstraction aw9523Analog(iodev_aw9523);

void setup() {
    Wire.setSDA(12);
    Wire.setSCL(13);
    Wire.begin();
    setupMenu();

    // now we configure our backlight to be controlled by the AW9523 LED controller
    iodev_aw9523.writeGlobalControl(true, AW9523IoAbstraction::CURRENT_HALF);
    lcd.configureAnalogBacklight(&aw9523Analog, backlightPin);
    lcd.setBacklight(menuBacklight.getCurrentValue());

    // You can provide a callback function to be called when the title is clicked. We call withMenuDialogIfAvailable
    // which in turn calls the function provided if the menu dialog is available. You can then show a dialog, here
    // we show a version dialog.
    setTitlePressedCallback([](int) {
        withMenuDialogIfAvailable([](MenuBasedDialog *dlg) {
            dlg->setButtons(BTNTYPE_CLOSE, BTNTYPE_NONE);
            dlg->show("Pico example", false);
            char sz[20];
            tccore::copyTcMenuVersion(sz, sizeof sz);
            dlg->copyIntoBuffer(sz);
        });
    });
}

void loop() {
    taskManager.runLoop();
}

void CALLBACK_FUNCTION onBacklight(int id) {
    lcd.setBacklight(menuBacklight.getCurrentValue());
}

void CALLBACK_FUNCTION onRunMe(int id) {
    withMenuDialogIfAvailable([](MenuBasedDialog *dlg) {
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
        dlg->show("Run me pressed", false);
        dlg->copyIntoBuffer("from the menu");
    });
}
