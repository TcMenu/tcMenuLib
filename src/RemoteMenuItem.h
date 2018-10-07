#ifndef _REMOTE_MENU_ITEMS_H
#define _REMOTE_MENU_ITEMS_H

#include <Arduino.h>
#include "MenuItems.h"
#include <RemoteConnector.h>

class RemoteMenuItem : public MenuItem {
private:
    TagValueRemoteConnector *connector;
    static RemoteMenuItem* FIRST_INSTANCE;
public:
    RemoteMenuItem(const AnyMenuInfo *pgmMenuInfo, TagValueRemoteConnector* connector, MenuItem* next);
    void getCurrentState(char* szBuf, uint8_t len);
    friend void remoteItemUpdateLoop();
};

#endif
