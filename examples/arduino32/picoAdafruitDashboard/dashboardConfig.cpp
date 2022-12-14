
#include "picoAdafruitDashboard_menu.h"
#include <Adafruit_ILI9341.h>
#include "dashboardConfig.h"

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


// TickIcon icon=0, width=17, height=12, size=36
const uint8_t TickIconBitmap0[] PROGMEM = {
        0x00,0x40,0x00,0x00,0xe0,0x00,0x00,0xf0,0x01,0x00,0xf8,0x00,0x00,0x7c,0x00,0x04,0x3e,0x00,0x0e,0x1f,
        0x00,0x9f,0x0f,0x00,0xfe,0x07,0x00,0xfc,0x03,0x00,0xf8,0x01,0x00,0xf0,0x00,0x00
};

DrawableDashboard* mainDashboard;

DashDrawParametersIntUpdateRange::IntColorRange drawEnumColorRanges[] {
        {ILI9341_YELLOW, ILI9341_RED, 0, 1},
        {ILI9341_CYAN, ILI9341_BLUE, 2, 3}
};
DashDrawParametersIntUpdateRange drawEnumWithIntRange(ILI9341_WHITE, ILI9341_BLACK, ILI9341_BLACK, ILI9341_YELLOW,
                                                       &RobotoMedium24, drawEnumColorRanges, 2);

DashDrawParametersIntUpdateRange::IntColorRange drawAnalogColorRanges[] {
        {ILI9341_LIGHTGREY, ILI9341_BLUE, 0, 50},
        {ILI9341_YELLOW, ILI9341_RED, 51, 100}
};
DashDrawParametersIntUpdateRange drawAnalogValueWithIntRange(ILI9341_WHITE, ILI9341_BLACK, ILI9341_BLACK, ILI9341_WHITE,
                                                       &RobotoMedium24, drawAnalogColorRanges, 2);
DashDrawParameters titleDrawParameters(ILI9341_WHITE, ILI9341_BLACK, &RobotoMedium24);

class MyDrawableDashboardDelegate : public DrawableDashboardDelegate {
public:
    void dashboardDidOpen(BaseMenuRenderer *renderer) override {
        switches.getEncoder()->changePrecision(320, 100);
    }

    void dashboardWillDraw(unsigned int encVal, RenderPressMode mode) override {
        renderer.getDeviceDrawable()->drawXBitmap(Coord(300, 30), Coord(17, 12), TickIconBitmap0);
    }

    void dashboardDidDraw(unsigned int encVal, RenderPressMode mode) override {
        DeviceDrawable *pDrawable = renderer.getDeviceDrawable();
        const Coord &totalSize = Coord(pDrawable->getDisplayDimensions().x, 20);
        DrawableWrapper wrapper(pDrawable, &titleDrawParameters, &menuSettings, Coord(0, 200), totalSize);
        wrapper.getDrawable()->setDrawColor(wrapper.bgCol());
        wrapper.getDrawable()->drawBox(wrapper.offsetLocation(Coord(0, 200)), totalSize, true);
        wrapper.getDrawable()->setDrawColor(wrapper.fgCol());
        wrapper.getDrawable()->drawBox(wrapper.offsetLocation(Coord(0, 200)), Coord(encVal, 20), true);
        wrapper.endDraw();
    }
} myDrawableDashboardDelegate;

void setupDashboard() {
    mainDashboard = new DrawableDashboard(renderer.getDeviceDrawable(), &renderer, &YesNoWidget,
                                          DrawableDashboard::DASH_ON_RESET_CLICK_EXIT);
    mainDashboard->setDelegate(&myDrawableDashboardDelegate);
    mainDashboard->setBaseColors(RGB(0, 0, 0), RGB(220, 220, 220));
    mainDashboard->addDrawingItem(&menuAnalog, Coord(0, 0), &drawAnalogValueWithIntRange, 10, nullptr, 10);
    mainDashboard->addDrawingItem(&menuEnum, Coord(0, 50), &drawEnumWithIntRange, 10, nullptr, 10);
    mainDashboard->addDrawingItem(&menuSettings, Coord(0, 100), &titleDrawParameters, 0, "Dashboard");
    renderer.setCustomDrawingHandler(mainDashboard);
}