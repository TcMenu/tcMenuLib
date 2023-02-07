/**
 * This example shows the use of the Adafruit ILI9431 TFT with capacitive touch integrated. We have connected the TFT
 * device on the VSPI interface, and other pins are defined as follows (although you could change as needed):
 *
 * TFT on VSPI
 * TFT_CS 22
 * TFT_DC 21
 * --
 * SDA 5
 * SCK 4
 *
 * Assumed screen setup is LANDSCAPE.
 *
 * Getting started:
 * https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/
 */

#include "espCapTouchTft_menu.h"
#include <ArduinoEEPROMAbstraction.h>
#include <tcMenuVersion.h>

#define START_CALIBRATION_FOR_TOUCH true

using namespace iotouch;

void setup() {
    // This example uses both serial and wire, we must start them ob
    Serial.begin(115200);
    Wire.begin(5, 4);

    // initialise the eeprom class as we use EEPROM wrapper
    EEPROM.begin(512);

    // all tcmenu sketches must call this to initialise the menu system
    setupMenu();

    // when the root menu title is clicked show the standard application version dialog.
    setTitlePressedCallback([](int) {
        showVersionDialog(&applicationInfo);
    });

#ifdef START_CALIBRATION_FOR_TOUCH
    touchCalibrator.initCalibration([](bool starting) {
        static TouchOrientationSettings oldTouchSettings(false, false, false);
        if(starting) {
            oldTouchSettings = touchScreen.changeOrientation(TouchOrientationSettings(false, true, true));
            gfx.setRotation(0);
        } else {
            touchScreen.changeOrientation(oldTouchSettings);
            gfx.setRotation(1);
        }

    }, true);
#endif

}

void loop() {
    taskManager.runLoop();
}


// see tcMenu list documentation on thecoderscorner.com
int CALLBACK_FUNCTION fnNewRuntimeListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
   switch(mode) {
       case RENDERFN_ACTIVATE:
           // called whenever an item is activated.
           serlogF2(SER_DEBUG, "List Activate ", row);
           return true;
    case RENDERFN_INVOKE:
        // called whenever a list item is invoked.
        serlogF2(SER_DEBUG, "List Invoke ", row);
        return true;
    case RENDERFN_NAME:
        // Lists are based on rows, the row 0 is the title, and the row LIST_PARENT_ITEM_POS is a special row that
        // represents the back button. Each row has a name and a value. Here we just print the row number for the name
        ltoaClrBuff(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_VALUE:
        // Lists are based on rows, the row 0 is the title, and the row LIST_PARENT_ITEM_POS is a special row that
        // represents the back button. Each row has a name and value, for the value we just print V and the row
        buffer[0] = 'V'; buffer[1]=0;
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    case RENDERFN_EEPROM_POS: return 0xffff; // lists are generally not saved to EEPROM
    default: return false;
    }
}

// Here we present a simple Hex text field, it accepts values between 0-9 and A-F
int CALLBACK_FUNCTION CustomTextCallbackRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    // See https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/based-on-runtimemenuitem/
    auto text = static_cast<TextMenuItem *>(item);
    switch(mode) {
    case RENDERFN_NAME:
        // Here we override the name, when we do so, we return true to indicate that we have overridden the name.
        // Returning false here indicates that the default name will be used.
        strcpy(buffer, "HexData");
        return true; // override the name
    case RENDERFN_GETRANGE:
        // this provides the range of values for the current "row" or index. We can only ever edit from 0-9 and A-F
        return 15;
    case RENDERFN_GETPART: {
        // this gets the value for the current character being edited in raw format, suitable for rotary encoders and
        // other index based editors
        if(row == 0) return 0;
        int partVal = text->getTextValue()[row - 1];
        return (partVal >= 65) ? partVal - 55 : partVal - 48;
    }
    case RENDERFN_SET_VALUE:
        // This allows us to set a particular char from a rotary encoder, it is zero based between 0 and the value
        // returned by get range. For keyboards there is a special version that sets the character value as input on
        // a keyboard. You can decide when implementing if you need to support keyboards.
        if(row == 0) return false;
        if(buffer[0] < 10) text->setCharValue(row - 1, buffer[0] + '0');
        else text->setCharValue(row - 1, buffer[0] + 55);
        return true;
    }
    // always pass through unhandled events.
    return textItemRenderFn(item, row, mode, buffer, bufferSize);
}
