//
// Created by dave on 14/11/2022.
//

#ifndef TCLIBRARYDEV_UNICODEFONTHANDLER_H
#define TCLIBRARYDEV_UNICODEFONTHANDLER_H

#include <PlatformDetermination.h>
#include <SCCircularBuffer.h>
#include "DrawingPrimitives.h"
#include "GraphicsDeviceRenderer.h"

#define TC_UNICODE_CHAR_ERROR 0xffffffff

#ifndef _GFXFONT_H_
#define _GFXFONT_H_

//
// This section is copied from Adafruit_GFX, it provides support for using Adafruit GFX fonts with this library.
//
// Font structures for newer Adafruit_GFX (1.1 and later).
// Example fonts are included in 'Fonts' directory.
// To use a font in your Arduino sketch, #include the corresponding .h
// file and pass address of GFXfont struct to setFont().
//

typedef struct {
    uint16_t bitmapOffset; ///< Pointer into GFXfont->bitmap
    uint8_t width;         ///< Bitmap dimensions in pixels
    uint8_t height;        ///< Bitmap dimensions in pixels
    uint8_t xAdvance;      ///< Distance to advance cursor (x axis)
    int8_t xOffset;        ///< X dist from cursor pos to UL corner
    int8_t yOffset;        ///< Y dist from cursor pos to UL corner
} GFXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct {
    uint8_t *bitmap;  ///< Glyph bitmaps, concatenated
    GFXglyph *glyph;  ///< Glyph array
    uint16_t first;   ///< ASCII extents (first char)
    uint16_t last;    ///< ASCII extents (last char)
    uint8_t yAdvance; ///< Newline distance (y axis)
} GFXfont;

#endif

namespace tcgfx {

    /**
     * Each glyph in the font is represented by this struct. This describes how to render each character.
     */
    typedef struct {
        /** The character number in the array */
        uint32_t relativeChar: 15;
        /** offset into the bitmap array */
        uint32_t relativeBmpOffset:17;
        /** width of the bitmap in pixels */
        uint8_t width;
        /** height of the bitmap in pixels */
        uint8_t height;
        /** how far to advance in the x dimension */
        uint8_t xAdvance;
        /** the x offset from the UL corner */
        int8_t xOffset;
        /** the y offset from the UL corner, usually negative */
        int8_t yOffset;
    } UnicodeFontGlyph;

    /**
     * Each unicode block range has it's own characters, this allows us to be more efficient with memory overall.
     * The char nums in a glyph are actually calculated by startingId + charNum
     */
    typedef struct {
        /** The starting point for this block, all glyph entries are an offset from this */
        uint32_t startingNum;
        /** the array of bitmaps for each letter */
        const uint8_t *bitmap;
        /** The glyphs within this block */
        const UnicodeFontGlyph *glyphs;
        /** The number of points in this block */
        uint16_t numberOfPoints;
    } UnicodeFontBlock;

    /**
     * This represents the whole font, and is passed to an TcUnicode renderer requiring a font. It links the full
     * rendering instructions including bitmaps in one place.
     */
    typedef struct {
        /** the array of unicode glyphs */
        const UnicodeFontBlock *unicodeBlocks;
        /** the number of items */
        uint16_t numberOfBlocks;
        /** the height of each line */
        uint8_t yAdvance;
    } UnicodeFont;

    typedef void (*UnicodeCharacterHandler)(void* callbackData, uint32_t convertedChar);

    /**
     * Tells the UTF8 encoder which mode it is in, either extended ASCII or UTF8
     */
    enum UnicodeEncodingMode { ENCMODE_UTF8, ENCMODE_EXT_ASCII };

    /**
     * Represents an item that can be drawn using the TcMenu font drawing functions. Regardless of if it is Adafruit
     * or TcUnicode we wrap it in one of these so the drawing code is always the same.
     */
    class GlyphWithBitmap {
    private:
        const uint8_t* bitmapData = nullptr;
        const UnicodeFontGlyph* glyph = nullptr;
    public:
        /**
         * @return the actual bitmap data with offset already applied
         */
        const uint8_t *getBitmapData() const {
            return bitmapData;
        }

        /**
         * @return the glyph instructions for rendering.
         */
        const UnicodeFontGlyph* getGlyph() const {
            return glyph;
        }

        void setBitmapData(const uint8_t *bm) {
            GlyphWithBitmap::bitmapData = bm;
        }

        void setGlyph(const UnicodeFontGlyph* g) {
            GlyphWithBitmap::glyph = g;
        }
    };

    /**
     * A completely asynchronous implementation of a UTF8 encoder. Can work with the Print interface as it expects no
     * end to the stream, and requires only a very simple change in the write function.
     */
    class Utf8Text {
    public:
        enum DecoderState {
            WAITING_BYTE_0, WAITING_BYTE_1, WAITING_BYTE_2, WAITING_BYTE_3,
            UTF_CHAR_FOUND, UTF_DECODER_ERROR
        };
    private:
        DecoderState decoderState = WAITING_BYTE_0;
        uint32_t currentUtfChar = 0U;
        int extraCharsNeeded = 0;
        UnicodeCharacterHandler handler;
        void* userData;
        const UnicodeEncodingMode encodingMode;
    public:
        explicit Utf8Text(UnicodeCharacterHandler handler, void* userData, UnicodeEncodingMode mode) : handler(handler),
                userData(userData), encodingMode(mode) {}
        void reset();

        void pushChar(char ch);
        void pushChars(const char *str);
        bool hasErrorOccurred() { return decoderState == UTF_DECODER_ERROR; }

    private:
        void error();
        uint16_t shiftAmount();
    };

    void handleUtf8Drawing(void* userData, uint32_t ch);

    class UnicodeFontHandler {
    public:
        enum HandlerMode { HANDLER_SIZING_TEXT, HANDLER_DRAWING_TEXT};
    private:
        Utf8Text utf8;
        DeviceDrawable* drawable;
        union {
            const UnicodeFont *unicodeFont;
            const GFXfont * adaFont;
        };
        bool fontAdafruit = false;
        Coord currentPosition {};
        HandlerMode handlerMode = HANDLER_DRAWING_TEXT;
        uint16_t xExtentCurrent = 0;
    public:
        explicit UnicodeFontHandler(DeviceDrawable *drawable, UnicodeEncodingMode mode): utf8(handleUtf8Drawing, this, mode), drawable(drawable), unicodeFont(nullptr), currentPosition() {}
        /**
         * Sets the font to be a TcUnicode font
         * @param font a tcUnicode font
         */
        void setFont(const UnicodeFont *font) { unicodeFont = font; fontAdafruit = false; }
        /**
         * sets the font to be an Adafruit font
         * @param font an adafruit font
         */
        void setFont(const GFXfont *font) { adaFont = font; fontAdafruit = true; }

        /**
         * Sets the font based on the magnification, 0 value means the font is a unicode font,
         * any other value is adafruit. The usual way is to set magnification to 0 for unicode,
         * otherwise set it to 1 or more for adafruit.
         * @param font a void font pointer to be decided using mag
         * @param mag when -1=Unicode, otherwise Adafruit
         */
        void setFontFromMag(const void* font, int mag);

        /**
         * Set the position at which the next item will be printed.
         * @param where
         */
        void setCursor(const Coord& where) {
            currentPosition = where;
        }

        /**
         * Print a UTF8 string using the selected font, if characters are missing they are skipped
         * @param text the UTF8 text to print
         */
        void printUtf8(const char *text);

        /**
         * Prints a unicode character using the current font
         * @param unicodeChar the character to print.
         */
        void writeUnicode(uint32_t unicodeText);

        /**
         * Get the extents of the text provided in UTF8
         * @param text the text to get the UTF8 version of
         * @param baseline pointer to in for¬ the baseline of the text (optional)
         * @return the x and y extent of the text
         */
        Coord textExtents(const char *text, int *baseline);

        /**
         * Get the extent of a single unicode character
         * @param theChar the unicode character to get the extent of
         * @return the x and y extent of the character
         */
        Coord textExtent(uint32_t theChar);

        /**
         * Get the absolute baseline of the font, this is the total height including any parts considered below
         * the yheight.
         * @return the baseline
         */
        int getBaseline();

        /**
         * Can be used to implement the print interface, the UTF8 implementation for this library is completely
         * asynchronous and keeps internal state, so it will wait until enouogh characters arrive to actually print
         * something.
         * @param data a byte of data in utf8
         */
        void write(uint8_t data);

        void internalHandleUnicodeFont(uint32_t ch);
    private:
        bool findCharInFont(uint32_t ch, GlyphWithBitmap& glyphBitmap) const;
        int getYAdvance() const { return (int32_t)(fontAdafruit ? adaFont->yAdvance : unicodeFont->yAdvance); }
    };
}

#endif //TCLIBRARYDEV_UNICODEFONTHANDLER_H
