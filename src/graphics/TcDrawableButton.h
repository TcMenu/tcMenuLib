/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file TcDrawableButton.h
 * @brief a button that can be rendered onto any drawable, remembers it's position, and can integrate with touch screens
 */


#ifndef TCMENU_TCDRAWABLEBUTTON_H
#define TCMENU_TCDRAWABLEBUTTON_H

#include <PlatformDetermination.h>
#include "DrawingPrimitives.h"
#include "GraphicsDeviceRenderer.h"
#include "DeviceDrawableHelper.h"

#define DRAW_BUTTON_FLAG_ICON_BOOL 0
#define DRAW_BUTTON_FLAG_IS_DIRTY 1

namespace tcgfx {

/**
 * A drawable button that can render onto TcMenu drawables, it easily integrates with touch screens and draws onto a
 * very wide range of displays. Has semantics similar to a regular button on all displays. It has a position and size,
 * color,
 */
    class TcDrawableButton {
    public:
        enum ButtonDrawingMode {
            NORMAL, SELECTED, PRESSED, NOT_SELECTABLE
        };
    private:
        Coord where;
        Coord size;
        color_t bgColor;
        color_t fgColor;
        color_t selectedColor;
        union {
            const char *text;
            const tcgfx::DrawableIcon *icon;
        };
        DeviceFontDrawingMode fontMode;
        ButtonDrawingMode drawingMode = NORMAL;
        uint8_t flags;
    public:
        /**
         * Create a button object that has only text at a particular point and size and color.
         * @param where the position to start drawing.
         * @param size the size of the button
         * @param bgCol the background color
         * @param fgCol the foreground color
         * @param text the text to be presented.
         */
        TcDrawableButton(const Coord &where, const Coord &size, color_t bgCol, color_t fgCol, color_t selCol,
                         const char *text, const DeviceFontDrawingMode& font);

        /**
         * Create a button object that has an icon at a particular point and size and color.
         * @param where the position to start drawing.
         * @param size the size of the button
         * @param bgCol the background color
         * @param fgCol the foreground color
         * @param text the text to be presented or nullptr if there is no text.
         */
        TcDrawableButton(const Coord &where, const Coord &size, color_t bgCol, color_t fgCol, color_t selCol,
                         tcgfx::DrawableIcon *icon);

        /**
         * Returns true if the touch coordinates provided are within the bounds of this button, it also sets the pressed flag
         * @param location the touch location
         * @return true if within the buttons bounds
         */
        bool touchInBounds(const Coord &location);

        /**
         * Set the button mode
         * @param pressed
         */
        void setButtonDrawingMode(ButtonDrawingMode mode) {
            drawingMode = mode;
            setDirty(true);
        }

        /**
         * Marks the button as dirty and therefore requiring a screen update.
         * @param d
         */
        void setDirty(bool d) {
            bitWrite(flags, DRAW_BUTTON_FLAG_IS_DIRTY, d);
        }

        bool isDirty() const { return bitRead(flags, DRAW_BUTTON_FLAG_IS_DIRTY); }

        /**
         * @return the current drawing mode of the button
         */
        ButtonDrawingMode getButtonDrawingMode() { return drawingMode; }

        /**
         * paint onto the drawable, using a sub-device if it is supported, will only draw when dirty flag is set.
         * @param drawable the drawable to draw onto
         */
        void paintButton(DeviceDrawable *drawable);
    };

}

#endif //TCMENU_TCDRAWABLEBUTTON_H
