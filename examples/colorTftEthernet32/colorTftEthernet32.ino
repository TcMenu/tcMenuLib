#include "colorTftEthernet32_menu.h"
#include <IoAbstractionWire.h>
#include <EepromAbstractionWire.h>
#include <AnalogDeviceAbstraction.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

/*
 * Shows how to use adagraphics with a TFT panel and an ethernet module.
 * This is a 32 bit example, which by default targets 32 bit devices.
 * Assumed board for this is a SAMD based MKR board.
 * 
 * For more details see the README.md file in this directory.
 */

// we are going to allow control of the menu over local area network
// so therefore must configure ethernet..
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 96);

// we set up a TFT display first using the exact graphics variable used in the designer.
Adafruit_ILI9341 gfx(6, 7);

// we also want to customise the colours and menu spacing. So we create this config object too
// which we initialise during the setup method.
AdaColorGfxMenuConfig colorConfig;

// We also store/restore state from an i2c EEPROM chip
I2cAt24Eeprom eeprom(0x50, PAGESIZE_AT24C128);

// then we setup the IO expander that the also set up in the designer for input.
IoAbstractionRef io8574 = ioFrom8574(0x20, 0); // on addr 0x20 and interrupt pin 0

// and we create an analog device with enhanced range because we are using a 32bit board.
ArduinoAnalogDevice analogDevice(12, 10);

float fractionsPerUnit;

//
// here we provide a completely custom configuration of color and spacing
//
void prepareCustomConfiguration() {
    // first we set the spacing around title, items and widgets. The make padding function follows the
    // same standard as CSS. Top, right, bottom, left.
	makePadding(colorConfig.titlePadding, 12, 5, 12, 5); // top, right, bottom & left
	makePadding(colorConfig.itemPadding, 5, 3, 6, 5);   // top, right, bottom & left
	makePadding(colorConfig.widgetPadding, 5, 10, 0, 5);// top, right, bottom & left

    // and then the foreground, background and font of the title
	colorConfig.bgTitleColor = RGB(50, 100, 200);
	colorConfig.fgTitleColor = RGB(0, 0, 0);
	colorConfig.titleFont = &FreeSans18pt7b;
	colorConfig.titleBottomMargin = 10; // the space between title and items.

    // and then the colours for items.
	colorConfig.bgItemColor = RGB(0, 0, 0);
	colorConfig.fgItemColor = RGB(222, 222, 222);
	colorConfig.itemFont = &FreeSans9pt7b;

    // and when items are selected.
	colorConfig.bgSelectColor = RGB(0, 50, 200);
	colorConfig.fgSelectColor = RGB(255, 255, 255);
	colorConfig.widgetColor = RGB(30, 30, 30);

    // you can set the size scaling of the fonts
	colorConfig.titleFontMagnification = 1;
	colorConfig.itemFontMagnification = 1;

    // and if you really want to, provide alternative bitmaps for the edit / active icon
    // otherwise both icons must be set to NULL.
    //colorConfig.editIcon = myEditIcon;
    //colorConfig.activeIcon = myActiveIcon;
    //colorConfig.editIconWidth = myEditWidth;
    //colorConfig.editIconHeight = myEditheight;
    colorConfig.editIcon = NULL;
    colorConfig.activeIcon = NULL;
}

void setup() {
    // spin up the Ethernet library
    Ethernet.begin(mac, ip);

    // we are responsible for setting up the initial graphics
    gfx.begin();
    gfx.setRotation(3);

    // we used an i2c device (io8574) so must initialise wire too
    Wire.begin();

    // and set up the dac on the 32 bit board.
    analogDevice.initPin(A0, DIR_OUT);
    analogDevice.initPin(A1, DIR_IN);
    fractionsPerUnit = ((float)analogDevice.getMaximumRange(DIR_IN, A1)) / 5.0;

    // we have our own graphics configuration. it must be initialised
    // before calling setupMenu..
    prepareCustomConfiguration();

    // set up the menu
    setupMenu();

    // and then load back the previous state
    menuMgr.load(eeprom);

    taskManager.scheduleFixedRate(2250, [] {
        Serial.print(".");
        float a1Value = analogDevice.getCurrentValue(A1);
        menuVoltA1.setFloatValue(a1Value / fractionsPerUnit);
    });
}

void loop() {
    taskManager.runLoop();
}

void writeToDac() {
    float volts = menuVoltage.getCurrentValue() / 2.0;
    float curr = menuCurrent.getCurrentValue() / 100.0;
    float total = volts * curr;
    analogDevice.setCurrentValue(A0, (unsigned int)total);
    menuVoltA0.setFloatValue(total);
}

void CALLBACK_FUNCTION onVoltageChange(int id) {
    writeToDac();
}

void CALLBACK_FUNCTION onCurrentChange(int id) {
    writeToDac();
}

void CALLBACK_FUNCTION onLimitMode(int id) {
    // TODO - your menu change code
}

void CALLBACK_FUNCTION onSaveRom(int id) {
    // save out the state, in a real system we could detect power down for this.
    menuMgr.save(eeprom);
}
