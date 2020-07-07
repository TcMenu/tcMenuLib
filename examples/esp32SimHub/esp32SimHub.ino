#include "esp32SimHub_menu.h"
#include <tcMenuU8g2.h>
#include <Wire.h>

U8G2_SH1106_128X64_NONAME_F_SW_I2C gfx(U8G2_R0, 15, 4, 16);

void setup() {
    Wire.begin();
    gfx.begin();
    Serial.begin(115200);
    setupMenu();
}

void loop() {
    taskManager.runLoop();

}
