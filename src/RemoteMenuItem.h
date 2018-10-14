/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _REMOTE_MENU_ITEMS_H
#define _REMOTE_MENU_ITEMS_H

#include <Arduino.h>
#include "MenuItems.h"
#include <RemoteConnector.h>

/**
 * @file RemoteMenuItem.h
 * 
 * This file contains the extra types needed for remote menu items, they are not in the main MenuItems.h header because
 * they require all the remote headers be included.
 */

/**
 * The information block for a floating point menu component. DO NOT move these imtes without considering AnyMenuInfo!!!
 */
struct RemoteMenuInfo {
	char name[NAME_SIZE_T];
	uint16_t id;
	uint16_t eeprom;
	uint16_t remoteNum;
	MenuCallbackFn callback;
};

/**
 * A menu item that holds the current connectivity state of a remote connection, configured using a remote menu info structure.
 */
class RemoteMenuItem : public MenuItem {
private:
    TagValueRemoteConnector *connector;
    static RemoteMenuItem* FIRST_INSTANCE;
public:
    RemoteMenuItem(const RemoteMenuInfo *pgmMenuInfo, TagValueRemoteConnector* connector, MenuItem* next);
    void getCurrentState(char* szBuf, uint8_t len);
	int getRemoteNum() { return (int) pgm_read_word_near(&((RemoteMenuInfo*)info)->remoteNum);}

	/** allow the associated task to access the item update loop */
    friend void remoteItemUpdateLoop();
};

#endif
