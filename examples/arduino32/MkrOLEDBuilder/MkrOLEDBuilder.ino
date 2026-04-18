/*
 * Simple demo project that runs on a MKR1300 board with an Ethernet module
 * and OLED screen. It uses the fluent menu builder and all-in-one plugin
 * support added in 4.5.
 *
 * Setup:
 *
 * OLED: configured on standard I2C GPIOs using an SH1106 driver.
 * Ethernet: We have the regular MKR ethernet shield.
 * Menu, a simple menu structure defined in TcMenu Designer Turbo
 *
 * To build your own menu: https://designer.thecoderscorner.com/
 * Getting started: https://www.thecoderscorner.com/products/apps/tcmenu-designer/
 * Documentation: https://tcmenu.github.io/documentation/
 */


#include "MkrOLEDBuilder_menu.h"
#include <Ethernet.h>
#include <AnalogDeviceAbstraction.h>

// contains the tcMenu library included graphical widget title components.
#include "graphics/TcThemeBuilder.h"
#include "stockIcons/wifiAndConnectionIcons8x7.h"

//
// This variable is the RAM data for scroll choice item Foods
char ScrollRam[] = "1\0        2\0        3\0        4\0        5\0        ~";

// We add a title widget that shows when a user is connected to the device. Connection icons
// are in the standard icon set we included at the top.
TitleWidget connectedWidget(iconsConnection, 2, 8, 8);

// we are going to allow control of the menu over local area network
// so therefore must configure ethernet. We'll use the well known Mac
// from many an Arduino example!
byte mac[] = {
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
  };

void setup() {
    // On SAMD/MKR it's best to wait for the serial port to start before proceeding.
    while (!Serial && millis() < 15000) {}

    // Before we do anything else, we start things up.
    Serial.begin(115200);
    Wire.begin();

    TcThemeBuilder builder(renderer);
    builder.addingTitleWidget(connectedWidget)
           .apply();

    // when the title is clicked, we show the standard version information dialog.
    setTitlePressedCallback([](int id) {
        showVersionDialog(&applicationInfo);
    });

    // This is always needed, it starts up the menu that we've built.
    setupMenu();

    serlogF(SER_DEBUG, "Starting ethernet..");

    // Here we start up the Ethernet library using the standard static IP procedure.
    // We pull the IP address from the IoT menu item and then call Ethernet.begin. In a real
    // system you'd do more checking and exit if there was a failure.
    byte* rawIp = getMenuIPAddress().getIpAddress();
    if (rawIp[0] == 127) {
        IPAddress ipAddr(192,168, 0, 200);
        Ethernet.begin(mac, ipAddr);
        serlogF(SER_DEBUG, "Ethernet defaulted on 192.168.0.200");
    } else {
        IPAddress ipAddr(rawIp[0], rawIp[1], rawIp[2], rawIp[3]);
        Ethernet.begin(mac, ipAddr);
        char sz[20];
        getMenuIPAddress().copyValue(sz, sizeof(sz));
        serlogF2(SER_DEBUG, "Ethernet available on ", sz);
    }

    // here we attach an extra listener to the IoT remote monitor menu item
    // this allows us to update the callback
    getMenuIoTMonitor().registerCommsNotification([](CommunicationInfo comms) {
        connectedWidget.setCurrentState(comms.connected ? 1 : 0);
    });

    // We now copy the current value out of the IP address item and print it to the console.

    // here we use task manager to schedule some tasks to happen ever 500 millis
    // https://tcmenu.github.io/documentation/arduino-libraries/taskmanager-io/task-manager-scheduling-guide/
    taskManager.schedule(repeatMillis(500), [] {
        // Here we adjust some menu items at runtime, every 500 millis.
        //https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/
        getMenuKitchen().setCurrentValue(random(100));
        getMenuLounge().setCurrentValue(random(100));
    });
}

void loop() {
    taskManager.runLoop();

}
