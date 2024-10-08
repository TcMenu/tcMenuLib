#ifndef TCLIBRARYDEV_FIXTURES_EXTERN_H
#define TCLIBRARYDEV_FIXTURES_EXTERN_H

#include <tcMenu.h>
#include <MockEepromAbstraction.h>

extern AnalogMenuItem menuNumTwoDp;
extern AnalogMenuItem menuHalvesOffs;
extern IpAddressMenuItem menuIpAddr;
extern AnalogMenuItem menuSubAnalog;
extern BackMenuItem menuBackSub;
extern SubMenuItem menuSub;
extern AnalogMenuItem menuAnalog;
extern AnalogMenuItem menuAnalog2;
extern EnumMenuItem menuEnum1;
extern BooleanMenuItem boolItem1;
extern TextMenuItem textMenuItem1;
extern AnalogMenuItem menuCaseTemp;
extern FloatMenuItem menuFloatItem;
extern ActionMenuItem menuPressMe;
extern BackMenuItem menuBackSecondLevel;
extern SubMenuItem menuSecondLevel;
extern AnalogMenuItem menuRHSTemp;
extern AnalogMenuItem menuLHSTemp;
extern BackMenuItem menuBackStatus;
extern SubMenuItem menuStatus;
extern AnalogMenuItem menuContrast;
extern BooleanMenuItem menu12VStandby;
extern BackMenuItem menuBackSettings;
extern SubMenuItem menuSettings;
extern EnumMenuItem menuChannel;
extern AnalogMenuItem menuVolume;

extern AnalogMenuItem menuSimple1;
extern AnalogMenuItem menuSimple2;

extern int idOfCallback;
extern MockEepromAbstraction eeprom;

extern const char pgmMyName[];
extern NoRenderer noRenderer;

extern const char *uuid1;
extern const char *uuid2;
extern const char *uuid3;

bool checkEditorHints(int start, int end, CurrentEditorRenderingHints::EditorRenderingType ty);

inline void printMenuItem(MenuItem* menuItem) {
    if(menuItem == nullptr) {
        serdebugF("NULL");
    }
    else {
        char buffer[20];
        menuItem->copyNameToBuffer(buffer, sizeof buffer);
        serdebug3(menuItem->getId(), menuItem->getMenuType(),buffer);
    }
}


#endif //TCLIBRARYDEV_FIXTURES_EXTERN_H
