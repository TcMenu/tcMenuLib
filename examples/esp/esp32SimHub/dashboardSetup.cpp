
#include "dashboardSetup.h"
#include "Fonts/RobotoMonoBold60pt.h"
#include <Fonts/FreeSans18pt7b.h>

DrawableDashboard* dashCustomDrawing;
CustomDashboardDelegate dashboardDelegate;

void CustomDashboardDelegate::dashboardWillOpen(BaseMenuRenderer *renderer) {
    ledsChanged = true;
    for (uint16_t &ledState : ledColors) ledState = 0;
}

void CustomDashboardDelegate::setLed(int i, uint16_t color) {
    if (i < 0 || i > LED_STATES) return;
    ledColors[i] = color;
    ledsChanged = true;
}

void CustomDashboardDelegate::dashboardDidDraw(unsigned int encVal, RenderPressMode pressMode) {
    if(!ledsChanged) return;
    ledsChanged = false;
    int widthOfOneLed = renderer.getDeviceDrawable()->getDisplayDimensions().x / LED_STATES;
    int startX = widthOfOneLed / 2;

    for (uint16_t ledState : ledColors) {
        renderer.getDeviceDrawable()->setDrawColor(ledState);
        renderer.getDeviceDrawable()->drawCircle(Coord(startX, 12), 11, true);
        startX += widthOfOneLed;
    }
}

// Drawing parameters are used to tell each item how to paint onto the display, they range from simple DashDrawParameters
// that always draw in the same color, DashDrawParametersUpdate that also highlights the item when updated for a moment,
// DashDrawParametersIntUpdateRange that can change colors when certain ranges are met, and finally a text matching
// DashDrawParametersTextUpdateRange that changes color for certain string matches.
DashDrawParametersIntUpdateRange::IntColorRange const rpmRanges[] = {
        { ILI9341_GREEN, ILI9341_BLACK, 7000, 13000},
        { ILI9341_ORANGE, ILI9341_BLACK, 13000, 14000 },
        { ILI9341_RED, ILI9341_YELLOW , 14000, 20000},
};
DashDrawParametersIntUpdateRange rpmDrawParams(ILI9341_WHITE, ILI9341_BLACK, ILI9341_WHITE, ILI9341_BLACK, &FreeSans18pt7b, rpmRanges, 3);

DashDrawParametersTextUpdateRange::TextColorOverride const gearRanges[] = {
        { "R", ILI9341_GREEN, ILI9341_BLACK },
        { "N", ILI9341_ORANGE, ILI9341_BLACK }
};
DashDrawParametersTextUpdateRange gearDrawParams(ILI9341_WHITE, ILI9341_BLACK, ILI9341_WHITE, ILI9341_PURPLE, &RobotoMono_SemiBold60pt7b,
                                                 gearRanges, 2, DashDrawParameters::NO_TITLE_VALUE_LEFT);

DashDrawParameters white18ptNoUpdate(ILI9341_WHITE, ILI9341_BLACK, &FreeSans18pt7b);

DashDrawParametersUpdate white18ptUpdateRightParam(ILI9341_WHITE, ILI9341_BLACK, ILI9341_CYAN, ILI9341_PURPLE, &FreeSans18pt7b);

DashDrawParametersUpdate yellow9PtUpdateLeft(ILI9341_YELLOW, ILI9341_BLACK, ILI9341_CYAN, ILI9341_PURPLE,
                                             &FreeSans9pt7b, DashDrawParameters::TITLE_LEFT_VALUE_LEFT);
DashDrawParametersUpdate yellow9PtUpdateRight(ILI9341_YELLOW, ILI9341_BLACK, ILI9341_CYAN, ILI9341_PURPLE,
                                              &FreeSans9pt7b, DashDrawParameters::TITLE_LEFT_VALUE_RIGHT);

//
// We then add each drawing item to the dashboard that we created earlier in the constructor. For each item, we need to provide:
// * pointer to the menu item that will have its value displayed
// * the left, top coordinate as a Coord
// * one of the drawing parameters as declared above
// * the number of characters spacing required for the value
// * optionally, text to override the menu name for the title
//
void setupDashboard() {
    serdebugF("Starting dashboard setup");
    dashCustomDrawing = new DrawableDashboard(renderer.getDeviceDrawable(), &renderer, nullptr, DrawableDashboard::DASH_ON_RESET_CLICK_EXIT);
    dashCustomDrawing->clearItems();
    dashCustomDrawing->addDrawingItem(&menuGear, Coord(15, 50), &gearDrawParams, 1);
    dashCustomDrawing->addDrawingItem(&menuTyreTemp, Coord(125, 125), &white18ptUpdateRightParam, 5, "TMP");
    dashCustomDrawing->addDrawingItem(&menuRPM, Coord(125, 45), &rpmDrawParams, 5);
    dashCustomDrawing->addDrawingItem(&menuSpeed, Coord(125, 85), &white18ptNoUpdate, 5, "MPH");
    dashCustomDrawing->addDrawingItem(&menuDashboard, Coord(5, 215), &yellow9PtUpdateLeft, 10);
    dashCustomDrawing->addDrawingItem(&menuLap, Coord(200, 215), &yellow9PtUpdateRight, 5);
    dashCustomDrawing->setDelegate(&dashboardDelegate);
    serdebugF("Finished dash setup");

    renderer.setCustomDrawingHandler(dashCustomDrawing);
    serdebugF("Registered dash");
}
