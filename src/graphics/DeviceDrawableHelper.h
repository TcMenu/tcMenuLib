/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file DeviceDrawableHelper.h
 * @brief A few helper classes that provides useful functions on top of a tcMenu device drawable.
 */

#ifndef TCLIBRARYDEV_DEVICEDRAWABLEHELPER_H
#define TCLIBRARYDEV_DEVICEDRAWABLEHELPER_H

#include <PlatformDetermination.h>
#include <tcUnicodeHelper.h>
#include "GfxMenuConfig.h"
#include "DeviceDrawable.h"

namespace tcgfx {
    /**
     * Represents a native font as used by most themes, it has a void pointer to the font data, and a magnification.
     */
    class NativeFontDesc {
    private:
        const void* ptr;
        uint8_t mag;

    public:
        NativeFontDesc(const void *ptr, uint8_t mag) : ptr(ptr), mag(mag) {}
        NativeFontDesc(const NativeFontDesc& other) = default;
        NativeFontDesc& operator= (const NativeFontDesc& other) = default;

        uint8_t getMag() const { return mag; }
        const void* getPtr() const { return ptr; }
    };

    /**
     * Describes the various font systems that are supported by the wrapper and dashboards, that is native library fonts,
     * tcUnicode(adafruit) and tcUnicode proper. For native adafruit through the library, select native font.
     */
    enum DeviceFontMode {
        ADAFRUIT_FONT, TCUNICODE_FONT, NATIVE_FONT, NO_FONT_SEL
    };

    /**
     * Wraps a font type and the associated pointer within an object that can help set the font.
     */
    class DeviceFontDrawingMode {
    private:
        union {
            const GFXfont *adaFont;
            const UnicodeFont *uniFont;
            NativeFontDesc nativeFont;
        };
        DeviceFontMode mode;
    public:

        /**
         * Default constructor that sets the font mode to no font.
         */
        DeviceFontDrawingMode() {
            mode = NO_FONT_SEL;
            adaFont = nullptr;
        }

        DeviceFontDrawingMode(const DeviceFontDrawingMode& other) = default;
        DeviceFontDrawingMode& operator=(const DeviceFontDrawingMode& other) = default;

        /**
         * Create a tcUnicode font mode with an adafruit font that will render through tcMenu.
         * @param adaTc the adafruit font to render with tcUnicode
         */
        explicit DeviceFontDrawingMode(const GFXfont* adaTc) {
            mode = ADAFRUIT_FONT;
            adaFont = adaTc;
        }

        /**
         * Create a tcUnicode font mode with a unicode font that will render through tcMenu.
         * @param adaTc the unicode font to render with tcUnicode
         */
        explicit DeviceFontDrawingMode(const UnicodeFont* adaTc) {
            mode = TCUNICODE_FONT;
            uniFont = adaTc;
        }
        /**
         * Create a native font mode that will render through the native library.
         * @param adaTc the native font description
         */
        explicit DeviceFontDrawingMode(const NativeFontDesc& nativeFontDesc) {
            mode = NATIVE_FONT;
            nativeFont = nativeFontDesc;
        }

        bool isTcUnicode() { return mode == TCUNICODE_FONT || mode == ADAFRUIT_FONT; }

        void setFontTcUnicode(UnicodeFontHandler* handler) {
            if(mode == ADAFRUIT_FONT) {
                handler->setFont(adaFont);
            } else if(mode == TCUNICODE_FONT){
                handler->setFont(uniFont);
            }
        }

        const NativeFontDesc& getNativeDesc() const {
            return nativeFont;
        }
    };


    /**
     * Wraps a drawable regardless of if we are on a sub device or root device, this class handles all the differences
     * between the two with helper functions for dealing with palette colors and offset differences. It also has helpers
     * that work out if TcUnicode(Adafruit or Unicode)/Native fonts are being used and calls the right drawing functions
     */
    class DeviceDrawableHelper {
    private:
        DeviceDrawable *rootDrawable = nullptr;
        DeviceDrawable *drawable = nullptr;
        Coord startPos = {};
        DeviceFontDrawingMode fontMode;
        bool isSubDevice = false;
    public:
        /**
         * Create a drawable helper given the drawable, palette, position and size, if it is possible, it will create
         * a subdevice for drawing. When you call drawing functions that take a where coordinate, use the offsetLocation
         * function that corrects for the sub drawable for you automatically.
         *
         * @param root the root device drawable
         * @param palette the palette of colors for the sub drawable
         * @param paletteSize the size of the palette
         * @param startPosition the starting position on the screen to draw at
         * @param size how large the area is
         */
        DeviceDrawableHelper(DeviceDrawable* root, color_t *palette, uint8_t paletteSize, const Coord &startPosition, const Coord &size);

        /**
         * For global constructions, we can also initially construct as root, and it will reconfigure later.
         * @param root the drawable that will act as root
         */
        explicit DeviceDrawableHelper(DeviceDrawable* root);


        void reConfigure(color_t *palette, uint8_t paletteSize, const Coord &startPosition, const Coord &size);

        /**
         * Get the drawable, not that this could be either the root device or a sub-device, always use offset location
         * to ensure your where parameters are correctly positioned
         * @return the drawable
         */
        DeviceDrawable *getDrawable() { return drawable; }

        /**
         * If on a sub-device, this call will correct the coordinate to match the area of the screen that the sub-device
         * is drawing to.
         * @param source the source in screen coordinates
         * @return the coordinates correct for the current drawable
         */
        Coord offsetLocation(const Coord &source) const {
            return isSubDevice ? Coord(source.x - startPos.x, source.y - startPos.y) : source;
        }

        /**
         * If on a sub-device, this call will correct the coordinate to match the area of the screen that the sub-device
         * is drawing to.
         * @param source the source in screen coordinates
         * @param xOffs an additional x offset to apply
         * @param yOffs an additional y offset to apply
         * @return the coordinates correct for the current drawable
         */
        Coord offsetLocation(const Coord &source, int xOffs, int yOffs) const;

        /**
         * You must call this once drawing is concluded to ensure that the results are drawn, in the case of a sub-device
         * this is critical.
         */
        void endDraw() {
            if (isSubDevice) {
                drawable->endDraw(true);
                drawable = rootDrawable;
            }
        }

        /**
         * Set the font drawing mode for the subsequent text operations
         * @param font the font drawing mode
         */
        void setFont(const DeviceFontDrawingMode& font) {
            fontMode = font;
        }

        /**
         * Draw text in the current font onto the display using the current drawable. Ensure you've called setFont
         * before this with a suitable font.
         * @param where where to draw (will automatically apply offsetLocation)
         * @param color the color to draw in (not the index)
         * @param text the text to be presented
         */
        void drawText(const Coord& where, color_t color, const char* text);
        /**
         * Gets the size of the text for the current font as set by setFont. It's coordinates are returned, and the
         * baseline is copied into the baseline parameter
         * @param text the text to measure
         * @param bl the baseline pointer to fill in - int pointer
         * @return the size of the text.
         */
        Coord textExtents(const char* text, int* bl);

        /**
         * Set the font from the theme font pointer and magnification values, by determining if tcUnicode is enabled
         * and then choosing either native or TcUnicode based operation.
         */
        void setFontFromParameters(const void* font, uint8_t mag);
    };

}

#endif //TCLIBRARYDEV_DEVICEDRAWABLEHELPER_H
