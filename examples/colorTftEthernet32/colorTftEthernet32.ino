#include "colorTftEthernet32_menu.h"
#include <IoAbstractionWire.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

/*
 * Shows how to use adagraphics with a TFT panel and an ethernet module.
 * This is a 32 bit example, which by default targets 32 bit devices.
 * 
 * For more details see the README.md file in this directory.
 */

// we set up a TFT display first using the exact graphics variable used in the designer.
Adafruit_ILI9341 gfx(6, 7);
AdaColorGfxMenuConfig colorConfig;


// then we setup the IO expander that the also set up in the designer for input.
IoAbstractionRef io8574 = ioFrom8574(0x20, 0); // on addr 0x20 and interrupt pin 0

//
// here we provide a completely custom configuration of color and spacing
//
void prepareCustomConfiguration() {
    // first we set the spacing around title, items and widgets. The make padding function follows the
    // same standard as CSS. Top, right, bottom, left.
	makePadding(colorConfig.titlePadding, 12, 5, 12, 5); // top, right, bottom & left
	makePadding(colorConfig.itemPadding, 5, 5, 3, 5);   // top, right, bottom & left
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
    //colorConfig.editIcon = myEditIcon;
    //colorConfig.activeIcon = myActiveIcon;
    //colorConfig.editIconWidth = myEditWidth;
    //colorConfig.editIconHeight = myEditheight;
}

void setup() {
    // we are responsible for setting up the initial graphics
    gfx.begin();
    gfx.setRotation(3);

    // we used an i2c device (io8574) so must initialise wire too
    Wire.begin();

    prepareCustomConfiguration();

    // and lastly set up the menu
    setupMenu();
}

void loop() {
    taskManager.runLoop();
}

void CALLBACK_FUNCTION onVoltageChange(int id) {
    // TODO - your menu change code
}

void CALLBACK_FUNCTION onCurrentChange(int id) {
    // TODO - your menu change code
}

void CALLBACK_FUNCTION onLimitMode(int id) {
    // TODO - your menu change code
}
