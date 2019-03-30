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

