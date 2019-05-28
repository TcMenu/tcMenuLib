#include <Wire.h>
#include <IoAbstractionWire.h>
#include <LiquidCrystalIO.h>
#include "tcMenuLiquidCrystal.h"
#include <tcMenu.h>

void onPressMe(int);
#define PRESSMECALLBACK onPressMe
#include <tcm_test/testFixtures.h>

const char applicationName[] PROGMEM = "Graphics Test";


IoAbstractionRef io23017 = ioFrom23017(0x20, ACTIVE_LOW_OPEN, 2);
#define LCD_RS 8
#define LCD_EN 9
#define LCD_D4 10
#define LCD_D5 11
#define LCD_D6 12
#define LCD_D7 13
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
LiquidCrystalRenderer renderer(lcd, 20, 4);

void setup() {
    Wire.begin();
    Serial.begin(115200);
	Serial.print("Testing LiquidCrystal driver");
	Serial.println(applicationName);

    lcd.setIoAbstraction(io23017);
    lcd.begin(20, 4);
  
    switches.initialise(io23017, true);
	menuMgr.initForEncoder(&renderer, &menuVolume, 6, 7, 5);
}

void loop() {
    taskManager.runLoop();
}

const char helloWorld[] PROGMEM = "Pairing mode..";
const char secondMsg[] PROGMEM = "Pairing done..";

void onPressMe(int /*id*/) {
    BaseDialog* dialog = renderer.getDialog();
    if(dialog == NULL) return; // no dialog available.

    dialog->show(helloWorld, [] (ButtonType type, void *data) {
        if(type == BTNTYPE_ACCEPT) {
            BaseDialog* dialog = renderer.getDialog();
            dialog->show(secondMsg);
            dialog->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
            dialog->copyIntoBuffer((char*)data);
        }
    });
    dialog->setButtons(BTNTYPE_NONE, BTNTYPE_CANCEL);
    dialog->copyIntoBuffer("Nothing found");
    dialog->setUserData((void*)"Something else");
    taskManager.scheduleOnce(2000, [] {
        BaseDialog* dialog = renderer.getDialog();
        dialog->copyIntoBuffer("Daves Phone");
        dialog->setButtons(BTNTYPE_ACCEPT, BTNTYPE_CANCEL, 1);
    });
}