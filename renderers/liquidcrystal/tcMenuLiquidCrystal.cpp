/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * LiquidCrystalIO renderer that renders menus onto this type of display. This file is a plugin file and should not
 * be directly edited, it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 * 
 * LIBRARY REQUIREMENT
 * This renderer is designed for use with this library: https://github.com/davetcc/LiquidCrystalIO
 */

#include "tcMenuLiquidCrystal.h"

LiquidCrystalRenderer::LiquidCrystalRenderer(LiquidCrystal& lcd, uint8_t dimX, uint8_t dimY) : BaseMenuRenderer(dimX) {
	this->dimY = dimY;
	this->lcd = &lcd;
}

void LiquidCrystalRenderer::render() {
	uint8_t locRedrawMode = redrawMode;
	redrawMode = MENUDRAW_NO_CHANGE;
	if (locRedrawMode == MENUDRAW_COMPLETE_REDRAW) {
		lcd->clear();
	}

	countdownToDefaulting();

	MenuItem* item = currentRoot;
	uint8_t cnt = 0;

	// first we find the first currently active item in our single linked list
	if (offsetOfCurrentActive() >= dimY) {
		uint8_t toOffsetBy = (offsetOfCurrentActive() - dimY) + 1;

		if(lastOffset != toOffsetBy) locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
		lastOffset = toOffsetBy;

		while (item != NULL && toOffsetBy--) {
			item = item->getNext();
		}
	}
	else {
		if(lastOffset != 0xff) locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
		lastOffset = 0xff;
	}

	// and then we start drawing items until we run out of screen or items
	while (item && cnt < dimY) {
		if (locRedrawMode != MENUDRAW_NO_CHANGE || item->isChanged()) {
			renderMenuItem(cnt, item);
		}
		++cnt;
		item = item->getNext();
	}
}

void LiquidCrystalRenderer::renderMenuItem(uint8_t row, MenuItem* item) {
	if (item == NULL || row > dimY) return;

	item->setChanged(false);

	lcd->setCursor(0, row);

	memset(buffer, 32, bufferSize);
	buffer[bufferSize] = 0;

	buffer[0] = item->isEditing() ? '=' : (item->isActive() ? '>' : ' ');
    uint8_t finalPos = item->copyNameToBuffer(buffer, 1, bufferSize);
    buffer[finalPos] = 32;

	menuValueToText(item, JUSTIFY_TEXT_RIGHT);
	lcd->print(buffer);
}

