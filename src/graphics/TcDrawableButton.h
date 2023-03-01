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
#define DRAW_BUTTON_MONO 2

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
                         const tcgfx::DrawableIcon *icon);

        /**
         * Create an icon based button where it is more convenient to set the color and positioning later
         * @param icon the icon to draw with
         */
        explicit TcDrawableButton(const tcgfx::DrawableIcon *icon);

        /**
         * Create a text based b1utton where it is more convenient to set the color and positioning later
         * @param text the text to draw
         * @param font the font to draw with
         */
        TcDrawableButton(const char *text, const DeviceFontDrawingMode& font);

        /**
         * Returns true if the touch coordinates provided are within the bounds of this button, it also sets the pressed flag
         * @param location the touch location
         * @return true if within the buttons bounds
         */
        bool touchInBounds(const Coord &location) const;

        /**
         * On mono  OLED displays there is no in-between value that suits greying out, the easiest is to remove from view
         * and the only selected action we have is to invert the button.
         * @param mono true- when on a monochrmoe two color display
         */
        void setButtonOnMonoDisplay(bool mono) { bitWrite(flags, DRAW_BUTTON_MONO, mono); }

        /**
         * @return true if the button hides when unselectable, otherwise false and the button will grey out instead.
         */
        bool isButtonOnMonoDisplay() { return bitRead(flags, DRAW_BUTTON_MONO); }

        /**
         * Set the button mode
         * @param pressed
         */
        void setButtonDrawingMode(ButtonDrawingMode mode) {
            if(drawingMode != mode) setDirty(true);
            drawingMode = mode;
        }

        /**
         * Marks the button as dirty and therefore requiring a screen update.
         * @param d
         */
        void setDirty(bool d) {
            bitWrite(flags, DRAW_BUTTON_FLAG_IS_DIRTY, d);
        }

        /**
         * If the button needs painting
         * @return true if needs painting, otherwise false
         */
        bool isDirty() const { return bitRead(flags, DRAW_BUTTON_FLAG_IS_DIRTY); }

        /**
         * If the button is an icon button
         * @return true if icon, false if text
         */
        bool isIconDrawn() const { return bitRead(flags, DRAW_BUTTON_FLAG_ICON_BOOL); }

        /**
         * @return the current drawing mode of the button
         */
        ButtonDrawingMode getButtonDrawingMode() { return drawingMode; }

        /**
         * paint onto the drawable, using a sub-device if it is supported, will only draw when dirty flag is set.
         * @param drawable the drawable to draw onto
         */
        void paintButton(DeviceDrawable *drawable);

        /**
         * Set a new position for the button, note you will need to clear from the old location
         * @param position the new screen position
         */
        void setPosition(const Coord& position);

        /**
         * Set both the position and size of the button at the same time
         * @param position the screen position
         * @param newSize the new size
         */
        void setPositionAndSize(const Coord& position, const Coord& newSize);

        /**
         * Change the colors associated with the button
         * @param bgCol the background
         * @param fgCol the foreground
         * @param selCol the selected background
         */
        void setColors(color_t bgCol, color_t fgCol, color_t selCol);

        void setText(const char* newText);
    };

}

#endif //TCMENU_TCDRAWABLEBUTTON_H
