/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "TcMenuGxEPDeInk.h"

void TcMenuGxEPDeInk::internalDrawText(const Coord &where, const void *font, int mag, const char *text) {
    display.setTextWrap(false);
    int baseline=0;
    Coord exts = textExtents(font, mag, "(;y", &baseline);
    auto yCursor = font ? (where.y + (exts.y - baseline)) : where.y;
    display.setCursor(where.x, static_cast<int16_t>(yCursor));
    display.setTextColor(drawColor);
    display.print(text);
}

void TcMenuGxEPDeInk::drawBitmap(const Coord &where, const DrawableIcon *icon, bool selected) {
    if(icon->getIconType() == DrawableIcon::ICON_XBITMAP) {
        display.drawXBitmap(where.x, where.y, icon->getIcon(selected), icon->getDimensions().x, icon->getDimensions().y, drawColor);
    } else if(icon->getIconType() == DrawableIcon::ICON_MONO) {
        display.drawBitmap(where.x, where.y, icon->getIcon(selected), icon->getDimensions().x, icon->getDimensions().y, drawColor, backgroundColor);
    }
}

void TcMenuGxEPDeInk::drawXBitmap(const Coord &where, const Coord &size, const uint8_t *data) {
    display.fillRect(where.x, where.y, size.x, size.y, backgroundColor);
    display.drawXBitmap(where.x, where.y, data, size.x, size.y, drawColor);
}

void TcMenuGxEPDeInk::drawBox(const Coord &where, const Coord &size, bool filled) {
    if(filled) {
       display.fillRect(where.x, where.y, size.x, size.y, drawColor);
    } else {
        display.drawRect(where.x, where.y, size.x, size.y, drawColor);
    }
}

void TcMenuGxEPDeInk::drawCircle(const Coord &where, int radius, bool filled) {
    if(filled) {
        display.fillCircle(where.x, where.y, radius, drawColor);
    } else {
        display.drawCircle(where.x, where.y, radius, drawColor);
    }
}

void TcMenuGxEPDeInk::drawPolygon(const Coord *points, int numPoints, bool filled) {
    if(numPoints == 2) {
        display.drawLine(points[0].x, points[0].y, points[1].x, points[1].y, drawColor);
    }
    else if(numPoints == 3) {
        if(filled) {
            display.fillTriangle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, drawColor);
        }
        else {
            display.drawTriangle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, drawColor);
        }
    }

}

void TcMenuGxEPDeInk::computeBaselineIfNeeded(const GFXfont* font) {
    // we cache the last baseline, if the font is unchanged, don't calculate again
    if(computedFont == font && computedBaseline > 0) return;

    // we need to work out the biggest glyph and maximum extent beyond the baseline, we use 4 chars 'Agj(' for this
    const char sz[] = "Agj(";
    int height = 0;
    int bl = 0;
    const char* current = sz;
    auto fontLast = pgm_read_word(&font->last);
    auto fontFirst = pgm_read_word(&font->first);
    while(*current && (*current < fontLast)) {
        size_t glIdx = *current - fontFirst;
        auto allGlyphs = (GFXglyph*)pgm_read_ptr(&font->glyph);
        int glyphHeight = int(pgm_read_byte(&allGlyphs[glIdx].height));
        if (glyphHeight > height) height = glyphHeight;
        auto yOffset = int8_t(pgm_read_byte(&allGlyphs[glIdx].yOffset));
        bl += glyphHeight + yOffset;
        current++;
    }
    computedFont = font;
    computedBaseline = bl / 4;
    computedHeight = height;
}

void TcMenuGxEPDeInk::transaction(bool isStarting, bool redrawNeeded) {
    if (isStarting && redrawNeeded) {
        display.setFullWindow();
        if (!firstPageDone) {
            firstPageDone = true;
            display.firstPage();
        }
    } else if (redrawNeeded) {
        display.nextPage();
    }
}

Coord TcMenuGxEPDeInk::internalTextExtents(const void *font, int mag, const char *text, int *baseline) {
    auto f = static_cast<const GFXfont *>(font);
    display.setFont(f);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds((char*)text, 3, font?30:2, &x1, &y1, &w, &h);

    if(font == nullptr) {
        // for the default font, the starting offset is 0, and we calculate the height.
        if(baseline) *baseline = 0;
        return Coord(w, h);
    }
    else {
        computeBaselineIfNeeded(f);
        if(baseline) *baseline = (computedBaseline * mag);
        return Coord(int(w), (computedHeight * mag));
    }
}

void TcMenuGxEPDeInk::drawPixel(uint16_t x, uint16_t y) {
    display.drawPixel(static_cast<int16_t>(x), static_cast<int16_t>(y), drawColor);
}

UnicodeFontHandler * TcMenuGxEPDeInk::createFontHandler() {
    return new UnicodeFontHandler(new TcMenuGxEPDFontPlotter(display), ENCMODE_UTF8);
}

void TcMenuGxEPDFontPlotter::drawPixel(uint16_t x, uint16_t y, uint32_t color) {
    display.drawPixel(static_cast<int16_t>(x), static_cast<int16_t>(y), color);
}

void TcMenuGxEPDFontPlotter::setCursor(const Coord &where) {
    display.setCursor(where.x, where.y);
}

Coord TcMenuGxEPDFontPlotter::getCursor() {
    return Coord(display.getCursorX(), display.getCursorY());
}

Coord TcMenuGxEPDFontPlotter::getDimensions() {
    return Coord(display.width(), display.height());
}
