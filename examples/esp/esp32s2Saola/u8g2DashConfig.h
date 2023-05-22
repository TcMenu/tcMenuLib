// A few references from the dashboard source exported for the main file.

#ifndef TCEXAMPLE_DASHBOARDCONFIG_H
#define TCEXAMPLE_DASHBOARDCONFIG_H

#include "generated/esp32s2Saola_menu.h"
#include "extras/DrawableDashboard.h"

void setupDashboard();
extern TitleWidget wifiWidget; // reference from the ino file
extern DrawableDashboard* mainDashboard; // reference from the dashboard file

#endif //TCEXAMPLE_DASHBOARDCONFIG_H
