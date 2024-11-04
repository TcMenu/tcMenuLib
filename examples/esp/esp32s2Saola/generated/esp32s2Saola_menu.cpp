/*
    The code in this file uses open source libraries provided by thecoderscorner

    DO NOT EDIT THIS FILE, IT WILL BE GENERATED EVERY TIME YOU USE THE UI DESIGNER
    INSTEAD EITHER PUT CODE IN YOUR SKETCH OR CREATE ANOTHER SOURCE FILE.

    All the variables you may need access to are marked extern in this file for easy
    use elsewhere.
 */

// Generated for Arduino ESP32 by TcMenu 4.3.1 on 2024-09-28T18:27:34.834990800Z.

#include <tcMenu.h>
#include "esp32s2Saola_menu.h"
#include "../ThemeMonoInverseBuilder.h"

// Global variable declarations
const PROGMEM  ConnectorLocalInfo applicationInfo = { "ESP32-S2 Saola board", "b447b433-fe4f-4ce7-8746-d94bfeefc707" };
TcMenuRemoteServer remoteServer(applicationInfo);
ArduinoEEPROMAbstraction glArduinoEeprom(&EEPROM);
EepromAuthenticatorManager authManager(4);
U8G2_SH1106_128X64_NONAME_F_HW_I2C gfx(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE);
U8g2Drawable gfxDrawable(&gfx, &Wire);
GraphicsDeviceRenderer renderer(30, applicationInfo.name, &gfxDrawable);
WiFiServer server(3333);
EthernetInitialisation ethernetInitialisation(&server);
EthernetTagValTransport ethernetTransport;
TagValueRemoteServerConnection ethernetConnection(ethernetTransport, ethernetInitialisation);

// Global Menu Item declarations
const PROGMEM char pgmStrConnectivityAuthenticatorText[] = { "Authenticator" };
EepromAuthenticationInfoMenuItem menuConnectivityAuthenticator(pgmStrConnectivityAuthenticatorText, NO_CALLBACK, 20, nullptr);
const PROGMEM char pgmStrConnectivityIoTMonitorText[] = { "IoT Monitor" };
RemoteMenuItem menuConnectivityIoTMonitor(pgmStrConnectivityIoTMonitorText, 19, &menuConnectivityAuthenticator);
const PROGMEM AnyMenuInfo minfoConnectivityIPAddress = { "IP Address", 15, 0xffff, 0, NO_CALLBACK };
IpAddressMenuItem menuConnectivityIPAddress(&minfoConnectivityIPAddress, IpAddressStorage(127, 0, 0, 1), &menuConnectivityIoTMonitor, INFO_LOCATION_PGM);
const char enumStrConnectivityWiFiMode_0[] PROGMEM = "Station";
const char enumStrConnectivityWiFiMode_1[] PROGMEM = "Soft AP";
const char* const enumStrConnectivityWiFiMode[] PROGMEM  = { enumStrConnectivityWiFiMode_0, enumStrConnectivityWiFiMode_1 };
const PROGMEM EnumMenuInfo minfoConnectivityWiFiMode = { "WiFi Mode", 18, 64, 1, NO_CALLBACK, enumStrConnectivityWiFiMode };
EnumMenuItem menuConnectivityWiFiMode(&minfoConnectivityWiFiMode, 0, &menuConnectivityIPAddress, INFO_LOCATION_PGM);
const PROGMEM AnyMenuInfo minfoConnectivityPasscode = { "Passcode", 17, 42, 0, NO_CALLBACK };
TextMenuItem menuConnectivityPasscode(&minfoConnectivityPasscode, "", 22, &menuConnectivityWiFiMode, INFO_LOCATION_PGM);
const PROGMEM AnyMenuInfo minfoConnectivitySSID = { "SSID", 16, 20, 0, NO_CALLBACK };
TextMenuItem menuConnectivitySSID(&minfoConnectivitySSID, "", 22, &menuConnectivityPasscode, INFO_LOCATION_PGM);
const PROGMEM SubMenuInfo minfoConnectivity = { "Connectivity", 14, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackConnectivity(&minfoConnectivity, &menuConnectivitySSID, INFO_LOCATION_PGM);
SubMenuItem menuConnectivity(&minfoConnectivity, &menuBackConnectivity, nullptr, INFO_LOCATION_PGM);
const PROGMEM AnyMenuInfo minfoExtrasMyList = { "My List", 13, 0xffff, 0, onListSelected };
ListRuntimeMenuItem menuExtrasMyList(&minfoExtrasMyList, 0, fnExtrasMyListRtCall, nullptr, INFO_LOCATION_PGM);
const PROGMEM AnyMenuInfo minfoExtrasColor = { "Color", 12, 16, 0, NO_CALLBACK };
Rgb32MenuItem menuExtrasColor(&minfoExtrasColor, RgbColor32(0, 0, 0), false, &menuExtrasMyList, INFO_LOCATION_PGM);
const PROGMEM AnyMenuInfo minfoExtrasText = { "Text", 11, 11, 0, NO_CALLBACK };
TextMenuItem menuExtrasText(&minfoExtrasText, "", 5, &menuExtrasColor, INFO_LOCATION_PGM);
const PROGMEM SubMenuInfo minfoExtras = { "Extras", 10, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackExtras(&minfoExtras, &menuExtrasText, INFO_LOCATION_PGM);
SubMenuItem menuExtras(&minfoExtras, &menuBackExtras, &menuConnectivity, INFO_LOCATION_PGM);
const PROGMEM BooleanMenuInfo minfoSelectMeNewBoolItem = { "New BoolItem", 21, 0xffff, 1, NO_CALLBACK, NAMING_CHECKBOX };
BooleanMenuItem menuSelectMeNewBoolItem(&minfoSelectMeNewBoolItem, true, nullptr, INFO_LOCATION_PGM);
const PROGMEM AnyMenuInfo minfoSelectMePressMe = { "Press Me", 9, 0xffff, 0, pressMeActionRun };
ActionMenuItem menuSelectMePressMe(&minfoSelectMePressMe, &menuSelectMeNewBoolItem, INFO_LOCATION_PGM);
const PROGMEM FloatMenuInfo minfoSelectMeFloat2 = { "Float 2", 8, 0xffff, 3, NO_CALLBACK };
FloatMenuItem menuSelectMeFloat2(&minfoSelectMeFloat2, 0.0, &menuSelectMePressMe, INFO_LOCATION_PGM);
const PROGMEM FloatMenuInfo minfoSelectMeFloat1 = { "Float 1", 7, 0xffff, 3, NO_CALLBACK };
FloatMenuItem menuSelectMeFloat1(&minfoSelectMeFloat1, 0.0, &menuSelectMeFloat2, INFO_LOCATION_PGM);
const PROGMEM SubMenuInfo minfoSelectMe = { "Select Me", 6, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackSelectMe(&minfoSelectMe, &menuSelectMeFloat1, INFO_LOCATION_PGM);
SubMenuItem menuSelectMe(&minfoSelectMe, &menuBackSelectMe, &menuExtras, INFO_LOCATION_PGM);
const PROGMEM BooleanMenuInfo minfoDoorOpen = { "Door Open", 5, 10, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menuDoorOpen(&minfoDoorOpen, false, &menuSelectMe, INFO_LOCATION_PGM);
const char enumStrFoods_0[] PROGMEM = "Pizza";
const char enumStrFoods_1[] PROGMEM = "Pasta";
const char enumStrFoods_2[] PROGMEM = "Salad";
const char enumStrFoods_3[] PROGMEM = "Pie";
const char* const enumStrFoods[] PROGMEM  = { enumStrFoods_0, enumStrFoods_1, enumStrFoods_2, enumStrFoods_3 };
const PROGMEM EnumMenuInfo minfoFoods = { "Foods", 4, 8, 3, NO_CALLBACK, enumStrFoods };
EnumMenuItem menuFoods(&minfoFoods, 0, &menuDoorOpen, INFO_LOCATION_PGM);
const PROGMEM AnalogMenuInfo minfoHalves = { "Halves", 3, 6, 200, NO_CALLBACK, 0, 2, "" };
AnalogMenuItem menuHalves(&minfoHalves, 0, &menuFoods, INFO_LOCATION_PGM);
const PROGMEM AnalogMenuInfo minfoDecEdit = { "Dec Edit", 2, 4, 1000, NO_CALLBACK, 0, 10, "oC" };
AnalogMenuItem menuDecEdit(&minfoDecEdit, 0, &menuHalves, INFO_LOCATION_PGM);
const PROGMEM AnalogMenuInfo minfoIntEdit = { "Int Edit", 1, 2, 100, NO_CALLBACK, 0, 1, "%" };
AnalogMenuItem menuIntEdit(&minfoIntEdit, 0, &menuDecEdit, INFO_LOCATION_PGM);

void setupMenu() {
    // First we set up eeprom and authentication (if needed).
    setSizeBasedEEPROMStorageEnabled(false);
    menuMgr.setEepromRef(&glArduinoEeprom);
    authManager.initialise(menuMgr.getEepromAbstraction(), 200);
    menuMgr.setAuthenticator(&authManager);
    // Now add any readonly, non-remote and visible flags.
    menuConnectivityIoTMonitor.setLocalOnly(true);
    menuConnectivityAuthenticator.setLocalOnly(true);

    // Code generated by plugins and new operators.
    gfx.begin();
    renderer.setUpdatesPerSecond(10);
    switches.init(internalDigitalIo(), SWITCHES_POLL_EVERYTHING, true);
    menuMgr.initForEncoder(&renderer, &menuIntEdit, 5, 6, 7);
    remoteServer.addConnection(&ethernetConnection);
    installMonoInverseTitleTheme(renderer, MenuFontDef(nullptr, 1), MenuFontDef(nullptr, 1), true, BaseGraphicalRenderer::TITLE_ALWAYS, false);

    // We have an IoT monitor, register the server
    menuConnectivityIoTMonitor.setRemoteServer(remoteServer);

    // We have an EEPROM authenticator, it needs initialising
    menuConnectivityAuthenticator.init();
}

