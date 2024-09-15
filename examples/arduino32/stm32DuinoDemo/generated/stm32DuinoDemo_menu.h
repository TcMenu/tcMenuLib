/*
    The code in this file uses open source libraries provided by thecoderscorner

    DO NOT EDIT THIS FILE, IT WILL BE GENERATED EVERY TIME YOU USE THE UI DESIGNER
    INSTEAD EITHER PUT CODE IN YOUR SKETCH OR CREATE ANOTHER SOURCE FILE.

    All the variables you may need access to are marked extern in this file for easy
    use elsewhere.
 */

#ifndef MENU_GENERATED_CODE_H
#define MENU_GENERATED_CODE_H

#include <Arduino.h>
#include <tcMenu.h>
#include "tcMenuU8g2.h"
#include "EthernetTransport.h"
#include <RemoteConnector.h>
#include <RuntimeMenuItem.h>
#include <EditableLargeNumberMenuItem.h>
#include <RemoteMenuItem.h>
#include <ScrollChoiceMenuItem.h>
#include <IoAbstraction.h>
#include <EepromItemStorage.h>
#include <mbed/HalStm32EepromAbstraction.h>
#include <RemoteAuthentication.h>

// variables we declare that you may need to access
extern const PROGMEM ConnectorLocalInfo applicationInfo;
extern TcMenuRemoteServer remoteServer;
extern U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI gfx;
extern U8g2Drawable gfxDrawable;
extern GraphicsDeviceRenderer renderer;
extern EthernetServer server;
extern EthernetInitialisation ethernetInitialisation;

// Any externals needed by IO expanders, EEPROMs etc


// Global Menu Item exports
extern EnumMenuItem menuDecimalStep;
extern AnalogMenuItem menuDecimal;
extern EditableLargeNumberMenuItem menuRuntimesLgeNum1;
extern AnalogMenuItem menuRuntimesHalves1;
extern EepromAuthenticationInfoMenuItem menuRuntimesAuthenticator;
extern RemoteMenuItem menuRuntimesIoTMonitor;
extern ListRuntimeMenuItem menuRuntimesCustomList;
extern TextMenuItem menuRuntimesText;
extern BackMenuItem menuBackRuntimes;
extern SubMenuItem menuRuntimes;
extern ScrollChoiceMenuItem menuMoreItemsScroll;
extern FloatMenuItem menuMoreItemsNumber;
extern ActionMenuItem menuMoreItemsPressMe;
extern BooleanMenuItem menuMoreItemsPower;
extern BooleanMenuItem menuMoreItemsToppings;
extern EnumMenuItem menuMoreItemsOptions;
extern BackMenuItem menuBackMoreItems;
extern SubMenuItem menuMoreItems;
extern ActionMenuItem menuSettingsSaveNow;
extern EnumMenuItem menuSettingsDefault;
extern AnalogMenuItem menuSettingsOfst78;
extern AnalogMenuItem menuSettingsOfst45;
extern AnalogMenuItem menuSettingsOfst33;
extern BackMenuItem menuBackSettings;
extern SubMenuItem menuSettings;
extern AnalogMenuItem menuStatusWoW;
extern AnalogMenuItem menuStatusMotor;
extern AnalogMenuItem menuStatusCurrent;
extern BackMenuItem menuBackStatus;
extern SubMenuItem menuStatus;
extern ActionMenuItem menu78;
extern ActionMenuItem menu45;
extern ActionMenuItem menu33;

// Provide a wrapper to get hold of the root menu item and export setupMenu
inline MenuItem& rootMenuItem() { return menu33; }
void setupMenu();

// Callback functions must always include CALLBACK_FUNCTION after the return type
#define CALLBACK_FUNCTION

void CALLBACK_FUNCTION decimalDidChange(int id);
int fnRuntimesCustomListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);
void CALLBACK_FUNCTION largeNumDidChange(int id);
void CALLBACK_FUNCTION onDecimalStepChange(int id);
void CALLBACK_FUNCTION saveWasPressed(int id);

#endif // MENU_GENERATED_CODE_H
