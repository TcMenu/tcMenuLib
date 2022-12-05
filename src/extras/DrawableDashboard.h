
#ifndef TCMENU_DRAWABLE_DASHBOARD_H
#define TCMENU_DRAWABLE_DASHBOARD_H

#include "BaseRenderers.h"
#include <tcUnicodeHelper.h>

class DashDrawParameters {
public:
    enum DashAlign {
        TITLE_LEFT_VALUE_LEFT, TITLE_LEFT_VALUE_RIGHT,
        NO_TITLE_VALUE_LEFT, NO_TITLE_VALUE_RIGHT,
        TITLE_RIGHT_VALUE_LEFT, TITLE_RIGHT_VALUE_RIGHT };
protected:
    DashAlign alignment;
    union {
        const GFXfont *adaFont;
        const UnicodeFont *uniFont;
    };
    color_t fgColor;
    color_t bgColor;
    bool isAdaFont = true;
public:
    DashDrawParameters(color_t fgColor_, color_t bgColor_, const GFXfont* font_, DashAlign align = TITLE_RIGHT_VALUE_RIGHT);
    DashDrawParameters(color_t fgColor_, color_t bgColor_, const UnicodeFont* font_, DashAlign align = TITLE_RIGHT_VALUE_RIGHT);

    bool isTitleDrawn() { return alignment != NO_TITLE_VALUE_LEFT && alignment != NO_TITLE_VALUE_RIGHT; }
    bool isTitleLeftAlign() { return alignment == TITLE_LEFT_VALUE_LEFT || alignment == TITLE_LEFT_VALUE_RIGHT; }
    bool isValueLeftAlign() { return alignment == TITLE_RIGHT_VALUE_LEFT || alignment == TITLE_LEFT_VALUE_LEFT || alignment == NO_TITLE_VALUE_LEFT; }
    
    bool isAdafruitFont() { return adaFont; }
    const GFXfont* getAsAdaFont() { return adaFont; }
    const UnicodeFont* getAsUnicodeFont() { return uniFont; }

    virtual color_t getBgColor(MenuItem *item, bool updated)  { return bgColor; }
    virtual color_t getFgColor(MenuItem *item, bool updated)  { return fgColor; }
    virtual color_t getTitleBgColor(MenuItem *item, bool updated)  { return bgColor; }
    virtual color_t getTitleFgColor(MenuItem *item, bool updated)  { return fgColor; }
};

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

    color_t getBgColor(MenuItem* item, bool updated) override { return updated ? bgUpdateColor : bgColor; }

    color_t getFgColor(MenuItem* item, bool updated) override { return updated ? fgUpdateColor : fgColor; }
};

/**
 * A drawing parameter that updates the color based on ranges of integer values. For example one color could define
 * the integer values between 0..10 and another 11..20, in addition it can have a change in color for when the item
 * updates
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
            colorRanges(colorRanges_), numOfRanges(numberRanges) useUpdateColor(fgColor_ != fgUpdateColor_ || bgColor_ != bgUpdateColor_) { }

    DashDrawParametersIntUpdateRange(color_t fgColor_, color_t bgColor_, color_t fgUpdateColor_, color_t bgUpdateColor_,
                                     const UnicodeFont *font_, const IntColorRange colorRanges_[], int numberRanges,
                                     DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
            DashDrawParametersUpdate(fgColor_, bgColor_, fgUpdateColor_, bgUpdateColor_, font_, align),
            colorRanges(colorRanges_), numOfRanges(numberRanges) useUpdateColor(fgColor_ != fgUpdateColor_ || bgColor_ != bgUpdateColor_) { }

private:
    int findIndexForChoice(MenuItem* item);
    color_t getBgColor(MenuItem *item, bool updated) override;
    color_t getFgColor(MenuItem *item, bool updated) override;
};

/**
 * A drawing parameter that updates the color based on the text of a menu item. You can define various string values for
 * matching, the string value should be in program memory. In addition it can have a set of colors to handle change
 * on update.
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
                                     const Unicodeont *font_, const TextColorOverride colorOverrides_[], int numberRanges,
                                     DashAlign align = TITLE_RIGHT_VALUE_RIGHT) :
            DashDrawParametersUpdate(fgColor_, bgColor_, fgUpdateColor_, bgUpdateColor_, font_, align), colorOverrides(colorOverrides_),
            numOfRanges(numberRanges), useUpdateColor(fgColor_ != fgUpdateColor_ || bgColor_ != bgUpdateColor_) {}

private:
    int findIndexForChoice(MenuItem* item);
    color_t getBgColor(MenuItem *item, bool updated) override;
    color_t getFgColor(MenuItem *item, bool updated) override;
};

/**
 * Each item that is to appear in the dashboard can be attached to a menu item, this is the drawing class that will
 * present a given item in the dashboard.
 */
class DashMenuItem {
private:
    MenuItem *item;
    Coord screenLoc;
    DashDrawParameters *parameters;
    int updateCountDown;
    Coord titleExtents;
    int numChars;
    int valueWidth;
    int countDownTicks;
    char titleText[20];
public:
    DashMenuItem() : item(nullptr), screenLoc(0, 0), parameters(nullptr), updateCountDown(0), numChars(0), valueWidth(0),
                     titleText(), titleExtents(0, 0) {}
    DashMenuItem(MenuItem *theItem, Coord topLeft, DashDrawParameters* params, int numCharsInValue, const char* titleOverride, int countDownTicks);
    DashMenuItem(const DashMenuItem &other) = default;
    DashMenuItem& operator= (const DashMenuItem& other) = default;

    uint16_t getKey() const {
        return item != nullptr ? item->getId() : 0;
    }

    bool needsPainting();
    void setFont(DashDrawParameters* params, UnicodeFontHandler* unicodeHandler);
    void paintTitle();
    void paintItem(Adafruit_GFX *myGfx, DeviceDrawable* canvasDrawable, GFXcanvas1* canvas);
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
    TitleWidget* firstWidget;
    BaseMenuRenderer *renderer;
    AdafruitDrawable *canvasDrawable;
    BtreeList<uint16_t, DashMenuItem> drawingItems;
    DrawingMode drawingMode;
    bool running;
public:
    DrawableDashboard(DeviceDrawable *device, BaseMenuRenderer* renderer, TitleWidget* widgets, DashboardMode drawingMode)
            : firstWidget(widgets),renderer(renderer), drawingItems() { }
    ~DrawableDashboard() override = default;

    void clearItems() { drawingItems.clear(); }
    void addDrawingItem(MenuItem *theItem, Coord topLeft, DashDrawParameters* params, int numCharsInValue, const char* titleOverrideText = nullptr);
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
};

#endif //TCMENU_DRAWABLE_DASHBOARD_H
