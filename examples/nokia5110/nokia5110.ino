/*
 * Shows how to use adagraphics with a mono buffered panel and an ethernet
 * module.This is an 8 bit example, which by default targets the mega 2560.
 * For ethernet control it uses UIPEthernet library instead of Ethernet2.
 * Be careful using in production boards as UIP is GPL. Fine for small
 * home projects. TcMenu has an Apache license.
 * 
 * For more details see the README.md file in this directory.
 */

#include "nokia5110_menu.h"
#include <Adafruit_PCD8544.h>
#include <EepromAbstraction.h>
#include <UIPEthernet.h>

// We are going to load and save using the inbuilt AVR EEPROM storage
AvrEeprom eeprom;

// we are going to allow control of the menu over local area network
// so therefore must configure ethernet..
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 0, 96);

// when we use the Ada graphics support, the designer only generates an export definition
// for the graphics variable. It is our responsibility to both declare it and initialise
// the display. Here we're using software SPI, it's a small display so there's no issue.
Adafruit_PCD8544 gfx = Adafruit_PCD8544(35, 34, 38, 37, 36);

void setup() {
    // as said earlier, it is our responsibility to provide a display that
    // is fully configured.
    gfx.begin();
	gfx.setRotation(0);
    gfx.setContrast(50);
	gfx.clearDisplay();
    gfx.display();

    // start serial, second line for hardware usb on 32 bit boards.
    while(!Serial);
    Serial.begin(115200);

    // and start up the internet to allow remote control of the menu.
    Ethernet.begin(mac, ip);

    addWidgetToTitleArea();

    // initialise the menu
    setupMenu();

    // we can load the menu back from eeprom, the second parameter is an
    // optional override of the magic key. This key is saved out with the
    // menu, and the values are only loaded when the key matches.
    menuMgr.load(eeprom, 0xd00d);
}

//
// If you had a power down detection circuit on your board you'd call this when
// the power down scenario started. Notice we use the same key as load.
// https://www.thecoderscorner.com/electronics/microcontrollers/psu-control/detecting-power-loss-in-powersupply/ 
//
void onPowerDownDetected() {
    menuMgr.save(eeprom, 0xd00d);
}

void loop() {
    // all sketches using task manager must call this very frequently and
    // never use delay(). See IoAbstraction.
    taskManager.runLoop();
}

//
// These three methods are called back when the menu changes, see in the designer
// where we define these menu items.
//

void CALLBACK_FUNCTION onHallLight(int /*id*/) {
    Serial.print("Hall light is now ");
    Serial.println(menuHall.getCurrentValue());
}

void CALLBACK_FUNCTION onLivingRoomLight(int /*id*/) {
    Serial.print("Living Room light is now ");
    Serial.println(menuLiving.getCurrentValue());
}

void CALLBACK_FUNCTION onKitchenLight(int /*id*/) {
    Serial.print("Kitchen light is now ");
    Serial.println(menuKitchen.getCurrentValue());
}

//
// Here we define a widget that is rendered in the title area, it shows if there is
// a remote connection. First we declare the bitmaps, then the widget, and then
// put the widget in the renderer.
//

const uint8_t iconConnectionNone[] PROGMEM = {
	0b01111111,
	0b01100011,
	0b01010101,
	0b01001001,
	0b01010101,
	0b01100011,
	0b01111111,
};

const uint8_t iconConnected[] PROGMEM = {
	0b01111111,
	0b01000001,
	0b01000001,
	0b01000001,
	0b01000001,
	0b01001001,
	0b01111111,
};

// here is the definition of the actual widget, where we assign the above bitmaps to the widget.
const uint8_t* const iconsConnection[] PROGMEM = { iconConnectionNone, iconConnected };
TitleWidget connectedWidget(iconsConnection, 2, 8, 7);

//
// below we ask the remote connector to inform us of any changes in connection state. This is
// the callback we pass.
//
void onCommsChange(CommunicationInfo info) {
    if(info.remoteNo == 0) {
        connectedWidget.setCurrentState(info.connected ? 1 : 0);
    }
}

void addWidgetToTitleArea() {
    // first register for changes in connection state in the Ethernet connection.
    remoteServer.getRemoteConnector(0)->setCommsNotificationCallback(onCommsChange);

    // and give the renderer our widget.
    renderer.setFirstWidget(&connectedWidget);
}
