/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "MenuItems.h"

MenuItem::MenuItem(MenuType menuType, const AnyMenuInfo* menuInfo, MenuItem* next) {
	this->flags = 0;
	this->menuType = menuType;
	this->info = menuInfo;
	this->next = next;
    this->setChanged(true); // items always start out needing redrawing.
}

bool MenuItem::isSendRemoteNeeded(uint8_t remoteNo) {
    return bitRead(flags, (remoteNo + (int)MENUITEM_REMOTE_SEND0));
}

void MenuItem::setSendRemoteNeeded(uint8_t remoteNo, bool needed) {
	bitWrite(flags, (remoteNo + (int)MENUITEM_REMOTE_SEND0), (needed && !isLocalOnly()));
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
	MenuCallbackFn fn = get_info_callback(&info->callback);

	if(fn != NULL) {
		fn(getId());
	}
}

uint8_t MenuItem::copyNameToBuffer(char* buf, int offset, int size) {
	const char* name = info->name;
    uint8_t ret = safeProgCpy(buf + offset, name, size - offset);
    return ret + offset;
}

// on avr boards we store all info structures in progmem, so we need this code to
// pull the enum structures out of progmem. Otherwise we just read it out normally

#ifdef __AVR__

void EnumMenuItem::copyEnumStrToBuffer(char* buffer, int size, int idx) {
    char** itemPtr = ((char**)pgm_read_ptr_near(&((EnumMenuInfo*)info)->menuItems) + idx);
    char* itemLoc = (char *)pgm_read_ptr_near(itemPtr);
    strncpy_P(buffer, itemLoc, size);
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
}

int EnumMenuItem::getLengthOfEnumStr(int idx) {
    EnumMenuInfo* enumInfo = (EnumMenuInfo*)info;
    const char * const* choices = enumInfo->menuItems;
    const char * choice = choices[idx];
    return strlen(choice);
}

#endif

void TextMenuItem::setTextValue(const char* text, bool silent) {
	// skip if they are the same
	if(strncmp(menuText, text, textLength()) == 0) return;

	strncpy(menuText, text, textLength());
	menuText[textLength() - 1] = 0;
	setChanged(true);
	setSendRemoteNeededAll();
	if(!silent) triggerCallback();
}

bool isSame(float d1, float d2) {
	float result = abs(d1 - d2);
	return result < 0.0000001;
}

void FloatMenuItem::setFloatValue(float newVal, bool silent) {
	if(isSame(newVal, currValue)) return;
	
	this->currValue = newVal;
	setSendRemoteNeededAll();
	setChanged(true);
	if(!silent) triggerCallback();
}

void ValueMenuItem::setCurrentValue(uint16_t val, bool silent) {
	if(val == currentValue || val > getMaximumValue()) return;
	
	setChanged(true);
	setSendRemoteNeededAll();
	currentValue = val;
    if(!silent)	triggerCallback();
}
