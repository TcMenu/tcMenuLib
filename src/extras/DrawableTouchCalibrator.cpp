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
#define TOUCH_WHITE RGB(255, 255, 255)
#define TOUCH_ORANGE RGB(255, 69, 0)
#define TOUCH_YELLOW RGB(255, 255, 0)

// comment / uncomment to debug the touch positions
#define NEED_XYZ_DEBUG_PRINT

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
    if(touchState == NOT_TOUCHED && !needsRedrawing) return;
    needsRedrawing = false;

    drawable->startDraw();
    drawable->setDrawColor(TOUCH_BLACK);
    drawable->drawCircle(Coord(oldX, oldY), 10, true);

    Coord dims = drawable->getDisplayDimensions();
    drawable->drawBox(Coord(0, 100), Coord(dims.x, 45), true);
    drawable->setColors(TOUCH_WHITE, TOUCH_BLACK);
    char sz[50];
    strncpy_P(sz, TXT_TOUCH_CONFIGURE, sizeof(sz));
    auto titleConfig = renderer->getGraphicsPropertiesFactory().configFor(nullptr, tcgfx::ItemDisplayProperties::COMPTYPE_TITLE);
    drawable->drawText(Coord(0, 100), titleConfig->getFont(), titleConfig->getFontMagnification(), sz);

#ifdef NEED_XYZ_DEBUG_PRINT
    auto itemConfig = renderer->getGraphicsPropertiesFactory().configFor(nullptr, tcgfx::ItemDisplayProperties::COMPTYPE_ITEM);
    strcpy(sz, "x: ");
    fastftoa(sz, touchScreen->getLastX(), 3, sizeof sz);
    strcat(sz, ", y: ");
    fastftoa(sz, touchScreen->getLastY(), 3, sizeof sz);
    strcat(sz, ", z: ");
    fastltoa(sz, touchState, 1, NOT_PADDED, sizeof sz);
    drawable->setDrawColor(TOUCH_BLACK);
    drawable->drawBox(Coord(0, 140), Coord(dims.x, 70), true);
    drawable->setColors(TOUCH_YELLOW, TOUCH_BLACK);
    drawable->drawText(Coord(0, 140), itemConfig->getFont(), 0, sz);

    drawable->setDrawColor(TOUCH_BLACK);
    drawable->setColors(TOUCH_YELLOW, TOUCH_BLACK);

    strcpy(sz, "sx:");
    fastftoa(sz, calibrationHandler.getMinX(), 3, sizeof sz);
    strcat(sz, ", ex:");
    fastftoa(sz, calibrationHandler.getMaxX(), 3, sizeof sz);
    drawable->drawText(Coord(0, 160), itemConfig->getFont(), 0, sz);

    strcpy(sz, ", sy:");
    fastftoa(sz, calibrationHandler.getMinY(), 3, sizeof sz);
    strcat(sz, ", ey:");
    fastftoa(sz, calibrationHandler.getMaxY(), 3, sizeof sz);
    drawable->drawText(Coord(0, 180), itemConfig->getFont(), 0, sz);
#endif

    int buttonSize = 75;
    int16_t rightCorner = dims.x - buttonSize;

    drawable->setDrawColor(TOUCH_YELLOW);
    drawable->drawBox(Coord(0, 0), Coord(buttonSize, buttonSize), true);
    drawable->drawBox(Coord(rightCorner, 0), Coord(buttonSize, buttonSize), true);
    drawable->drawBox(Coord(rightCorner, dims.y - buttonSize), Coord(buttonSize, buttonSize), true);
    drawable->setDrawColor(TOUCH_ORANGE);
    drawable->drawBox(Coord(0, dims.y - buttonSize), Coord(buttonSize, buttonSize), true);
    drawable->setDrawColor(TOUCH_WHITE);
    drawable->drawText(Coord(0, dims.y - (buttonSize - 6)), titleConfig->getFont(), titleConfig->getFontMagnification(), "[X]");

    oldX = int(touchScreen->getLastX() * (float) dims.x);
    oldY = int(touchScreen->getLastY() * (float) dims.y);
    drawable->setDrawColor(TOUCH_ORANGE);
    drawable->drawCircle(Coord(oldX, oldY), 10, true);
    drawable->endDraw();

    // the saved values must ALWAYS be in the original format before any rotations or inverting.
    auto orientation = touchScreen->getOrientation();
    auto lastX = orientation.isXInverted() ? (1 - touchScreen->getLastX()) : touchScreen->getLastX();
    auto lastY = orientation.isYInverted() ? (1 - touchScreen->getLastY()) : touchScreen->getLastY();

    if(oldY < buttonSize) {
        // top
        if(oldX < buttonSize) {
            calibrationHandler.setXPosition(lastX, orientation.isXInverted());
            calibrationHandler.setYPosition(lastY, orientation.isYInverted());
        } else if(oldX > rightCorner) {
            calibrationHandler.setXPosition(lastX, !orientation.isXInverted());
            calibrationHandler.setYPosition(lastY, orientation.isYInverted());
        }
    } else if(oldY > (dims.y - buttonSize)) {
        // bottom
        if(oldX < buttonSize) {
            // bottom left - exit
            saveCalibration();
            giveItBack();
        }
        else if(oldX > rightCorner) {
            calibrationHandler.setYPosition(lastY, !orientation.isYInverted());
        }
    }
    
}

void IoaTouchScreenCalibrator::giveItBack() {
    serlogF(SER_TCMENU_INFO, "Calibrator UI finished");
    renderer->giveBackDisplay();
    renderer->setCustomDrawingHandler(lastDrawing);
    touchScreen->setCalibration(calibrationHandler);
    touchScreen->enableCalibration(true);
    lastDrawing = nullptr;
    if(screenPrep != nullptr) screenPrep(false);
}

void IoaTouchScreenCalibrator::reCalibrateNow() {
    if(screenPrep != nullptr) screenPrep(true);
    needsRedrawing = true;
    lastDrawing = renderer->getCurrentCustomDrawing();
    touchScreen->enableCalibration(false);
    renderer->setCustomDrawingHandler(this);
    calibrationHandler.setCalibrationValues(0.0F, 1.0F, 0.0F, 1.0F);
    renderer->takeOverDisplay();
    serlogF(SER_TCMENU_INFO, "Calibrator UI starting");

}

float fltFromU32(uint32_t val) {
    return float(val) / 10000000.0F;
}

uint32_t fltToU32(float val) {
    return uint32_t(val * 10000000.0F);
}

void IoaTouchScreenCalibrator::saveCalibration() {
    EepromAbstraction *rom = menuMgr.getEepromAbstraction();
    if(!rom) {
        serlogF(SER_ERROR, "Calibration - No ROM defined");
    }

    serlogF2(SER_TCMENU_INFO, "Saving IOA touch calibration at ", romPos);
    rom->write16(romPos, calibrationMagicIoa);
    rom->write32(romPos + 2, fltToU32(calibrationHandler.getMinX()));
    rom->write32(romPos + 6, fltToU32(calibrationHandler.getMaxX()));
    rom->write32(romPos + 10, fltToU32(calibrationHandler.getMinY()));
    rom->write32(romPos + 14, fltToU32(calibrationHandler.getMaxY()));
    serlogF3(SER_TCMENU_INFO, "XMin/Max", calibrationHandler.getMinX(), calibrationHandler.getMaxX())
    serlogF3(SER_TCMENU_INFO, "YMin/Max", calibrationHandler.getMinY(), calibrationHandler.getMaxY())
    serlogF2(SER_TCMENU_INFO, "Saved touch data ", rom->hasErrorOccurred());
}

bool IoaTouchScreenCalibrator::loadCalibration() {
    serlogF2(SER_TCMENU_INFO, "Loading IOA touch calibration at ", romPos);
    EepromAbstraction *rom = menuMgr.getEepromAbstraction();
    if(!rom || rom->read16(romPos) != calibrationMagicIoa) {
        serlogF(SER_TCMENU_INFO, "Calibrator ROM mismatch");
        return false;
    }
    calibrationHandler.setCalibrationValues(
            fltFromU32(rom->read32(romPos + 2)), fltFromU32(rom->read32(romPos + 6)),
            fltFromU32(rom->read32(romPos + 10)), fltFromU32(rom->read32(romPos + 14)));
    if(rom->hasErrorOccurred()) {
        serlogF(SER_ERROR, "Calibrator ROM error");
        return false;
    }

    serlogF3(SER_TCMENU_INFO, "XMin/Max", calibrationHandler.getMinX(), calibrationHandler.getMaxX())
    serlogF3(SER_TCMENU_INFO, "YMin/Max", calibrationHandler.getMinY(), calibrationHandler.getMaxY())

    touchScreen->setCalibration(calibrationHandler);
    touchScreen->enableCalibration(true);

    serlogF(SER_TCMENU_INFO, "Loaded calibration OK");
    return true;
}
