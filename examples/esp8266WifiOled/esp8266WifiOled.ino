#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "esp8266WifiOled_menu.h"
#include <ESP8266WiFi.h>

Adafruit_SSD1306 gfx(128, 32, &Wire, 16);

void setup() {
    Serial.begin(115200);

    if(!gfx.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {
        Serial.println("Display not allocated - check connections");
        for(;;) yield();
    }

	gfx.clearDisplay();

    setupMenu();

    taskManager.scheduleFixedRate(()-> {

    })
}

void loop() {
    taskManager.runLoop();
}


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
