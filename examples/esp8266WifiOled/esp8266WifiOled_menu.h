/*
    The code in this file uses open source libraries provided by thecoderscorner

    DO NOT EDIT THIS FILE, IT WILL BE GENERATED EVERY TIME YOU USE THE UI DESIGNER
    INSTEAD EITHER PUT CODE IN YOUR SKETCH OR CREATE ANOTHER SOURCE FILE.

    All the variables you may need access to are marked extern in this file for easy
    use elsewhere.
 */

#ifndef MENU_GENERATED_CODE_H
#define MENU_GENERATED_CODE_H

#include <tcMenu.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RemoteConnector.h>
#include <ESP8266WiFi.h>
#include "EthernetTransport.h"
#include "tcMenuAdaFruitGfx.h"

// all define statements needed
#define TCMENU_USING_PROGMEM true

// all variables that need exporting
extern AdaColorGfxMenuConfig config;
extern Adafruit_SSD1306 gfx;
extern AdaFruitGfxMenuRenderer renderer;
extern const char applicationName[];

// all menu item forward references.
extern EnumMenuItem menuWinOpening;
extern EnumMenuItem menuHeaterPower;
extern BackMenuItem menuBackSetup;
extern SubMenuItem menuSetup;
extern BooleanMenuItem menuElectricHeater;
extern BooleanMenuItem menuWindowOpen;
extern AnalogMenuItem menuCucumberTemp;
extern AnalogMenuItem menuTomatoTemp;

// Callback functions always follow this pattern: void CALLBACK_FUNCTION myCallback();
#define CALLBACK_FUNCTION

void CALLBACK_FUNCTION onWindowOpen(int id);
void CALLBACK_FUNCTION onElectricHeater(int id);
void CALLBACK_FUNCTION onHeaterPower(int id);
void CALLBACK_FUNCTION onWindowOpening(int id);

void setupMenu();

#endif // MENU_GENERATED_CODE_H
