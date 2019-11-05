/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>
#include <IoLogging.h>
#include "EditableLargeNumberMenuItem.h"
#include <IoLogging.h>

void LargeFixedNumber::clear() {
	for (uint8_t i = 0; i < sizeof(bcdRepresentation); i++) bcdRepresentation[i] = 0;
	negative = false;
}

void LargeFixedNumber::setValue(uint32_t whole, uint32_t fraction, bool negative) {
	clear();
	convertToBcdPacked(fraction, 0, fractionDp);
	convertToBcdPacked(whole, fractionDp, 12);
	this->negative = negative;
}

uint32_t LargeFixedNumber::fromBcdPacked(int start, int end) {
	int32_t modulo = dpToDivisor((end - start) - 1);
	int32_t res = 0;
	for (int i = start; i < end; i++) {
		res = res + (getDigit(i) * modulo);
		modulo /= 10L;
	}
	return res;
}

void LargeFixedNumber::convertToBcdPacked(int32_t value, int start, int end) {
	int32_t modulo = dpToDivisor((end - start) - 1);
	for (int i = start; i < end; i++) {
		setDigit(i, min(9, value / modulo));
		value = value % modulo;
		modulo /= 10L;
	}
}

int LargeFixedNumber::getDigit(int digit) {
	if (digit > 11) return false;
	uint8_t r = bcdRepresentation[digit / 2];
	if ((digit % 2) == 0) {
		return r & 0x0f;
	}
	else {
		return (int)(r >> 4);
	}
}

void LargeFixedNumber::setDigit(int digit, int val) {
	if (digit > 11) return;
	if ((digit % 2) == 0) {
		bcdRepresentation[digit / 2] = (bcdRepresentation[digit / 2] & 0xf0) | val;
	}
	else {
		bcdRepresentation[digit / 2] = (bcdRepresentation[digit / 2] & 0x0f) | (val << 4);
	}
}

void EditableLargeNumberMenuItem::setLargeNumberFromString(const char* val) {
	int offset = 0;
	bool negative = false;
	if (val && val[0] == '-') {
		offset++;
		negative = true;
	}
	int32_t whole = parseIntUntilSeparator(val, offset);
	int32_t fract = parseIntUntilSeparator(val, offset);
	data.setValue(whole, fract, negative);
	setSendRemoteNeededAll();
	setChanged(true);
}

void wrapEditor(bool editRow, char val, char* buffer, int bufferSize) {
	if (editRow) {
		appendChar(buffer, '[', bufferSize);
		appendChar(buffer, val, bufferSize);
		appendChar(buffer, ']', bufferSize);
	}
	else {
		appendChar(buffer, val, bufferSize);
	}
}

int largeNumItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
	if (item->getMenuType() != MENUTYPE_LARGENUM_VALUE) return 0;
	EditableLargeNumberMenuItem* numItem = reinterpret_cast<EditableLargeNumberMenuItem*>(item);
	LargeFixedNumber *num = numItem->getLargeNumber();

	switch (mode) {
	case RENDERFN_VALUE: {
		buffer[0] = 0;
		bool editingMode = row > 0 && row < 0xf;
		bool hadNonZero = false;
		uint8_t editPosition = 0;

		if (editingMode || num->isNegative()) {
			wrapEditor(row == 1, num->isNegative() ? '-' : '+', buffer, bufferSize);
		}

		row = row - 2;

		for (int i = num->decimalPointIndex(); i < (numItem->getNumberOfParts() - 1); i++) {
			char txtVal = num->getDigit(i) + '0';
			hadNonZero |= txtVal != '0';
			if (hadNonZero || editingMode) {
				wrapEditor(row == editPosition, txtVal, buffer, bufferSize);
			}
			editPosition++;
		}
		appendChar(buffer, '.', bufferSize);

		for (int i = 0; i < num->decimalPointIndex(); i++) {
			char txtVal = num->getDigit(i) + '0';
			wrapEditor(row == editPosition, txtVal, buffer, bufferSize);
			editPosition++;
		}
		return true;
	}
	case RENDERFN_GETRANGE: {
		return row == 1 ? 1 : 9;
	}
	case RENDERFN_SET_VALUE: {
		int idx = row - 1;
		if (idx == 0) {
			num->setNegative(buffer[0]);
			return true;
		}
		idx--;
		int dpIndex = (numItem->getNumberOfParts() - 1) - num->decimalPointIndex();
		int pos = idx >= dpIndex ? idx - dpIndex : idx + num->decimalPointIndex();
		num->setDigit(pos, buffer[0]);
		return true;
	}
	case RENDERFN_NAME: {
		if (buffer) buffer[0] = 0;
		return true;
	}
	case RENDERFN_GETPART: {
		int idx = row - 1;
		if (idx == 0) {			
			return num->isNegative();
		}
		idx--;
		int dpIndex = (numItem->getNumberOfParts() - 1) - num->decimalPointIndex();
		int pos =  idx >= dpIndex ? idx - dpIndex : idx + num->decimalPointIndex();
		return num->getDigit(pos);
	}
	default: return false;
	}
}