/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>
#include <IoLogging.h>
#include "RuntimeMenuItem.h"

static uint16_t nextAvailableRandomId = RANDOM_ID_START;

const char ALLOWABLE_EDIT_CHARACTERS[] PROGMEM = " .,0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!\"$%^&*()_-+=@';:#{/?\\|<>#{}~";

int nextRandomId() {
	return nextAvailableRandomId++;
}

RuntimeMenuItem::RuntimeMenuItem(MenuType menuType, uint16_t id, RuntimeRenderingFn renderFn,
	uint8_t itemPosition, uint8_t numberOfRows, MenuItem* next)	: MenuItem(menuType, NULL, next) {
	this->id = id;
	this->noOfParts = numberOfRows;
	this->renderFn = renderFn;
	this->itemPosition = itemPosition;
}

ListRuntimeMenuItem::ListRuntimeMenuItem(uint16_t id, int numberOfRows, RuntimeRenderingFn renderFn, MenuItem* next)
	: RuntimeMenuItem(MENUTYPE_RUNTIME_LIST, id, renderFn, 0xff, numberOfRows, next) {
	activeItem = 0;
}

void TextMenuItem::setTextValue(const char* text, bool silent) {
	// skip if they are the same
	if (strncmp(data, text, textLength()) == 0) return;

	strncpy(data, text, textLength());
	cleanUpArray();
	setChanged(true);
	setSendRemoteNeededAll();
	if (!silent) triggerCallback();
}

void TextMenuItem::cleanUpArray() {
	uint8_t len = 0;
	uint8_t actualLen = textLength();
	data[actualLen - 1] = 0;
	while (len < actualLen && data[len] != 0) len++;
	for (int i = len; i < actualLen; i++) {
		data[i] = 0;
	}
}

bool TextMenuItem::setCharValue(uint8_t location, char val) {
	if (location >= (noOfParts - 1)) return false;

	data[location] = val;
	// always ensure zero terminated at last position.
	cleanUpArray();
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
    default: return false;
	}
}

int backSubItemRenderFn(RuntimeMenuItem* /*item*/, uint8_t /*row*/, RenderFnMode mode, char* buffer, int /*bufferSize*/) {
	switch (mode) {
	case RENDERFN_VALUE:
		buffer[0] = 0;
		return true;
    default: return false;
	}
}

inline char charFromEditableSet(int i) {
	const char* ptr = &ALLOWABLE_EDIT_CHARACTERS[i];
	return pgm_read_byte_near(ptr);
}

int findPositionInEditorSet(char ch) {
	if (ch == 0) return 0;
	for (int i = 0; i < ALLOWABLE_CHARS_ENCODER_SIZE; i++) {
		char pgmCmp = charFromEditableSet(i);
		if (pgmCmp == ch) {
			return i + 1;
		}
	}
	return 0; // return zero terminator when not found
}

int textItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
	if (item->getMenuType() != MENUTYPE_TEXT_VALUE) return 0;
	TextMenuItem* txtItem = reinterpret_cast<TextMenuItem*>(item);

	switch (mode) {
	case RENDERFN_VALUE: {
		buffer[0] = 0;
		row--;
		for (int i = 0; i < txtItem->textLength(); ++i) {
			char txtVal = txtItem->getTextValue()[i];

			if (i == row) appendChar(buffer, '[', bufferSize);
			appendChar(buffer, txtVal, bufferSize);
			if (i == row) appendChar(buffer, ']', bufferSize);

			if (!txtVal) break;
		}
		return true;
	}
	case RENDERFN_GETRANGE: {
		// we only enter more rows if there isn't a null terminator
		// but position 0 is always allowed for editing.
		int idx = row - 1;
		return (idx > 1 && txtItem->getTextValue()[idx - 1] == 0) ? 0 : ALLOWABLE_CHARS_ENCODER_SIZE;
	}
	case RENDERFN_SET_VALUE: {
		int idx = row - 1;
		int offset = buffer[0];
		if (offset == 0) {
			txtItem->setCharValue(idx, 0);
			return true;
		}
		return txtItem->setCharValue(idx, charFromEditableSet(offset - 1));
	}
	case RENDERFN_NAME: {
		if (buffer) buffer[0] = 0;
		return true;
	}
	case RENDERFN_GETPART: {
		return findPositionInEditorSet(txtItem->getTextValue()[row - 1]);
	}
    default: return false;
	}
}

void IpAddressMenuItem::setIpAddress(const char * ipData) {
	memset(data, 0, sizeof(data));
	char part[4];
	uint8_t currPart = 0;
	while (*ipData && currPart < 4) {
		part[0] = 0;
		while (*ipData && *ipData != '.') {
			appendChar(part, *ipData, sizeof(part));
			ipData++;
		}
		serdebugF3("IpPart", getId(), part);
		setIpPart(currPart, atoi(part));
		currPart++;
		if(*ipData) ipData++;
	}
}

void IpAddressMenuItem::setIpAddress(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4) {
	data[0] = p1;
	data[1] = p2;
	data[2] = p3;
	data[3] = p4;
	setChanged(true);
	setSendRemoteNeededAll();
}
