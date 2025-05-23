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
#include <RemoteConnector.h>
#include "SerialTransport.h"
#include <ScrollChoiceMenuItem.h>
#include <IoAbstraction.h>
#include <EepromItemStorage.h>
#include "picoAdafruitDashboard_langSelect.h"

// variables we declare that you may need to access
extern const PROGMEM ConnectorLocalInfo applicationInfo;
extern TcMenuRemoteServer remoteServer;
extern Adafruit_ILI9341 gfx;
extern AdafruitDrawable gfxDrawable;
extern GraphicsDeviceRenderer renderer;
extern const UnicodeFont OpenSansRegular18pt[];
extern const UnicodeFont RobotoMedium24[];

// Any externals needed by IO expanders, EEPROMs etc


// Global Menu Item exports
extern BooleanMenuItem menuSettingsCheckBox;
extern BooleanMenuItem menuSettingsYesNo;
extern Rgb32MenuItem menuSettingsRGB;
extern ActionMenuItem menuSettingsAction;
extern BackMenuItem menuBackSettings;
extern SubMenuItem menuSettings;
extern BooleanMenuItem menuPower;
extern EnumMenuItem menuEnum;
extern FloatMenuItem menuFloat;
extern AnalogMenuItem menuAnalog;

// Provide a wrapper to get hold of the root menu item and export setupMenu
inline MenuItem& rootMenuItem() { return menuAnalog; }
void setupMenu();

// Callback functions must always include CALLBACK_FUNCTION after the return type
#define CALLBACK_FUNCTION

void CALLBACK_FUNCTION onSettingsAction(int id);

#endif // MENU_GENERATED_CODE_H
