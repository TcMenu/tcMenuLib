//
// Created by David Cherry on 17/07/2020.
//

#ifndef TCLIBRARYDEV_MOTIONDETECTION_H
#define TCLIBRARYDEV_MOTIONDETECTION_H

#include <TaskManager.h>
#include <Arduino_LSM9DS1.h>
#include "nano33ble_menu.h"

class MotionDetection : public Executable {
public:
    void initialise() {
        IMU.begin();
        auto imuRate = IMU.magneticFieldSampleRate();
        taskManager.scheduleFixedRate(max(int(imuRate), 100), this);
    }

    void exec() {
        if(IMU.magneticFieldAvailable()) {
            float x, y, z;
            IMU.readMagneticField(x, y, z);
            menuAccelerometerMagX.setFloatValue(x);
            menuAccelerometerMagY.setFloatValue(y);
            menuAccelerometerMagZ.setFloatValue(z);
        }

        if(IMU.accelerationAvailable()) {
            float x, y, z;
            IMU.readAcceleration(x, y, z);
            menuAccelerometerAccelX.setFloatValue(x);
            menuAccelerometerAccelY.setFloatValue(y);
            menuAccelerometerAccelZ.setFloatValue(z);
        }
    }
};

#endif //TCLIBRARYDEV_MOTIONDETECTION_H
