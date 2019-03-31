#ifndef _TESTFIXTURES_H_
#define _TESTFIXTURES_H_

#include <tcMenu.h>

/**
 * @file testFixtures.h 
 * 
 * A full complete menu tree structure suitable for basic unit testing. Contains most types
 */

const AnalogMenuInfo minfoRHSTemp = { "R HS Temp", 8, 0xffff, 255, NO_CALLBACK, 0, 2, "C" };
AnalogMenuItem menuRHSTemp(&minfoRHSTemp, 0, NULL);
const AnalogMenuInfo minfoLHSTemp = { "L HS Temp", 7, 0xffff, 255, NO_CALLBACK, 0, 2, "C" };
AnalogMenuItem menuLHSTemp(&minfoLHSTemp, 0, &menuRHSTemp);
const SubMenuInfo minfoStatus = { "Status", 5, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackStatus(&menuLHSTemp, (const AnyMenuInfo*)&minfoStatus);
SubMenuItem menuStatus(&minfoStatus, &menuBackStatus, NULL);
const AnalogMenuInfo minfoContrast = { "Contrast", 10, 6, 255, NO_CALLBACK, 0, 2, "" };
AnalogMenuItem menuContrast(&minfoContrast, 0, NULL);
const BooleanMenuInfo minfo12VStandby = { "12V Standby", 4, 0xffff, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menu12VStandby(&minfo12VStandby, false, &menuContrast);
const SubMenuInfo minfoSettings = { "Settings", 3, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackSettings(&menu12VStandby, (const AnyMenuInfo*)&minfoSettings);
SubMenuItem menuSettings(&minfoSettings, &menuBackSettings, &menuStatus);
const char enumStrChannel_0[] = "CD Player";
const char enumStrChannel_1[] = "Turntable";
const char enumStrChannel_2[] = "Computer";
const char* const enumStrChannel[] = { enumStrChannel_0, enumStrChannel_1, enumStrChannel_2 };
const EnumMenuInfo minfoChannel = { "Channel", 2, 4, 2, NO_CALLBACK, enumStrChannel };
EnumMenuItem menuChannel(&minfoChannel, 0, &menuSettings);
const AnalogMenuInfo minfoVolume = { "Volume", 1, 2, 255, NO_CALLBACK, -190, 2, "dB" };
AnalogMenuItem menuVolume(&minfoVolume, 0, &menuChannel);

#endif //_TESTFIXTURES_H_
