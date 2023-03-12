//
// This file contains a simple dashboard that is set up using the dashboard classes
// within the menu library extras. See ref docs:
//
// Reference docs https://www.thecoderscorner.com/ref-docs/tcmenu/html/_drawable_dashboard_8h.html
//

#include "generated/picoAdafruitDashboard_menu.h"
#include <Adafruit_ILI9341.h>
#include "dashboardConfig.h"
#include <graphics/TcDrawableButton.h>

#define LCD_LIGHT_BLUE RGB(133, 226, 236)
#define LCD_SEL_BLUE RGB(123, 196, 206)

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
DashDrawParametersIntUpdateRange drawEnumWithIntRange(ILI9341_BLACK, LCD_LIGHT_BLUE, ILI9341_BLACK, ILI9341_YELLOW,
                                                       NativeFontDesc(&RobotoMedium24, 1), drawEnumColorRanges, 2);

// As above we create another one for the analog item, it has two ranges.
// Note that this is a parameter, not the actual dashboard item, they are defined below
DashDrawParametersIntUpdateRange::IntColorRange drawAnalogColorRanges[] {
        {ILI9341_LIGHTGREY, ILI9341_BLUE, 0, 50},
        {ILI9341_YELLOW, ILI9341_RED, 51, 100}
};
DashDrawParametersIntUpdateRange drawAnalogValueWithIntRange(ILI9341_BLACK, LCD_LIGHT_BLUE, ILI9341_BLACK, ILI9341_YELLOW,
                                                             NativeFontDesc(&RobotoMedium24, 1), drawAnalogColorRanges, 2);

DrawableIcon iconForButton3(-1, Coord(17,12), tcgfx::DrawableIcon::ICON_XBITMAP, YesNoWidIcon0, YesNoWidIcon1);

//
// Although the dashboard support provides a wide range of menu drawing capabilities, it does not cover every case
// so you can create a delegate that can do the extra drawing and other functions that are needed. Here is a simple
// example that draws something that resembles a tab with menu items within it, and three buttons that select according
// to the encoder state.
//
// For drawing onto device drawble see:
//  https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/rendering-with-tcmenu-lcd-tft-oled/#drawing-direct-to-the-display-with-devicedrawabl
// For more on the delegate class and all the points where you can extend
//  https://www.thecoderscorner.com/ref-docs/tcmenu/html/class_drawable_dashboard_delegate.html
//
class MyDrawableDashboardDelegate : public DrawableDashboardDelegate {
private:
    TcDrawableButton button1;
    TcDrawableButton button2;
    TcDrawableButton button3;
    int lastEncoderTurn = 0;
public:
    MyDrawableDashboardDelegate() : button1(Coord(10, 190), Coord(90, 40), LCD_LIGHT_BLUE, ILI9341_BLACK, LCD_SEL_BLUE, "Btn1",
                                            DeviceFontDrawingMode(NativeFontDesc(&RobotoMedium24, 1))),
                                    button2(Coord(115, 190), Coord(90, 40), LCD_LIGHT_BLUE, ILI9341_BLACK, LCD_SEL_BLUE, "Btn2",
                                            DeviceFontDrawingMode(NativeFontDesc(&RobotoMedium24, 1))),
                                    button3(Coord(220, 190), Coord(90, 40), LCD_LIGHT_BLUE, ILI9341_BLACK, LCD_SEL_BLUE, &iconForButton3) { }

    // this is called before the dashboard titles are drawn when first presented.
    // you return true to tell the core code that you've already cleared the screen, otherwise false.
    bool dashboardWillOpen(BaseMenuRenderer *renderer) override {
        // when the dashboard first opens, we reset our buttons to the initial state, and then set the encoders range
        // and current value to match that, which is a range of 0..2 and current of 0.
        button1.setButtonDrawingMode(tcgfx::TcDrawableButton::SELECTED);
        button2.setButtonDrawingMode(tcgfx::TcDrawableButton::NORMAL);
        button3.setButtonDrawingMode(tcgfx::TcDrawableButton::NORMAL);
        lastEncoderTurn = 0;
        switches.getEncoder()->changePrecision(2, 0); // encoder has range 0,1,2 and initial value of 0.

        // we can still make native library calls here if we wish.
        gfx.fillScreen(ILI9341_BLACK);

        // here we draw two boxes that make what looks like a tab
        gfxDrawable.setDrawColor(LCD_LIGHT_BLUE);
        gfxDrawable.drawBox(Coord(10, 10), Coord(80, 25), true);
        gfxDrawable.drawBox(Coord(10, 35), Coord(300, 75), true);

        // and this is how we put some text into it, the text is off-screen buffered if the device supports it.
        // the wrapper takes care of the differences in different text implementations. We must call endDraw when
        // we've finished with the helper class, as if the display is double buffered, it will push it at that point.
        color_t palette[2] = { LCD_LIGHT_BLUE, RGB(0, 0, 0) };
        DeviceDrawableHelper helper(&gfxDrawable, palette, 2, Coord(10, 10), Coord(80, 25));
        helper.getDrawable()->setDrawColor(palette[0]);
        helper.setFont(DeviceFontDrawingMode(NativeFontDesc(&RobotoMedium24, 1)));
        helper.getDrawable()->drawBox(helper.offsetLocation(Coord(10, 10)), Coord(80, 25), true);
        helper.drawText(helper.offsetLocation(Coord(12, 10)), palette[1], "Test");
        helper.endDraw();

        // we cleared the screen - true
        return true;
    }

    // this is called before the dashboard draws any items
    void dashboardWillDraw(unsigned int encVal, RenderPressMode mode) override {
        // Drawing a simple bitmap example
        renderer.getDeviceDrawable()->drawXBitmap(Coord(300, 150), Coord(17, 12), TickIconBitmap0);

        // here we store the last encoder position, and if it has changed from last time, we update all the buttons
        // with which one is now selected. Doing so will make the button repaint.
        if(lastEncoderTurn != encVal) {
            lastEncoderTurn = encVal;
            button1.setButtonDrawingMode(encVal == 0 ? TcDrawableButton::SELECTED : TcDrawableButton::NORMAL);
            button2.setButtonDrawingMode(encVal == 1 ? TcDrawableButton::SELECTED : TcDrawableButton::NORMAL);
            button3.setButtonDrawingMode(encVal == 2 ? TcDrawableButton::SELECTED : TcDrawableButton::NORMAL);
        }

        // Here we tell each button we have to repaint itself, it is lazy and will only repaint when needed
        button1.paintButton(renderer.getDeviceDrawable());
        button2.paintButton(renderer.getDeviceDrawable());
        button3.paintButton(renderer.getDeviceDrawable());
    }

    // this is called after the dashboard has drawn all items, you get the current value of the encoder and the
    // current state of the select button.
    void dashboardDidDraw(unsigned int encVal, RenderPressMode mode) override {
        // get the drawing device from the renderer and its dimensions
        DeviceDrawable *pDrawable = renderer.getDeviceDrawable();
        const Coord &totalSize = Coord(pDrawable->getDisplayDimensions().x, 20);
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
    mainDashboard->addDrawingItem(&menuAnalog, Coord(20, 40), &drawAnalogValueWithIntRange, 10, nullptr, 10);
    mainDashboard->addDrawingItem(&menuEnum, Coord(20, 68), &drawEnumWithIntRange, 10, nullptr, 10);

    // lastly, add the dashboard to the renderer, this is important, the dashboard implements CustomDrawing so it
    // handles taking over the display and reset notification.
    renderer.setCustomDrawingHandler(mainDashboard);
}
