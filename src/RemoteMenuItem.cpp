/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>
#include "RemoteMenuItem.h"
#include "tcUtil.h"

RemoteMenuItem* RemoteMenuItem::FIRST_INSTANCE;

void remoteItemUpdateLoop() {
    RemoteMenuItem::FIRST_INSTANCE->setChanged(true);
    RemoteMenuItem::FIRST_INSTANCE->setSendRemoteNeededAll();
}

RemoteMenuItem::RemoteMenuItem(const RemoteMenuInfo *pgmMenuInfo, TagValueRemoteConnector* connector, MenuItem* next) : MenuItem(MENUTYPE_REMOTE_VALUE, (const AnyMenuInfo*)pgmMenuInfo, next) {
    this->connector = connector; 
    bool registerTask = (FIRST_INSTANCE == NULL);
    FIRST_INSTANCE = this;
    if(registerTask) taskManager.scheduleFixedRate(10, remoteItemUpdateLoop, TIME_SECONDS);
}

const char NO_LINK_STR[] PGM_TCM = "No Link";

void RemoteMenuItem::getCurrentState(char *szBuf, uint8_t len) {
    szBuf[0]=0;

    if(!connector->isConnected()) {
        safeProgCpy(szBuf, NO_LINK_STR, len);
    }
    else {
        strcat(szBuf, connector->getRemoteName());
        appendChar(szBuf, ' ', len);
        fastltoa(szBuf, connector->getRemoteMajorVer(), 2, NOT_PADDED, len);
        appendChar(szBuf, '.', len);
        fastltoa(szBuf, connector->getRemoteMinorVer(), 2, NOT_PADDED, len);
        appendChar(szBuf, ' ', len);
        appendChar(szBuf, connector->getRemotePlatform() == PLATFORM_JAVA_API ? 'J' : 'A', len);
    }
}
