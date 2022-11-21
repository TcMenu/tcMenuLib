/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "UnicodeFontHandler.h"

using namespace tcgfx;

void tcgfx::Utf8Text::reset() {
    // clear the text buffer
    extraCharsNeeded = 0;
    decoderState = WAITING_BYTE_0;
}

void tcgfx::Utf8Text::pushChars(const char *str) {
    while (*str) {
        pushChar(*str);
        str++;
    }
}

void tcgfx::Utf8Text::pushChar(char data) {
    if (encodingMode == ENCMODE_EXT_ASCII) {
        handler(userData, data);
        return;
    }

    if (decoderState == WAITING_BYTE_0) {
        if ((data & 0x80) == 0) {
            currentUtfChar = (uint8_t) data;
            extraCharsNeeded = 0;
            decoderState = UTF_CHAR_FOUND;
        } else if ((data & 0b11100000) == 0b11000000) {
            currentUtfChar |= (data & 0x1FU);
            decoderState = WAITING_BYTE_1;
            extraCharsNeeded = 1;
        } else if ((data & 0b11110000) == 0b11100000) {
            currentUtfChar = data & 0x0FU;
            decoderState = WAITING_BYTE_1;
            extraCharsNeeded = 2;
        } else if ((data & 0b11111000) == 0b11110000) {
            currentUtfChar |= (data & 0x07U);
            decoderState = WAITING_BYTE_1;
            extraCharsNeeded = 3;
        } else {
            error();
        }
    } else if (decoderState == WAITING_BYTE_1) {
        uint16_t uni = data & 0xc0;
        if ((data & 0xc0) == 0x80) {
            currentUtfChar |= (uni << shiftAmount());
            decoderState = (extraCharsNeeded == 1) ? UTF_CHAR_FOUND : WAITING_BYTE_2;
        } else {
            error();
        }
    } else if (decoderState == WAITING_BYTE_2) {
        if ((data & 0xc0) == 0x80) {
            uint16_t uni = data & 0xc0;
            currentUtfChar = uni << 10U;
            decoderState = (extraCharsNeeded == 2) ? UTF_CHAR_FOUND : WAITING_BYTE_3;
        } else {
            error();
        }
    } else if (decoderState == WAITING_BYTE_3) {
        if ((data & 0xc0) == 0x80) {
            uint32_t uni = data & 0xc0;
            currentUtfChar = uni << 16U;
            decoderState = UTF_CHAR_FOUND;
        } else {
            error();
        }
    }

    // we only go ahead and register the character when a character was found.
    if (decoderState == UTF_CHAR_FOUND) {
        decoderState = WAITING_BYTE_0;
        handler(userData, currentUtfChar);
    }
}

void tcgfx::Utf8Text::error() {
    decoderState = UTF_DECODER_ERROR;
    extraCharsNeeded = 0;
    currentUtfChar = 0;
    handler(userData, TC_UNICODE_CHAR_ERROR);
}

uint16_t tcgfx::Utf8Text::shiftAmount() {
    if (decoderState == WAITING_BYTE_1) {
        return extraCharsNeeded == 3 ? 3 : extraCharsNeeded == 2 ? 4 : 5;
    } else if (decoderState == WAITING_BYTE_2) {
        return extraCharsNeeded == 3 ? 9 : 10;
    } else if (decoderState == WAITING_BYTE_3) {
        return 15;
    } else error();
}

void tcgfx::UnicodeFontHandler::printUtf8(const char *text) {
    utf8.reset();
    handlerMode = HANDLER_DRAWING_TEXT;
    utf8.pushChars(text);
    utf8.reset();
}

tcgfx::Coord tcgfx::UnicodeFontHandler::textExtents(const char *text, int *baseline) {
    handlerMode = HANDLER_SIZING_TEXT;
    xExtentCurrent = 0;
    utf8.reset();
    utf8.pushChars(text);
    handlerMode = HANDLER_DRAWING_TEXT;

    if(baseline) {
        *baseline = getBaseline();
    }
    return Coord((int)xExtentCurrent, getYAdvance());
}

void tcgfx::UnicodeFontHandler::writeUnicode(uint32_t unicodeText) {
    // make sure it's printable.
    if (currentPosition.x > (int32_t) drawable->getDisplayDimensions().x) return;

    GlyphWithBitmap gb;
    if(!findCharInFont(unicodeText, gb)) return;
    uint8_t w = gb.getGlyph()->width, h = gb.getGlyph()->height;
    int8_t xo = gb.getGlyph()->xOffset, yo = gb.getGlyph()->yOffset;
    uint8_t xx, yy, bits = 0, bit = 0;

    uint16_t xDim = drawable->getDisplayDimensions().x;
    uint16_t yDim = drawable->getDisplayDimensions().y;
    uint16_t x = currentPosition.x;
    uint16_t y = currentPosition.y;

    const uint8_t* bitmap = gb.getBitmapData();
    int bo = 0;
    for (yy = 0; yy < h; yy++) {
        int locY = max(0, y + yo + yy);
        bool yOK = (locY < yDim);
        for (xx = 0; xx < w; xx++) {
            if (!(bit++ & 7)) {
                bits = bitmap[bo++];
            }
            if (bits & 0x80) {
                int locX = max(0, int(x +xo + xx));
                if (locX < xDim && yOK) {
                    drawable->drawPixel(locX, locY);
                }
            }
            bits <<= 1;
        }
    }
    currentPosition.x = currentPosition.x + (int32_t)gb.getGlyph()->xAdvance;
}

tcgfx::Coord tcgfx::UnicodeFontHandler::textExtent(uint32_t theChar) {
    GlyphWithBitmap gb;
    if (!findCharInFont(theChar, gb)) {
        return Coord(0, getYAdvance());
    }
    return Coord(gb.getGlyph()->xAdvance, getYAdvance());
}

const UnicodeFontGlyph *findWithinGlyphs(const UnicodeFontBlock* block, uint32_t ch) {
    size_t start = 0;
    size_t end = block->numberOfPoints - 1;
    bool failed = false;
    while (!failed) {
        if(block->glyphs[start].relativeChar == ch) return &block->glyphs[start];
        if(block->glyphs[end].relativeChar == ch) return &block->glyphs[end];

        size_t middle = ((end - start) / 2) + start;
        uint32_t charNumMiddle = block->glyphs[middle].relativeChar;
        if (charNumMiddle == ch) return &block->glyphs[middle];
        if (ch < charNumMiddle) {
            end = middle;
        } else {
            start = middle;
        }
        if ((end - start) < 2) failed = true;
    }
    return nullptr;
}


UnicodeFontGlyph globalGlyphForFindChar;

bool UnicodeFontHandler::findCharInFont(uint32_t code, GlyphWithBitmap& glyphBitmap) const {
    if (fontAdafruit) {
        if (code < adaFont->first && code > adaFont->last) return false;
        uint32_t idx = code - adaFont->first;
        glyphBitmap.setBitmapData(&adaFont->bitmap[adaFont->glyph[idx].bitmapOffset]);
        globalGlyphForFindChar.relativeChar = code;
        globalGlyphForFindChar.height = adaFont->glyph[idx].height;
        globalGlyphForFindChar.width = adaFont->glyph[idx].width;
        globalGlyphForFindChar.xAdvance = adaFont->glyph[idx].xAdvance;
        globalGlyphForFindChar.xOffset = adaFont->glyph[idx].xOffset;
        globalGlyphForFindChar.yOffset = adaFont->glyph[idx].yOffset;
        globalGlyphForFindChar.relativeBmpOffset = 0; // unused
        glyphBitmap.setGlyph(&globalGlyphForFindChar);
        return true;
    } else {
        for (uint16_t i = 0; i < unicodeFont->numberOfBlocks; i++) {
            uint32_t startingNum = unicodeFont->unicodeBlocks[i].startingNum;
            if (code >= startingNum) {
                const UnicodeFontGlyph *glyph = findWithinGlyphs(&unicodeFont->unicodeBlocks[i], code - startingNum);
                if (glyph != nullptr) {
                    glyphBitmap.setGlyph(glyph);
                    glyphBitmap.setBitmapData(&unicodeFont->unicodeBlocks[i].bitmap[glyph->relativeBmpOffset]);
                    return true;
                }
            }
        }
    }

    return false;
}

void UnicodeFontHandler::internalHandleUnicodeFont(uint32_t ch) {
    if (ch == TC_UNICODE_CHAR_ERROR) {
        utf8.reset();
        return;
    }

    switch(handlerMode) {
        case HANDLER_SIZING_TEXT:
            xExtentCurrent += textExtent(ch).x;
            break;
        case HANDLER_DRAWING_TEXT:
            writeUnicode(ch);
            break;
    }
}

void UnicodeFontHandler::write(uint8_t data) {
    handlerMode = HANDLER_DRAWING_TEXT;
    utf8.pushChar((char)data);
}

int UnicodeFontHandler::getBaseline() {
    auto current = "(|jy";
    uint16_t height = 0;
    int bl = 0;
    while (*current) {
        GlyphWithBitmap gb;
        if(findCharInFont(*current, gb)) {
            if (gb.getGlyph()->height > height) height = gb.getGlyph()->height;
            bl = gb.getGlyph()->height + gb.getGlyph()->yOffset;
        }
        current++;
    }
    return bl;
}

void UnicodeFontHandler::setFontFromMag(const void *font, int mag) {
    if(mag == 0) {
        setFont((tcgfx::UnicodeFont*)font);
    } else {
        setFont((GFXfont*) font);
    }
}

void tcgfx::handleUtf8Drawing(void *data, uint32_t ch) {
    auto fontHandler = reinterpret_cast<UnicodeFontHandler *>(data);
    fontHandler->internalHandleUnicodeFont(ch);
}
