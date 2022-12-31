//
// This file contains a simple dashboard that is set up using the dashboard classes
// within the menu library extras. See ref docs:
//
// Reference docs https://www.thecoderscorner.com/ref-docs/tcmenu/html/_drawable_dashboard_8h.html
//

#include "picoAdafruitDashboard_menu.h"
#include <Adafruit_ILI9341.h>
#include "dashboardConfig.h"

// START a title widget that was built using TcMenu Designers widget generator, see the "Code" menu for the widget
// generator. This is added to both the main renderer and the dashboard.

// YesNo icon=0, width=17, height=12, size=36
const uint8_t YesNoWidIcon0[] PROGMEM = {
        0x00,0x40,0x00,0x00,0xe0,0x00,0x00,0xf0,0x01,0x00,0xf8,0x00,0x00,0x7c,0x00,0x04,0x3e,0x00,0x0e,0x1f,
        0x00,0x9f,0x0f,0x00,0xfe,0x07,0x00,0xfc,0x03,0x00,0xf8,0x01,0x00,0xf0,0x00,0x00
};
// YesNo icon=1, width=17, height=12, size=36
const uint8_t YesNoWidIcon1[] PROGMEM = {
        0x3e,0xf0,0x01,0x7c,0xf8,0x00,0xf8,0x7c,0x00,0xf0,0x3f,0x00,0xe0,0x1f,0x00,0xc0,0x0f,0x00,0xe0,0x0f,
        0x00,0xf0,0x1f,0x00,0xf8,0x3e,0x00,0x7c,0x7c,0x00,0x3e,0xf8,0x00,0x1f,0xf0,0x01
};
const uint8_t* const YesNoWidIcons[] PROGMEM = { YesNoWidIcon0, YesNoWidIcon1 };

// Widget Generator yesNo
TitleWidget YesNoWidget(YesNoWidIcons, 2, 17, 12, nullptr);

// END title widget

// TickIcon icon=0, width=17, height=12, size=36
const uint8_t TickIconBitmap0[] PROGMEM = {
        0x00,0x40,0x00,0x00,0xe0,0x00,0x00,0xf0,0x01,0x00,0xf8,0x00,0x00,0x7c,0x00,0x04,0x3e,0x00,0x0e,0x1f,
        0x00,0x9f,0x0f,0x00,0xfe,0x07,0x00,0xfc,0x03,0x00,0xf8,0x01,0x00,0xf0,0x00,0x00
};

// we define a global pointer to the dashboard, it will be created during the dash setup.
DrawableDashboard* mainDashboard;

// Any value item or scroll choice (integer based items) can have ranges of colors, these allow the color to be
// selected based on the value, IE for a rev-counter above 8000 RPM may be shown in red, otherwise green. These
// represented as below. In this case we are creating for an enum value.
// Note that this is a parameter, not the actual dashboard item, they are defined below
DashDrawParametersIntUpdateRange::IntColorRange drawEnumColorRanges[] {
        {ILI9341_YELLOW, ILI9341_RED, 0, 1},
        {ILI9341_CYAN, ILI9341_BLUE, 2, 3}
};
DashDrawParametersIntUpdateRange drawEnumWithIntRange(ILI9341_WHITE, ILI9341_BLACK, ILI9341_BLACK, ILI9341_YELLOW,
                                                       &RobotoMedium24, drawEnumColorRanges, 2);

// As above we create another one for the analog item, it has two ranges.
// Note that this is a parameter, not the actual dashboard item, they are defined below
DashDrawParametersIntUpdateRange::IntColorRange drawAnalogColorRanges[] {
        {ILI9341_LIGHTGREY, ILI9341_BLUE, 0, 50},
        {ILI9341_YELLOW, ILI9341_RED, 51, 100}
};
DashDrawParametersIntUpdateRange drawAnalogValueWithIntRange(ILI9341_WHITE, ILI9341_BLACK, ILI9341_BLACK, ILI9341_WHITE,
                                                       &RobotoMedium24, drawAnalogColorRanges, 2);

// and lastly we create a simple one for the title.
// Note that this is a parameter, not the actual dashboard item, they are defined below
DashDrawParameters titleDrawParameters(ILI9341_WHITE, ILI9341_BLACK, &RobotoMedium24);

//
// Although the dashboard support provides a wide range of menu drawing capabilities, it does not cover every case
// so you can create a delegate that can do the extra drawing and other functions that are needed. Here is a simple
// example that draws a few extra items onto the display
// For drawing onto device drawble see:
//  https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/rendering-with-tcmenu-lcd-tft-oled/#drawing-direct-to-the-display-with-devicedrawabl
// For more on the delegate class and all the points where you can extend
//  https://www.thecoderscorner.com/ref-docs/tcmenu/html/class_drawable_dashboard_delegate.html
//
class MyDrawableDashboardDelegate : public DrawableDashboardDelegate {
public:
    // this is called after the dashboard has opened
    void dashboardDidOpen(BaseMenuRenderer *renderer) override {
        switches.getEncoder()->changePrecision(320, 100);
    }

    // this is called before the dashboard draws any items
    void dashboardWillDraw(unsigned int encVal, RenderPressMode mode) override {
        renderer.getDeviceDrawable()->drawXBitmap(Coord(300, 30), Coord(17, 12), TickIconBitmap0);
    }

    // this is called after the dashboard has drawn all items, you get the current value of the encoder and the
    // current state of the select button.
    void dashboardDidDraw(unsigned int encVal, RenderPressMode mode) override {
        // get the drawing device from the renderer and its dimensions
        DeviceDrawable *pDrawable = renderer.getDeviceDrawable();
        const Coord &totalSize = Coord(pDrawable->getDisplayDimensions().x, 20);

        // create a dashboard wrapper that can work out if we are on a sub device or the main device.
        DrawableWrapper wrapper(pDrawable, &titleDrawParameters, &menuSettings, Coord(0, 200), totalSize);

        // now we draw onto the drawable, we don't need to care if it is buffered or not
        wrapper.getDrawable()->setDrawColor(wrapper.bgCol());
        wrapper.getDrawable()->drawBox(wrapper.offsetLocation(Coord(0, 200)), totalSize, true);
        wrapper.getDrawable()->setDrawColor(wrapper.fgCol());
        wrapper.getDrawable()->drawBox(wrapper.offsetLocation(Coord(0, 200)), Coord(encVal, 20), true);

        // lastly we tell the wrapper that we are done, if needed it will push  the buffer to the screen
        wrapper.endDraw();
    }
} myDrawableDashboardDelegate;

void setupDashboard() {
    // Reference docs https://www.thecoderscorner.com/ref-docs/tcmenu/html/_drawable_dashboard_8h.html

    // create a dashboard instance, giving it the renderer and drawable, any widgets to display in the top corner,
    // and the mode in which it is to operate
    mainDashboard = new DrawableDashboard(renderer.getDeviceDrawable(), &renderer, &YesNoWidget,
                                          DrawableDashboard::DASH_ON_RESET_CLICK_EXIT);

    // here we tell the dashboard about the delegate we created above.
    mainDashboard->setDelegate(&myDrawableDashboardDelegate);

    // now prepare the base colors
    mainDashboard->setBaseColors(RGB(0, 0, 0), RGB(220, 220, 220));

    // here we set up the entries on the dashboard, this is where we provide the menu item and position on the display
    // for each entry. A parameter object that we defined above is then associated with an item. Note that more than
    // one entry can share a parameter.
    mainDashboard->addDrawingItem(&menuAnalog, Coord(0, 0), &drawAnalogValueWithIntRange, 10, nullptr, 10);
    mainDashboard->addDrawingItem(&menuEnum, Coord(0, 50), &drawEnumWithIntRange, 10, nullptr, 10);
    mainDashboard->addDrawingItem(&menuSettings, Coord(0, 100), &titleDrawParameters, 0, "Dashboard");

    // lastly, add the dashboard to the renderer, this is important, the dashboard implements CustomDrawing so it
    // handles taking over the display and reset notification.
    renderer.setCustomDrawingHandler(mainDashboard);
}
