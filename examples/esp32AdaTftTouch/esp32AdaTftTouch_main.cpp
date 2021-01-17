#include <Arduino.h>
#include "esp32AdaTftTouch_menu.h"
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <ArduinoEEPROMAbstraction.h>
#include <MenuTouchScreenEncoder.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include "AmplifierController.h"

#define YPOS_PIN 32
#define XNEG_PIN 33
#define XPOS_PIN 2
#define YNEG_PIN 0

bool connectedToWifi = false;
EepromAuthenticatorManager authManager;
TitleWidget wifiWidget(iconsWifi, 5, 16, 12, nullptr);
AnalogMenuItem* adjustMenuItems[] = {&menuSettingsLine1Adj, &menuSettingsLine2Adj, &menuSettingsLine3Adj};
AmplifierController controller(adjustMenuItems);
ArduinoAnalogDevice analogDevice;
MenuResistiveTouchScreen* pTouchScreen;

void prepareWifiForUse();

void setup() {
    SPI.begin();
    Serial.begin(115200);

    EEPROM.begin(512);
    menuMgr.setEepromRef(new ArduinoEEPROMAbstraction(&EEPROM));
    authManager.initialise(menuMgr.getEepromAbstraction(), 200);

    prepareWifiForUse();

    renderer.setFirstWidget(&wifiWidget);

    renderer.setGraphicsDevice(&gfx, &FreeSans12pt7b, &FreeSans18pt7b, true, 1);
    menuVolume.setCurrentValue(60);

    setupMenu();

    menuMgr.load(MENU_MAGIC_KEY, [] {
        // when the eeprom is not initialised, put sensible defaults in there.
        menuMgr.getEepromAbstraction()->writeArrayToRom(150, reinterpret_cast<const uint8_t *>(pgmDefaultChannelNames), sizeof pgmDefaultChannelNames);
        menuVolume.setCurrentValue(20);
        menuDirect.setBoolean(true);
    });

    pTouchScreen = new MenuResistiveTouchScreen(&analogDevice, internalDigitalIo(), XPOS_PIN, XNEG_PIN, YPOS_PIN,
                                                YNEG_PIN, &renderer, BaseResistiveTouchScreen::LANDSCAPE);
    pTouchScreen->calibrateMinMaxValues(0.27F, 0.92F, 0.04F, 0.93F);
    pTouchScreen->start();
}

void powerDownCapture() {
    menuMgr.save(MENU_MAGIC_KEY);
    EEPROM.commit();
}

void prepareWifiForUse() {
    // this sketch assumes you've successfully connected to the Wifi before, does not
    // call begin.. You can initialise the wifi whichever way you wish here.
    if(strlen(menuConnectivitySSID.getTextValue())==0) {
        // no SSID come up as an access point
        WiFi.mode(WIFI_AP);
        WiFi.softAP("tcmenu", "secret");
        serdebugF("Started up in AP mode, connect with 'tcmenu' and 'secret'");
    }
    else {
        WiFi.begin(menuConnectivitySSID.getTextValue(), menuConnectivityPasscode.getTextValue());
        WiFi.mode(WIFI_STA);
        serdebugF("Connecting to Wifi using settings from connectivity menu");
    }

    // now monitor the wifi level every few seconds and report it in a widget.
    taskManager.scheduleFixedRate(3000, [] {
        if(WiFiClass::status() == WL_CONNECTED) {
            if(!connectedToWifi) {
                IPAddress localIp = WiFi.localIP();
                Serial.print("Now connected to WiFi");
                Serial.println(localIp);
                menuConnectivityIPAddress.setIpAddress(localIp[0], localIp[1], localIp[2], localIp[3]);
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

void loop() {
    taskManager.runLoop();
}

void CALLBACK_FUNCTION onVolumeChanged(int id) {
    controller.onVolumeChanged();
}

void CALLBACK_FUNCTION onChannelChanged(int id) {
    controller.onChannelChanged();
}

void CALLBACK_FUNCTION onAudioDirect(int id) {
    controller.onAudioDirect(menuDirect.getBoolean());
}

void CALLBACK_FUNCTION onMuteSound(int id) {
    controller.onMute(menuMute.getBoolean());
}
