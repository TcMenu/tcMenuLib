//
// ESP32 S2 example based on Saola board with a dashboard configuration onto an OLED display
// I2C on standard pin, 8 and 9 with an SH1106 display
// encoder on 5, 6 with button on 7
// Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/
//

#include "generated/esp32s2Saola_menu.h"
#include <PlatformDetermination.h>

#include <TaskManagerIO.h>
#include <EEPROM.h>
#include <tcMenuVersion.h>
#include <WiFi.h>
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <tcUtil.h>
#include "u8g2DashConfig.h"

#define MENU_WIFIMODE_STATION 0
bool  connectedToWiFi = false;
void startWiFiAndListener();

// we add two widgets that show both the connection status and wifi signal strength
// these are added to the renderer and rendered upon any change.
// https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/rendering-with-tcmenu-lcd-tft-oled/#titlewidget-for-presenting-state-in-icon-form
TitleWidget wifiWidget(iconsWifi, 5, 16, 12);

void setup() {
    // before proceeding we must start wire and serial, then call setup menu.
    Serial.begin(115200);
    serdebugF("Starting ESP32-S2 example");
    Wire.begin();
    Wire.setClock(1000000);
    EEPROM.begin(512);

    setupMenu();

    // always call load after setupMenu, as the EEPROM you chose in initialised only after this setupMenu()
    menuMgr.load();

    // set the number of rows in the list.
    menuExtrasMyList.setNumberOfRows(42);

    // next start WiFi and register our wifi widget
    startWiFiAndListener();
    renderer.setFirstWidget(&wifiWidget);

    // lastly we capture when the root title is pressed present a standard version dialog.
    setTitlePressedCallback([](int titleCb) {
        showVersionDialog(&applicationInfo);
    });

    // this project also contains a dashboard, see the u8g2DashConfig files.
    setupDashboard();
}

void loop() {
    taskManager.runLoop();
}

void startWiFiAndListener() {
    // You can choose between station and access point mode by setting the connectivity/Wifi Mode option to your
    // own choice
    if(menuConnectivityWiFiMode.getCurrentValue() == MENU_WIFIMODE_STATION) {
        // we are in station mode
        WiFi.begin(menuConnectivitySSID.getTextValue(), menuConnectivityPasscode.getTextValue());
        WiFi.mode(WIFI_STA);
        serdebugF("Connecting to Wifi using settings from connectivity menu");
    }
    else {
        // we are in access point mode
        WiFi.mode(WIFI_AP);
        char ssid[25];
        char pwd[25];
        copyMenuItemValueDefault(&menuConnectivitySSID, ssid, sizeof ssid, "tcmenu");
        copyMenuItemValueDefault(&menuConnectivityPasscode, pwd, sizeof pwd, "secret");
        WiFi.softAP(ssid, pwd);
        serdebugF3("Started up in AP mode, connect with ", ssid, pwd);
    }

    // now monitor the wifi level every second and report it in a widget.
    taskManager.scheduleFixedRate(1000, [] {
        if(WiFi.status() == WL_CONNECTED) {
            if(!connectedToWiFi) {
                IPAddress localIp = WiFi.localIP();
                Serial.print("Now connected to WiFi");
                Serial.println(localIp);
                menuConnectivityIPAddress.setIpAddress(localIp[0], localIp[1], localIp[2], localIp[3]);
                connectedToWiFi = true;
            }
            wifiWidget.setCurrentState(fromWiFiRSSITo4StateIndicator(WiFi.RSSI()));
        }
        else {
            connectedToWiFi = false;
            wifiWidget.setCurrentState(0);
        }
    });
}

void CALLBACK_FUNCTION pressMeActionRun(int id) {
    menuMgr.save();
    EEPROM.commit();
    auto dlg = renderer.getDialog();
    if(!dlg->isInUse()) {
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
        dlg->showRam("Saved", false);
        dlg->copyIntoBuffer("to flash");
    }
}


// This callback needs to be implemented by you, see the below docs:
//  1. List Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
//  2. ScrollChoice Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
int CALLBACK_FUNCTION fnExtrasMyListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    if(mode == RENDERFN_VALUE && row != LIST_PARENT_ITEM_POS) {
        strncpy(buffer, "Val", bufferSize);
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    }
    return defaultRtListCallback(item, row, mode, buffer, bufferSize);
}

void CALLBACK_FUNCTION onListSelected(int id) {
    Serial.print("List item select "); Serial.println(menuExtrasMyList.getActiveIndex());
}
