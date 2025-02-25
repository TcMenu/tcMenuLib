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
#include <tcUnicodeHelper.h>
#include "tcMenuAdaFruitGfx.h"
#include "EthernetTransport.h"
#include <RemoteConnector.h>
#include <ScrollChoiceMenuItem.h>
#include <RuntimeMenuItem.h>
#include <IoAbstraction.h>
#include <EepromItemStorage.h>

// variables we declare that you may need to access
extern const PROGMEM ConnectorLocalInfo applicationInfo;
extern TcMenuRemoteServer remoteServer;
extern Adafruit_ST7735 gfx;
extern AdafruitDrawable gfxDrawable;
extern GraphicsDeviceRenderer renderer;
extern WiFiServer server;
extern EthernetInitialisation ethernetInitialisation;
extern const UnicodeFont OpenSansRegular12pt[];
extern const UnicodeFont OpenSansCyrillicLatin14[];

// Any externals needed by IO expanders, EEPROMs etc


// Global Menu Item exports
extern BooleanMenuItem menuWiFiConnected;
extern IpAddressMenuItem menuWiFiIPAddress;
extern BackMenuItem menuBackWiFi;
extern SubMenuItem menuWiFi;
extern FloatMenuItem menuAnalogA1Value;
extern AnalogMenuItem menuAnalogA0DAC;
extern BackMenuItem menuBackAnalog;
extern SubMenuItem menuAnalog;
extern ActionMenuItem menuShowXbmpShowImage;
extern ScrollChoiceMenuItem menuShowXbmpXbmp;
extern BackMenuItem menuBackShowXbmp;
extern SubMenuItem menuShowXbmp;
extern ActionMenuItem menuScrollTextStartScroll;
extern TextMenuItem menuScrollTextText;
extern BackMenuItem menuBackScrollText;
extern SubMenuItem menuScrollText;
extern ActionMenuItem menuZoomTextStartZoom;
extern AnalogMenuItem menuDiscoSpeed;
extern BackMenuItem menuBackDisco;
extern SubMenuItem menuDisco;

// Provide a wrapper to get hold of the root menu item and export setupMenu
inline MenuItem& rootMenuItem() { return menuDisco; }
void setupMenu();

// Callback functions must always include CALLBACK_FUNCTION after the return type
#define CALLBACK_FUNCTION

int fnShowXbmpXbmpRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);
void CALLBACK_FUNCTION onAnalogDacChange(int id);
void CALLBACK_FUNCTION onShowXbmp(int id);
void CALLBACK_FUNCTION onStartDisco(int id);
void CALLBACK_FUNCTION onStartScroll(int id);

#endif // MENU_GENERATED_CODE_H
