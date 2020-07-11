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
#include <JoystickSwitchInput.h>
#include "SimhubConnector.h"

void setupMenu();  // forward reference of the menu setup function.
extern const PROGMEM ConnectorLocalInfo applicationInfo;  // defines the app info to the linker.

// Global variables that need exporting

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C gfx;
extern U8g2GfxMenuConfig gfxConfig;
extern U8g2MenuRenderer renderer;
extern ArduinoAnalogDevice analogDevice;

// Callback functions must always include CALLBACK_FUNCTION after the return type
#define CALLBACK_FUNCTION

// Global Menu Item exports

extern AnalogMenuItem menuSettingsTestItem1;
extern SubMenuItem menuSettings;
extern AnalogMenuItem menuGear;
extern BooleanMenuItem menuSimHubLink;
extern AnalogMenuItem menuRPM;
extern AnalogMenuItem menuSpeed;

#endif // MENU_GENERATED_CODE_H
