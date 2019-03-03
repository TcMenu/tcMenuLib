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
}

bool MenuItem::isSendRemoteNeeded(uint8_t remoteNo) {
	remoteNo += 5;
	return (flags & (1 << remoteNo)) != 0;
}

void MenuItem::setSendRemoteNeeded(uint8_t remoteNo, bool needed) {
	remoteNo += 5;
	bitWrite(flags, remoteNo, needed);
}

void MenuItem::setSendRemoteNeededAll() {
	flags = flags | MENUITEM_ALL_REMOTES;
}

void MenuItem::triggerCallback() {
	MenuCallbackFn fn = get_info_callback(&info->callback);

	if(fn != NULL) {
		fn(getId());
	}
}

uint8_t MenuItem::copyNameToBuffer(char* buf, uint8_t offset, uint8_t size) {
	const char* name = info->name;
    uint8_t ret = safeProgCpy(buf + offset, name, size - offset);
    return ret + offset;
}

#ifdef __AVR__

void EnumMenuItem::copyEnumStrToBuffer(char* buffer, int size, int idx) {
    copy_info_ptr_ptr_array(buffer, &((EnumMenuInfo*)info)->menuItems, size, idx);
}

int EnumMenuItem::getLengthOfEnumStr(int idx) {
    return get_info_len_ptr_ptr_array(&((EnumMenuInfo*)info)->menuItems, idx);
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
    strlen(choice);
}

#endif

void TextMenuItem::setTextValue(const char* text) {
	// skip if they are the same
	if(strncmp(menuText, text, textLength()) == 0) return;

	strncpy(menuText, text, textLength());
	menuText[textLength() - 1] = 0;
	setChanged(true);
	setSendRemoteNeededAll();
	triggerCallback();
}

bool isSame(float d1, float d2) {
	float result = abs(d1 - d2);
	return result < 0.0000001;
}

void FloatMenuItem::setFloatValue(float newVal) {
	if(isSame(newVal, currValue)) return;
	
	this->currValue = newVal;
	setSendRemoteNeededAll();
	setChanged(true);
	triggerCallback();
}

void ValueMenuItem::setCurrentValue(uint16_t val) {
	if(val == currentValue) return;
	
	setChanged(true);
	setSendRemoteNeededAll();
	currentValue = val;
	triggerCallback();

}