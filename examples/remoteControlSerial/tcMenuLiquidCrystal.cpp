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
	this->backChar = '<';
	this->forwardChar = '>';
	this->editChar = '=';
}

LiquidCrystalRenderer::~LiquidCrystalRenderer() { 
    delete this->buffer; 
    if(dialog) delete dialog;
}

void LiquidCrystalRenderer::setEditorChars(char back, char forward, char edit) {
	backChar = back;
	forwardChar = forward;
	editChar = edit;
}

void LiquidCrystalRenderer::renderList() {
	ListRuntimeMenuItem* runList = reinterpret_cast<ListRuntimeMenuItem*>(currentRoot);
	
	uint8_t maxY = min(dimY, runList->getNumberOfParts());
	uint8_t currentActive = runList->getActiveIndex();
	
	uint8_t offset = 0;
	if (currentActive >= maxY) {
		offset = (currentActive+1) - maxY;
	}

	for (int i = 0; i < maxY; i++) {
		uint8_t current = offset + i;
		RuntimeMenuItem* toDraw = (current==0) ? runList->asBackMenu() : runList->getChildItem(current - 1);
		renderMenuItem(i, toDraw);
	}

	// reset the list item to a normal list again.
	runList->asParent();
}

void LiquidCrystalRenderer::render() {
	uint8_t locRedrawMode = redrawMode;
	redrawMode = MENUDRAW_NO_CHANGE;
	if (locRedrawMode == MENUDRAW_COMPLETE_REDRAW) {
		lcd->clear();
	}

	countdownToDefaulting();

	if (currentRoot->getMenuType() == MENUTYPE_RUNTIME_LIST ) {
		if (currentRoot->isChanged() || locRedrawMode != MENUDRAW_NO_CHANGE) {
			renderList();
		}
	}
	else {
		MenuItem* item = currentRoot;
		uint8_t cnt = 0;

		// first we find the first currently active item in our single linked list
		if (offsetOfCurrentActive() >= dimY) {
			uint8_t toOffsetBy = (offsetOfCurrentActive() - dimY) + 1;

			if (lastOffset != toOffsetBy) locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
			lastOffset = toOffsetBy;

			while (item != NULL && toOffsetBy--) {
				item = item->getNext();
			}
		}
		else {
			if (lastOffset != 0xff) locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
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
}

void LiquidCrystalRenderer::renderMenuItem(uint8_t row, MenuItem* item) {
	if (item == NULL || row > dimY) return;

	item->setChanged(false);
	lcd->setCursor(0, row);

	int offs;
	if (item->getMenuType() == MENUTYPE_BACK_VALUE) {
		buffer[0] = item->isActive() ? backChar : ' ';
		buffer[1] = backChar;
		offs = 2;
	}
	else {
		buffer[0] = item->isEditing() ? editChar : (item->isActive() ? forwardChar : ' ');
		offs = 1;
	}
    uint8_t finalPos = item->copyNameToBuffer(buffer, offs, bufferSize);
	for(uint8_t i = finalPos; i < bufferSize; ++i)  buffer[i] = 32;
	buffer[bufferSize] = 0;

	if (isItemActionable(item)) {
		buffer[bufferSize - 1] = forwardChar;
	}
	else {
		menuValueToText(item, JUSTIFY_TEXT_RIGHT);
	}
	serdebugF3("Buffer: ", row, buffer);
	lcd->print(buffer);
}

BaseDialog* LiquidCrystalRenderer::getDialog() {
    if(dialog == NULL) {
        dialog = new LiquidCrystalDialog(this);
    }
    return dialog;
}

// dialog

void LiquidCrystalDialog::internalRender(int currentValue) {
    LiquidCrystalRenderer* lcdRender = ((LiquidCrystalRenderer*)MenuRenderer::getInstance());
    LiquidCrystal* lcd = lcdRender->getLCD();
    if(needsDrawing == MENUDRAW_COMPLETE_REDRAW) {
        lcd->clear();
    }

    char data[20];
    strncpy_P(data, headerPgm, sizeof(data));
    data[sizeof(data)-1]=0;
    lcd->setCursor(0,0);
    lcd->print(data);
    
    // we can only print the buffer on a newline when there's enough rows.
    // so on 16x2 we have to show the buffer over the header. It's all we
    // can do.
    int nextY = 3;
    if(isCompressedMode()) {
        int len = strlen(lcdRender->getBuffer());
        int startX = lcdRender->getBufferSize() - len;
        lcd->setCursor(startX,0);
        lcd->print(lcdRender->getBuffer());
        nextY = 1; 
    }
    else {
        lcd->setCursor(0,1);
        lcd->print(lcdRender->getBuffer());
    }

    if(button1 != BTNTYPE_NONE) {
        copyButtonText(data, 0, currentValue);
        lcd->setCursor(0, nextY);
        lcd->print(data);
    }
    if(button2 != BTNTYPE_NONE) {
        copyButtonText(data, 1, currentValue);
        int startX = lcdRender->getBufferSize() - strlen(data);
        lcd->setCursor(startX, nextY);
        lcd->print(data);
    }
}
