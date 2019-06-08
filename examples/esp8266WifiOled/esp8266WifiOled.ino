/**
 * ESP8266 / ESP32 WiFi and OLED example. Works out of the box with Heltek Kits 8 & 32.
 * 
 * This example shows a very basic home automation system, in this case we simulate a greenhouse
 * growing tomatoes and cucumbers. We can check the temperature of the areas, check if windows
 * are opened, open the window, configure the heater and see if it's on.
 * 
 * It is not a full working automation project, but rather designed to show how easily such a thing
 * could be achieved with TcMenu.
 * 
 * The circuit uses a PCF8574 with both the input via rotary encoder and the outputs for the heater
 * and window opener driven by it too.
 */

#include <Wire.h>
#include "esp8266WifiOled_menu.h"
#include <IoAbstractionWire.h>
#include <ArduinoEEPROMAbstraction.h>
#include <RemoteAuthentication.h>
#ifdef ESP32 
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif

// contains the graphical widget title components.
#include "wifiAndConnectionIcons.h"


// here we define the heater and window pins on the PCF8574
// 0, 1 are A and B from encoder, 2 is OK button.
#define WINDOW_PIN 3
#define HEATER_PIN 4

// this is the interrupt pin connection from the PCF8574 back to the ESP8266 board.
#define IO_INTERRUPT_PIN 12

// the width and height of the attached OLED display.
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
//
// We create an adafruit 1306 display driver and also the tcMenu display configuration options
//
U8G2_SSD1306_128X64_NONAME_F_SW_I2C gfx(U8G2_R0, 15, 4);
IoAbstractionRef io8574 = ioFrom8574(0x20, IO_INTERRUPT_PIN);

// eeprom wrapper, initialised in setup.
ArduinoEEPROMAbstraction *eeprom = NULL;

//
// state used later on by the heater and window control functions
//
bool heaterOn;
bool windowOpen;

// we add two widgets that show both the connection status and wifi signal strength
// these are added to the renderer and rendered upon any change.
TitleWidget connectedWidget(iconsConnection, 2, 16, 10);
TitleWidget wifiWidget(iconsWifi, 5, 16, 10, &connectedWidget);

// when there's a change in communication status (client connects for example) this gets called.
void onCommsChange(CommunicationInfo info) {
    if(info.remoteNo == 0) {
        connectedWidget.setCurrentState(info.connected ? 1 : 0);
    }
    // this relies on logging in IoAbstraction's ioLogging.h, to turn it on visit the file for instructions.
    serdebugF4("Comms notify (rNo, con, enum)", info.remoteNo, info.connected, info.errorMode);
}

//
// here we just start serial for debugging and try to initialise the display and menu
//
void setup() {

    // set up the inbuilt ESP32 rom to use for load and store.
    EEPROM.begin(512);
    eeprom = new ArduinoEEPROMAbstraction(&EEPROM);

    Serial.begin(115200);

    // add a callback for connection changes, see function above.
    remoteServer.getRemoteConnector(0)->setCommsNotificationCallback(onCommsChange);

    // initialise the authentication to use the eeprom at position 50 onwards.
    authenticator.initialise(eeprom, 50);

    // start up the display.
    gfx.begin();

    // this sketch assumes you've successfully connected to the Wifi before, does not
    // call begin.. You can initialise the wifi whichever way you wish here.
    WiFi.begin("SSID", "PWD");
    WiFi.mode(WIFI_STA);

    // now monitor the wifi level every second and report it in a widget.
    taskManager.scheduleFixedRate(1000, [] {
        if(WiFi.status() == WL_CONNECTED) {
            int qualityIcon = 0;
            long strength = WiFi.RSSI();
            if(strength > -95) qualityIcon = 1;
            else if(strength > -85) qualityIcon = 2;
            else if(strength > -70) qualityIcon = 3;
            else if(strength > -55) qualityIcon = 4;
            wifiWidget.setCurrentState(qualityIcon);
            Serial.print("LocalIP"); Serial.println(WiFi.localIP());
        }
        else wifiWidget.setCurrentState(0);
        

    });

    renderer.setFirstWidget(&wifiWidget);

    // initialise the menu.
    setupMenu();

    //
    // here we simulate the temprature changing.
    // temprature doesn't change that often, even 5 seconds is probably too short.
    // in a normal system you'd probably do something other than call random..
    //
    taskManager.scheduleFixedRate(5000, [] {
        menuTomatoTemp.setCurrentValue(random(255));
        menuCucumberTemp.setCurrentValue(random(255));        
    });

    //
    // Now we configure our simulated heater and window.. We use inverse
    // logic on the 8574 because it's better at pulling down to GND than up.
    //
    ioDevicePinMode(io8574, WINDOW_PIN, OUTPUT);
    ioDevicePinMode(io8574, HEATER_PIN, OUTPUT);
    ioDeviceDigitalWrite(io8574, WINDOW_PIN, HIGH);
    ioDeviceDigitalWriteS(io8574, HEATER_PIN, HIGH);

    menuMgr.load(*eeprom);
}

//
// as with all apps based off IoAbstraction's taskManager we must call loop
// very frequently. On ESP8266 it's absolutely essential that you don't do
// anything here that takes a long time as the board will get reset by it's
// internal watchdog.
//
void loop() {
    taskManager.runLoop();
}


//
// here we have the function that is called initially when the window is open
// and then repeatedly called by rescheduling itself until the window is closed
//
void windowOpenFn() {
    if(menuWindowOpen.getBoolean()) {
        windowOpen = !windowOpen;
        // 0 is narrow opening, 1 is wide. We simulate this by adjusting the speed of the call.
        int windowDelay = menuWinOpening.getCurrentValue() == 0 ? 500 : 250;
        taskManager.scheduleOnce(windowDelay, windowOpenFn);
    }
    else windowOpen = false;

    ioDeviceDigitalWriteS(io8574, WINDOW_PIN, windowOpen);
}

void heaterOnFn() {
    if(menuElectricHeater.getBoolean()) {
        heaterOn = !heaterOn;
        // the power is low medium or high. We simulate by changing the delay
        int heaterDelay = (menuHeaterPower.getCurrentValue() + 1) * 500;
        taskManager.scheduleOnce(heaterDelay, heaterOnFn);
    }
    else heaterOn = false;

    ioDeviceDigitalWriteS(io8574, HEATER_PIN, heaterOn);
}

//
// Below are the call back methods that are executed when a menu item changes.
//

void CALLBACK_FUNCTION onWindowOpen(int /*id*/) {
    Serial.print("Window is ");
    Serial.println(menuWindowOpen.getBoolean() ? "Open" : "Closed");
    if(menuWindowOpen.getBoolean()) {
        windowOpenFn();
    }
}

const char heaterOnHighWarningPgm[] PROGMEM = "Set heater to";

//
// this method is called when the dialog is dismissed.
//
void onDialogFinished(ButtonType btnPressed, void* /*userdata*/) {
    // if user did not accept the change, set back to the first setting.
    if(btnPressed != BTNTYPE_OK) {
        menuHeaterPower.setCurrentValue(1);
    }
}

//
// Called when the heater power is changed, we draw a dialog when there is a 
// change that is greater that 1. The dialog is configured to call the method
// above when dismissed.
//
void CALLBACK_FUNCTION onHeaterPower(int /*id*/) {
    Serial.print("Heater power setting changed to ");
    int pwr = menuHeaterPower.getCurrentValue();
    Serial.println(pwr);
    if(pwr >= 1) {
        BaseDialog* dlg = renderer.getDialog();
        if(dlg) {
            dlg->setButtons(BTNTYPE_OK, BTNTYPE_CANCEL, 1);
            dlg->show(heaterOnHighWarningPgm, true, onDialogFinished); // true = shows on remote sessions.
            char sz[10];
            menuHeaterPower.copyEnumStrToBuffer(sz, sizeof(sz), pwr);
            dlg->copyIntoBuffer(sz);
        }
    }
}

void CALLBACK_FUNCTION onWindowOpening(int /*id*/) {
    Serial.print("Window setting changed");
    Serial.println(menuWinOpening.getCurrentValue());
}

void CALLBACK_FUNCTION onElectricHeater(int /*id*/) {
    Serial.print("Electric heater ");
    Serial.println(menuElectricHeater.getBoolean() ? "ON" : "OFF");
    if(menuElectricHeater.getBoolean()) {
        heaterOnFn();
    }
}

const char allSavedPgm[] PROGMEM = "All saved to EEPROM";
void CALLBACK_FUNCTION onSaveAll(int id) {
    Serial.println("Saving values to EEPROM");
    ArduinoEEPROMAbstraction eepromWrapper(&EEPROM);
    menuMgr.save(*eeprom);
    // on esp you must commit after calling save.
    EEPROM.commit();

    BaseDialog* dlg = renderer.getDialog();
    if(dlg) {
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
        dlg->show(allSavedPgm, false); // false = shows only on device
        dlg->copyIntoBuffer("Committed to FLASH");
    }

}
