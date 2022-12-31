/**
 * This is a simple example menu wise that has a dashboard view incorporated within it. As of 3.0 the dashboard
 * support has been formalized into the main project. This example presents a very simple dashboard that takes
 * effect when the display is taken over.
 *
 * Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/
 */

#include "picoAdafruitDashboard_menu.h"
#include <TaskManagerIO.h>
#include "dashboardConfig.h"

void setup() {
    // This example logs using IoLogging, see the following guide to enable
    // https://www.thecoderscorner.com/products/arduino-libraries/io-abstraction/arduino-logging-with-io-logging/
    IOLOG_START_SERIAL

    // prepare the SPI device on the right pins, configure as needed!
    SPI.setRX(4);
    SPI.setTX(3);
    SPI.setSCK(2);
    SPI.begin();

    setupMenu();

    // see the dashboardConfig.* files in this directory for the dashboard implementation
    setupDashboard();

    // here we just change a few menu items frequently, on a 150 millis schedule
    taskManager.scheduleFixedRate(150, [] {
        menuFloat.setFloatValue((rand() % 100000) / 100.0F);
        menuAnalog.setCurrentValue((rand() % 1000));
        YesNoWidget.setCurrentState(rand() % 2);
    });

    // here we add an extra switch that triggers the action callback on menuSettingsAction
    switches.addSwitch(22, [](pinid_t, bool) { menuSettingsAction.triggerCallback(); });

    // add a title widget to the screen, see title widget docs
    // the widget is defined in dashboardConfig.h/cpp
    renderer.setFirstWidget(&YesNoWidget);
}

void loop() {
    taskManager.runLoop();
}

// when either the extra button is pressed or the action item is triggered from the menu, take over
// the display which starts the dashboard.
void CALLBACK_FUNCTION onSettingsAction(int id) {
    renderer.takeOverDisplay(); // start the dashboard now.
}
