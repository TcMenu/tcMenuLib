/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>
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

void TextMenuItem::setCharValue(uint8_t location, char val) {
	if (location >= noOfParts) return;

	data[location] = val;
	setChanged(true);
	setSendRemoteNeededAll();
}

int ipAddressRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
	if (item->getMenuType() != MENUTYPE_IPADDRESS) return 0;
	IpAddressMenuItem* ipItem = reinterpret_cast<IpAddressMenuItem*>(item);

	switch (mode) {
	case RENDERFN_VALUE: {
		buffer[0] = 0;
		byte* data = ipItem->getIpAddress();
		if (row == 1) appendChar(buffer, '=', bufferSize);
		fastltoa(buffer, data[0], 3, NOT_PADDED, bufferSize);

		appendChar(buffer, (row == 2) ? '=' : '.', bufferSize);
		fastltoa(buffer, data[1], 3, NOT_PADDED, bufferSize);

		appendChar(buffer, (row == 3) ? '=' : '.', bufferSize);
		fastltoa(buffer, data[2], 3, NOT_PADDED, bufferSize);

		appendChar(buffer, (row == 4) ? '=' : '.', bufferSize);
		fastltoa(buffer, data[3], 3, NOT_PADDED, bufferSize);
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
	if (item->getMenuType() != MENUTYPE_TEXT_VALUE) return 0;
	TextMenuItem* txtItem = reinterpret_cast<TextMenuItem*>(item);

	switch (mode) {
	case RENDERFN_VALUE: {
		buffer[0] = 0;
		for (int i = 0; i < txtItem->textLength(); ++i) {
			if ((i + 1) == row) appendChar(buffer, '[', bufferSize);
			appendChar(buffer, txtItem->getTextValue()[i], bufferSize);
			if ((i + 1) == row) appendChar(buffer, ']', bufferSize);
		}
		return true;
	}
	case RENDERFN_GETPART: 
		return (int)txtItem->getTextValue()[row - 1];
	case RENDERFN_SET_VALUE: {
		txtItem->setCharValue(row - 1, buffer[0]);
		return true;
	}
	case RENDERFN_NAME: {
		if (buffer) buffer[0] = 0;
		return true;
	}
	case RENDERFN_GETRANGE: return 255;
	}
}