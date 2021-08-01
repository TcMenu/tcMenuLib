//
// Created by David Cherry on 23/01/2021.
//

#ifndef TCMENU_ESP_AMPLIFIER_TOUCH_CALIBRATOR_H
#define TCMENU_ESP_AMPLIFIER_TOUCH_CALIBRATOR_H

#include <Arduino.h>
#include "esp32Amplifier_menu.h"
#include <graphics/BaseGraphicalRenderer.h>
#include <graphics/MenuTouchScreenEncoder.h>

using namespace tcgfx;

/**
 * This class implements custom drawing so that it can take over the display and render a touch screen calibration
 * UI, no expense spared here :). Touch and hold in the bottom corner of your display to clear it.
 */
class TouchScreenCalibrator : public CustomDrawing {
private:
    DeviceDrawable* drawable;
    MenuTouchScreenManager* touchScreen;
    int oldX= 0, oldY=0;
    unsigned int takeOverCount=0;
public:
    explicit TouchScreenCalibrator(MenuTouchScreenManager *touchScreen)
            : drawable(nullptr), touchScreen(touchScreen) {}

    void reset() override {
        renderer.takeOverDisplay();
    }

    void started(BaseMenuRenderer *currentRenderer) override {
        if(renderer.getRendererType() != RENDER_TYPE_CONFIGURABLE) {
            renderer.giveBackDisplay(); // we cannot work with non configurable rendering
        }
        drawable = reinterpret_cast<GraphicsDeviceRenderer*>(currentRenderer)->getDeviceDrawable();
        drawable->startDraw();
        drawable->setDrawColor(TFT_BLACK);
        drawable->drawBox(Coord(0,0), drawable->getDisplayDimensions(), true);
        drawable->endDraw();
        takeOverCount = 0;
    }

    void renderLoop(unsigned int currentValue, RenderPressMode userClick) override {

        // If the dialog is in use then we leave this loop immediately to give it priority.
        // As of 2.1 onwards this behaviour is up to you, you can choose to have higher priority than dialogs.
        if(renderer.getDialog() != nullptr && renderer.getDialog()->isInUse()) {
            renderer.giveBackDisplay();
            return;
        }

        drawable->startDraw();
        drawable->setDrawColor(TFT_BLACK);
        drawable->drawCircle(Coord(oldX, oldY), 10, true);

        drawable->drawBox(Coord(0,0), Coord(drawable->getDisplayDimensions().x, 45), true);
        drawable->setColors(TFT_ORANGE, TFT_BLACK);
        drawable->drawText(Coord(0, 2), nullptr, 0, "Calibrate screen");

        char sz[40];
        strcpy(sz, "x: ");
        fastftoa(sz, touchScreen->getLastX(), 3, sizeof sz);
        strcat(sz, ", y: ");
        fastftoa(sz, touchScreen->getLastY(), 3, sizeof sz);
        strcat(sz, ", z: ");
        fastltoa(sz, touchScreen->getLastTouchState(), 1, NOT_PADDED, sizeof sz);

        drawable->setColors(TFT_YELLOW, TFT_BLACK);
        drawable->drawText(Coord(0, 22), nullptr, 0, sz);

        oldX = int(touchScreen->getLastX() * (float)drawable->getDisplayDimensions().x);
        oldY = int(touchScreen->getLastY() * (float)drawable->getDisplayDimensions().y);
        drawable->setDrawColor(TFT_YELLOW);
        drawable->drawCircle(Coord(oldX, oldY), 10, true);

        if(oldX < 40 && oldY < 40 && touchScreen->getLastTouchState() == iotouch::HELD) {
            renderer.giveBackDisplay();
        }
    }
};

#endif //TCMENU_ESP_AMPLIFIER_TOUCH_CALIBRATOR_H
