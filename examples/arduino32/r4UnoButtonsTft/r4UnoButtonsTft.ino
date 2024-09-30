/**
 * An example sketch for the Arduino Uno R4 Wifi board that includes support for a TFT screen and keypad input. It
 * also has remote connectivity in the form of the inbuilt Wi-Fi module.
 *
 * For the example I connected the TFT as follows:
 *   SCLK - 13, COPI - 12, CS - 10, RST - 12, RS/DC - 9
 * and for the keypad
 *   UP - 5, DOWN - 4, LEFT - 6, RIGHT - 7
 */
#include "r4UnoButtonsTft_menu.h"
#include "xbmpImages.h"
#include "IoAbstraction.h"
#include <graphics/TcThemeBuilder.h>
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <tcMenuVersion.h>
#include <Arduino_LED_Matrix.h>
#include <WiFi.h>

bool wifiFailed = false;

TitleWidget wifiWidget(iconsWifi, 5, 16, 12);
TitleWidget connectedWidget(iconsConnection, 2, 16, 12);

// This is the Arduino library that lets us control the LED matrix.
ArduinoLEDMatrix matrix;

void setup() {
    Serial.begin(115200);

    internalAnalogDevice().initPin(DAC, DIR_OUT);
    internalAnalogDevice().initPin(A1, DIR_IN);
    internalAnalogDevice().setCurrentValue(DAC, 0);
    matrix.begin();

    setupMenu();

    menuShowXbmpXbmp.setNumberOfRows(2);

    taskManager.schedule(repeatMillis(1500), [] {
        menuAnalogA1Value.setFloatValue(internalAnalogDevice().getCurrentFloat(A1) * analogReference());
        if(!wifiFailed) {
            wifiWidget.setCurrentState(fromWiFiRSSITo4StateIndicator(WiFi.RSSI()));
        }
    });

    if (WiFi.status() == WL_NO_MODULE) {
        serlogF(SER_ERROR, "WiFi module failure");
        wifiFailed = true;
        wifiWidget.setCurrentState(0);
    }

    if(!wifiFailed) {
        String fv = WiFi.firmwareVersion();
        if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
            serlogF(SER_WARNING, "Please upgrade the firmware");
        }
        WiFi.config(IPAddress(192, 168, 0, 160));
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
}

void loop() {
    taskManager.runLoop();
}

// In this case we show the image descriptions for the Xbitmap menu. We only need override the value callback. For each
// time the scroll choice needs the value for an index, it calls back here.
//
// This callback needs to be implemented by you, see the below docs:
//  1. List Docs - https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
//  2. ScrollChoice Docs - https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
int CALLBACK_FUNCTION fnShowXbmpXbmpRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    switch(mode) {
        case RENDERFN_VALUE:
            strncpy(buffer, imageWithDescription[row].getName(), bufferSize);
            return true;
        default:
            return defaultRtListCallback(item, row, mode, buffer, bufferSize);
    }
}

void CALLBACK_FUNCTION onAnalogDacChange(int id) {
    internalAnalogDevice().setCurrentFloat(DAC, menuAnalogA0DAC.getCurrentValue() / 100.0F);
}

void drawCanvasToLeds(GFXcanvas1& canvas) {
    uint32_t output[3] = {0};
    int position = 0;
    for(int row=0; row<8; row++) {
        for(int col=0; col<12; col++) {
            auto pix = canvas.getPixel(col, row);
            if(pix) {
                output[position / 32] |= (1 << ((31 - (position % 32))));
                serlogF4(SER_DEBUG, "Pix/y pos on ", col, row, position);
            }
            position++;
        }
    }
    serlogHexDump(SER_DEBUG, "Output=", output, 8);
    matrix.loadFrame(output);
}

void CALLBACK_FUNCTION onShowXbmp(int id) {
    GFXcanvas1 canvas(12, 8);
    auto imgIdx = menuShowXbmpXbmp.getCurrentValue();
    if(imgIdx < 0 || imgIdx >= NUMBER_OF_XBMPS) return;

    canvas.drawXBitmap(0, 0, imageWithDescription[imgIdx].getData(), 12, 8, 1);
    drawCanvasToLeds(canvas);
}
