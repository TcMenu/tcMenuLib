/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file DrawingPrimitives.h
 * @brief contains a series of core components needed by all graphical renderers
 */

#ifndef TCMENU_DRAWING_PRIMITIVES_H
#define TCMENU_DRAWING_PRIMITIVES_H

#include <PlatformDetermination.h>

namespace tcgfx {

/**
 * Represents no mask
 */
#define DRAW_NO_MASK 0xff


#ifdef NEED_32BIT_COLOR_T_ALPHA
    /**
     * Defines a basic color type that can be used with RGB() macro
     * regardless of the color depth.
     */
    typedef uint32_t color_t;
/** this macro creates an RGB color based on R, G, B values between 0 and 255 */
#define RGB(r, g, b) (uint32_t)( 0xff000000UL | ((r)<<16UL) | ((g)<<8UL) | (b) )
#else
    /**
     * Defines a basic color type that can be used with RGB() macro
     * regardless of the color depth.
     */
    typedef uint16_t color_t;
/** this macro creates an RGB color based on R, G, B values between 0 and 255 */
#define RGB(r, g, b) (uint16_t)( (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3) )
#endif

    /**
     * A font definition that holds both a possible font pointer and also the magnification
     */
     struct MenuFontDef {
         const void* fontData;
         const uint8_t fontMag;

         MenuFontDef(const void* data, uint8_t mag) : fontData(data), fontMag(mag) { }
     };

    /**
     * Defines padding for menu rendering when using graphical renderers. Each
     * position can hold the value 0..15
     */
    struct MenuPadding {
        uint16_t top: 4;
        uint16_t right: 4;
        uint16_t bottom: 4;
        uint16_t left: 4;

        MenuPadding(int top_, int right_, int bottom_, int left_) {
            top = top_;
            bottom = bottom_;
            right = right_;
            left = left_;
        }

        explicit MenuPadding(int equalAll = 0) {
            top = bottom = right = left = equalAll;
        }
    };

    /**
     * Populate a padding structure with values using the same form as HTML, top, right, bottom, left.
     * @param padding reference type of padding
     * @param top the top value
     * @param right the right value
     * @param bottom the bottom value
     * @param left the left value
     */
    inline void makePadding(MenuPadding& padding, int top, int right, int bottom, int left) {
        padding.top = top;
        padding.right = right;
        padding.bottom = bottom;
        padding.left = left;
    }

    /**
     * Used to represent a border in terms of top, right, bottom and left widths.
     */
    struct MenuBorder {
        uint8_t top:2;
        uint8_t left:2;
        uint8_t bottom:2;
        uint8_t right:2;

        MenuBorder() = default;

        explicit MenuBorder(uint8_t equalSides) {
            top = left = bottom = right = equalSides;
        }

        MenuBorder(uint8_t top_, uint8_t right_, uint8_t bottom_, uint8_t left_) {
            top = top_;
            right = right_;
            left = left_;
            bottom = bottom_;
        }

        bool areAllBordersEqual() const {
            return (top == left) && (left == right) && (right == bottom);
        }

        bool isBorderOff() const {
            return areAllBordersEqual() && top == 0;
        }
    };

#ifndef TC_COORD_DEFINED
#define TC_COORD_DEFINED
    /** A structure that holds both X and Y direction in a single 32 bit integer. Both x and y are public */
    struct Coord {
        /** default construction sets values to 0 */
        Coord() {
            x = y = 0;
        }

        /**
         * Create a coord based on an X and Y location
         * @param x the x location
         * @param y the y location
         */
        Coord(int x, int y) {
            this->x = x;
            this->y = y;
        }

        Coord(const Coord &other) = default;
        Coord& operator = (const Coord& other) = default;

        int16_t x;
        int16_t y;
    };

#endif // TC_COORD_DEFINED

    /**
     * Represents an icon that can be presented for actionable menu items such as submenus, boolean items, and action items.
     * It can have two items states, one for selected and one for normal.
     */
    class DrawableIcon {
    public:
        enum IconType : uint8_t {
            /** the image is in the well know XBM format, no palette info provided */
            ICON_XBITMAP,
            /** the image is a regular monochrome bitmap in native format, no palette info provided */
            ICON_MONO,
            /** the image is in a palette format, the palette information will be populated, it contains
             * rendering instructions for 2 bits per pixel */
            ICON_PALLETE_2BPP,
            /** the image is in a palette format, the palette information will be populated, it contains
             * rendering instructions for 4 bits per pixel */
            ICON_PALLETE_4BPP,
            /** the image is in a format that can be pushed directly to the display, no palette info provided*/
            ICON_NATIVE
        };
        enum MemoryLocation : uint8_t {
            /** Indication that this image is in progmem, this is the default */
            STORED_IN_ROM,
            /** This image is in RAM, if it can be supported by the display. EG board with progmem, driver support */
            STORED_IN_RAM,
            /** Not yet supported, will provide for loading images from external storage in a future version */
            LOAD_FROM_STORAGE
        };
    private:
        uint16_t menuId;
        Coord dimensions;
        IconType iconType;
        MemoryLocation location;
        const uint8_t *normalIcon;
        const uint8_t *selectedIcon;
        const color_t* palette;
    public:
        /**
         * Creates an empty drawable icon, used mainly by collection support
         */
        DrawableIcon() : menuId(0), dimensions(0, 0), iconType(ICON_XBITMAP), location(STORED_IN_ROM),
                         normalIcon(nullptr), selectedIcon(nullptr), palette(nullptr) {}

        /**
         * Copy constructor to copy an existing drawable icon
         */
        DrawableIcon(const DrawableIcon &other) = default;
        DrawableIcon& operator=(const DrawableIcon& other) = default;

        /**
         * Create a drawable icon providing the size, icon type, and image data
         * @param id the menu id that this icon belongs to
         * @param size the size of the image, better to be in whole byte sizes
         * @param ty the type of the image, so the renderer knows what to do with it.
         * @param normal the image in the normal state.
         * @param selected the image in the selected state.
         */
        DrawableIcon(uint16_t id, const Coord &size, IconType ty, const uint8_t *normal,
                     const uint8_t *selected = nullptr)
                : menuId(id), dimensions(size), iconType(ty), location(STORED_IN_ROM), normalIcon(normal),
                  selectedIcon(selected), palette(nullptr) {}
        /**
         * Create a drawable icon providing the size, icon type, and image data along with a palette, this is suited
         * for 2 and 4 bit per pixel images
         * @param id the menu id that this icon belongs to
         * @param size the size of the image, better to be in whole byte sizes
         * @param palette the palette data for the image, will define this as a palette based image
         * @param normal the image in the normal state.
         * @param selected the image in the selected state.
         */
        DrawableIcon(uint16_t id, const Coord &size, IconType ty, const color_t* paletteEntries, const uint8_t *normal,
                     const uint8_t *selected = nullptr)
                : menuId(id), dimensions(size), iconType(ty), location(STORED_IN_ROM), normalIcon(normal),
                  selectedIcon(selected), palette(paletteEntries) {}

        /**
         * Get the icon data for the current state
         * @param selected true if the item is selected (IE pressed or ON for booleans)
         * @return the icon data, which may be in program memory on AVR and ESP.
         */
        const uint8_t *getIcon(bool selected) const {
            return (selected && selectedIcon != nullptr) ? selectedIcon : normalIcon;
        }

        /**
         * Get the palette information (if available) for this icon
         * @return either icon palette data if available or nullptr.
         */
        const color_t* getPalette() const {
            return palette;
        }

        /**
         * @return the dimensions of the image
         */
        Coord getDimensions() const {
            return dimensions;
        };

        /**
         * @return the icon type for the image
         */
        IconType getIconType() const {
            return iconType;
        }

        uint16_t getKey() const {
            return menuId;
        }

        void setFromValues(const Coord &size, IconType ty, const uint8_t *normal, const uint8_t *selected = nullptr) {
            this->dimensions = size;
            this->iconType = ty;
            this->normalIcon = normal;
            this->selectedIcon = selected;
        }
    };

} // namespace tcgfx

#endif //TCMENU_DRAWING_PRIMITIVES_H
