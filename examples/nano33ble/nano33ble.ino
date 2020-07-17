#include "nano33ble_menu.h"
#include "SensorManager.h"
#include "MotionDetection.h"
#include <AnalogDeviceAbstraction.h>

const int analogInputPin = A0;
const int pwmOutputPin = 2;

SensorManager sensorManager;
MotionDetection motionDetection;
ArduinoAnalogDevice analogDevice;

void setup() {
    setupMenu();
    sensorManager.initialise();
    motionDetection.initialise();
    analogDevice.initPin(pwmOutputPin, DIR_OUT);
    analogDevice.initPin(analogInputPin, DIR_IN);

    taskManager.scheduleFixedRate(100, [] {
       menuAnalogReadingsInA0.setFloatValue(analogDevice.getCurrentFloat(analogInputPin));
    });
}

void loop() {
    taskManager.runLoop();

}

void CALLBACK_FUNCTION onPWMChanged(int id) {
    auto newPwm = menuAnalogReadingsOutputPWM.getCurrentValue() / 100.0F;
    analogDevice.setCurrentFloat(pwmOutputPin, newPwm);
}
