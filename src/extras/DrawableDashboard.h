
#ifndef TCMENU_DRAWABLE_DASHBOARD_H
#define TCMENU_DRAWABLE_DASHBOARD_H

/**
 * @file
 * @brief DrawableDashboard.h a series of helper classes for building dashboards out of menu items.
 */

#include "BaseRenderers.h"
#include <tcUnicodeHelper.h>
#include <graphics/GraphicsDeviceRenderer.h>
#include <graphics/DeviceDrawableHelper.h>

/**
 * Base class for draw parameters, used for static items that do not change.
 * This class stores the alignment of text, the font and colors are stored in this
 * item. Fonts can be either Adafruit or TcUnicode.
 */
class DashDrawParameters {
public:
    /**
     * Controls the alignment of an item in the dashboard. Options are fairly self-explanatory.
     */
    enum DashAlign {
        TITLE_LEFT_VALUE_LEFT, TITLE_LEFT_VALUE_RIGHT,
        NO_TITLE_VALUE_LEFT, NO_TITLE_VALUE_RIGHT,
        TITLE_RIGHT_VALUE_LEFT, TITLE_RIGHT_VALUE_RIGHT,
    };
protected:
    DashAlign alignment;
    color_t fgColor;
    color_t bgColor;
    DeviceFontDrawingMode fontMode;
public:
    /**
     * @brief Creates a dash parameter that has a background, foreground, font, and alignment. In this case the font is an
     * Adafruit graphics font via tcUnicodeHelper.
     * @param fgColor_ the foreground color
     * @param bgColor_ the background color
     * @param font_ the font to draw with
     * @param align the alignment
     */
    DashDrawParameters(color_t fgColor_, color_t bgColor_, const GFXfont* font_, DashAlign align = TITLE_RIGHT_VALUE_RIGHT);
    /**
     * @brief Creates a dash parameter that has a background, foreground, font, and alignment. In this case the font is a
     * tcUnicode font.
     * @param fgColor_ the foreground color
     * @param bgColor_ the background color
     * @param font_ the font to draw with
     * @param align the alignment
     */
    DashDrawParameters(color_t fgColor_, color_t bgColor_, const UnicodeFont* font_, DashAlign align = TITLE_RIGHT_VALUE_RIGHT);
    /**
     * @brief Creates a dash parameter that has a background, foreground, font, and alignment. In this case the font is a
     * native font that works with the library directly.
     * @param fgColor_ the foreground color
     * @param bgColor_ the background color
     * @param font_ the font to draw with
     * @param align the alignment
     */
    DashDrawParameters(color_t fgColor_, color_t bgColor_, const NativeFontDesc& font_, DashAlign align = TITLE_RIGHT_VALUE_RIGHT);

    /**
     * @return true if the title is drawn, otherwise it returns false
     */
    bool isTitleDrawn() { return alignment != NO_TITLE_VALUE_LEFT && alignment != NO_TITLE_VALUE_RIGHT; }
    /**
     * @return true if the title is aligned to the left of the value
     */
    bool isTitleLeftAlign() { return alignment == TITLE_LEFT_VALUE_LEFT || alignment == TITLE_LEFT_VALUE_RIGHT; }
    /**
     * @return true if the title is aligned to the right of the value
     */
    bool isValueLeftAlign() { return alignment == TITLE_RIGHT_VALUE_LEFT || alignment == TITLE_LEFT_VALUE_LEFT || alignment == NO_TITLE_VALUE_LEFT; }

    /**
     * @return the font mode that is being used. See the enum for details
     */
    const DeviceFontDrawingMode& getFontMode() const { return fontMode; }

    /**
     * the background color method is overloaded, each implementation has a different way of handling it.
     * @return the background color, in this class it is fixed
     */
    virtual color_t getBgColor(MenuItem *item, bool updated)  { return bgColor; }
    /**
     * the foreground color method is overloaded, each implementation has a different way of handling it.
     * @return the background color, in this class it is fixed
     */
    virtual color_t getFgColor(MenuItem *item, bool updated)  { return fgColor; }

    /**
     * the background title color method is overloaded, each implementation has a different way of handling it.
     * @return the background color for the title, in this class it is fixed
     */
    virtual color_t getTitleBgColor(MenuItem *item, bool updated)  { return bgColor; }
    /**
     * the foreground title color method is overloaded, each implementation has a different way of handling it.
     * @return the foreground color for the title, in this class it is fixed
     */
    virtual color_t getTitleFgColor(MenuItem *item, bool updated)  { return fgColor; }
};

/**
 * In addition to regular dash draw parameters this class adds support to store colors for items that have updated
 */
class DashDrawParametersUpdate : public DashDrawParameters {
private:
    color_t fgUpdateColor;
    color_t bgUpdateColor;
public:
    DashDrawParametersUpdate(color_t fgColor_, color_t bgColor_, color_t fgUpdateColor_, color_t bgUpdateColor_,
                                 const GFXfont* font_, DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
                                 DashDrawParameters(fgColor_, bgColor_, font_, align), fgUpdateColor(fgUpdateColor_),
                                 bgUpdateColor(bgUpdateColor_) {}
    DashDrawParametersUpdate(color_t fgColor_, color_t bgColor_, color_t fgUpdateColor_, color_t bgUpdateColor_,
                                 const UnicodeFont* font_, DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
                                 DashDrawParameters(fgColor_, bgColor_, font_, align), fgUpdateColor(fgUpdateColor_),
                                 bgUpdateColor(bgUpdateColor_) {}
    DashDrawParametersUpdate(color_t fgColor_, color_t bgColor_, color_t fgUpdateColor_, color_t bgUpdateColor_,
                                 const NativeFontDesc& font_, DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
                                 DashDrawParameters(fgColor_, bgColor_, font_, align), fgUpdateColor(fgUpdateColor_),
                                 bgUpdateColor(bgUpdateColor_) {}

    color_t getBgColor(MenuItem* item, bool updated) override { return updated ? bgUpdateColor : bgColor; }

    color_t getFgColor(MenuItem* item, bool updated) override { return updated ? fgUpdateColor : fgColor; }
};

/**
 * A drawing parameter that updates the color based on ranges of integer values. For example one color could define
 * the integer values between 0..10 and another 11..20, in addition it can have a change in color for when the item
 * updates.
 *
 * Supported types are AnalogMenuItem, EnumMenuItem, BooleanMenuItem (values 0 and 1), and ScrollChoiceMenuItem.
 */
class DashDrawParametersIntUpdateRange : public DashDrawParametersUpdate {
public:
    struct IntColorRange {
        color_t fgColor;
        color_t bgColor;
        int minValue;
        int maxValue;
    };
private:
    const IntColorRange* colorRanges;
    int numOfRanges;
    bool useUpdateColor;
public:
    DashDrawParametersIntUpdateRange(color_t fgColor_, color_t bgColor_, color_t fgUpdateColor_, color_t bgUpdateColor_,
                                     const GFXfont *font_, const IntColorRange colorRanges_[], int numberRanges,
                                     DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
            DashDrawParametersUpdate(fgColor_, bgColor_, fgUpdateColor_, bgUpdateColor_, font_, align),
            colorRanges(colorRanges_), numOfRanges(numberRanges), useUpdateColor(fgColor_ != fgUpdateColor_ || bgColor_ != bgUpdateColor_) { }

    DashDrawParametersIntUpdateRange(color_t fgColor_, color_t bgColor_, color_t fgUpdateColor_, color_t bgUpdateColor_,
                                     const UnicodeFont *font_, const IntColorRange colorRanges_[], int numberRanges,
                                     DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
            DashDrawParametersUpdate(fgColor_, bgColor_, fgUpdateColor_, bgUpdateColor_, font_, align),
            colorRanges(colorRanges_), numOfRanges(numberRanges), useUpdateColor(fgColor_ != fgUpdateColor_ || bgColor_ != bgUpdateColor_) { }

    DashDrawParametersIntUpdateRange(color_t fgColor_, color_t bgColor_, color_t fgUpdateColor_, color_t bgUpdateColor_,
                                     const NativeFontDesc& font_, const IntColorRange colorRanges_[], int numberRanges,
                                     DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
            DashDrawParametersUpdate(fgColor_, bgColor_, fgUpdateColor_, bgUpdateColor_, font_, align),
            colorRanges(colorRanges_), numOfRanges(numberRanges), useUpdateColor(fgColor_ != fgUpdateColor_ || bgColor_ != bgUpdateColor_) { }
private:
    int findIndexForChoice(MenuItem* item);
    color_t getBgColor(MenuItem *item, bool updated) override;
    color_t getFgColor(MenuItem *item, bool updated) override;
};

/**
 * A drawing parameter that updates the color based on the text of a menu item. You can define various string values for
 * matching, the string value should be in program memory. In addition it can have a set of colors to handle change
 * on update.
 *
 * It works with any runtime menu item such as TextMenuItem and others.
 */
class DashDrawParametersTextUpdateRange : public DashDrawParametersUpdate {
public:
    struct TextColorOverride {
        const char* text;
        color_t fgColor;
        color_t bgColor;
    };
private:
    const TextColorOverride* colorOverrides;
    int numOfRanges;
    bool useUpdateColor;
public:
    DashDrawParametersTextUpdateRange(color_t fgColor_, color_t bgColor_, color_t fgUpdateColor_, color_t bgUpdateColor_,
                                     const GFXfont *font_, const TextColorOverride colorOverrides_[], int numberRanges,
                                     DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
            DashDrawParametersUpdate(fgColor_, bgColor_, fgUpdateColor_, bgUpdateColor_, font_, align), colorOverrides(colorOverrides_),
            numOfRanges(numberRanges), useUpdateColor(fgColor_ != fgUpdateColor_ || bgColor_ != bgUpdateColor_) {}

    DashDrawParametersTextUpdateRange(color_t fgColor_, color_t bgColor_, color_t fgUpdateColor_, color_t bgUpdateColor_,
                                     const UnicodeFont *font_, const TextColorOverride colorOverrides_[], int numberRanges,
                                     DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
            DashDrawParametersUpdate(fgColor_, bgColor_, fgUpdateColor_, bgUpdateColor_, font_, align), colorOverrides(colorOverrides_),
            numOfRanges(numberRanges), useUpdateColor(fgColor_ != fgUpdateColor_ || bgColor_ != bgUpdateColor_) {}

    DashDrawParametersTextUpdateRange(color_t fgColor_, color_t bgColor_, color_t fgUpdateColor_, color_t bgUpdateColor_,
                                      const NativeFontDesc &font_, const TextColorOverride colorOverrides_[], int numberRanges,
                                      DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
            DashDrawParametersUpdate(fgColor_, bgColor_, fgUpdateColor_, bgUpdateColor_, font_, align), colorOverrides(colorOverrides_),
            numOfRanges(numberRanges), useUpdateColor(fgColor_ != fgUpdateColor_ || bgColor_ != bgUpdateColor_) {}
private:
    int findIndexForChoice(MenuItem* item);
    color_t getBgColor(MenuItem *item, bool updated) override;
    color_t getFgColor(MenuItem *item, bool updated) override;
};

// forward reference
class DrawableDashboard;

/**
 * Each item that is to appear in the dashboard can be attached to a menu item, this is the drawing class that will
 * present a given item in the dashboard.
 */
class DashMenuItem {
private:
    DrawableDashboard* dashboard;
    MenuItem *item;
    Coord screenLoc;
    DashDrawParameters *parameters;
    int updateCountDown;
    Coord titleExtents;
    int numChars;
    int valueWidth;
    int countDownTicks;
    int baseline;
    char titleText[20];
public:
    DashMenuItem() : dashboard(nullptr), item(nullptr), screenLoc(0, 0), parameters(nullptr), updateCountDown(0), titleExtents(0, 0),
                     numChars(0), valueWidth(0), countDownTicks(0), baseline(0), titleText() {}
    DashMenuItem(DrawableDashboard* dashboard, MenuItem *theItem, Coord topLeft, DashDrawParameters* params, int numCharsInValue, const char* titleOverride, int countDownTicks);
    DashMenuItem(const DashMenuItem &other) = default;
    DashMenuItem& operator= (const DashMenuItem& other) = default;

    uint16_t getKey() const {
        return item != nullptr ? item->getId() : 0;
    }

    bool needsPainting();
    void paintTitle(DeviceDrawable* canvasDrawable);
    void paintItem(DeviceDrawable* canvasDrawable);
};

/**
 * Allows for easy extension of the below drawable dashboard by providing callbacks when important events occur, mainly
 * before and after a dashboard is opened or drawn to. It also tells you when a dashboard was closed, and if the display
 * has reset due to a timeout. This provides an easy way to customize further the dashboard support in nearly all cases.
 * The two "draw" functions are called in the rendering loop, one before any drawing, the other afterwards.
 */
class DrawableDashboardDelegate {
public:
    /**
     * Indicates that the dashboard has closed and no longer displayed
     */
    virtual void dashboardDidClose() {}
    /**
     * Indicates that the dashboard will open, called before any other work is done by the dashboard.
     * @return true if you have already cleared the screen, otherwise false to have the core code do it.
     */
    virtual bool dashboardWillOpen(BaseMenuRenderer* /*where*/) { return false; }
    /**
     * Indicates that the dashboard has already opened, called after any other work is done by the dashboard.
     */
    virtual void dashboardDidOpen(BaseMenuRenderer* /*where*/) {}
    /**
     * Indicates that the dashboard will start drawing, called before any other work is done by the dashboard. The
     * current value of the rotary encoder and state of the button are provided.
     */
    virtual void dashboardWillDraw(unsigned int /*currentValue*/, RenderPressMode /*mode*/) {}
    /**
     * Indicates that the dashboard drawing is completed, called after any other work is done by the dashboard. The
     * current value of the rotary encoder and state of the button are provided.
     */
    virtual void dashboardDidDraw(unsigned int /*currentValue*/, RenderPressMode /*mode*/) {}
    /**
     * Indicates that the display reset has been received from the renderer.
     */
    virtual void displayDidReset() {}
};

/**
 * Drawable Dashboard is a configurable dashboard that can be used to draw a series of menu items in a more configurable
 * dashboard style way. It is possible to further customise the class purely by extending it yourself and handling any
 * drawing in renderLoop() before calling the super implementation in this class.
 *
 * It is capable of presenting items either with or without titles in a large number of formats. Optionally you can add
 * title widgets to it for them to be presented at the same time.
 */
class DrawableDashboard : public CustomDrawing {
public:
    /** Allows you to define how the dashboard will be displayed, and dismissed. */
    enum DashboardMode: uint8_t { DASH_ON_RESET_CLICK_EXIT, DASH_ON_RESET_MANUAL_EXIT, DASH_FULLY_MANUAL, DASH_MANUAL_START_CLICK_EXIT };
private:
    DrawableDashboardDelegate *delegate = nullptr;
    TitleWidget* firstWidget;
    BaseGraphicalRenderer *renderer;
    DeviceDrawable *drawable;
    BtreeList<uint16_t, DashMenuItem> drawingItems;
    color_t screenBg = 0;
    color_t coreItemFg = 0;
    DashboardMode drawingMode;
    bool running;
public:
    DrawableDashboard(DeviceDrawable *device, BaseGraphicalRenderer* renderer, TitleWidget* widgets, DashboardMode drawingMode)
            : firstWidget(widgets), renderer(renderer), drawable(device), drawingItems(), drawingMode(drawingMode), running(false) { }
    ~DrawableDashboard() override = default;
    void setBaseColors(color_t screenBgCol, color_t coreFgCol) {
        screenBg = screenBgCol;
        coreItemFg = coreFgCol;
    }
    void setDelegate(DrawableDashboardDelegate* dashDelegate) { this->delegate = dashDelegate; }

    void clearItems() { drawingItems.clear(); }
    void addDrawingItem(MenuItem *theItem, Coord topLeft, DashDrawParameters* params, int numCharsInValue,
                        const char* titleOverrideText = nullptr, int updateTicks = 5);
    void stop();
    void reset() override;
    void started(BaseMenuRenderer *currentRenderer) override;

    /**
     * actually do the drawing, this is essentially the runloop. Work out what's changed and draw it.
     *
     * @param currentValue This is the encoder position if the menu is using an encoder
     * @param userClicked this represents the status of the select button, see RenderPressMode for more details
     */
    void renderLoop(unsigned int currentValue, RenderPressMode userClick) override;

    void drawWidgets(bool force);

    uint8_t getDisplayNumber() { return renderer->getDisplayNumber(); }
};

#endif //TCMENU_DRAWABLE_DASHBOARD_H
