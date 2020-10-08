/**
 * This example shows how to use the simhub connector to produce a realistic steering wheel simulation.
 * In this case we use a SH1106 display and a Jostick encoder to control the menu, but using the tcMenu
 * Designer you could change the display and input to whatever was more suitable for your design.
 *
 * We provide an example of how to make a custom dashboard view similiar to those found on custom
 * racing steering wheels. The view comes into play when the menu has been left inactive for more
 * than around 30 seconds. You can change this by configuring the renderer's reset interval.
 *
 * Quick instructions (see the link below for more detailed instructions).
 *
 * * Enable the custom serial protocol from SimHub settings
 * * In Custom Serial, configure the serial port and baud to match your settings.
 * * In the "after connection established" enter exactly: 'simhubStart\n'
 * * In the "before disconnect" enter exactly: 'simhubEnd\n'
 *
 * We now need to map each menu item ID to a SimHub statistic:
 *
 * * Add a new Update entry and enter: '1=' + [RPM] + '\n'
 * * Add a new Update entry and enter: '2=' + [MPH] + '\n'
 * * Add a new Update entry and enter: '4=' + [Gear] + '\n'
 *
 * See the Simhub guide for full instructions:
 * https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-plugins/simhub-connector/
 */

#include "esp32SimHub_menu.h"
#include <Wire.h>
#include <IoLogging.h>
#include <AnalogDeviceAbstraction.h>
#include "DashCustomDrawing.h"
#include "RobotoMonoBold60pt.h"
#include "Fonts/FreeSans18pt7b.h"
#include "Fonts/FreeSans9pt7b.h"

// here we pulsate an LED using the ESP32's DAC
const int dacPin = 32;
float ledLevel = 0.0;
float ledAdjustment = 0.01;

const int lcdBacklightPin = 5;

// we need access to the analog device of the ESP32
ArduinoAnalogDevice analogDevice;
DashCustomDrawing* dashCustomDrawing;
void setupDashboard();
bool currentLedStatus = false;

void setup() {
    SPI.begin(18, 19, 23);
    SPI.setFrequency(20000000);

    pinMode(lcdBacklightPin, OUTPUT);
    digitalWrite(lcdBacklightPin, LOW);

    // we up the clock speed on the i2c bus to high speed setting 1, 400KHz and start the display
    gfx.begin();

    // we start the serial bus at 115200 so it's ready for our connector to use
    Serial.begin(115200);

    // initialise the menu library, the following line was automatically added to setup by tcMenu.
    setupMenu();

    // Here we register a custom drawing class, it will handle the reset events and also any
    // events to do with taking over the display.
    dashCustomDrawing = new DashCustomDrawing(&gfx);
    renderer.setCustomDrawingHandler(dashCustomDrawing);
    setupDashboard();
    renderer.takeOverDisplay();

    menuGear.setTextValue("N");

    analogDevice.initPin(dacPin, DIR_OUT);
    taskManager.scheduleFixedRate(10, [] {
        ledLevel += ledAdjustment;
        if(ledLevel > 0.98) ledAdjustment = -0.01;
        if(ledLevel < 0.01) ledAdjustment = 0.01;
        analogDevice.setCurrentFloat(dacPin, ledLevel);
    });

    //
    // if you want to test your rendering without simhub connected, uncomment the below code, it will update all the
    // values a few times a second.
    //
/*    taskManager.scheduleFixedRate(3000, [] {
        menuTyreTemp.setCurrentValue(random(50) + 40);
        auto gear = random(10);
        if(gear == 0) menuGear.setTextValue("N");
        else if(gear > 8) menuGear.setTextValue("R");
        else {
            char sz[2];
            sz[0] = gear + '0';
            sz[1] = 0;
            menuGear.setTextValue(sz);
        }
    });

    taskManager.scheduleFixedRate(250, [] {
        for(int i=0;i<10;i++) {
            dashCustomDrawing->setLed(i, i%2==currentLedStatus);
        }
        currentLedStatus = currentLedStatus == 0 ? 1 : 0;
    });

    taskManager.scheduleFixedRate(100, [] {
        int rpm = menuRPM.getCurrentValue();
        int speed = menuSpeed.getCurrentValue();

        speed = min(250, max(5, speed + (rand() % 4) - 1));
        rpm = min(16000, max(1500, rpm + (rand() % 100) - 25));
        menuRPM.setCurrentValue(rpm);
        menuSpeed.setCurrentValue(speed);
    });*/

    taskManager.scheduleFixedRate(250, [] {
        int rpm = menuRPM.getCurrentValue();
        uint16_t color;
        if(rpm < 6500) {
            color = (rpm < 5000) ? ILI9341_BLACK : (rpm < 6000) ? ILI9341_GREEN : ILI9341_RED;
        }
        else {
            color = (currentLedStatus) ? ILI9341_RED : ILI9341_BLUE;
            currentLedStatus = !currentLedStatus;
        }
        for (int i = 0; i < LED_STATES; i++) {
            dashCustomDrawing->setLed(i, color);
        }
    });
}

// All IoAbstraction and TcMenu sketches need the runLoop to be called very frequently, preferably in the loop
// method with nothing else in the loop that can cause delays.
void loop() {
    taskManager.runLoop();
}

//
// WARNING: In BETA support only at the moment.
// You can try the dashboard support in this version, but in the next version of tcMenu Designer we'll support editing
// SimHub dashboards within the application. It is highly probable that the API will change before it comes out of
// BETA.
//

// Drawing parameters are used to tell each item how to paint onto the display, they range from simple DashDrawParameters
// that always draw in the same color, DashDrawParametersUpdate that also highlights the item when updated for a moment,
// DashDrawParametersIntUpdateRange that can change colors when certain ranges are met, and finally a text matching
// DashDrawParametersTextUpdateRange that changes color for certain string matches.
DashDrawParametersIntUpdateRange::IntColorRange const rpmRanges[] = {
              { ILI9341_GREEN, ILI9341_BLACK, 7000, 13000},
              { ILI9341_ORANGE, ILI9341_BLACK, 13000, 14000 },
              { ILI9341_RED, ILI9341_YELLOW , 14000, 20000},
      };
DashDrawParametersIntUpdateRange rpmDrawParams(ILI9341_WHITE, ILI9341_BLACK, ILI9341_WHITE, ILI9341_BLACK, &FreeSans18pt7b, rpmRanges, 3);

DashDrawParametersTextUpdateRange::TextColorOverride const gearRanges[] = {
        { "R", ILI9341_GREEN, ILI9341_BLACK },
        { "N", ILI9341_ORANGE, ILI9341_BLACK }
};
DashDrawParametersTextUpdateRange gearDrawParams(ILI9341_WHITE, ILI9341_BLACK, ILI9341_WHITE, ILI9341_PURPLE, &RobotoMono_SemiBold60pt7b,
                                                 gearRanges, 2, DashDrawParameters::NO_TITLE_VALUE_LEFT);

DashDrawParameters white18ptNoUpdate(ILI9341_WHITE, ILI9341_BLACK, &FreeSans18pt7b);

DashDrawParametersUpdate white18ptUpdateRightParam(ILI9341_WHITE, ILI9341_BLACK, ILI9341_CYAN, ILI9341_PURPLE, &FreeSans18pt7b);

DashDrawParametersUpdate yellow9PtUpdateLeft(ILI9341_YELLOW, ILI9341_BLACK, ILI9341_CYAN, ILI9341_PURPLE,
                                             &FreeSans9pt7b, DashDrawParameters::TITLE_LEFT_VALUE_LEFT);
DashDrawParametersUpdate yellow9PtUpdateRight(ILI9341_YELLOW, ILI9341_BLACK, ILI9341_CYAN, ILI9341_PURPLE,
                                             &FreeSans9pt7b, DashDrawParameters::TITLE_LEFT_VALUE_RIGHT);

//
// We then add each drawing item to the dashboard that we created earlier in the constructor. For each item, we need to provide:
// * pointer to the menu item that will have its value displayed
// * the left, top coordinate as a Coord
// * one of the drawing paramters as declared above
// * the number of characters spacing required for the value
// * optionally, text to override the menu name for the title
//
void setupDashboard() {
    dashCustomDrawing->clearItems();
    dashCustomDrawing->addDrawingItem(&menuGear, Coord(15, 60), &gearDrawParams, 1);
    dashCustomDrawing->addDrawingItem(&menuTyreTemp, Coord(145, 125), &white18ptUpdateRightParam, 5, "TMP");
    dashCustomDrawing->addDrawingItem(&menuRPM, Coord(145, 45), &rpmDrawParams, 5);
    dashCustomDrawing->addDrawingItem(&menuSpeed, Coord(145, 85), &white18ptNoUpdate, 5, "MPH");
    dashCustomDrawing->addDrawingItem(&menuDashboard, Coord(5, 220), &yellow9PtUpdateLeft, 10);
    dashCustomDrawing->addDrawingItem(&menuLap, Coord(200, 220), &yellow9PtUpdateRight, 5);
}

void CALLBACK_FUNCTION onConnectionChange(int id) {
    // we registered a call back on the simhub link menu item
    // this is the menu item that gets set/cleared by simhub.
    // you can take custom action on it here if needed.
    serdebugF2("Simhub connection", menuSimHubLink.getBoolean());
}

void CALLBACK_FUNCTION onDashChanged(int id) {
    // if you had more than one dash, here you could switch between them, in a similar way to setupDashboard.
}

void CALLBACK_FUNCTION onShowDash(int id) {
    renderer.takeOverDisplay();
}
