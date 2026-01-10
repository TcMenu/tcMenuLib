#include "mkrEthernetOled_menu.h"

// This variable is the RAM data for scroll choice item Foods
//                  1234567890 1234567890 1234567890 1234567890 1234567890
char ScrollRam[] = "Pizza\0    Pasta    \0Salad\0    Apple\0    Orange\0  ~";


void setup() {
    while (!Serial && millis() < 15000) {
    }
    Serial.begin(115200);
    Wire.begin();
    serlogF(SER_DEBUG, "TcMenu MKR example starting");
    setupMenu();
    serlogF(SER_DEBUG, "TcMenu MKR example started");

}

void loop() {
    taskManager.runLoop();

}
