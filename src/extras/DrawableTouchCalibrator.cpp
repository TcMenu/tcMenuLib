/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "DrawableTouchCalibrator.h"
#include <BaseDialog.h>
#include "../lang/language_select.h"

using namespace tcextras;
using namespace tcgfx;
using namespace iotouch;

#define TOUCH_BLACK RGB(0,0,0)
#define TOUCH_ORANGE RGB(255, 69, 0)
#define TOUCH_YELLOW RGB(255, 255, 0)

void IoaTouchScreenCalibrator::started(BaseMenuRenderer *currentRenderer) {
    if (currentRenderer->getRendererType() != RENDER_TYPE_CONFIGURABLE) {
        giveItBack();
    }
    drawable = renderer->getDeviceDrawable();
    drawable->startDraw();
    drawable->setDrawColor(TOUCH_BLACK);
    drawable->drawBox(Coord(0, 0), drawable->getDisplayDimensions(), true);
    drawable->endDraw();
}

void IoaTouchScreenCalibrator::renderLoop(unsigned int currentValue, RenderPressMode userClick) {

    iotouch::TouchState touchState = touchScreen->getLastTouchState();
    if(touchState != iotouch::TOUCHED && touchState != iotouch::HELD) return;

    drawable->startDraw();
    drawable->setDrawColor(TOUCH_BLACK);
    drawable->drawCircle(Coord(oldX, oldY), 10, true);

    Coord dims = drawable->getDisplayDimensions();
    drawable->drawBox(Coord(0, 50), Coord(dims.x, 45), true);
    drawable->setColors(TOUCH_ORANGE, TOUCH_BLACK);
    char sz[40];
    strncpy_P(sz, TXT_TOUCH_CONFIGURE, sizeof(sz));
    auto titleConfig = renderer->getGraphicsPropertiesFactory().configFor(nullptr, tcgfx::ItemDisplayProperties::COMPTYPE_TITLE);
    drawable->drawText(Coord(0, 45), titleConfig->getFont(), titleConfig->getFontMagnification(), sz);

    strcpy(sz, "x: ");
    fastftoa(sz, touchScreen->getLastX(), 3, sizeof sz);
    strcat(sz, ", y: ");
    fastftoa(sz, touchScreen->getLastY(), 3, sizeof sz);
    strcat(sz, ", z: ");
    fastltoa(sz, touchState, 1, NOT_PADDED, sizeof sz);

    int buttonSize = 40;
    int16_t rightCorner = dims.x - buttonSize;

    drawable->setDrawColor(TOUCH_YELLOW);
    drawable->drawBox(Coord(0, 0), Coord(buttonSize, buttonSize), true);
    drawable->drawBox(Coord(dims.x - buttonSize, 0), Coord(buttonSize, buttonSize), true);
    drawable->drawBox(Coord(dims.x - buttonSize, rightCorner), Coord(buttonSize, buttonSize), true);
    drawable->setDrawColor(TOUCH_ORANGE);
    drawable->drawBox(Coord(0, rightCorner), Coord(buttonSize, buttonSize), true);
    drawable->drawText(Coord(0, dims.y - 5), titleConfig->getFont(), titleConfig->getFontMagnification(), "[X]");

    drawable->setColors(TOUCH_YELLOW, TOUCH_BLACK);
    drawable->drawText(Coord(0, 22), nullptr, 0, sz);

    oldX = int(touchScreen->getLastX() * (float) dims.x);
    oldY = int(touchScreen->getLastY() * (float) dims.y);
    drawable->setDrawColor(TOUCH_YELLOW);
    drawable->drawCircle(Coord(oldX, oldY), 10, true);
    drawable->endDraw();

    if(oldY < buttonSize) {
        // top
        if(oldX < buttonSize) {
            calibrationHandler.setCalibrationValues(touchScreen->getLastX(), calibrationHandler.getMaxX(),
                                           touchScreen->getLastY(), calibrationHandler.getMaxY());
        } else if(oldX > rightCorner) {
            calibrationHandler.setCalibrationValues(calibrationHandler.getMinX(), touchScreen->getLastX(),
                                           touchScreen->getLastY(), calibrationHandler.getMaxY());
        }

    } else if(oldY > (dims.y - buttonSize)) {
        // bottom
        if(oldX < buttonSize) {
            // bottom left - exit
            saveCalibration();
            giveItBack();
        }
        else if(oldX > rightCorner) {
            calibrationHandler.setCalibrationValues(calibrationHandler.getMinX(), touchScreen->getLastX(),
                                           calibrationHandler.getMinY(), touchScreen->getLastY());
        }
    }
    
}

void IoaTouchScreenCalibrator::giveItBack() {
    renderer->giveBackDisplay();
    renderer->setCustomDrawingHandler(lastDrawing);
    lastDrawing = nullptr;
    if(screenPrep != nullptr) screenPrep(true);
}

void IoaTouchScreenCalibrator::reCalibrateNow() {
    if(screenPrep != nullptr) screenPrep(true);
    lastDrawing = renderer->getCurrentCustomDrawing();
    renderer->setCustomDrawingHandler(this);
    renderer->takeOverDisplay();
}

float fltFromU32(uint32_t val) {
    return float(val) / 100000.0F;
}

uint32_t fltToU32(float val) {
    return uint32_t(val * 100000.0F);
}

void IoaTouchScreenCalibrator::saveCalibration() {
    serlogF2(SER_TCMENU_INFO, "Saving IOA touch calibration at ", romPos);
    EepromAbstraction *rom = menuMgr.getEepromAbstraction();
    rom->write16(romPos, calibrationMagicIoa);
    rom->write32(romPos + 2, fltToU32(calibrationHandler.getMinX()));
    rom->write32(romPos + 6, fltToU32(calibrationHandler.getMaxX()));
    rom->write32(romPos + 10, fltToU32(calibrationHandler.getMinY()));
    rom->write32(romPos + 14, fltToU32(calibrationHandler.getMaxY()));
    serlogF2(SER_TCMENU_INFO, "Saved touch data ", rom->hasErrorOccurred());
}

bool IoaTouchScreenCalibrator::loadCalibration() {
    serlogF2(SER_TCMENU_INFO, "Loading IOA touch calibration at ", romPos);
    EepromAbstraction *rom = menuMgr.getEepromAbstraction();
    if(rom->read16(romPos) != calibrationMagicIoa) return false;
    calibrationHandler.setCalibrationValues(
            fltFromU32(rom->read32(romPos + 2)), fltFromU32(rom->read32(romPos + 6)),
            fltFromU32(rom->read32(romPos + 10)), fltFromU32(rom->read32(romPos + 14)));
    if(rom->hasErrorOccurred()) return false;

    serlogF(SER_TCMENU_INFO, "Loaded calibration OK");
    return true;
}
