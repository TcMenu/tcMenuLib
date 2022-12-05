#include <BaseRenderers.h>
#include "DrawableDashboard.h"

void
DrawableDashboard::addDrawingItem(MenuItem *theItem, Coord topLeft, DashDrawParameters *params, int numCharsInValue,
                                  const char *titleOverrideText = nullptr) {
    drawingItems.add(DashMenuItem(theItem, topLeft, params, numCharsInValue, titleOverrideText));
    serdebugF2("added item to list #", drawingItems.count());
}

void DrawableDashboard::stop() {
    running = false;
    renderer->giveBackDisplay();
}

void DrawableDashboard::reset() {
    if (drawingMode == DASH_ON_RESET_CLICK_EXIT || drawingMode == DASH_ON_RESET_MANUAL_EXIT) {
        if (!running) renderer->takeOverDisplay();
    }
}

void DrawableDashboard::started(BaseMenuRenderer *currentRenderer) {
    renderer = currentRenderer;
    myGfx->setTextSize(1);
    myGfx->fillScreen(0);
    serdebugF2("drawing titles #", drawingItems.count())

    for (int i = 0; i < drawingItems.count(); i++) {
        auto drawing = drawingItems.itemAtIndex(i);
        serdebugF("drawing title")

        drawing->paintTitle();
    }
    serdebugF("started2")
}

void DrawableDashboard::renderLoop(unsigned int currentValue, RenderPressMode userClick) {
    if (userClick != RPRESS_NONE && (drawingMode == DASH_ON_RESET_CLICK_EXIT || drawingMode == DASH_MANUAL_START_CLICK_EXIT)) {
        stop();
        return;
    }

    for (int i = 0; i < drawingItems.count(); i++) {
        auto drawing = drawingItems.itemAtIndex(i);
        if (drawing->needsPainting()) {
            drawing->paintItem(myGfx, canvasDrawable, canvas);
        }
    }
}

DashDrawParameters::DashDrawParameters(color_t fgColor_, color_t bgColor_, const GFXfont *font_,
                                       DashAlign align = TITLE_RIGHT_VALUE_RIGHT) {
    alignment = align;
    adaFont = font_;
    isAdaFont = true;
    fgColor = fgColor_;
    bgColor = bgColor_;
}

DashDrawParameters::DashDrawParameters(color_t fgColor_, color_t bgColor_, const UnicodeFont *font_,
                                       DashAlign align = TITLE_RIGHT_VALUE_RIGHT) {
    alignment = align;
    uniFont = font_;
    isAdaFont = false;
    fgColor = fgColor_;
    bgColor = bgColor_;
}

int DashDrawParametersIntUpdateRange::findIndexForChoice(MenuItem* item) {
    if(isMenuBasedOnValueItem(item)) {
        auto val = reinterpret_cast<ValueMenuItem*>(item)->getCurrentValue();
        for(int i=0;i<numOfRanges;i++) {
            if(val >= colorRanges[i].minValue && val <= colorRanges[i].maxValue) {
                return i;
            }
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

DashMenuItem::DashMenuItem(MenuItem *theItem, Coord topLeft, DashDrawParameters* params, int numCharsInValue, const char* titleOverride, int countDownTicks)
        : item(theItem) screenLoc(topLeft), updateCountDown(countDownTicks), numChars(numCharsInValue), valueWidth(0), titleExtents(0, 0), countDownTicks(countDownTicks) {

    if(titleOverride) {
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

void DashMenuItem::setFont(DashDrawParameters* params, UnicodeFontHandler* unicodeHandler) {
    if(parameters->isAdafruitFont()) {
        unicodeHandler->setFont(parameters->getAsAdaFont());
    } else {
        unicodeHandler->setFont(parameters->getAsUnicodeFont());
    }
}

void DashMenuItem::paintTitle() {
    int baseline;
    UnicodeFontHandler* unicodeHandler = gfxDrawable.getUnicodeHandler(true);

    titleExtents = unicodeHandler->textExtents(titleText, &baseline);
    valueWidth = unicodeHandler->textExtents("0", &baseline).x * numChars;
    valueWidth = int(valueWidth * 1.20);

    if(!parameters->isTitleDrawn()) return;

    auto startX = (parameters->isTitleLeftAlign()) ? screenLoc.x : screenLoc.x + valueWidth + 1;

    if(parameters->getTitleBgColor(item, false) != ILI9341_BLACK) {
        gfxDrawable.setDrawColor(parameters->getTitleBgColor(item, false));
        gfxDrawable.drawBox(Coord(startX, screenLoc.y), Coord(titleExtents.x, titleExtents.y), true);
    }
    unicodeHandler->setDrawColor(parameters->getTitleFgColor(item, false));
    unicodeHandler->setCursor(Coord(startX, screenLoc.y + titleExtents.y));
    unicodeHandler->print(titleText);
}

void DashMenuItem::paintItem(Adafruit_GFX *myGfx, DeviceDrawable* canvasDrawable, GFXcanvas1* canvas) {
    item->setChanged(false);
    char sz[20];
    copyMenuItemValue(item, sz, sizeof(sz));
    const Coord &dims = canvasDrawable->getDisplayDimensions();
    canvasDrawable->setDrawColor(0);
    canvasDrawable->drawBox(Coord(0, 0), Coord(dims.x, dims.y), true);

    UnicodeFontHandler* unicodeHandler = canvasDrawable->getUnicodeHandler(true);
    setFont(parameters, unicodeHandler);
    auto padding = 0;
    if(!parameters->isValueLeftAlign()) {
        int baseline;
        Coord valueLen = unicodeHandler->textExtents(sz, &baseline);
        padding = valueWidth - (valueLen.x + 4);
    }
    unicodeHandler->setDrawColor(1);
    unicodeHandler->setCursor(padding, unicodeHandler->getYAdvance());
    unicodeHandler->print(sz);
    auto startX = (parameters->isTitleLeftAlign()) ? screenLoc.x + titleExtents.x + 5 : screenLoc.x;
    drawCookieCutBitmap(myGfx, startX, screenLoc.y, canvas->getBuffer(), valueWidth, titleExtents.y,
                        canvas->width(), 0, 0,parameters->getFgColor(item, updateCountDown > 1),
                        parameters->getBgColor(item, updateCountDown > 1));
}
