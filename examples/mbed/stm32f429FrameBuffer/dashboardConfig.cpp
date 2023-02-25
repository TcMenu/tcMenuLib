//
// This file contains a simple dashboard that is set up using the dashboard classes
// within the menu library extras. See ref docs:
//
// Reference docs https://www.thecoderscorner.com/ref-docs/tcmenu/html/_drawable_dashboard_8h.html
//

#include "stm32f429FrameBuffer_menu.h"
#include "dashboardConfig.h"
#include <graphics/TcDrawableButton.h>
#include <stockIcons/wifiAndConnectionIcons16x12.h>

#define LCD_LIGHT_BLUE RGB(133, 226, 236)
#define LCD_SEL_BLUE RGB(103, 176, 186)
#define LCD_RED RGB(255, 0, 0)
#define LCD_YELLOW RGB(255, 255, 0)
#define LCD_BLACK RGB(0, 0, 0)
#define LCD_BLUE RGB(0, 0, 255)
#define LCD_LIGHTGREY RGB(192, 192, 192)
#define LCD_WHITE RGB(255, 255, 255)

// Widget Generator yesNo
TitleWidget connectedWidget(iconsConnection, 2, 16, 12, nullptr);

// we define a global pointer to the dashboard, it will be created during the dash setup.
DrawableDashboard* mainDashboard;

// Any value item or scroll choice (integer based items) can have ranges of colors, these allow the color to be
// selected based on the value, IE for a rev-counter above 8000 RPM may be shown in red, otherwise green. These
// represented as below. In this case we are creating for an enum value.
// Note that this is a parameter, not the actual dashboard item, they are defined below
DashDrawParametersIntUpdateRange::IntColorRange drawVoltageColorRanges[] {
        {LCD_COLOR_YELLOW, LCD_COLOR_RED, 2400, 2430},
        {LCD_WHITE, LCD_BLUE, 2440, 3000}
};
DashDrawParametersIntUpdateRange drawVoltageWithIntRange(LCD_BLACK, LCD_LIGHT_BLUE, LCD_BLACK, LCD_YELLOW,
                                                       &RobotoMedium24, drawVoltageColorRanges, 2);

// As above we create another one for the analog item, it has two ranges.
// Note that this is a parameter, not the actual dashboard item, they are defined below
DashDrawParametersIntUpdateRange::IntColorRange drawPowerColorRanges[] {
        {LCD_LIGHTGREY, LCD_BLUE, 2000, 2100},
        {LCD_YELLOW, LCD_RED, 2100, 3000}
};
DashDrawParametersIntUpdateRange drawPowerValueWithIntRange(LCD_BLACK, LCD_LIGHT_BLUE, LCD_BLACK, LCD_YELLOW,
                                                             &RobotoMedium24, drawPowerColorRanges, 2);

//
// Although the dashboard support provides a wide range of menu drawing capabilities, it does not cover every case,
// so you can create a delegate that can do the extra drawing and other functions that are needed. Here is a simple
// example that draws something that resembles a tab with menu items within it, and three buttons that select according
// to the encoder state.
//
// For drawing onto device drawable see:
//  https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/rendering-with-tcmenu-lcd-tft-oled/#drawing-direct-to-the-display-with-devicedrawabl
// For more on the delegate class and all the points where you can extend
//  https://www.thecoderscorner.com/ref-docs/tcmenu/html/class_drawable_dashboard_delegate.html
//
class MyDrawableDashboardDelegate : public DrawableDashboardDelegate {
private:
    TcDrawableButton button1;
    TcDrawableButton button2;
    int lastEncoderTurn = 0;
public:
    MyDrawableDashboardDelegate() : button1(Coord(10, 240), Coord(105, 60), LCD_LIGHT_BLUE, LCD_BLACK, LCD_SEL_BLUE, "AC",
                                            DeviceFontDrawingMode(&RobotoMedium24)),
                                    button2(Coord(125, 240), Coord(105, 60), LCD_LIGHT_BLUE, LCD_BLACK, LCD_SEL_BLUE, "Batt",
                                            DeviceFontDrawingMode(&RobotoMedium24)) { }

    // this is called before the dashboard titles are drawn when first presented.
    // you return true to tell the core code that you've already cleared the screen, otherwise false.
    bool dashboardWillOpen(BaseMenuRenderer *renderer) override {
        // when the dashboard first opens, we reset our buttons to the initial state, and then set the encoders range
        // and current value to match that, which is a range of 0..1 and current of 0.
        button1.setButtonDrawingMode(tcgfx::TcDrawableButton::SELECTED);
        button2.setButtonDrawingMode(tcgfx::TcDrawableButton::NORMAL);

        auto dr = ::renderer.getDeviceDrawable();

        // clear the screen
        dr->setDrawColor(LCD_BLACK);
        dr->drawBox(Coord(0,0), dr->getDisplayDimensions(), true);

        // here we draw two boxes that make what looks like a tab
        dr->setDrawColor(LCD_LIGHT_BLUE);
        dr->drawBox(Coord(10, 10), Coord(80, 25), true);
        dr->drawBox(Coord(10, 35), Coord(220, 75), true);

        // and this is how we put some text into it, the text is off-screen buffered if the device supports it.
        // the wrapper takes care of the differences in different text implementations. We must call endDraw when
        // we've finished with the helper class, as if the display is double buffered, it will push it at that point.
        color_t palette[2] = { LCD_LIGHT_BLUE, RGB(0, 0, 0) };
        DeviceDrawableHelper helper(dr, palette, 2, Coord(10, 10), Coord(80, 25));
        helper.getDrawable()->setDrawColor(palette[0]);
        helper.setFont(DeviceFontDrawingMode(&RobotoMedium24));
        helper.getDrawable()->drawBox(helper.offsetLocation(Coord(10, 10)), Coord(80, 25), true);
        helper.drawText(helper.offsetLocation(Coord(12, 10)), palette[1], "Power");
        helper.endDraw();

        // we cleared the screen - true
        return true;
    }

    // this is called before the dashboard draws any items
    void dashboardWillDraw(unsigned int encVal, RenderPressMode mode) override {
        // here we store the last encoder position, and if it has changed from last time, we update all the buttons
        // with which one is now selected. Doing so will make the button repaint.
        if(touchScreen.getLastTouchState() == iotouch::TOUCHED || touchScreen.getLastTouchState() == iotouch::HELD) {
            // check if either button has been selected.
            Coord touchLocation = touchScreen.getLastScreenCoord();
            button1.setButtonDrawingMode(button1.touchInBounds(touchLocation) ? TcDrawableButton::SELECTED : TcDrawableButton::NORMAL);
            button2.setButtonDrawingMode(button2.touchInBounds(touchLocation) ? TcDrawableButton::SELECTED : TcDrawableButton::NORMAL);

            // if the touch is outside the button area, give back the display
            if(touchLocation.y < 200) {
                mainDashboard->stop();
            }
        }

        // Here we tell each button we have to repaint itself, it is lazy and will only repaint when needed
        button1.paintButton(renderer.getDeviceDrawable());
        button2.paintButton(renderer.getDeviceDrawable());
    }

    // this is called after the dashboard has drawn all items, you get the current value of the encoder and the
    // current state of the select button.
    void dashboardDidDraw(unsigned int encVal, RenderPressMode mode) override {
        // get the drawing device from the renderer and its dimensions
        DeviceDrawable *pDrawable = renderer.getDeviceDrawable();
        const Coord &totalSize = Coord(pDrawable->getDisplayDimensions().x, 20);
        serlogF3(SER_DEBUG, "totalSize=", totalSize.x, totalSize.y);
    }
} myDrawableDashboardDelegate;

void setupDashboard() {
    // Reference docs https://www.thecoderscorner.com/ref-docs/tcmenu/html/_drawable_dashboard_8h.html

    // create a dashboard instance, giving it the renderer and drawable, any widgets to display in the top corner,
    // and the mode in which it is to operate. I personally don't mind using "new" during on-off configuration, and
    // it makes the example read better, but could equally be created globally.
    mainDashboard = new DrawableDashboard(renderer.getDeviceDrawable(), &renderer, &connectedWidget,
                                          DrawableDashboard::DASH_ON_RESET_CLICK_EXIT);

    // here we tell the dashboard about the delegate we created above.
    mainDashboard->setDelegate(&myDrawableDashboardDelegate);

    // now prepare the base colors
    mainDashboard->setBaseColors(RGB(0, 0, 0), RGB(220, 220, 220));

    // here we set up the entries on the dashboard, this is where we provide the menu item and position on the display
    // for each entry. A parameter object that we defined above is then associated with an item. Note that more than
    // one entry can share a parameter.
    mainDashboard->addDrawingItem(&menuACLine, Coord(20, 40), &drawVoltageWithIntRange, 5, nullptr, 10);
    mainDashboard->addDrawingItem(&menuConsumption, Coord(20, 68), &drawPowerValueWithIntRange, 5, nullptr, 10);

    // lastly, add the dashboard to the renderer, this is important, the dashboard implements CustomDrawing, so it
    // handles taking over the display and reset notification.
    renderer.setCustomDrawingHandler(mainDashboard);
}
