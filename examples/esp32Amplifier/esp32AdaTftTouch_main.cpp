#include <Arduino.h>
#include "esp32AdaTftTouch_menu.h"
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <ArduinoEEPROMAbstraction.h>
#include <graphics/MenuTouchScreenEncoder.h>
#include "AmplifierController.h"
#include "app_icondata.h"
#include "TouchCalibrator.h"

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
    SPI.setFrequency(20000000);
    SPI.begin();
    Serial.begin(115200);

    EEPROM.begin(512);
    menuMgr.setEepromRef(new ArduinoEEPROMAbstraction(&EEPROM));
    authManager.initialise(menuMgr.getEepromAbstraction(), 200);

    prepareWifiForUse();

    renderer.setFirstWidget(&wifiWidget);

    setupMenu();

    menuMgr.load(MENU_MAGIC_KEY, [] {
        // when the eeprom is not initialised, put sensible defaults in there.
        menuMgr.getEepromAbstraction()->writeArrayToRom(150, reinterpret_cast<const uint8_t *>(pgmDefaultChannelNames), sizeof pgmDefaultChannelNames);
        menuVolume.setCurrentValue(20);
        menuDirect.setBoolean(true);
    });

    pTouchScreen = new MenuResistiveTouchScreen(&analogDevice, internalDigitalIo(), XPOS_PIN, XNEG_PIN, YPOS_PIN,
                                                YNEG_PIN, &renderer, MenuResistiveTouchScreen::LANDSCAPE);
    pTouchScreen->calibrateMinMaxValues(0.240F, 0.895F, 0.09F, 0.88F);
    pTouchScreen->start();

    renderer.setCustomDrawingHandler(new TouchScreenCalibrator(pTouchScreen));
    renderer.prepareDisplay(false, nullptr, 4, nullptr, 4, true);

    // first we get the graphics factory
    auto & factory = renderer.getGraphicsPropertiesFactory();

    // now we add the icons that we want to use with certain menu items
    const Coord iconSize(APPICONS_WIDTH, APPICONS_HEIGHT);
    factory.addImageToCache(DrawableIcon(menuSettings.getId(), iconSize, DrawableIcon::ICON_XBITMAP, settingsIcon40Bits));
    factory.addImageToCache(DrawableIcon(menuStatus.getId(), iconSize, DrawableIcon::ICON_XBITMAP, statusIcon40Bits));
    factory.addImageToCache(DrawableIcon(menuMute.getId(), iconSize, DrawableIcon::ICON_XBITMAP, muteOffIcon40Bits, muteOnIcon40Bits));

    // and now we define that row 3 of the main menu will have three columns, drawn as icons
    factory.addGridPosition(&menuSettings, GridPosition(GridPosition::DRAW_AS_ICON_ONLY,
                                                        GridPosition::JUSTIFY_CENTER_NO_VALUE, 3, 1, 4, 45));
    factory.addGridPosition(&menuStatus, GridPosition(GridPosition::DRAW_AS_ICON_ONLY,
                                                      GridPosition::JUSTIFY_CENTER_NO_VALUE, 3, 2, 4, 45));
    factory.addGridPosition(&menuMute, GridPosition(GridPosition::DRAW_AS_ICON_ONLY,
                                                    GridPosition::JUSTIFY_CENTER_NO_VALUE, 3, 3, 4, 45));

    // here is how we completely redefine the drawing of a specific item, you can also define for submenu or default
    color_t specialPalette[] { ILI9341_WHITE, ILI9341_RED, ILI9341_BLACK, ILI9341_BLUE};
    factory.setDrawingPropertiesForItem(ItemDisplayProperties::COMPTYPE_TITLE, menuStatus.getId(), specialPalette,
                                        MenuPadding(4), nullptr, 4, 10, 30,
                                        GridPosition::JUSTIFY_CENTER_WITH_VALUE );
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
