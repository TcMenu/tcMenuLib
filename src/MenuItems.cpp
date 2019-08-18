/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "MenuItems.h"
#include "RuntimeMenuItem.h"

MenuItem::MenuItem(MenuType menuType, const AnyMenuInfo* menuInfo, MenuItem* next) {
	this->flags = 0;
	this->menuType = menuType;
	if(menuInfo != NULL) this->info = menuInfo;
	this->next = next;
    this->setChanged(true); // items always start out needing redrawing.
}

bool MenuItem::isSendRemoteNeeded(uint8_t remoteNo) {
    return bitRead(flags, (remoteNo + (int)MENUITEM_REMOTE_SEND0));
}

void MenuItem::setSendRemoteNeeded(uint8_t remoteNo, bool needed) {
	bitWrite(flags, (remoteNo + (int)MENUITEM_REMOTE_SEND0), (needed && !isLocalOnly()));
}

void MenuItem::setEditing(bool active) {
	bool isEditOnEntry = isEditing();
	bitWrite(flags, MENUITEM_EDITING, active); setChanged(true);
	if (isMenuRuntimeMultiEdit(this) && !active && isEditOnEntry) {
		EditableMultiPartMenuItem<void*>* item = reinterpret_cast<EditableMultiPartMenuItem<void*>*>(this);
		item->stopMultiEdit();
	}
}

void MenuItem::setSendRemoteNeededAll() {
    // make sure local only fields are never marked for sending.
    if(isLocalOnly()) clearSendRemoteNeededAll();

	flags = flags | MENUITEM_ALL_REMOTES;
}

void MenuItem::clearSendRemoteNeededAll() {
	flags = flags & (~MENUITEM_ALL_REMOTES);
}

void MenuItem::triggerCallback() {
	if (isMenuRuntime(this)) {
		return asRuntimeItem(this)->runCallback();
	}
	else {
		MenuCallbackFn fn = get_info_callback(&info->callback);
		if (fn != NULL) fn(getId());
	}
}

uint8_t MenuItem::copyNameToBuffer(char* buf, int offset, int size) {
	if (isMenuRuntime(this)) {
		asRuntimeItem(this)->copyRuntimeName(buf + offset, size - offset);
		return strlen(buf + offset) + offset;
	}
	else {
		const char* name = info->name;
		uint8_t ret = safeProgCpy(buf + offset, name, size - offset);
		return ret + offset;
	}
}

uint16_t MenuItem::getId()
{
	if (isMenuRuntime(this)) {
		return asRuntimeItem(this)->getRuntimeId();
	}
	
	return get_info_uint(&info->id);
}

uint16_t MenuItem::getMaximumValue()
{
	if (isMenuRuntime(this)) {
		return asRuntimeItem(this)->getNumberOfParts();
	}
	else return get_info_uint(&info->maxValue);
}

uint16_t MenuItem::getEepromPosition()
{
	if (isMenuRuntime(this)) return asRuntimeItem(this)->getRuntimeEeprom();
	
	return get_info_uint(&info->eepromAddr);
}

// on avr boards we store all info structures in progmem, so we need this code to
// pull the enum structures out of progmem. Otherwise we just read it out normally

#ifdef __AVR__

void EnumMenuItem::copyEnumStrToBuffer(char* buffer, int size, int idx) {
    char** itemPtr = ((char**)pgm_read_ptr_near(&((EnumMenuInfo*)info)->menuItems) + idx);
    char* itemLoc = (char *)pgm_read_ptr_near(itemPtr);
    strncpy_P(buffer, itemLoc, size);
	buffer[size - 1] = 0;
}

int EnumMenuItem::getLengthOfEnumStr(int idx) {
    char** itemPtr = ((char**)pgm_read_ptr_near(&((EnumMenuInfo*)info)->menuItems) + idx);
    char* itemLoc = (char *)pgm_read_ptr_near(itemPtr);
    return strlen_P(itemLoc);
}

#else 

void EnumMenuItem::copyEnumStrToBuffer(char* buffer, int size, int idx) {
    EnumMenuInfo* enumInfo = (EnumMenuInfo*)info;
    const char * const* choices = enumInfo->menuItems;
    const char * choice = choices[idx];
    strncpy(buffer, choice, size);
	buffer[size - 1] = 0;
}

int EnumMenuItem::getLengthOfEnumStr(int idx) {
    EnumMenuInfo* enumInfo = (EnumMenuInfo*)info;
    const char * const* choices = enumInfo->menuItems;
    const char * choice = choices[idx];
    return strlen(choice);
}

#endif

bool isSame(float d1, float d2) {
	float result = abs(d1 - d2);
	return result < 0.0000001;
}

void AnalogMenuItem::copyValue(char * buffer, uint8_t bufferSize) {
	uint8_t dpNeeded = getDecimalPlacesForDivisor();
	WholeAndFraction wf = getWholeAndFraction();

	ltoaClrBuff(buffer, wf.whole, 5, NOT_PADDED, bufferSize);

	if (dpNeeded != 0) {
		appendChar(buffer, '.', sizeof bufferSize);
		fastltoa(buffer, wf.fraction, dpNeeded, '0', bufferSize);
	}
	uint8_t numLen = strlen(buffer);
	if(numLen < bufferSize) copyUnitToBuffer(buffer + numLen, bufferSize - numLen);
}

float AnalogMenuItem::getAsFloatingPointValue() {
	WholeAndFraction wf = getWholeAndFraction();
	float fract = (float(wf.fraction) / float(getActualDecimalDivisor()));
	return  (wf.whole < 0) ? (wf.whole - fract) : (wf.whole + fract);
}

uint16_t AnalogMenuItem::getActualDecimalDivisor() {
	uint16_t divisor = getDivisor();
	if (divisor < 2) return 1;
	return (divisor > 1000) ? 10000 : (divisor > 100) ? 1000 : (divisor > 10) ? 100 : 10;
}

WholeAndFraction AnalogMenuItem::getWholeAndFraction() {
	WholeAndFraction wf;
	int calcVal = int16_t(getCurrentValue()) + getOffset();
	int divisor = getDivisor();

	if (divisor < 2) {
		wf.whole = calcVal;
		wf.fraction = 0;
	}
	else {
		wf.whole = calcVal / int16_t(divisor);
		int fractMax = getActualDecimalDivisor();
		wf.fraction = abs((calcVal % divisor)) * (fractMax / divisor);
	}
	return wf;
}

void AnalogMenuItem::setFromWholeAndFraction(WholeAndFraction wf) {
	int fractMax = getActualDecimalDivisor();
	uint16_t divisor = getDivisor();
	int correctedFraction = wf.fraction / (fractMax / divisor);
	setCurrentValue(((wf.whole * getDivisor()) + correctedFraction) - getOffset());
	serdebugF4("setWF ", wf.whole, wf.fraction, getCurrentValue());
}

void AnalogMenuItem::setFromFloatingPointValue(float value) {
	WholeAndFraction wf;
	wf.whole = value;
	wf.fraction = abs(value - float(wf.whole)) * getActualDecimalDivisor();

	setFromWholeAndFraction(wf);
}

uint8_t AnalogMenuItem::getDecimalPlacesForDivisor() {
	uint16_t divisor = getDivisor();
	if (divisor < 2) return 0;

	return (divisor > 1000) ? 4 : (divisor > 100) ? 3 : (divisor > 10) ? 2 : 1;
}

void FloatMenuItem::setFloatValue(float newVal, bool silent) {
	if(isSame(newVal, currValue)) return;
	
	this->currValue = newVal;
	setSendRemoteNeededAll();
	setChanged(true);
	if(!silent) triggerCallback();
}

void ValueMenuItem::setCurrentValue(uint16_t val, bool silent) {
	if (val == currentValue || val > getMaximumValue()) {
		return;
	}
	
	setChanged(true);
	setSendRemoteNeededAll();
	currentValue = val;
    if(!silent)	triggerCallback();
}
