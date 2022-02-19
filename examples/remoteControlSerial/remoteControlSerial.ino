#include "remoteControlSerial_menu.h"

/*
 * Shows how to use basic Serial remote communication using the inbuilt serial port
 * For more details see the README.md file in this directory.
 */

// in our code we defined a scroll that is based on items in RAM, we therefore need to create the array in ram
// with the same name we provided in the sketch, it is arranged of a flat array of width by number of items.
const char * choiceRamArray = "item1item2item3item4 ";

//
// This function is registered in setup to be called every 200 millis by task manager.
// It updates the analog voltage menu items to show the values on each of the analog pins.
// When we call the setCurrentValue (or setBooleanValue on booleans) not only is the menu
// item updated on the display, but it's also marked as needing sending remotely as well.
//
void updateAnalogMenuItems() {
    menuA0.setCurrentValue(analogRead(A0));
    menuA1.setCurrentValue(analogRead(A1));
    menuA2.setCurrentValue(analogRead(A2));
}

void setup() {
    // for 32 bit boards we should wait for serial before proceeding.
    Serial.begin(115200);
    while(!Serial);

    // we are going to be toggling this so need to set it as output.
    pinMode(LED_BUILTIN, OUTPUT);

    // added by designer to initialise the menu.
    setupMenu();

    // we used the simple ada graphics plugin so in this case we only need to do any additional
    // set up after setupMenu has been called.
    gfx.setContrast(50);

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

// see tcMenu list documentation on thecoderscorner.com
// https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
int CALLBACK_FUNCTION fnRtListRtCall(RuntimeMenuItem * item, uint8_t row, RenderFnMode mode, char * buffer, int bufferSize) {
   switch(mode) {
    case RENDERFN_INVOKE:
        serdebugF2("List item selected:", row);
        return true;
    case RENDERFN_NAME:
        if(row == LIST_PARENT_ITEM_POS) {
            strcpy(buffer, "MyList");
        } else {
            strcpy(buffer, "Name");
            fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        }
        return true;
    case RENDERFN_VALUE:
        buffer[0] = 'V'; buffer[1]=0;
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_EEPROM_POS: return 0xFFFF; // lists are generally not saved to EEPROM
    default: return false;
    }
}
