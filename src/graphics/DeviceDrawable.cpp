
#include "DeviceDrawable.h"
#include "GraphicsDeviceRenderer.h"
#include <tcUnicodeHelper.h>

using namespace tcgfx;

void DeviceDrawable::drawText(const Coord& where, const void* font, int mag, const char* text) {
    auto handler = getUnicodeHandler(false);
    if(handler) {
        handler->setDrawColor(drawColor);
        setTcFontAccordingToMag(handler, font, mag);
        handler->setCursor((int)where.x, (int)where.y + (handler->getYAdvance() - handler->getBaseline()));
        handler->print(text);
    } else {
        internalDrawText(where, font, mag, text);
    }
}

UnicodeFontHandler *DeviceDrawable::getUnicodeHandler(bool enableIfNeeded) {
    if(fontHandler == nullptr && enableIfNeeded) {
        fontHandler = createFontHandler();
    }
    return fontHandler; // if null, there is no font handler.
}

UnicodeFontHandler *DeviceDrawable::createFontHandler() {
    return fontHandler = new UnicodeFontHandler(new DrawableTextPlotPipeline(this), ENCMODE_UTF8);
}

Coord DeviceDrawable::textExtents(const void *font, int mag, const char *text, int *baseline) {
    auto handler = getUnicodeHandler(false);
    if(handler) {
        setTcFontAccordingToMag(handler, font, mag);
        return handler->textExtents(text, baseline, false);
    } else {
        return internalTextExtents(font, mag, text, baseline);
    }
}

void tcgfx::setTcFontAccordingToMag(UnicodeFontHandler* handler, const void* font, int mag) {
    if(mag == 0) {
        handler->setFont((UnicodeFont*) font);
    } else {
        handler->setFont((GFXfont*) font);
    }
}
