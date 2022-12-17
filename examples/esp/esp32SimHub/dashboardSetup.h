#ifndef TCLIBRARYDEV_DASHBOARDSETUP_H
#define TCLIBRARYDEV_DASHBOARDSETUP_H

#include <Arduino.h>
#include "esp32SimHub_menu.h"
#include <extras/DrawableDashboard.h>

#define LED_STATES 10

class CustomDashboardDelegate : public DrawableDashboardDelegate {
private:
    uint16_t ledColors[LED_STATES];
    bool ledsChanged = true;
public:

    void setLed(int i, uint16_t color);

    void dashboardWillOpen(BaseMenuRenderer *renderer) override;
    void dashboardDidDraw(unsigned int encVal, RenderPressMode pressMode) override;
};

extern DrawableDashboard* dashCustomDrawing; // from the ino file.
extern CustomDashboardDelegate dashboardDelegate; // from the ino file.
void setupDashboard();

#endif //TCLIBRARYDEV_DASHBOARDSETUP_H
