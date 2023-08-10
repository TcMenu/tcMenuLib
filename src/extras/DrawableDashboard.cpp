#include <BaseRenderers.h>
#include "DrawableDashboard.h"
#include "ScrollChoiceMenuItem.h"

using namespace tcgfx;

void DrawableDashboard::addDrawingItem(MenuItem *theItem, Coord topLeft, DashDrawParameters *params, int numCharsInValue,
                                  const char *titleOverrideText, int updateTicks) {
    drawingItems.add(DashMenuItem(this, theItem, topLeft, params, numCharsInValue, titleOverrideText, updateTicks));
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
    if(renderer != currentRenderer) {
        serlogF(SER_ERROR, "Dashboard renderer mismatch!!");
    }
    drawable->startDraw();
    bool screenCleared = false;
    if(delegate != nullptr) screenCleared = delegate->dashboardWillOpen(renderer);

    if(!screenCleared) {
        drawable->setDrawColor(screenBg);
        drawable->drawBox(Coord(0, 0), drawable->getDisplayDimensions(), true);
    }

    serdebugF2("drawing titles #", drawingItems.count());

    for (size_t i = 0; i < drawingItems.count(); i++) {
        auto drawing = drawingItems.itemAtIndex(i);
        drawing->paintTitle(drawable);
        drawing->paintItem(drawable);
    }

    drawWidgets(true);

    if(delegate != nullptr) delegate->dashboardDidOpen(renderer);
    drawable->endDraw(true);
}

void DrawableDashboard::renderLoop(unsigned int currentValue, RenderPressMode userClick) {
    drawable->startDraw();
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
    drawable->endDraw(true);
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
    fontMode = DeviceFontDrawingMode(font_);
    fgColor = fgColor_;
    bgColor = bgColor_;
}

DashDrawParameters::DashDrawParameters(color_t fgColor_, color_t bgColor_, const UnicodeFont *font_, DashAlign align) {
    alignment = align;
    fontMode = DeviceFontDrawingMode(font_);
    fgColor = fgColor_;
    bgColor = bgColor_;
}

DashDrawParameters::DashDrawParameters(color_t fgColor_, color_t bgColor_, const NativeFontDesc& font_, DashAlign align) {
    alignment = align;
    fontMode = DeviceFontDrawingMode(font_);
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

DashMenuItem::DashMenuItem(DrawableDashboard* dashboard, MenuItem *theItem, Coord topLeft, DashDrawParameters* params,
                           int numCharsInValue, const char* titleOverride, int countDownTicks) : dashboard(dashboard),
                           item(theItem), screenLoc(topLeft),parameters(params), updateCountDown(countDownTicks),
                           titleExtents(0, 0), numChars(numCharsInValue), valueWidth(0), countDownTicks(countDownTicks),
                           baseline(0), titleText{} {

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

    if(item->isChanged(dashboard->getDisplayNumber())) updateCountDown = countDownTicks;

    if(updateCountDown > 0) --updateCountDown;
    return item->isChanged(dashboard->getDisplayNumber()) || updateCountDown != 0;
}

void DashMenuItem::paintTitle(DeviceDrawable* drawableRoot) {
    color_t palette[2];
    palette[0] = parameters->getTitleBgColor(item, item->isChanged(dashboard->getDisplayNumber()));
    palette[1] = parameters->getTitleFgColor(item, item->isChanged(dashboard->getDisplayNumber()));
    DeviceDrawableHelper wrapper(drawableRoot);
    wrapper.setFont(parameters->getFontMode());

    titleExtents = wrapper.textExtents(titleText, &baseline);
    valueWidth = wrapper.textExtents("0", &baseline).x * numChars;
    valueWidth = int(valueWidth * 1.20);

    Coord requiredSize(titleExtents.x + valueWidth + 10, titleExtents.y);
    wrapper.reConfigure(palette, 2, screenLoc, requiredSize);
    if(!parameters->isTitleDrawn()) return;

    auto startX = (parameters->isTitleLeftAlign()) ? 0 : valueWidth + 1;
    wrapper.getDrawable()->setDrawColor(palette[0]);
    wrapper.getDrawable()->drawBox(wrapper.offsetLocation(screenLoc), requiredSize, true);

    const Coord &position = wrapper.offsetLocation(screenLoc, startX, 0);
    wrapper.drawText(position, palette[1], titleText);
    wrapper.endDraw();
}

void DashMenuItem::paintItem(DeviceDrawable* drawableRoot) {
    item->setChanged(false);
    char sz[20];
    color_t palette[2];
    palette[0] = parameters->getBgColor(item, item->isChanged(dashboard->getDisplayNumber()));
    palette[1] = parameters->getFgColor(item, item->isChanged(dashboard->getDisplayNumber()));
    DeviceDrawableHelper wrapper(drawableRoot, palette, 2, screenLoc, Coord(valueWidth, titleExtents.y));

    copyMenuItemValue(item, sz, sizeof(sz));
    wrapper.getDrawable()->setDrawColor(palette[0]);
    wrapper.getDrawable()->drawBox(wrapper.offsetLocation(screenLoc), Coord(valueWidth, titleExtents.y), true);

    auto padding = 0;
    wrapper.setFont(parameters->getFontMode());
    Coord valueLen = wrapper.textExtents(sz, &baseline);
    if(!parameters->isValueLeftAlign()) {
        padding = valueWidth - (valueLen.x + 4);
    }
    wrapper.drawText(wrapper.offsetLocation(screenLoc, padding, 0), palette[1], sz);
    wrapper.endDraw();
}
