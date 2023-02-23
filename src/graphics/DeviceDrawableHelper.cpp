
#include "DeviceDrawableHelper.h"

using namespace tcgfx;

DeviceDrawableHelper::DeviceDrawableHelper(DeviceDrawable* root, color_t* palette, uint8_t paletteSize, const Coord& startPosition, const Coord& size) {
    isSubDevice = false;
    rootDrawable = root;
    drawable = root;
    reConfigure(palette, paletteSize, startPosition, size);
}

void DeviceDrawableHelper::reConfigure(color_t *palette, uint8_t paletteSize, const Coord &startPosition, const Coord &size) {
    isSubDevice = false;
    if (rootDrawable->getSubDeviceType() != tcgfx::DeviceDrawable::NO_SUB_DEVICE) {
        auto subDrawable = rootDrawable->getSubDeviceFor(startPosition, size, palette, paletteSize);
        if (subDrawable) {
            isSubDevice = true;
            drawable = subDrawable;
            drawable->startDraw();
            startPos = startPosition;
        } else {
            drawable = rootDrawable;
        }
    }
}

Coord DeviceDrawableHelper::offsetLocation(const Coord &source, int xOffs, int yOffs) const {
    // force within screen, do not allow negative values
    if(xOffs < 0) xOffs = 0;
    if(yOffs < 0) yOffs = 0;

    return isSubDevice ? Coord((source.x + xOffs) - startPos.x, (source.y + yOffs) - startPos.y) : Coord(
            source.x + xOffs, source.y + yOffs);
}

Coord DeviceDrawableHelper::textExtents(const char* what, int* bl) {
    if(fontMode.isTcUnicode()) {
        UnicodeFontHandler *unicodeHandler = drawable->getUnicodeHandler(false);
        if(unicodeHandler == nullptr) {
            serlogF(SER_ERROR, "Bad font mode");
            return {0,0};
        }
        fontMode.setFontTcUnicode(unicodeHandler);
        return unicodeHandler->textExtents(what, bl);
    } else {
        auto nativeFont = fontMode.getNativeDesc();
        return drawable->textExtents(nativeFont.getPtr(), nativeFont.getMag(), what, bl);
    }
}

void DeviceDrawableHelper::drawText(const Coord& where, color_t drawColor, const char* what) {
    if(fontMode.isTcUnicode()) {
        UnicodeFontHandler *unicodeHandler = drawable->getUnicodeHandler(false);
        if(unicodeHandler == nullptr) {
            serlogF(SER_ERROR, "Bad font mode");
            return;
        }
        fontMode.setFontTcUnicode(unicodeHandler);
        unicodeHandler->setDrawColor(drawable->getUnderlyingColor(drawColor));
        unicodeHandler->setCursor(Coord(where.x, where.y + (unicodeHandler->getYAdvance() - unicodeHandler->getBaseline())));
        unicodeHandler->print(what);
    } else {
        drawable->setDrawColor(drawColor);
        auto font = fontMode.getNativeDesc();
        // we know upfront we are rendering a native font, skip direct to that for performance.
        drawable->internalDrawText(where, font.getPtr(), font.getMag(), what);
    }
}

DeviceDrawableHelper::DeviceDrawableHelper(DeviceDrawable *root) {
    rootDrawable = root;
    drawable = root;
    isSubDevice = false;
    startPos = { 0 , 0 };
}

void DeviceDrawableHelper::setFontFromParameters(const void* font, uint8_t mag) {
    if(getDrawable()->isTcUnicodeEnabled()) {
        if(mag == 0) {
            setFont(DeviceFontDrawingMode((const UnicodeFont*) font));
        } else {
            setFont(DeviceFontDrawingMode((const GFXfont*) font));
        }
    } else {
        setFont(DeviceFontDrawingMode(NativeFontDesc(font, mag)));
    }
}
