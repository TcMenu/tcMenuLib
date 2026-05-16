/**
* ESP8266 example of the *simplest* possible menu on u8g2.
*
* This example shows about the most basic example possible and should be useful for anyone
* trying to get started with either adafruit graphics or u8g2. It is missing title widgets,
* remote capabilities,  and many other things but makes for the simplest possible starting
* point for a graphical build.
* To build your own menu: https://designer.thecoderscorner.com/
* Getting started: https://www.thecoderscorner.com/products/apps/tcmenu-designer/
* Documentation: https://www.thecoderscorner.com/products/arduino-libraries/
*/

#include "SimpleToaster_menu.h"
#include <Wire.h>
#include <IoAbstractionWire.h>

// this is the interrupt pin connection from the PCF8574 back to the ESP8266 board.
#define IO_INTERRUPT_PIN 12

const char pgmCommittedToRom[] PROGMEM = "Saved to ROM";


// Declaring any arrays used by enum/list items
const char* strTypeEnumEntries[] = { "Bread", "Bagel", "Pancake" };

void buildMenu(TcMenuBuilder& builder) {
    builder.usingDynamicEEPROMStorage()
        .analogBuilder(MENU_TOASTER_POWER_ID, "Toaster power", ROM_SAVE, NoMenuFlags, 0, nullptr)
            .offset(0).divisor(1).step(1).maxValue(10).unit("").endItem()
        .enumItem(MENU_TYPE_ID, "Type", ROM_SAVE, strTypeEnumEntries, 3, NoMenuFlags, 0, nullptr)
        .boolItem(MENU_FROZEN_ID, "Frozen", ROM_SAVE, NAMING_YES_NO, NoMenuFlags, false, nullptr)
        .actionItem(MENU_START_TOASTING_ID, "Start toasting", NoMenuFlags, onStartToasting)
        .subMenu(MENU_SETTINGS_ID, "Settings", NoMenuFlags, nullptr)
            .boolItem(MENU_SAFETY_LOCK_ID, "Safety lock", ROM_SAVE, NAMING_TRUE_FALSE, NoMenuFlags, false, nullptr)
            .textItem(MENU_SETTINGS_USER_NAME_ID, "User Name", ROM_SAVE, 5, NoMenuFlags, "", onNameChanged)
            .largeNumberItem(MENU_SETTINGS_SERIAL_NUMBER_ID, "Serial Number", ROM_SAVE, LargeFixedNumber(8, 0, 0U, 0U, false), true, NoMenuFlags, nullptr)
            .actionItem(MENU_SETTINGS_SAVE_SETTINGS_ID, "SaveSettings", NoMenuFlags, onSaveSettings)
            .endSub()
        .subMenu(MENU_EXTRAS_ID, "Extras", NoMenuFlags, nullptr)
            .rgb32Item(MENU_EXTRAS_R_G_B_ID, "RGB", ROM_SAVE, false, NoMenuFlags, RgbColor32(221, 85, 238), nullptr)
            .ipAddressItem(MENU_EXTRAS_IP_ID, "Ip", ROM_SAVE, NoMenuFlags, IpAddressStorage(192, 168, 0, 0), nullptr)
            .timeItem(MENU_EXTRAS_TIME_ID, "Time", ROM_SAVE, NoMenuFlags, EDITMODE_TIME_24H, TimeStorage(14, 00, 00, 0), nullptr)
            .dateItem(MENU_EXTRAS_DATE_ID, "Date", ROM_SAVE, NoMenuFlags, DateStorage(1, 1, 2022), nullptr)
            .endSub();
}



void setup() {
    // If you use i2c and serial devices, be sure to start wire / serial.
    Wire.begin();
    Serial.begin(115200);

    serlogF(SER_DEBUG, "Starting simple U8G2 demo");

    // here we initialise the EEPROM class to 512 bytes of storage
    // don't commit often to this, it's in FLASH
    EEPROM.begin(512);

    // This is added by tcMenu Designer automatically during the first setup.
    setupMenu();

    // lastly we load state from EEPROM.
    menuMgr.load(0xD00D);
}

void loop() {
    taskManager.runLoop();

}

//
// Called when the name field been has changed
//
void CALLBACK_FUNCTION onNameChanged(int id) {
    Serial.print("Name changed to ");
    Serial.println(getMenuUserName().getTextValue());
}

//
// This is attached to the save action on settings, in a real system we may have a
// low voltage detector or other solution for saving.
//
void CALLBACK_FUNCTION onSaveSettings(int id) {
    menuMgr.save(0xD00D);
    EEPROM.commit();

    // here is a brief example of how to show a dialog, usually for information
    // or yes/no answers.
    withMenuDialogIfAvailable([] (MenuBasedDialog* dlg) {
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
        dlg->show(pgmCommittedToRom, false);
        dlg->copyIntoBuffer("just so you know");
    });
}

//
// this is the callback function that we declared in the designer for action
// "Start Toasting". This will be called when the action is performed. Notice
// instead of using callbacks for every toaster setting, we just get the value
// associated with the menu item directly.
//
void CALLBACK_FUNCTION onStartToasting(int id) {
    Serial.println("Let's start toasting");
    Serial.print("Power:  "); Serial.println(getMenuToasterPower().getCurrentValue());
    Serial.print("Type:   "); Serial.println(getMenuType().getCurrentValue());
    Serial.print("Frozen: "); Serial.println(getMenuFrozen().getCurrentValue());
}
