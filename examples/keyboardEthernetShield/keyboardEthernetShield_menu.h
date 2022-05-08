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
#include <LiquidCrystalIO.h>
#include "EthernetTransport.h"
#include <RemoteConnector.h>
#include <RuntimeMenuItem.h>
#include <ScrollChoiceMenuItem.h>
#include <RemoteMenuItem.h>
#include <EditableLargeNumberMenuItem.h>
#include <IoAbstractionWire.h>
#include <IoAbstraction.h>
#include <EepromAbstraction.h>
#include <RemoteAuthentication.h>
#include "tcMenuLiquidCrystal.h"

// variables we declare that you may need to access
extern const PROGMEM ConnectorLocalInfo applicationInfo;
extern TcMenuRemoteServer remoteServer;
extern LiquidCrystal lcd;
extern LiquidCrystalRenderer renderer;
extern EthernetServer server;
extern EthernetInitialisation ethernetInitialisation;
extern EthernetServer server2;
extern EthernetInitialisation ethernetInitialisation2;

// Any externals needed by IO expanders, EEPROMs etc
extern IoAbstractionRef ioexp_io23017;

// Global Menu Item exports
extern ActionMenuItem menuRomChoicesSave;
extern TextMenuItem menuRomChoicesValue;
extern ScrollChoiceMenuItem menuRomChoicesItemNum;
extern BackMenuItem menuBackRomChoices;
extern SubMenuItem menuRomChoices;
extern BooleanMenuItem menuAdditionalBoolFlagFlag4;
extern BooleanMenuItem menuAdditionalBoolFlagFlag3;
extern BooleanMenuItem menuAdditionalBoolFlagFlag2;
extern BooleanMenuItem menuAdditionalBoolFlagFlag1;
extern BackMenuItem menuBackAdditionalBoolFlag;
extern SubMenuItem menuAdditionalBoolFlag;
extern ListRuntimeMenuItem menuAdditionalCountList;
extern ScrollChoiceMenuItem menuAdditionalNumChoices;
extern ScrollChoiceMenuItem menuAdditionalRomChoice;
extern Rgb32MenuItem menuAdditionalRGB;
extern BackMenuItem menuBackAdditional;
extern SubMenuItem menuAdditional;
extern EepromAuthenticationInfoMenuItem menuConnectivityAuthenticator;
extern RemoteMenuItem menuConnectivityIoTMonitor;
extern ActionMenuItem menuConnectivitySaveToEEPROM;
extern TextMenuItem menuConnectivityText;
extern IpAddressMenuItem menuConnectivityIpAddress;
extern TextMenuItem menuConnectivityChangePin;
extern BackMenuItem menuBackConnectivity;
extern SubMenuItem menuConnectivity;
extern EnumMenuItem menuFruits;
extern AnalogMenuItem menuFiths;
extern EditableLargeNumberMenuItem menuLargeNum;
extern AnalogMenuItem menuDecimalTens;
extern AnalogMenuItem menuInteger;
extern BooleanMenuItem menuHiddenItem;
extern AnalogMenuItem menuAnalog1;
extern TimeFormattedMenuItem menuTime;

// Provide a wrapper to get hold of the root menu item and export setupMenu
inline MenuItem& rootMenuItem() { return menuTime; }
void setupMenu();

// Callback functions must always include CALLBACK_FUNCTION after the return type
#define CALLBACK_FUNCTION

int fnAdditionalCountListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);
int fnAdditionalNumChoicesRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);
void CALLBACK_FUNCTION onAnalog1(int id);
void CALLBACK_FUNCTION onChangePin(int id);
void CALLBACK_FUNCTION onFiths(int id);
void CALLBACK_FUNCTION onInteger(int id);
void CALLBACK_FUNCTION onItemChange(int id);
void CALLBACK_FUNCTION onSaveToEeprom(int id);
void CALLBACK_FUNCTION onSaveValue(int id);

#endif // MENU_GENERATED_CODE_H
