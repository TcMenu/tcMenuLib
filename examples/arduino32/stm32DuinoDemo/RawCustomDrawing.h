
#ifndef TCLIBRARYDEV_RAWCUSTOMDRAWING_H
#define TCLIBRARYDEV_RAWCUSTOMDRAWING_H

#include "generated/stm32DuinoDemo_menu.h"

// Here we implement the custom drawing capability of the renderer, it allows us to receive reset and custom drawing
// requests. More info in the link below:
//  https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/renderer-take-over-display/
//
class MyCustomDrawing : public CustomDrawing {
private:
    GraphicsDeviceRenderer& dev;
    int ticks;
public:
    MyCustomDrawing(GraphicsDeviceRenderer& r) : dev(r), ticks(0) {}

    void registerWithRenderer() {
        dev.setCustomDrawingHandler(this);
    }

    void started(BaseMenuRenderer *currentRenderer) override {
        // called once when the take-over display  is started before calling renderLoop so you can set things up.
        switches.getEncoder()->changePrecision(100, 50);
    }

    void reset() override {
        renderer.takeOverDisplay();
    }

    void renderLoop(unsigned int currentValue, RenderPressMode userClick) override {
        // called in a game loop between takeOverDisplay and giveBackDisplay, at this point you renderer the display.
        if(userClick == RPRESS_PRESSED) {
            dev.giveBackDisplay();
        }
        else if(++ticks % 10 == 1) {
            // Why write your own code using device drawable? The main reason is, that it works exactly the same over
            // adafruit, u8g2 and TFTeSPI with a moderately complete API.
            DeviceDrawable *dd = dev.getDeviceDrawable();
            dd->startDraw();
            const Coord &dims = dd->getDisplayDimensions();
            dd->setDrawColor(BLACK);
            dd->drawBox(Coord(0, 0), dims, true);
            dd->setColors(WHITE, BLACK);
            auto height = int(dims.y) - 16;
            int width = int(dims.x) - 20;
            dd->drawText(Coord(rand() % width, (rand() % height) + 10), nullptr, 1, "hello");
            dd->drawText(Coord(rand() % width, (rand() % height) + 10), nullptr, 1, "world");
            char sz[10];
            ltoaClrBuff(sz, currentValue, 4, NOT_PADDED, sizeof sz);
            dd->drawText(Coord(0, 0), nullptr, 1, sz);
            dd->endDraw();
        }
    }
} myCustomDrawing(renderer);


#endif //TCLIBRARYDEV_RAWCUSTOMDRAWING_H
