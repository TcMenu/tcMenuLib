/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "TcDrawableButton.h"

#define GREYED_OUT_COLOR RGB(162, 162, 162)

using namespace tcgfx;

TcDrawableButton::TcDrawableButton(const Coord& where, const Coord& size, color_t bgCol, color_t fgCol, color_t selCol,
                                   const char* text, const DeviceFontDrawingMode& fontMode) : where(where), size(size),
                                   bgColor(bgCol), fgColor(fgCol), selectedColor(selCol), text(text), fontMode(fontMode) {
    flags = 0;
    bitWrite(flags, DRAW_BUTTON_FLAG_ICON_BOOL, false);
    setDirty(true);
}

TcDrawableButton::TcDrawableButton(const char *text, const DeviceFontDrawingMode& font)
        : where(), size(), bgColor(0), fgColor(0), selectedColor(0), text(text), fontMode(font) {
    flags = 0;
    bitWrite(flags, DRAW_BUTTON_FLAG_ICON_BOOL, false);
    setDirty(true);
}


TcDrawableButton::TcDrawableButton(const Coord& where, const Coord& size, color_t bgCol, color_t fgCol, color_t selCol,
                                   const DrawableIcon* icon) : where(where), size(size), bgColor(bgCol),
                                   fgColor(fgCol), selectedColor(selCol), icon(icon) {
    flags = 0;
    bitWrite(flags, DRAW_BUTTON_FLAG_ICON_BOOL, true);
    setDirty(true);
}

TcDrawableButton::TcDrawableButton(const DrawableIcon *icon) : where(), size(), bgColor(0),
                                                                                   fgColor(0), selectedColor(0), icon(icon) {
    flags = 0;
    bitWrite(flags, DRAW_BUTTON_FLAG_ICON_BOOL, true);
    setDirty(true);
}


bool TcDrawableButton::touchInBounds(const Coord& location) const {
    if(drawingMode == NOT_SELECTABLE) return false; // can't select in this mode

    return (location.x > where.x && location.x < where.x + size.x
        && location.y > where.y && location.y < where.y + size.y);
}

void TcDrawableButton::paintButton(DeviceDrawable* dr) {
    if(!isDirty()) return;
    setDirty(false);

    color_t palette[4] = { bgColor, fgColor, selectedColor, GREYED_OUT_COLOR };

    DeviceDrawableHelper helper(dr, palette, 4, where, size);

    color_t bgCol;
    color_t fgCol;

    if(isButtonOnMonoDisplay()) {
        fgCol = drawingMode == SELECTED || drawingMode == PRESSED ? bgColor : fgColor;
        bgCol = drawingMode == SELECTED || drawingMode == PRESSED ? fgColor : bgColor;
    } else {
        fgCol = drawingMode == NOT_SELECTABLE ? GREYED_OUT_COLOR : fgColor;
        bgCol = drawingMode == NORMAL || drawingMode == NOT_SELECTABLE ? bgColor : selectedColor;
    }

    helper.getDrawable()->setDrawColor(bgCol);
    helper.getDrawable()->drawBox(helper.offsetLocation(where), size, true);

    if(isButtonOnMonoDisplay() && drawingMode == NOT_SELECTABLE) {
        helper.endDraw();
        return;
    }


    helper.getDrawable()->drawBox(where, size, true);
    if(isIconDrawn()) {
        // drawing an icon
        helper.getDrawable()->setColors(fgCol, bgCol);
        int16_t xOffset = (size.x - icon->getDimensions().x) / 2;
        int16_t yOffset = (size.y - icon->getDimensions().y) / 2;
        helper.getDrawable()->drawBitmap(helper.offsetLocation(where, xOffset, yOffset), icon, drawingMode != NORMAL);
    } else if(text != nullptr){
        // drawing text
        helper.getDrawable()->setDrawColor(drawingMode != NORMAL ? bgColor : selectedColor);
        int bl;
        helper.setFont(fontMode);
        Coord textSize = helper.textExtents(text, &bl);
        int16_t xOffset = (size.x - textSize.x) / 2;
        int16_t yOffset = (size.y - textSize.y) / 2;
        helper.drawText(helper.offsetLocation(where, xOffset, yOffset), fgCol, text);
    }
    helper.endDraw();
}

void TcDrawableButton::setText(const char *newText) {
    if(!isIconDrawn()) {
        text = newText;
    }
    setDirty(true);
}

void TcDrawableButton::setColors(color_t bgCol, color_t fgCol, color_t selCol) {
    bgColor = bgCol;
    fgColor = fgCol;
    selectedColor = selCol;
    setDirty(true);
}

void TcDrawableButton::setPositionAndSize(const Coord &position, const Coord &newSize) {
    where = position;
    size = newSize;
    setDirty(true);
}

void TcDrawableButton::setPosition(const Coord &position) {
    where = position;
    setDirty(true);
}

