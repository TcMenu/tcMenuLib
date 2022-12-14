/**
 * This is a simple example menu wise that has a dashboard view incorporated within it.
 */

#include "picoAdafruitDashboard_menu.h"
#include <TaskManagerIO.h>
#include "dashboardConfig.h"

void setup() {
    Serial1.begin(115200);
    SPI.setRX(4);
    SPI.setTX(3);
    SPI.setSCK(2);
    SPI.begin();

    setupMenu();

    setupDashboard();

    taskManager.scheduleFixedRate(150, [] {
        menuFloat.setFloatValue((rand() % 100000) / 100.0F);
        menuAnalog.setCurrentValue((rand() % 1000));
        YesNoWidget.setCurrentState(rand() % 2);
    });

    switches.addSwitch(22, [](pinid_t, bool) { menuSettingsAction.triggerCallback(); });

    renderer.setFirstWidget(&YesNoWidget);
}

void loop() {
    taskManager.runLoop();
}

void CALLBACK_FUNCTION onSettingsAction(int id) {
    renderer.takeOverDisplay(); // start the dashboard now.
}
