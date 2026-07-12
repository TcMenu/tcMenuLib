// A few references from the dashboard source exported for the main file.

#ifndef TCEXAMPLE_DASHBOARDCONFIG_H
#define TCEXAMPLE_DASHBOARDCONFIG_H

#include "stm32f429FrameBuffer_menu.h"
#include "extras/DrawableDashboard.h"

void setupDashboard();
extern TitleWidget connectedWidget;
extern DrawableDashboard* mainDashboard;

#endif //TCEXAMPLE_DASHBOARDCONFIG_H
