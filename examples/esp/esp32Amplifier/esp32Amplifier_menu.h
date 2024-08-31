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
#include "tcMenuTfteSpi.h"
#include <graphics/MenuTouchScreenEncoder.h>
#include <extras/DrawableTouchCalibrator.h>
#include "ClientEthernetTransport.h"
#include <RemoteConnector.h>
#include <RemoteMenuItem.h>
#include <RuntimeMenuItem.h>
#include <ScrollChoiceMenuItem.h>
#include <IoAbstraction.h>
#include <EepromItemStorage.h>
#include <ArduinoEEPROMAbstraction.h>
#include <RemoteAuthentication.h>

// variables we declare that you may need to access
extern const PROGMEM ConnectorLocalInfo applicationInfo;
extern TcMenuRemoteServer remoteServer;
extern TFT_eSPI tft;
extern TfteSpiDrawable tftDrawable;
extern GraphicsDeviceRenderer renderer;
extern iotouch::ResistiveTouchInterrogator touchInterrogator;
extern MenuTouchScreenManager touchScreen;
extern tcextras::IoaTouchScreenCalibrator touchCalibrator;
extern ClientEthernetInitialisation clientEthInit;
extern ClientEthernetTagValTransport clientEthTransport;

// Any externals needed by IO expanders, EEPROMs etc


// Global Menu Item exports
extern EepromAuthenticationInfoMenuItem menuConnectivityAuthenticator;
extern RemoteMenuItem menuConnectivityIoTMonitor;
extern TextMenuItem menuConnectivityPasscode;
extern TextMenuItem menuConnectivitySSID;
extern IpAddressMenuItem menuConnectivityIPAddress;
extern BackMenuItem menuBackConnectivity;
extern SubMenuItem menuConnectivity;
extern AnalogMenuItem menuStatusTest;
extern ListRuntimeMenuItem menuStatusDataList;
extern ActionMenuItem menuStatusShowDialogs;
extern AnalogMenuItem menuStatusRightVU;
extern AnalogMenuItem menuStatusLeftVU;
extern EnumMenuItem menuStatusAmpStatus;
extern BackMenuItem menuBackStatus;
extern SubMenuItem menuStatus;
extern ActionMenuItem menuSettingsSaveSettings;
extern AnalogMenuItem menuSettingsValveHeating;
extern AnalogMenuItem menuSettingsWarmUpTime;
extern ActionMenuItem menuChannelSettingsUpdateSettings;
extern TextMenuItem menuChannelSettingsName;
extern AnalogMenuItem menuChannelSettingsLevelTrim;
extern ScrollChoiceMenuItem menuChannelSettingsChannel;
extern BackMenuItem menuBackSettingsChannelSettings;
extern SubMenuItem menuSettingsChannelSettings;
extern BackMenuItem menuBackSettings;
extern SubMenuItem menuSettings;
extern BooleanMenuItem menuMute;
extern BooleanMenuItem menuDirect;
extern ScrollChoiceMenuItem menuChannels;
extern AnalogMenuItem menuVolume;

// Provide a wrapper to get hold of the root menu item and export setupMenu
inline MenuItem& rootMenuItem() { return menuVolume; }
void setupMenu();

// Callback functions must always include CALLBACK_FUNCTION after the return type
#define CALLBACK_FUNCTION

int fnChannelSettingsChannelRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);
int fnStatusDataListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);
void CALLBACK_FUNCTION onAudioDirect(int id);
void CALLBACK_FUNCTION onChannelChanged(int id);
void CALLBACK_FUNCTION onChannelSetttingsUpdate(int id);
void CALLBACK_FUNCTION onMuteSound(int id);
void CALLBACK_FUNCTION onSaveSettings(int id);
void CALLBACK_FUNCTION onShowDialogs(int id);
void CALLBACK_FUNCTION onVolumeChanged(int id);
void CALLBACK_FUNCTION valveHeatingChanged(int id);
void CALLBACK_FUNCTION warmUpChanged(int id);

#endif // MENU_GENERATED_CODE_H
