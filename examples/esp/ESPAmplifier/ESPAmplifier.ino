/**
 * This presents an amplifier control panel onto an ESP32. The example assumes an ESP32 with an ILI9341 screen driven
 * using TFT_eSPI, a resistive touch screen and WiFi in STA mode.
 *
 * YPOS_PIN 32
 * XNEG_PIN 33
 * XPOS_PIN 2
 * YNEG_PIN 0
 *
 * Screen setup for 320x240 LANDSCAPE.
 *
 * To build your own menu: https://designer.thecoderscorner.com/
 * Getting started: https://www.thecoderscorner.com/products/apps/tcmenu-designer/
 * Documentation: https://www.thecoderscorner.com/products/arduino-libraries/
 */

#include "ESPAmplifier_menu.h"
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <ArduinoEEPROMAbstraction.h>
#include <graphics/MenuTouchScreenEncoder.h>
#include <BaseDialog.h>
#include <tcMenuVersion.h>
#include "AmplifierController.h"
#include "app_icondata.h"
#include "TestingDialogController.h"
#include <extras/DrawableTouchCalibrator.h>
#include <graphics/TcThemeBuilder.h>

#define MENU_USING_CALIBRATION_MGR true

using namespace tcremote;
using namespace iotouch;

bool connectedToWifi = false;
TitleWidget wifiWidget(iconsWifi, 5, 16, 12, nullptr);
AmplifierController controller;

// we create a runtime list menu item  further down and it uses these as the possible values to display.
const char* simFilesForList[] = { "SuperFile1.txt",  "CustomFile.cpp", "SuperDuper.h", "File303.cpp", "File123443.h",
                                  "Another.file", "TestMe.txt", "Program.txt" };
#define SIM_FILES_ARRAY_SIZE 8

void prepareWifiForUse() {
    // this sketch assumes you've successfully connected to the Wifi before, does not
    // call begin.. You can initialise the wifi whichever way you wish here.
    if(strlen(getMenuSSID().getTextValue())==0) {
        // no SSID come up as an access point
        WiFi.mode(WIFI_AP);
        WiFi.softAP("tcmenu", "secret1234");
        serdebugF("Started up in AP mode, connect with 'tcmenu' and 'secret'");
    }
    else {
        WiFi.mode(WIFI_STA);
        WiFi.begin(getMenuSSID().getTextValue(), getMenuPasscode().getTextValue());
        serdebugF2("Connecting to Wifi using settings for ", getMenuSSID().getTextValue());
    }

    // now monitor the wifi level every few seconds and report it in a widget.
    taskManager.scheduleFixedRate(3000, [] {
        if(WiFiClass::status() == WL_CONNECTED) {
            if(!connectedToWifi) {
                IPAddress localIp = WiFi.localIP();
                Serial.print("Now connected to WiFi");
                Serial.println(localIp);
                getMenuIPAddress().setIpAddress(localIp[0], localIp[1], localIp[2], localIp[3]);
                connectedToWifi = true;
            }
            wifiWidget.setCurrentState(fromWiFiRSSITo4StateIndicator(WiFi.RSSI()));
        }
        else {
            connectedToWifi = false;
            wifiWidget.setCurrentState(0);
        }
    });
}

// Declaring any arrays used by enum/list items
const char* strAmpStatusEnumEntries[] = { "Warm up", "Warm Valves", "Ready", "DC Protection", "Overloaded", "Overheated" };

void buildMenu(TcMenuBuilder& builder) {
    builder        .analogBuilder(MENU_VOLUME_ID, "Volume", 2, NoMenuFlags, 0, onVolumeChanged)
            .offset(-180).divisor(2).step(1).maxValue(255).unit("dB").endItem()
        .scrollChoiceBuilder(MENU_CHANNELS_ID, "Channel", 4, NoMenuFlags, 0, onChannelChanged).fromRomChoices(150, 3, 16).endItem()
        .boolItem(MENU_DIRECT_ID, "Direct", 6, NAMING_TRUE_FALSE, NoMenuFlags, false, onAudioDirect)
        .boolItem(MENU_MUTE_ID, "Mute", DONT_SAVE, NAMING_TRUE_FALSE, NoMenuFlags, false, onMuteSound)
        .subMenu(MENU_SETTINGS_ID, "Settings", NoMenuFlags, nullptr)
            .subMenu(MENU_CHANNEL_SETTINGS_ID, "Channel Settings", NoMenuFlags, nullptr)
                .scrollChoiceBuilder(MENU_CHANNEL_SETTINGS_CHANNEL_ID, "Channel Num", DONT_SAVE, NoMenuFlags, 0, nullptr).ofCustomRtFunction(fnChannelSettingsChannelRtCall, 3).endItem()
                .analogBuilder(MENU_CHANNEL_SETTINGS_LEVEL_TRIM_ID, "Level Trim", 9, NoMenuFlags, 0, nullptr)
                    .offset(-10).divisor(2).step(1).maxValue(20).unit("dB").endItem()
                .textItem(MENU_CHANNEL_SETTINGS_NAME_ID, "Name", DONT_SAVE, 15, NoMenuFlags, "", nullptr)
                .actionItem(MENU_CHANNEL_SETTINGS_UPDATE_SETTINGS_ID, "Update Settings", NoMenuFlags, onChannelSetttingsUpdate)
                .endSub()
            .analogBuilder(MENU_SETTINGS_WARM_UP_TIME_ID, "Warm up time", 7, NoMenuFlags, 0, warmUpChanged)
                .offset(0).divisor(10).step(1).maxValue(300).unit("s").endItem()
            .analogBuilder(MENU_SETTINGS_VALVE_HEATING_ID, "Valve Heating", 15, NoMenuFlags, 0, valveHeatingChanged)
                .offset(0).divisor(10).step(1).maxValue(600).unit("s").endItem()
            .actionItem(MENU_SAVE_SETTINGS_ID, "Save settings", NoMenuFlags, onSaveSettings)
            .endSub()
        .subMenu(MENU_STATUS_ID, "Status", NoMenuFlags, nullptr)
            .enumItem(MENU_AMP_STATUS_ID, "Amp Status", DONT_SAVE, strAmpStatusEnumEntries, 6, NoMenuFlags, 0, nullptr)
            .analogBuilder(MENU_STATUS_LEFT_V_U_ID, "Left VU", DONT_SAVE, NoMenuFlags, 0, nullptr)
                .offset(-20000).divisor(1000).step(1).maxValue(30000).unit("dB").endItem()
            .analogBuilder(MENU_RIGHT_V_U_ID, "Right VU", DONT_SAVE, NoMenuFlags, 0, nullptr)
                .offset(-20000).divisor(1000).step(1).maxValue(30000).unit("dB").endItem()
            .actionItem(MENU_SHOW_DIALOGS_ID, "Show Dialogs", NoMenuFlags, onShowDialogs)
            .listItemRtCustom(MENU_STATUS_DATA_LIST_ID, "Data List", 0, fnStatusDataListRtCall, NoMenuFlags, nullptr)
            .analogBuilder(MENU_STATUS_TEST_ID, "Test", DONT_SAVE, NoMenuFlags, 0, nullptr)
                .offset(-5000).divisor(10).step(1).maxValue(65535).unit("U").endItem()
            .endSub()
        .subMenu(MENU_CONNECTIVITY_ID, "Connectivity", NoMenuFlags, nullptr)
            .ipAddressItem(MENU_I_P_ADDRESS_ID, "IP address", DONT_SAVE, MenuFlags().readOnly(), IpAddressStorage(127, 0, 0, 1), nullptr)
            .textItem(MENU_SSID_ID, "SSID", 17, 20, NoMenuFlags, "", nullptr)
            .textItem(MENU_PASSCODE_ID, "Passcode", 37, 20, NoMenuFlags, "", nullptr)
            .remoteConnectivityMonitor(MENU_CONNECTIVITY_IO_T_MONITOR_ID, "IoT Monitor", MenuFlags().localOnly())
            .eepromAuthenticationItem(MENU_CONNECTIVITY_AUTHENTICATOR_ID, "Authenticator", MenuFlags().localOnly(), nullptr)
            .endSub();
}


void performThemeAdjustments() {
    /**
     * In the event that we want to modify how menu items are drawn, the easiest way is to use TcThemeBuilder.
     * Here we use it to modify the drawing of certain menu items, we want three of the main menu items to render
     * using icons, to do so we call `menuItemOverride` on the builder, and it returns a "properties builder" that is
     * pre-prepared to adjust the menu item.
     *
     * See https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/themes/rendering-with-themes-icons-grids/
     */
    TcThemeBuilder themeBuilder(renderer);
    const Coord iconSize(APPICONS_WIDTH, APPICONS_HEIGHT);

    // get the properties editor for the settings submenu item itself and override it to draw as an icon. Also make it
    // draw in a specific row and column grid, then finally call `apply` to make the adjustment. Note that in this case
    // use a 4BPP (16 color) icon bitmap by providing both the data and the palette.
    themeBuilder.menuItemOverride(getMenuSettings())
        .withImage4bpp(Coord(31, 40), statusBitmap_palette0, statusBitmap0)
        .onRowCol(3, 1, 3)
        .apply();

    // Again we take a menu item override for the status submenu item, and this time it will render a single color bitmap
    themeBuilder.menuItemOverride(getMenuStatus())
        .withImageXbmp(iconSize, statusIcon40Bits)
        .onRowCol(3, 2, 3)
        .apply();

    // Again we take a menu item override for the mute boolean menu, and this time it will render a single color bitmap
    themeBuilder.menuItemOverride(getMenuMute())
        .withImageXbmp(iconSize, muteOffIcon40Bits, muteOnIcon40Bits)
        .onRowCol(3, 3, 3)
        .apply();

    /**
     * here is how we override drawing for items only when a submenu is active, you can also define at the item level.
     * Note that unlike above, this affects all title items in that sub menu. We could do the same for all action or
     * even regular items within a submenu.
     */
    color_t specialPalette[] { RGB(255, 255, 255), RGB(255, 0, 0), RGB(0, 0, 0), RGB(0, 0, 255) };
    themeBuilder.submenuPropertiesTitleOverride(getMenuStatus())
        .withPalette(specialPalette)
        .withPadding(MenuPadding(4))
        .withBorder(MenuBorder(2))
        .apply();

    // now we add our title widget for the wifi strength
    themeBuilder.addingTitleWidget(wifiWidget);

    // whenever adjusting themes, we should always apply them
    themeBuilder.apply();
}

void setup() {
    // Start up the devices and services that we need, these may change depending on board etc.
    SPI.setFrequency(20000000);
    SPI.begin();
    Serial.begin(115200);
    EEPROM.begin(512);

    // if needed you can enable extra logging
    //serEnableLevel(SER_TCMENU_DEBUG, true);

    setupMenu();

    menuMgr.load(MENU_MAGIC_KEY, [] {
        // when the eeprom is not initialised, put sensible defaults in there.
        menuMgr.getEepromAbstraction()->writeArrayToRom(150, reinterpret_cast<const uint8_t *>(pgmDefaultChannelNames), sizeof pgmDefaultChannelNames);
        // At this point during setup, things are not initialised so use the silent version of the menu set methods.
        // Otherwise, you'll potentially be calling code that isn't initialised.
        getMenuVolume().setCurrentValue(20, true);
        getMenuDirect().setBoolean(true, true);
    });

    prepareWifiForUse();

    // option 1 globally set HB timeouts.
    remoteServer.setHeartbeatIntervalAll(30000);

    // option 2 selectively set HB timeouts.
    //auto tvRemote = remoteServer.getRemoteConnector(0);
    //if(tvRemote != nullptr) tvRemote->setHeartbeatTimeout(30000);

    controller.initialise();
#if MENU_USING_CALIBRATION_MGR == true
    touchCalibrator.initCalibration([](bool isStarting) {
        static TouchOrientationSettings oldTouchSettings(false, false, false);
        if(isStarting) {
            tft.setRotation(0);
            oldTouchSettings = touchScreen.changeOrientation(TouchOrientationSettings(false, true, true));
        } else {
            tft.setRotation(1);
            touchScreen.changeOrientation(oldTouchSettings);
            EEPROM.commit();
        }
    }, true);

    // force a recalibration now if uncommented
    touchCalibrator.reCalibrateNow();
#else
    touchScreen.calibrateMinMaxValues(0.250F, 0.890F, 0.09F, 0.88F);
#endif // TC_TFT_ESPI_NEEDS_TOUCH

    performThemeAdjustments();

    setTitlePressedCallback([](int id){
        showVersionDialog(&applicationInfo);
    });

}

void loop() {
    taskManager.runLoop();

}

// This callback needs to be implemented by you, see the below docs:
//  1. List Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
//  2. ScrollChoice Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
int CALLBACK_FUNCTION fnStatusDataListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    switch(mode) {
    default:
        return defaultRtListCallback(item, row, mode, buffer, bufferSize);
    }
}


void CALLBACK_FUNCTION onMuteSound(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION onChannelSetttingsUpdate(int id) {
    // TODO - your menu change code
}

// This callback needs to be implemented by you, see the below docs:
//  1. List Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
//  2. ScrollChoice Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
int CALLBACK_FUNCTION fnChannelSettingsChannelRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    switch(mode) {
    default:
        return defaultRtListCallback(item, row, mode, buffer, bufferSize);
    }
}


void CALLBACK_FUNCTION onAudioDirect(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION onVolumeChanged(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION onShowDialogs(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION onChannelChanged(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION onSaveSettings(int id) {
    // TODO - your menu change code
}
