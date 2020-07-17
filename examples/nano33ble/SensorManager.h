//
// Created by David Cherry on 17/07/2020.
//

#ifndef TCLIBRARYDEV_SENSORMANAGER_H
#define TCLIBRARYDEV_SENSORMANAGER_H

#include <TaskManager.h>
#include <Arduino_HTS221.h>
#include <Arduino_LPS22HB.h>
#include <MenuItems.h>
#include "nano33ble_menu.h"

class SensorManager : Executable{
private:
    bool initialised{};
public:
    void initialise() {
        initialised = HTS.begin()  != 0;
        initialised = initialised && BARO.begin();
        initialised = initialised && taskManager.scheduleFixedRate(1, this, TIME_SECONDS) != TASKMGR_INVALIDID;
    }

    void exec() {
        menuTemp.setFromFloatingPointValue(HTS.readTemperature());
        menuHumidity.setFromFloatingPointValue(HTS.readHumidity());
        menuBPressure.setFromFloatingPointValue(BARO.readPressure());
    }
};

#endif //TCLIBRARYDEV_SENSORMANAGER_H
