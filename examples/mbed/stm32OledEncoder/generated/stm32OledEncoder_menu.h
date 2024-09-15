/*
    The code in this file uses open source libraries provided by thecoderscorner

    DO NOT EDIT THIS FILE, IT WILL BE GENERATED EVERY TIME YOU USE THE UI DESIGNER
    INSTEAD EITHER PUT CODE IN YOUR SKETCH OR CREATE ANOTHER SOURCE FILE.

    All the variables you may need access to are marked extern in this file for easy
    use elsewhere.
 */

#ifndef MENU_GENERATED_CODE_H
#define MENU_GENERATED_CODE_H

#include <PlatformDetermination.h>
#include <tcMenu.h>

#include <tcUnicodeHelper.h>
#include "Adafruit_SSD1306.h"
#include "tcMenuAdaFruitGfxMono.h"
#include "MBedEthernetTransport.h"
#include <RemoteConnector.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <ScrollChoiceMenuItem.h>
#include <EditableLargeNumberMenuItem.h>
#include <IoAbstraction.h>
#include <EepromItemStorage.h>
#include <mbed/HalStm32EepromAbstraction.h>
#include <RemoteAuthentication.h>

// variables we declare that you may need to access
extern const PROGMEM ConnectorLocalInfo applicationInfo;
extern TcMenuRemoteServer remoteServer;
extern SPI spi;
extern AdafruitSSD1306Spi* gfx;
extern AdafruitDrawable gfxDrawable;
extern GraphicsDeviceRenderer renderer;
extern MbedEthernetInitialiser mbedEthInitialisation;
extern const UnicodeFont OpenSansRegular7pt[];

// Any externals needed by IO expanders, EEPROMs etc


// Global Menu Item exports
extern TextMenuItem menuEdit;
extern AnalogMenuItem menuCommits;
extern EepromAuthenticationInfoMenuItem menuAuthenticator;
extern RemoteMenuItem menuIoTMonitor;
extern IpAddressMenuItem menuIP;
extern BackMenuItem menuBackConnectivity;
extern SubMenuItem menuConnectivity;
extern ActionMenuItem menuSaveAll;
extern FloatMenuItem menuAvgTemp;
extern ListRuntimeMenuItem menuCountingList;
extern ScrollChoiceMenuItem menuChoices;
extern BackMenuItem menuBackOther;
extern SubMenuItem menuOther;
extern Rgb32MenuItem menuRGB;
extern EditableLargeNumberMenuItem menuFrequency;
extern BooleanMenuItem menuPower;
extern EnumMenuItem menuFoods;
extern AnalogMenuItem menuTenths;
extern BackMenuItem menuBackEditing;
extern SubMenuItem menuEditing;
extern TimeFormattedMenuItem menuRTCTime;
extern DateFormattedMenuItem menuRTCDate;

// Provide a wrapper to get hold of the root menu item and export setupMenu
inline MenuItem& rootMenuItem() { return menuRTCDate; }
void setupMenu();

// Callback functions must always include CALLBACK_FUNCTION after the return type
#define CALLBACK_FUNCTION

int fnCountingListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);
void CALLBACK_FUNCTION onFoodChange(int id);
void CALLBACK_FUNCTION onFrequencyChanged(int id);
void CALLBACK_FUNCTION onSaveAll(int id);
void CALLBACK_FUNCTION onTenthsChaned(int id);

#endif // MENU_GENERATED_CODE_H
