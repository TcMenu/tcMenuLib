
#include <Arduino.h>
#include "RemoteMenuItem.h"

RemoteMenuItem* RemoteMenuItem::FIRST_INSTANCE;

void appendChar(char* str, char val, int len) {
    int i = 0;
    len -= 2;
    while(str[i] && len) {
        --len;
        ++i;
    } 
    str[i++] = val;
    str[i] = (char)0;
}

void fastitoa2(char* str, uint8_t val, int len) {
    len -= 3;
    int i=0;
    while(str[i] && len) {
        --len;
        ++i;
    } 

    if(val > 9) str[i++] = (char)((val / 10) + '0');
    str[i++] = (char)((val % 10) + '0');
    str[i] = (char)0;
}

void remoteItemUpdateLoop() {
    RemoteMenuItem::FIRST_INSTANCE->setChanged(true);
}

RemoteMenuItem::RemoteMenuItem(const AnyMenuInfo *pgmMenuInfo, TagValueRemoteConnector* connector, MenuItem* next) : MenuItem(MENUTYPE_REMOTE_VALUE, pgmMenuInfo, next) {
    this->connector = connector; 
    bool registerTask = (FIRST_INSTANCE == NULL);
    FIRST_INSTANCE = this;
    if(registerTask) taskManager.scheduleFixedRate(2000, remoteItemUpdateLoop);
}

const char NO_LINK_STR[] PROGMEM = "No Link";

void RemoteMenuItem::getCurrentState(char *szBuf, uint8_t len) {
    szBuf[0]=0;

    if(!connector->isConnected()) {
        strcpy_P(szBuf, NO_LINK_STR);
    }
    else {
        strcat(szBuf, connector->getRemoteName());
        appendChar(szBuf, ' ', len);
        fastitoa2(szBuf, connector->getRemoteMajorVer(), len);
        appendChar(szBuf, '.', len);
        fastitoa2(szBuf, connector->getRemoteMinorVer(), len);
        appendChar(szBuf, ' ', len);
        appendChar(szBuf, connector->getRemotePlatform() == PLATFORM_JAVA_API ? 'J' : 'A', len);
    }
}
