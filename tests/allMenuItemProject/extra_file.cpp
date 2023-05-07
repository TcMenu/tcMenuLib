#include <Arduino.h>
#include "generated/allMenuItemProject_menu.h"
#include <IoLogging.h>

void CALLBACK_FUNCTION helloWorld(int id) {
    serdebugF("External hello world")
}
