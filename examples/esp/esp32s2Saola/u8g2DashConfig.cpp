//
// This file contains a simple dashboard that is set up using the dashboard classes
// within the menu library extras. See ref docs:
//
// Reference docs https://www.thecoderscorner.com/ref-docs/tcmenu/html/_drawable_dashboard_8h.html
//

#include "u8g2DashConfig.h"
#include <graphics/TcDrawableButton.h>

using namespace tcgfx;

// we define a global pointer to the dashboard, it will be created during the dash setup.
DrawableDashboard* mainDashboard;
color_t defaultPalette[2] = {0, 1};

//
// Although the dashboard support provides a wide range of menu drawing capabilities, it does not cover every case,
// so you can create a delegate that can do the extra drawing and other functions that are needed. Here is a simple
// example that draws something that resembles a tab with menu items within it, and three buttons that select according
// to the encoder state.
//
// For drawing onto device drawable see:
//  https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/rendering-with-tcmenu-lcd-tft-oled/#drawing-direct-to-the-display-with-devicedrawable
// For more on the delegate class and all the points where you can extend
//  https://www.thecoderscorner.com/ref-docs/tcmenu/html/class_drawable_dashboard_delegate.html
//

class MyDrawableDashboardDelegate : public DrawableDashboardDelegate {
private:
    TcDrawableButton button;
public:
    MyDrawableDashboardDelegate(): button(Coord(5, 50), Coord(20, 10), 1, 0, 0, "Btn", DeviceFontDrawingMode(NativeFontDesc(u8g2_font_5x7_mr, 1))) {}

    void displayDidReset() override {
        // when the display resets, we want to take over the display and show the dash.
        renderer.takeOverDisplay();

    }

    bool dashboardWillOpen(BaseMenuRenderer *renderer) override {
        // change the encoder range when the dashboard opens to 0..1 with 0 selected.
        switches.getEncoder()->changePrecision(1, 0);

        // and set the button as monochrome
        button.setButtonOnMonoDisplay(true);

        // we did not clear the screen, so the core must do it. Return false = screen not cleared
        return false;
    }

    void dashboardDidDraw(unsigned int currentEncoder, RenderPressMode mode) override {
        // draw a box at the bottom of the screen and present a button on the left
        auto drawable = renderer.getDeviceDrawable();

        // here we demonstrate painting onto the device drawable for U8G2. Documentation link:
        // https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/rendering-with-tcmenu-lcd-tft-oled/#drawing-direct-to-the-display-with-devicedrawable
        DeviceDrawableHelper helper(drawable, defaultPalette, 2, Coord(115, 55), Coord(10, 10));
        helper.getDrawable()->setDrawColor(1);
        helper.getDrawable()->drawBox(helper.offsetLocation(Coord(100, 50)), Coord(10, 10), true);
        helper.endDraw();

        // here we manage the button, if the encoder value is 1, then the button is selected, otherwise it is normal
        button.setButtonDrawingMode(currentEncoder == 1 ? TcDrawableButton::SELECTED : TcDrawableButton::NORMAL);
        button.paintButton(drawable);

    }
} myDrawableDashboardDelegate;

DashDrawParametersUpdate drawWithUpdate(1, 0, 0, 1, NativeFontDesc(u8g2_font_5x7_mr, 1));

void setupDashboard() {
    // Reference docs https://www.thecoderscorner.com/ref-docs/tcmenu/html/_drawable_dashboard_8h.html

    // create a dashboard instance, giving it the renderer and drawable, any widgets to display in the top corner,
    // and the mode in which it is to operate. I personally don't mind using "new" during one-off configuration, and
    // it makes the example read better, but could equally be created globally.
    mainDashboard = new DrawableDashboard(renderer.getDeviceDrawable(), &renderer, &wifiWidget,
                                          DrawableDashboard::DASH_ON_RESET_CLICK_EXIT);

    // here we tell the dashboard about the delegate we created above.
    mainDashboard->setDelegate(&myDrawableDashboardDelegate);

    // we are on mono, not much choice with the base colors!
    mainDashboard->setBaseColors(BLACK, WHITE);

    // here we set up the entries on the dashboard, this is where we provide the menu item and position on the display
    // for each entry. A parameter object that we defined above is then associated with an item. Note that more than
    // one entry can share a parameter.
    mainDashboard->addDrawingItem(&menuIntEdit, Coord(20, 15), &drawWithUpdate, 5, nullptr, 10);
    mainDashboard->addDrawingItem(&menuHalves, Coord(20, 30), &drawWithUpdate, 5, nullptr, 10);

    // lastly, add the dashboard to the renderer, this is important, the dashboard implements CustomDrawing, so it
    // handles taking over the display and reset notification.
    renderer.setCustomDrawingHandler(mainDashboard);

}