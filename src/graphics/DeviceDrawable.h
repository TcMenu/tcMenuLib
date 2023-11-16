
#ifndef TCLIBRARYDEV_DEVICEDRAWABLE_H
#define TCLIBRARYDEV_DEVICEDRAWABLE_H

#include <PlatformDetermination.h>
#include "GfxMenuConfig.h"
#include "DrawingPrimitives.h"

class UnicodeFontHandler;

namespace tcgfx {
    /**
     * This is the interface that all graphical rendering devices extend from when using GraphicsDeviceRenderer. Instances
     * of this map all the graphics primitives for each display type. You can use it yourself in code to present moderately
     * complex displays without committing to a particular device. IE it would be trivial to switch between STM32 BCP LCD,
     * Adafruit library, TFTeSPI and U8G2.
     *
     * Example usage (assuming you have a drawable object pointer):
     *
     * ```
     * drawable->startDraw();
     * drawable->setDrawColor(RGB(0,0,0));
     * drawable->drawBox(Coord(0,0), Coord(320, 20), true);
     * drawable->setColors(RGB(255, 128, 0), RGB(0,0,0));
     * drawable->drawText(Coord(0,0), nullptr, 0, "hello world");
     * drawable->setDrawColor(RGB(0,0,255));
     * drawable->drawCircle(Coord(50, 50), 10, true);
     * drawable->endDraw();
     * ```
     *
     * The virtual methods add a small overhead, but allow us to easily support a very wide range of screens, in future
     * supporting new graphical displays will be far easier, and less maintenance going forwards.
     */
    class DeviceDrawable {
    public:
        enum SubDeviceType {
            NO_SUB_DEVICE, SUB_DEVICE_4BPP, SUB_DEVICE_2BPP
        };
    protected:
        UnicodeFontHandler *fontHandler = nullptr;
        color_t backgroundColor = 0, drawColor = 0;
        SubDeviceType subDeviceType = NO_SUB_DEVICE;
    public:
        virtual ~DeviceDrawable() = default;
        /**
         * @return the dimensions of the screen corrected for the current rotation
         */
        virtual Coord getDisplayDimensions() = 0;

        /**
         * Gets a sub device that can be rendered to that buffers a part of the screen up to a particular size, it is
         * for sprites or partial buffers when the display is not fully double buffered. This method returns nullptr when
         * such a sub-device is not available or not needed.
         * @param width the needed width in pixels
         * @param height the needed height in pixels
         * @param bitsPerPx the number of bits per pixel
         * @param palette the palette that will be used
         * @return the sub display, or null if one is not available.
         */
        virtual DeviceDrawable *
        getSubDeviceFor(const Coord &where, const Coord &size, const color_t *palette, int paletteSize) = 0;

        /**
         * Draw text at the location requested using the font and color information provided. If the TcUnicode flag is
         * enabled then all drawing will take place using TcUnicode, otherwise the native font support will be called,
         * as implemented by `internalDrawText`.
         * @param where the position on the screen
         * @param font the font to use
         * @param mag the magnification of the font - if supported
         * @param text the string to print
         */
        void drawText(const Coord &where, const void *font, int mag, const char *text);

        virtual void internalDrawText(const Coord &where, const void *font, int mag, const char *text) = 0;

        /**
         * Draw an icon bitmap at the specified location using the provided color, you can choose either the regular or
         * selected icon. Note that not all icon formats are available on every display.
         * @param where the position of the screen
         * @param icon the icon to draw
         * @param bg the background color to draw behind the bitmap
         * @param selected if the regular or selected icon should be used.
         */
        virtual void drawBitmap(const Coord &where, const DrawableIcon *icon, bool selected) = 0;

        /**
         * Draws an XBM at the provided location, of the given size, in the colours provided.
         * @param where the position on the screen
         * @param size the size of the image in pixels
         * @param data the raw bitmap data that should be presented
         * @param fgColor the color for the bitmap to use
         * @param bgColor the background color to fill first.
         */
        virtual void drawXBitmap(const Coord &where, const Coord &size, const uint8_t *data) = 0;

        /**
         * Draw either a hollow or filled both with the provided dimensions, in the color provided.
         * @param where the position of the box
         * @param size the size of the box
         * @param color the color of the box
         * @param filled if true, the box is filled, otherwise it is an outline only.
         */
        virtual void drawBox(const Coord &where, const Coord &size, bool filled) = 0;

        /**
         * Draw a circle that is either filled or not filled on the screen
         * @param where the position of the circle *centre*
         * @param radius the radius of the circle to draw
         * @param filled true if it is to be filled, otherwise false
         */
        virtual void drawCircle(const Coord &where, int radius, bool filled) = 0;

        /**
         * Draw a triangle or polygon onto the screen as a series of points, if the driver supports it. All drivers support
         * lines and triangles, but nothing above that is guaranteed.
         *
         * It may be that a triangle can be filled but a polygon cant. If points is two it will try and use draw line and
         * if points is 3 it will try to use draw triangle. There is no guarantee that the underlying driver will support
         * anything above draw triangle, so check your driver before using more than 3 points. TcMenu graphics libraries
         * never use this for anything higher than triangle, as it may not be supported.
         */
        virtual void drawPolygon(const Coord points[], int numPoints, bool filled) = 0;

        /**
         * the device must be able to map colors for situations where a palette is used. The default implementation
         * just returns the original color.
         * @param col the color to find the underlying color for
         * @return the actual color mapping for the display.
         */
        virtual color_t getUnderlyingColor(color_t col) { return col; }

        /**
         * Indicates the start or end of a transaction, for core devices, it indicates the end of a rendering loop, for
         * sub devices it means that we have finished with the
         * @param isStarting
         * @param redrawNeeded
         */
        virtual void transaction(bool isStarting, bool redrawNeeded) = 0;

        /**
         * This is the internal implementation of textExtents for when TcUnicode is not in use, always prefer the use
         * of textExtents unless you actually need to directly access the library functions.
         * @param font the font to get sizing for
         * @param mag the magnification of the font if supported
         * @param text the text to get the size of
         * @param baseline optionally, the base line will be populated if this is provided.
         * @return the X and Y dimensions
         */
        virtual Coord internalTextExtents(const void *font, int mag, const char *text, int *baseline) = 0;

        /**
         * gets the extents of the text, and optionally the baseline for the given font and text. The X axis of the returned
         * Coord is the text width, the Y axis has the height, finally the base line is the descent.
         * @param font the font to get sizing for
         * @param mag the magnification of the font if supported
         * @param text the text to get the size of
         * @param baseline optionally, the base line will be populated if this is provided.
         * @return the X and Y dimensions
         */
        Coord textExtents(const void *font, int mag, const char *text, int *baseline);

        /**
         * Draw a pixel in the current draw color at the location provided. This should be implemented in the most
         * optimal way possible for the platform
         * @param x the x location
         * @param y the y location
         */
        virtual void drawPixel(uint16_t x, uint16_t y) = 0;

        /**
         * Gets the width and height of the text in the font provided, returned in a Coord.
         * @param font the font to measure
         * @param mag the magnification if supported
         * @param text the text to measure
         * @return the X and Y dimensions
         */
        Coord textExtents(const void *font, int mag, const char *text) {
            return textExtents(font, mag, text, nullptr);
        }

        /**
         * Should be called before starting any rendering onto a device, in case there's anything that needs to be
         * done before drawing. This is a helper method that calls transaction()..
         */
        void startDraw() {
            transaction(true, false);
        }

        /**
         * Should be called after you've finished drawing, to ensure that everything is completed and drawn. This is a
         * helper method that calls transaction()..
         * @param needsDrawing true if there were changes, (default is true)
         */
        void endDraw(bool needsDrawing = true) {
            transaction(false, needsDrawing);
        }

        /**
         * set the draw color and background color to be used in subsequent operations
         * @param fg the foreground / draw color
         * @param bg the background color
         */
        void setColors(color_t fg, color_t bg) {
            backgroundColor = getUnderlyingColor(bg);
            drawColor = getUnderlyingColor(fg);
        }

        /**
         * Sets only the drawing color, leaving the background as it was before.
         * @param fg foreground / drawing color.
         */
        void setDrawColor(color_t fg) { drawColor = getUnderlyingColor(fg); }

        /**
         * Enables the use of tcUnicode characters, and at this point the drawable assumes all fonts are tcUnicode or
         * Adafruit GFX. This only prevents creation of the handler if called before getting the font handler.
         * @param enabled true if TcUnicode support is to be enabled, otherwise false.
         */
        void enableTcUnicode() { if (fontHandler == nullptr) fontHandler = createFontHandler(); }

        /**
         * Check if TcUnicode is enabled. Useful before calling `getFontHandler` to ensure the support is on.
         * @return true if on, otherwise false
         */
        bool isTcUnicodeEnabled() { return fontHandler != nullptr; }

        /**
         * Gets hold of the TcUnicode font handler that can render text onto the display. It has features similar to
         * print and Adafruit GFX that make rendering unicode character with UTF-8 easier.
         * @param enableIfNeeded if true, the TcUnicode enable flag will be enabled if needed.
         * @return the font handler if unicode is enabled, otherwise nullptr.
         */
        UnicodeFontHandler *getUnicodeHandler(bool enableIfNeeded = true);

        /**
         * If a native font handler has already been created, avoid creating a second instance and give the drawable
         * the same instance to save memory.
         * @param handler pointer to the existing handler
         */
        void setFontHandler(UnicodeFontHandler* handler) {fontHandler = handler;}

        /**
         * @return the type of sub-device that is supported by this display drawable.
         */
        SubDeviceType getSubDeviceType() { return subDeviceType; }

    protected:
        /**
         * It is best that all device drawables override this with an optimal implementation rather than rely on the
         * default one which indirects through draw pixel here adding a layer of latency
         * @return a font handler
         */
        virtual UnicodeFontHandler *createFontHandler();

        /**
         * Normally used internally by extension classes to set the type of subdevice they can support. Defaults to
         * none.
         * @param s the type of sub-device supported.
         */
        void setSubDeviceType(SubDeviceType s) { subDeviceType = s; }
    };

    void setTcFontAccordingToMag(UnicodeFontHandler* handler, const void* font, int mag);
}

#endif //TCLIBRARYDEV_DEVICEDRAWABLE_H
