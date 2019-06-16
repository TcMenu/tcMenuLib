/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>
#include <IoLogging.h>
#include "RuntimeMenuItem.h"

const AnyMenuInfo runtimeBlankInfo PROGMEM = { "", 0xffff, 0xffff, 0, NULL };

RuntimeMenuItem::RuntimeMenuItem(MenuType menuType, uint16_t id, uint16_t eeprom, RuntimeRenderingFn renderFn,
	uint8_t itemPosition, uint8_t numberOfRows, MenuItem* next = NULL)	: MenuItem(menuType, &runtimeBlankInfo, next) {
	this->id = id;
	this->noOfParts = numberOfRows;
	this->eeprom = eeprom;
	this->renderFn = renderFn;
	this->itemPosition = itemPosition;
}

ListRuntimeMenuItem::ListRuntimeMenuItem(uint16_t id, int numberOfRows, RuntimeRenderingFn renderFn, MenuItem* next = NULL)
	: RuntimeMenuItem(MENUTYPE_RUNTIME_LIST, id, 0xffff, renderFn, 0xff, numberOfRows, next) {
}

void TextMenuItem::setTextValue(const char* text, bool silent) {
	// skip if they are the same
	if (strncmp(data, text, textLength()) == 0) return;

	strncpy(data, text, textLength());
	data[textLength() - 1] = 0;
	setChanged(true);
	setSendRemoteNeededAll();
	if (!silent) triggerCallback();
}

bool TextMenuItem::setCharValue(uint8_t location, char val) {
	if (location >= (noOfParts - 1)) return false;

	data[location] = val;
	// always ensure zero terminated at last position.
	data[noOfParts - 1] = 0;
	setChanged(true);
	setSendRemoteNeededAll();
	return true;
}

void wrapForEdit(byte *ipData, uint8_t idx, uint8_t row, char* buffer, int bufferSize) {
	--row;

	if (idx == row) appendChar(buffer, '[', bufferSize);
	fastltoa(buffer, ipData[idx], 3, NOT_PADDED, bufferSize);
	if (idx == row) appendChar(buffer, ']', bufferSize);
}

int ipAddressRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
	if (item->getMenuType() != MENUTYPE_IPADDRESS) return 0;
	IpAddressMenuItem* ipItem = reinterpret_cast<IpAddressMenuItem*>(item);

	switch (mode) {
	case RENDERFN_VALUE: {
		buffer[0] = 0;
		byte* data = ipItem->getIpAddress();
		wrapForEdit(data, 0, row, buffer, bufferSize);
		appendChar(buffer, '.', bufferSize);
		wrapForEdit(data, 1, row, buffer, bufferSize);
		appendChar(buffer, '.', bufferSize);
		wrapForEdit(data, 2, row, buffer, bufferSize);
		appendChar(buffer, '.', bufferSize);
		wrapForEdit(data, 3, row, buffer, bufferSize);
		return true;
	}
	case RENDERFN_SET_VALUE: {
		ipItem->setIpPart(row - 1, (byte)buffer[0]);
		return true;
	}
	case RENDERFN_GETPART: {
		byte* data = ipItem->getIpAddress();
		return (int)data[row - 1];
	}
	case RENDERFN_NAME: {
		if (buffer) buffer[0] = 0;
		return true;
	}
	case RENDERFN_GETRANGE: return 255;
	}
}

int textItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
	serdebugF4("Rendering text item", row, mode, bufferSize);
	if (item->getMenuType() != MENUTYPE_TEXT_VALUE) return 0;
	TextMenuItem* txtItem = reinterpret_cast<TextMenuItem*>(item);

	switch (mode) {
	case RENDERFN_VALUE: {
		buffer[0] = 0;
		row--;
		for (int i = 0; i < txtItem->textLength(); ++i) {
			if (i == row) appendChar(buffer, '[', bufferSize);
			appendChar(buffer, txtItem->getTextValue()[i], bufferSize);
			if (i == row) appendChar(buffer, ']', bufferSize);
		}
		return true;
	}
	case RENDERFN_GETRANGE: {
		// we only enter more rows if there isn't a null terminator
		// but position 0 is always allowed for editing.
		return (row > 1 && txtItem->getTextValue()[row - 1] == 0) ? 0 : 255;
	}
	case RENDERFN_SET_VALUE: {
		int idx = row - 1;
		return txtItem->setCharValue(idx, buffer[0]);
	}
	case RENDERFN_NAME: {
		if (buffer) buffer[0] = 0;
		return true;
	}
	case RENDERFN_GETPART:
		return (int)txtItem->getTextValue()[row - 1];
	}
}