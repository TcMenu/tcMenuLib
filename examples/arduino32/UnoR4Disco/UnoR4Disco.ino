/**
* An example sketch for the Arduino Uno R4 Wifi board that includes support for a TFT screen and keypad input. It
 * also has remote connectivity in the form of the inbuilt Wi-Fi module. The sketch controls the LED Matrix on the R4
 * WiFi version somewhat akin to a disco.
 *
 * The "disco" functionality is in DiscoTime.h/cpp to keep this file cleaner.
 *
 * For the example I connected the TFT as follows:
 *   SCLK - 13, COPI - 12, CS - 10, RST - 12, RS/DC - 9
 * and for the keypad
 *   UP - 5, DOWN - 4, LEFT - 6, RIGHT - 7
 *
 * To build your own menu: https://designer.thecoderscorner.com/
 * Getting started: https://www.thecoderscorner.com/products/apps/tcmenu-designer/
 * Documentation: https://www.thecoderscorner.com/products/arduino-libraries/
 */

#include "UnoR4Disco_menu.h"
#include "xbmpImages.h"
#include "IoAbstraction.h"
#include <graphics/TcThemeBuilder.h>
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <WiFi.h>
#include "DiscoTime.h"

bool wifiFailed = false;

TitleWidget wifiWidget(iconsWifi, 5, 16, 12);
TitleWidget connectedWidget(iconsConnection, 2, 16, 12);

DiscoTime discoTime;

// At the moment the Wi-Fi on this board has a latency issue with non-blocking IO.
// We do not recommend using the Wi-Fi yet.
#define R4_START_WIFI false

void tryAndStartWifi() {
#if (R4_START_WIFI == true)
    serlogF(SER_DEBUG, "Attempt WiFi begin");
    int started = WiFi.begin("yourssid", "yourpwd");
    if(started == WL_CONNECTED) {
        serlogF(SER_DEBUG, "WiFi Up");
        menuWiFiConnected.setBoolean(true);
        auto ip = WiFi.localIP();
        menuWiFiIPAddress.setIpAddress(ip[0], ip[1], ip[2], ip[3]);
    } else {
        serlogF2(SER_ERROR, "Retrying in 2 seconds", started);
        taskManager.schedule(onceSeconds(2), tryAndStartWifi);
    }
#endif
}


void buildMenu(TcMenuBuilder& builder) {
    builder        .subMenu(MENU_DISCO_ID, "Disco", NoMenuFlags, nullptr)
            .analogBuilder(MENU_DISCO_SPEED_ID, "Speed", DONT_SAVE, NoMenuFlags, 2, nullptr)
                .offset(0).divisor(1).step(1).maxValue(10).unit("").endItem()
            .actionItem(MENU_ZOOM_TEXT_START_ZOOM_ID, "Start Disco", NoMenuFlags, onStartDisco)
            .endSub()
        .subMenu(MENU_SCROLL_TEXT_ID, "Scroll Text", NoMenuFlags, nullptr)
            .textItem(MENU_SCROLL_TEXT_TEXT_ID, "Text", DONT_SAVE, 16, NoMenuFlags, "TcMenu", nullptr)
            .actionItem(MENU_SCROLL_TEXT_START_SCROLL_ID, "Start Scroll", NoMenuFlags, onStartScroll)
            .endSub()
        .subMenu(MENU_SHOW_XBMP_ID, "Show Xbmp", NoMenuFlags, nullptr)
            .scrollChoiceBuilder(MENU_SHOW_XBMP_XBMP_ID, "Xbmp", DONT_SAVE, NoMenuFlags, 0, nullptr).ofCustomRtFunction(fnShowXbmpXbmpRtCall, 0).endItem()
            .actionItem(MENU_SHOW_XBMP_SHOW_IMAGE_ID, "Show Image", NoMenuFlags, onShowXbmp)
            .endSub()
        .subMenu(MENU_ANALOG_ID, "Analog", NoMenuFlags, nullptr)
            .analogBuilder(MENU_ANALOG_A0_D_A_C_ID, "A0 DAC", DONT_SAVE, NoMenuFlags, 0, onAnalogDacChange)
                .offset(0).divisor(1).step(1).maxValue(100).unit("%").endItem()
            .floatItem(MENU_ANALOG_A1_VALUE_ID, "A1 Value", DONT_SAVE, 1, NoMenuFlags, 0.0, nullptr)
            .endSub()
        .subMenu(MENU_WI_FI_ID, "WiFi", NoMenuFlags, nullptr)
            .ipAddressItem(MENU_WI_FI_I_P_ADDRESS_ID, "IP Address", DONT_SAVE, NoMenuFlags, IpAddressStorage(127, 0, 0, 1), nullptr)
            .boolItem(MENU_WI_FI_CONNECTED_ID, "Connected", DONT_SAVE, NAMING_YES_NO, NoMenuFlags, false, nullptr)
            .endSub();
}



void setup() {
    // start the serial port so that we can log
    Serial.begin(115200);

    // set up the DAC and analog input.
    internalAnalogDevice().initPin(DAC, DIR_OUT);
    internalAnalogDevice().initPin(A1, DIR_IN);
    internalAnalogDevice().setCurrentValue(DAC, 0);

    setupMenu();

    getMenuXbmp().setNumberOfRows(2);

    taskManager.schedule(repeatSeconds(1), [] {
        getMenuA1Value().setFloatValue(internalAnalogDevice().getCurrentFloat(A1) * analogReference());
        if(!wifiFailed) {
            wifiWidget.setCurrentState(fromWiFiRSSITo4StateIndicator(WiFi.RSSI()));
        }
    });

    // here we set up the Wi-Fi and prepare a title widget that represents the Wi-Fi status.
    if (WiFi.status() == WL_NO_MODULE) {
        serlogF(SER_ERROR, "WiFi module failure");
        wifiFailed = true;
        wifiWidget.setCurrentState(0);
        getMenuConnected().setBoolean(false);
    }

    if(!wifiFailed) {
        taskManager.schedule(onceMillis(100), tryAndStartWifi);
    }

    // The easiest way to adjust drawing parameters is using theme builder. Here we are going to add two title widgets
    // to the display, these appear top right. We use the stock provided icons.
    // See https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/themes/rendering-with-themes-icons-grids/
    TcThemeBuilder themeBuilder(renderer);
    themeBuilder.addingTitleWidget(wifiWidget)
        .addingTitleWidget(connectedWidget)
        .apply();

    setTitlePressedCallback([](int) {
        showVersionDialog(&applicationInfo);
    });

    discoTime.init();
}

void loop() {
    taskManager.runLoop();

}

// This callback needs to be implemented by you, see the below docs:
//  1. List Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
//  2. ScrollChoice Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
int CALLBACK_FUNCTION fnShowXbmpXbmpRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    switch(mode) {
    case RENDERFN_VALUE:
        strncpy(buffer, imageWithDescription[row].getName(), bufferSize);
        return true;
    default:
        return defaultRtListCallback(item, row, mode, buffer, bufferSize);
    }
}


void CALLBACK_FUNCTION onStartDisco(int id) {
    // called when the start disco action is clicked. We start the disco!
    discoTime.start(getMenuSpeed().getCurrentValue() * 20);
}


void CALLBACK_FUNCTION onShowXbmp(int id) {
    // called whenever the start xbitmap is called, it shows how to convert
    // an XBitmap onto the LEDs.
    GFXcanvas1 canvas(12, 8);
    auto imgIdx = getMenuXbmp().getCurrentValue();
    if(imgIdx < 0 || imgIdx >= NUMBER_OF_XBMPS) return;

    discoTime.picture(imageWithDescription[imgIdx].getData());
}


void CALLBACK_FUNCTION onAnalogDacChange(int id) {
    // here we take the current value of the DAC menu item and put it onto the dac.
    internalAnalogDevice().setCurrentFloat(DAC, getMenuA0DAC().getCurrentValue() / 100.0F);
}


void CALLBACK_FUNCTION onStartScroll(int id) {
    // here we start scrolling through text a word at a time when the start scroll item is clicked
    char sz[20];
    getMenuScrollText().copyValue(sz, sizeof sz);
    discoTime.text(sz);
}
