#include <BaseRenderers.h>
#include "DrawableDashboard.h"
#include "ScrollChoiceMenuItem.h"

void DrawableDashboard::addDrawingItem(MenuItem *theItem, Coord topLeft, DashDrawParameters *params, int numCharsInValue,
                                  const char *titleOverrideText, int updateTicks) {
    drawingItems.add(DashMenuItem(theItem, topLeft, params, numCharsInValue, titleOverrideText, updateTicks));
    serdebugF2("added item to list #", drawingItems.count());
}

void DrawableDashboard::stop() {
    running = false;
    renderer->giveBackDisplay();
    if(delegate != nullptr) delegate->dashboardDidClose();
}

void DrawableDashboard::reset() {
    if(delegate != nullptr) delegate->displayDidReset();
    if (drawingMode == DASH_ON_RESET_CLICK_EXIT || drawingMode == DASH_ON_RESET_MANUAL_EXIT) {
        if (!running) renderer->takeOverDisplay();
    }
}

void DrawableDashboard::started(BaseMenuRenderer *currentRenderer) {
    if(delegate != nullptr) delegate->dashboardWillOpen(renderer);

    renderer = currentRenderer;
    drawable->setDrawColor(screenBg);
    drawable->drawBox(Coord(0, 0), drawable->getDisplayDimensions(), true);
    serdebugF2("drawing titles #", drawingItems.count());

    for (size_t i = 0; i < drawingItems.count(); i++) {
        auto drawing = drawingItems.itemAtIndex(i);
        drawing->paintTitle(drawable);
        drawing->paintItem(drawable);
    }

    drawWidgets(true);

    if(delegate != nullptr) delegate->dashboardDidOpen(renderer);
}

void DrawableDashboard::renderLoop(unsigned int currentValue, RenderPressMode userClick) {
    if(delegate != nullptr) delegate->dashboardWillDraw(currentValue, userClick);

    if (userClick != RPRESS_NONE && (drawingMode == DASH_ON_RESET_CLICK_EXIT || drawingMode == DASH_MANUAL_START_CLICK_EXIT)) {
        serlogF(SER_TCMENU_INFO, "Dashboard exit, clicked");
        stop();
        return;
    }

    for (size_t i = 0; i < drawingItems.count(); i++) {
        auto drawing = drawingItems.itemAtIndex(i);
        if (drawing->needsPainting()) {
            drawing->paintItem(drawable);
        }
    }

    drawWidgets(false);

    if(delegate != nullptr) delegate->dashboardDidDraw(currentValue, userClick);
}

void DrawableDashboard::drawWidgets(bool force) {
    auto widget = firstWidget;
    int xPos = drawable->getDisplayDimensions().x;
    while(widget) {
        if(force || widget->isChanged()) {
            xPos -= (widget->getWidth() + 3);
            drawable->setColors(coreItemFg, screenBg);
            drawable->drawXBitmap(Coord(xPos, 3), Coord(widget->getWidth(), widget->getHeight()), widget->getCurrentIcon());
            widget->setChanged(false);
        }
        widget = widget->getNext();
    }
}

DashDrawParameters::DashDrawParameters(color_t fgColor_, color_t bgColor_, const GFXfont *font_, DashAlign align) {
    alignment = align;
    adaFont = font_;
    isAdaFont = true;
    fgColor = fgColor_;
    bgColor = bgColor_;
}

DashDrawParameters::DashDrawParameters(color_t fgColor_, color_t bgColor_, const UnicodeFont *font_, DashAlign align) {
    alignment = align;
    uniFont = font_;
    isAdaFont = false;
    fgColor = fgColor_;
    bgColor = bgColor_;
}

int DashDrawParametersIntUpdateRange::findIndexForChoice(MenuItem* item) {
    int val;
    if(isMenuBasedOnValueItem(item)) {
        val = reinterpret_cast<ValueMenuItem *>(item)->getCurrentValue();
    } else if(item->getMenuType() == MENUTYPE_SCROLLER_VALUE) {
        val = reinterpret_cast<ScrollChoiceMenuItem*>(item)->getCurrentValue();
    } else {
        return -1;
    }

    for(int i=0;i<numOfRanges;i++) {
        if(val >= colorRanges[i].minValue && val <= colorRanges[i].maxValue) {
            return i;
        }
    }

    return -1;
}

color_t DashDrawParametersIntUpdateRange::getBgColor(MenuItem *item, bool updated) {
    if (useUpdateColor && updated) return DashDrawParametersUpdate::getBgColor(item, updated);
    auto idx = findIndexForChoice(item);
    return (idx != -1) ? colorRanges[idx].bgColor : bgColor;
}

color_t DashDrawParametersIntUpdateRange::getFgColor(MenuItem *item, bool updated) {
    if (useUpdateColor && updated) return DashDrawParametersUpdate::getFgColor(item, updated);
    auto idx = findIndexForChoice(item);
    return (idx != -1) ? colorRanges[idx].fgColor : fgColor;
}

int DashDrawParametersTextUpdateRange::findIndexForChoice(MenuItem* item) {
    if(isMenuRuntime(item)) {
        auto rtMenu = reinterpret_cast<RuntimeMenuItem*>(item);
        char sz[32];
        rtMenu->copyValue(sz, sizeof sz);
        for(int i=0;i<numOfRanges;i++) {
            if(strcmp(colorOverrides[i].text, sz) == 0) {
                return i;
            }
        }
    }
    return -1;
}

color_t DashDrawParametersTextUpdateRange::getBgColor(MenuItem *item, bool updated) {
    if (useUpdateColor && updated) return DashDrawParametersUpdate::getBgColor(item, updated);
    auto idx = findIndexForChoice(item);
    return (idx != -1) ? colorOverrides[idx].bgColor : bgColor;
}

color_t DashDrawParametersTextUpdateRange::getFgColor(MenuItem *item, bool updated) {
    if (useUpdateColor && updated) return DashDrawParametersUpdate::getFgColor(item, updated);
    auto idx = findIndexForChoice(item);
    return (idx != -1) ? colorOverrides[idx].fgColor : fgColor;
}

color_t DashMenuItem::staticPalette[4] = {};

DashMenuItem::DashMenuItem(MenuItem *theItem, Coord topLeft, DashDrawParameters* params, int numCharsInValue,
                           const char* titleOverride, int countDownTicks) : item(theItem), screenLoc(topLeft),
                               parameters(params), updateCountDown(countDownTicks), titleExtents(0, 0),
                               numChars(numCharsInValue), valueWidth(0), countDownTicks(countDownTicks), baseline(0),
                               titleText{} {

    if(titleOverride != nullptr) {
        strncpy(titleText, titleOverride, sizeof(titleText));
    }
    else {
        theItem->copyNameToBuffer(titleText, sizeof(titleText));
    }
    titleText[sizeof(titleText)-1] = 0; // make sure it's null terminated.
}

bool DashMenuItem::needsPainting() {
    if (item == nullptr) return false;

    if(item->isChanged()) updateCountDown = countDownTicks;

    if(updateCountDown > 0) --updateCountDown;
    return item->isChanged() || updateCountDown != 0;
}

void DashMenuItem::setFont(UnicodeFontHandler* unicodeHandler) {
    if(parameters->isAdafruitFont()) {
        unicodeHandler->setFont(parameters->getAsAdaFont());
    } else {
        unicodeHandler->setFont(parameters->getAsUnicodeFont());
    }
}

void DashMenuItem::paintTitle(DeviceDrawable* drawableRoot) {
    UnicodeFontHandler* unicodeHandler = drawableRoot->getUnicodeHandler(true);
    setFont(unicodeHandler);
    titleExtents = unicodeHandler->textExtents(titleText, &baseline);
    valueWidth = unicodeHandler->textExtents("0", &baseline).x * numChars;
    valueWidth = int(valueWidth * 1.20);

    DrawableWrapper wrapper(drawableRoot, parameters, item, screenLoc, Coord(titleExtents.x + valueWidth, titleExtents.y), true);
    unicodeHandler = wrapper.getDrawable()->getUnicodeHandler(true);

    if(!parameters->isTitleDrawn()) return;

    auto startX = (parameters->isTitleLeftAlign()) ? 0 : valueWidth + 1;
    wrapper.getDrawable()->setDrawColor(wrapper.bgCol());
    wrapper.getDrawable()->drawBox(wrapper.offsetLocation(screenLoc), Coord(titleExtents.x + valueWidth, titleExtents.y), true);

    unicodeHandler->setDrawColor(wrapper.fgColUnderlying());
    unicodeHandler->setCursor(wrapper.offsetLocation(screenLoc, startX, titleExtents.y - baseline));
    setFont(unicodeHandler);
    unicodeHandler->print(titleText);

    wrapper.endDraw();
}

void DashMenuItem::paintItem(DeviceDrawable* drawableRoot) {
    item->setChanged(false);
    char sz[20];
    DrawableWrapper wrapper(drawableRoot, parameters, item, screenLoc, Coord(valueWidth, titleExtents.y));

    copyMenuItemValue(item, sz, sizeof(sz));
    wrapper.getDrawable()->setDrawColor(wrapper.bgCol());
    wrapper.getDrawable()->drawBox(wrapper.offsetLocation(screenLoc), Coord(valueWidth, titleExtents.y), true);

    UnicodeFontHandler* unicodeHandler = wrapper.getDrawable()->getUnicodeHandler(true);
    unicodeHandler->setDrawColor(wrapper.fgColUnderlying());
    setFont(unicodeHandler);
    auto padding = 0;
    if(!parameters->isValueLeftAlign()) {
        Coord valueLen = unicodeHandler->textExtents(sz, &baseline);
        padding = valueWidth - (valueLen.x + 4);
    }
    unicodeHandler->setCursor(wrapper.offsetLocation(screenLoc, padding, unicodeHandler->getYAdvance() - baseline));
    unicodeHandler->print(sz);

    wrapper.endDraw();
}
