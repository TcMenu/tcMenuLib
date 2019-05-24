#include <Arduino.h>
#include "GfxMenuConfig.h"

void prepareDefaultGfxConfig(ColorGfxMenuConfig<void*>* config) {
	makePadding(config->titlePadding, 5, 5, 20, 5);
	makePadding(config->itemPadding, 5, 5, 3, 5);
	makePadding(config->widgetPadding, 5, 10, 0, 5);

	config->bgTitleColor = RGB(255, 255, 0);
	config->fgTitleColor = RGB(0, 0, 0);
	config->titleFont = NULL;
	config->titleBottomMargin = 10;

	config->bgItemColor = RGB(0, 0, 0);
	config->fgItemColor = RGB(222, 222, 222);
	config->itemFont = NULL;

	config->bgSelectColor = RGB(0, 0, 200);
	config->fgSelectColor = RGB(255, 255, 255);
	config->widgetColor = RGB(30, 30, 30);

	config->titleFontMagnification = 4;
	config->itemFontMagnification = 2;
}

/**
 * The default editing icon for approx 100-150 dpi resolution displays 
 */
const uint8_t defEditingIcon[] PGM_TCM = {
		0b11111111,0b11111111,
		0b01111111,0b11111111,
		0b00011100,0b00000000,
		0b00000111,0b00000000,
		0b00000001,0b1100000,
		0b00000000,0b00111000,
		0b00000000,0b00111000,
		0b00000001,0b11000000,
		0b00000111,0b00000000,
		0b00011100,0b00000000,
		0b01111111,0b11111111,
		0b11111111,0b11111111
};

/**
 * The default active icon for approx 100-150 dpi resolution displays 
 */
const uint8_t defActiveIcon[] PGM_TCM = {
		0b00000000,0b11100000,
		0b00000000,0b11110000,
		0b00000000,0b11111000,
		0b00000000,0b11111100,
		0b00000000,0b11111110,
		0b11111111,0b11111111,
		0b11111111,0b11111111,
		0b00000000,0b11111110,
		0b00000000,0b11111100,
		0b00000000,0b11111000,
		0b00000000,0b11110000,
		0b00000000,0b11100000
};

/**
 * The low resolution icon for editing status
 */
const uint8_t loResEditingIcon[] PROGMEM = {
		0b00000000,
		0b01111110,
		0b00000000,
		0b00000000,
		0b01111110,
		0b00000000,
};

/**
 * The low resolution icon for indicating active status
 */
const uint8_t loResActiveIcon[] PROGMEM = {
		0b00011000,
		0b00011100,
		0b11111110,
		0b11111110,
		0b00011100,
		0b00011000,
};
