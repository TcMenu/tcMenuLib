#include "remoteControlSerial_menu.h"

/*
 * Shows how to use basic Serial remote communication using the inbuilt serial port
 * For more details see the README.md file in this directory.
 */

//
// This function is registered in setup to be called every 200 millis by task manager.
// It updates the analog voltage menu items to show the values on each of the analog pins.
// When we call the setCurrentValue (or setBooleanValue on booleans) not only is the menu
// item updated on the display, but it's also marked as needing sending remotely as well.
//
void updateAnalogMenuItems() {
    menuA0Voltage.setCurrentValue(analogRead(A0));
    menuA1Voltage.setCurrentValue(analogRead(A1));
    menuA2Voltage.setCurrentValue(analogRead(A2));
}

void setup() {
    // for 32 bit boards we should wait for serial before proceeding.
    while(!Serial);
    Serial.begin(115200);

    // for serial communication, we only need to setup the speed here.
    // in this example I am using a bluetooth module with serial1, you could switch to
    // regular Serial by changing the below line and change the menu designer code generator
    // too.
    Serial1.begin(9600);

    // we are going to be toggling this so need to set it as output.
    pinMode(LED_BUILTIN, OUTPUT);

    // added by designer to initialise the menu.
    setupMenu();

    // task manager is asked to schedule the udpate function below every 200ms
    taskManager.scheduleFixedRate(200, updateAnalogMenuItems);
}

void loop() {
    // added by designer to run the taskManager. Don't put any long running tasks in loop.
    taskManager.runLoop();
}

//
// When the push me (action item) is pressed, this method is called back.
// It purely inverts the current state of the built in LED.
//
void CALLBACK_FUNCTION onPushMe(int /*id*/) {
    bool ledCurrent = digitalRead(LED_BUILTIN);
    digitalWrite(LED_BUILTIN, !ledCurrent);
}
