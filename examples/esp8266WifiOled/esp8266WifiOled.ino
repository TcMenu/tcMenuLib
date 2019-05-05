/**
 * This example shows a very basic home automation system, in this case we simulate a greenhouse
 * growing tomatoes and cucumbers. We can check the temprature of the areas, check if windows
 * are opened, open the window, configure the heater and see if it's on.
 * 
 * It is not a full working automation project, but rather designed to show how easily such a thing
 * could be achieved with TcMenu.
 */

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "esp8266WifiOled_menu.h"
#include <ESP8266WiFi.h>

//
// We create an adafruit 1306 display driver and also the tcMenu display configuration options
//
Adafruit_SSD1306 gfx(128, 32, &Wire, 16);
AdaColorGfxMenuConfig config;

//
// here we prepare the configuration used by the adafruit display renderer with our custom
// settings. We can rely on the defaults, but they don't always suit the display in question.
// As the 1306 (128x32) has a slighly unusual ratio it needs custom configuration.
//
void prepareOledDisplayConfig() {
	makePadding(config.titlePadding, 1, 1, 1, 1);
	makePadding(config.itemPadding, 1, 1, 1, 1);
	makePadding(config.widgetPadding, 2, 2, 0, 2);

	config.bgTitleColor = WHITE;
	config.fgTitleColor = BLACK;
	config.titleFont = NULL;
	config.titleBottomMargin = 1;
	config.widgetColor = BLACK;
	config.titleFontMagnification = 1;

	config.bgItemColor = BLACK;
	config.fgItemColor = WHITE;
	config.bgSelectColor = BLACK;
	config.fgSelectColor = WHITE;
	config.itemFont = NULL;
	config.itemFontMagnification = 1;

    config.editIcon = loResEditingIcon;
    config.activeIcon = loResActiveIcon;
    config.editIconHeight = 6;
    config.editIconWidth = 8;
}

//
// here we just start serial for debugging and try to initialise the display and menu
//
void setup() {
    Serial.begin(115200);

    // start up the display.
    if(!gfx.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {
        Serial.println("Display not allocated - check connections");
        for(;;) yield();
    }
	gfx.clearDisplay();
    gfx.display();

    // initialise the rendering configuration, must be done before calling setupMenu().
    prepareOledDisplayConfig();

    // this sketch assumes you've successfully connected to the Wifi before, does not
    // call begin.. You can initialise the wifi whichever way you wish here.
    Serial.println("Waiting for WiFi connection");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());


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
// Below are the call back methods that are executed when a menu item changes.
//

void CALLBACK_FUNCTION onWindowOpen(int /*id*/) {
    Serial.print("Window is ");
    Serial.println(menuWindowOpen.getBoolean() ? "Open" : "Closed");
}

void CALLBACK_FUNCTION onHeaterPower(int /*id*/) {
    Serial.print("Heater power setting changed to ");
    Serial.println(menuHeaterPower.getCurrentValue());
}

void CALLBACK_FUNCTION onWindowOpening(int /*id*/) {
    Serial.print("Window setting changed");
    Serial.println(menuWinOpening.getCurrentValue());
}

void CALLBACK_FUNCTION onElectricHeater(int /*id*/) {
    Serial.print("Electric heater ");
    Serial.println(menuElectricHeater.getBoolean() ? "ON" : "OFF");
}
