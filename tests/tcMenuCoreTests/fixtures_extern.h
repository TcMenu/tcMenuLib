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

extern int idOfCallback;
extern MockEepromAbstraction eeprom;

static const char *uuid1 = "07cd8bc6-734d-43da-84e7-6084990becfc";
static const char *uuid2 = "07cd8bc6-734d-43da-84e7-6084990becfd";
static const char *uuid3 = "07cd8bc6-734d-43da-84e7-6084990becfe";

void printMenuItem(MenuItem* menuItem);

#endif //TCLIBRARYDEV_FIXTURES_EXTERN_H
