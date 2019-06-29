/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>
#include "RemoteMenuItem.h"
#include "RemoteConnector.h"
#include "tcUtil.h"
#include "BaseDialog.h"

RemoteMenuItem* RemoteMenuItem::instance = NULL;

const char REMOTE_NAME_PGM[] PROGMEM = "Remote";
const char NO_LINK_STR[] PROGMEM = "No Link";
const char REMOVE_CONN_HDR_PGM[] PROGMEM = "Close Connection";

// called when the close connection dialog completes.
void onRemoteInfoDialogComplete(ButtonType btn, void* data) {
	if (btn == BTNTYPE_OK) {
		TagValueRemoteConnector* connector = reinterpret_cast<TagValueRemoteConnector*>(data);
		connector->close();
	}
}

// called for each state of the list, base row and child rows.
int remoteInfoRenderFn(RuntimeMenuItem* /*item*/, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
	switch (mode) {
	case RENDERFN_NAME: {
		if (row < 0xff) {
			buffer[0] = 'R'; buffer[1] = 0;
			fastltoa(buffer, row, 2, NOT_PADDED, bufferSize);
		}
		else {
			safeProgCpy(buffer, REMOTE_NAME_PGM, bufferSize);
		}
		return true;
	}
	case RENDERFN_VALUE: {
		if (row == 0xff) {
			buffer[0] = 0;
			return true;
		}
		TagValueRemoteConnector* connector = RemoteMenuItem::getInstance()->getConnector(row);
		if (connector == NULL || !connector->isConnected()) {
			safeProgCpy(buffer, NO_LINK_STR, bufferSize);
		}
		else {
			strncpy(buffer, connector->getRemoteName(), bufferSize);
			buffer[bufferSize - 1] = 0; // make sure it's zero terminated
			appendChar(buffer, ':', bufferSize);
			char authStatus = connector->isAuthenticated() ? 'A' : (connector->isConnected() ? 'C' : 'D');
			appendChar(buffer, authStatus, bufferSize);
			appendChar(buffer, ':', bufferSize);
			fastltoa(buffer, connector->getRemoteMajorVer(), 2, NOT_PADDED, bufferSize);
			appendChar(buffer, '.', bufferSize);
			fastltoa(buffer, connector->getRemoteMinorVer(), 2, NOT_PADDED, bufferSize);
			if (strlen(buffer) < (unsigned int)bufferSize) {
				appendChar(buffer, ':', bufferSize);
				appendChar(buffer, connector->getRemotePlatform() + '0', bufferSize);
			}
		}
		return true;
	}
	case RENDERFN_INVOKE: {
		TagValueRemoteConnector* connector = RemoteMenuItem::getInstance()->getConnector(row);
		BaseDialog* dlg = MenuRenderer::getInstance()->getDialog();
		if (connector && dlg && !dlg->isInUse()) {
			dlg->setUserData(connector);
			dlg->setButtons(BTNTYPE_OK, BTNTYPE_CANCEL, 1);
			dlg->show(REMOVE_CONN_HDR_PGM, false, onRemoteInfoDialogComplete);
			dlg->copyIntoBuffer(connector->getRemoteName());
		}
		return true;
	}
	default: return false;
	}
}

void onRemoteItemCommsNotify(CommunicationInfo info) {
	if (info.remoteNo < RemoteMenuItem::getInstance()->getNumberOfParts()) {
		RemoteMenuItem::getInstance()->setSendRemoteNeededAll();
		RemoteMenuItem::getInstance()->setChanged(true);
	}

	// check if the pass thru should be called.
	RemoteMenuItem::getInstance()->doPassThru(info);
}

RemoteMenuItem::RemoteMenuItem(uint16_t id, int maxRemotes, MenuItem* next) 
	: ListRuntimeMenuItem(id, maxRemotes, remoteInfoRenderFn, next) {
	RemoteMenuItem::instance = this;
	connectors = new TagValueRemoteConnector*[maxRemotes];
	memset(connectors, 0, sizeof(TagValueRemoteConnector*) * maxRemotes);
}

void RemoteMenuItem::addConnector(TagValueRemoteConnector* connector) {
	if (connector->getRemoteNo() < getNumberOfParts()) {
		connector->setCommsNotificationCallback(onRemoteItemCommsNotify);
		connectors[connector->getRemoteNo()] = connector;
	}
}

const char AUTH_KEYS_MENU_PGM[] PROGMEM = "Authorised Keys";
const char AUTH_REMOVE_KEY[] PROGMEM = "Remove";
const char AUTH_REMOVE_ALL_KEYS[] PROGMEM = "Remove ALL keys?";
const char AUTH_EMPTY_KEY[] PROGMEM = "EmptyKey";

void onAuthenticateRemoveKeysDlgComplete(ButtonType btn, void* data) {
	if (btn == BTNTYPE_OK) {
		EepromAuthenticatorManager* mgr = reinterpret_cast<EepromAuthenticatorManager*>(data);
		mgr->resetAllKeys();
	}
}

int authenticationMenuItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
	EepromAuthenicationInfoMenuItem* authItem = reinterpret_cast<EepromAuthenicationInfoMenuItem*>(item);
	switch (mode) {
	case RENDERFN_NAME:
		if (row == 0xff) {
			safeProgCpy(buffer, AUTH_KEYS_MENU_PGM, bufferSize);
		}
		else if(row < authItem->getAuthManager()->getNumberOfEntries()) {
			authItem->getAuthManager()->copyKeyNameToBuffer(row, buffer, bufferSize);
			serdebugF4("Aname: ", row, buffer, bufferSize);
			if (buffer[0] == 0) safeProgCpy(buffer, AUTH_EMPTY_KEY, bufferSize);
		}
		else {
			buffer[0] = 0;
		}
		return true;
	case RENDERFN_VALUE:
		if (row == 0xff) {
			buffer[0] = 0;
		}
		else {
			safeProgCpy(buffer, AUTH_REMOVE_KEY, bufferSize);
		}
		return true;
	case RENDERFN_INVOKE: {
		BaseDialog* dlg = MenuRenderer::getInstance()->getDialog();
		if (row < authItem->getAuthManager()->getNumberOfEntries() && dlg && !dlg->isInUse()) {
			dlg->setUserData(authItem->getAuthManager());
			dlg->setButtons(BTNTYPE_OK, BTNTYPE_CANCEL, 1);
			dlg->show(AUTH_REMOVE_ALL_KEYS, false, onAuthenticateRemoveKeysDlgComplete);
			dlg->copyIntoBuffer("");
		}
		return true;
	}
	default: return false;
	}
}

EepromAuthenicationInfoMenuItem::EepromAuthenicationInfoMenuItem(uint16_t id, EepromAuthenticatorManager * authManager, MenuItem * next)
	: ListRuntimeMenuItem(id, authManager->getNumberOfEntries(), authenticationMenuItemRenderFn, next) {
	this->authManager = authManager;
}
