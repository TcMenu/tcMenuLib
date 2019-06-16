/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _REMOTE_MENU_ITEMS_H
#define _REMOTE_MENU_ITEMS_H

#include <Arduino.h>
#include "MenuItems.h"
#include <RemoteConnector.h>
#include <RemoteAuthentication.h>

/**
 * @file RemoteMenuItem.h
 * 
 * This file contains the extra types needed for remote menu items, they are not in the main MenuItems.h header because
 * they require all the remote headers be included.
 */

typedef TagValueRemoteConnector* TagValConnectorPtr;

/**
 * A menu item that holds the current connectivity state of all remote connections registered with it, it is a run time list
 * so needs no additional info structure. For each remote connection that you create, you register it with the addConnector(..)
 * call. This registers this as the callback. If you also need to receive updates, register yourself as a communication listener
 * and you'll receive updates as a pass thru. Note that this object is presently a singleton, one instance should manage all
 * connection state.
 */
class RemoteMenuItem : public ListRuntimeMenuItem {
private:
	CommsCallbackFn passThru;
	TagValConnectorPtr* connectors;
	static RemoteMenuItem* instance;
public:
	/**
	 * Construct a remote menu item providing the ID, maximum remotes supported and the next item
	 */
	RemoteMenuItem(uint16_t id, int maxRemotes, MenuItem* next = NULL);
	
	/**
	 * Add a connector to have it's status monitored by this remote. Note that the remote number must
	 * be within the range managed by this item.
	 * @param connector the connector to be monitored
	 */
	void addConnector(TagValueRemoteConnector* connector);

	/**
	 * Register a pass thru for other items that are also interested in comms updates
	 * @param passThru the callback to be called after this item has processed it
	 */
	void registerCommsNotification(CommsCallbackFn passThru) {
		this->passThru = passThru;
	}

	/**
	 * @return the connector at the specified row number
	 */
	TagValueRemoteConnector* getConnector(int i) {
		return (i < getNumberOfParts()) ? connectors[i] : NULL;
	}

	/**
	 * call the pass thru if it's registered
	 * @param info the comms info
	 */
	void doPassThru(CommunicationInfo info) {
		if (passThru) passThru(info);
	}

	/**
	 * @return the global instance of this object. One list manages all connections.
	 */
	static RemoteMenuItem* getInstance() { return instance; }
};

class EepromAuthenicationInfoMenuItem : public ListRuntimeMenuItem {
private:
	EepromAuthenticatorManager* authManager;
public:
	EepromAuthenicationInfoMenuItem(uint16_t id, EepromAuthenticatorManager* authManager, MenuItem* next);

	EepromAuthenticatorManager* getAuthManager() { return authManager; }
};

#endif
