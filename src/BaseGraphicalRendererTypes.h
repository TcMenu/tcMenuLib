/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_GRAPHICALRENDERERTYPES_H
#define TCMENU_GRAPHICALRENDERERTYPES_H

/**
 * this macro creates an RGB color based on R, G, B values between 0 and 255
 * regardless of actual color space available.
 */
#define RGB(r, g, b) (uint16_t)( (((r)>>3)<<11) | (((g)>>2)<<5) | ((b)>>3) )

/**
 * Defines padding for menu rendering when using the standard AdaGfx renderer. Each
 * position can hold the value 0..15
 */
struct MenuPadding {
    uint16_t top: 4;
    uint16_t right : 4;
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

/** A structure that holds both X and Y direction in a single 32 bit integer. Both x and y are public */
struct Coord {
    /**
     * Create a coord based on an X and Y location
     * @param x the x location
     * @param y the y location
     */
    Coord(int x, int y) {
        this->x = x;
        this->y = y;
    }

    Coord(const Coord& other) {
        this->x = other.x;
        this->y = other.y;
    }

    int32_t x:15;
    int32_t y:15;
};

/**
 * Provides a platform independent means of identifying where on the screen a particular menu item resides using a
 * simple grid layout
 */
class GridPosition {
public:
    /**
     * Represents how the item in this position should be drawn.
     */
    enum GridDrawingMode: byte {
        /** Drawn as text in the form of Item on the left, value on the right */
        DRAW_TEXTUAL_ITEM,
        /** Drawn two buttons with the value in the middle, to move through a range of values */
        DRAW_INTEGER_AS_UP_DOWN,
        /** Drawn as a scroll slider, to move through a series of values */
        DRAW_INTEGER_AS_SCROLL,
        /** Drawn as an icon with no text underneath */
        DRAW_AS_ICON_ONLY,
        /** Drawn as an icon with text underneath  */
        DRAW_AS_ICON_TEXT,
        /** Drawn as a title, usually reserved for the title at the top of the page */
        DRAW_TITLE_ITEM
    };

    enum GridJustification: byte {
        JUSTIFY_TITLE_LEFT_VALUE_RIGHT,
        JUSTIFY_TITLE_LEFT_WITH_VALUE,
        JUSTIFY_CENTER_WITH_VALUE,
        JUSTIFY_RIGHT_WITH_VALUE,
        JUSTIFY_LEFT_NO_VALUE,
        JUSTIFY_CENTER_NO_VALUE,
        JUSTIFY_RIGHT_NO_VALUE
    };
private:
    /** the number of columns in the grid */
    uint32_t gridSize: 4;
    /** the position in the columns */
    uint32_t gridPosition: 4;
    /** the height or 0 if not overridden. Between 0..255*/
    uint32_t gridHeight: 8;
    /** the row ordering for the item */
    uint32_t rowPosition: 8;
    /** the drawing mode that should be used */
    uint32_t drawingMode: 5;
    /** the text justification that should be used */
    uint32_t justification: 3;
public:
    GridPosition() : gridSize(0), gridPosition(0), gridHeight(0), rowPosition(0), drawingMode(0),
                     justification(JUSTIFY_TITLE_LEFT_VALUE_RIGHT) { }

    GridPosition(const GridPosition& other) = default;

    /**
     * Create a simple grid position that represents a row with a single column with optional override of the row height
     * @param mode the mode in which to draw the item
     * @param height the height of the item or leave blank for default
     */
    GridPosition(GridDrawingMode mode, GridJustification justification, int row, int height = 0) : gridSize(1), gridPosition(1),
                    gridHeight(height),rowPosition(row), drawingMode(mode), justification(justification) { }

    /**
     * Create a more complex multi column grid with height, this represents a single row with one or more columns,
     * a position in the columns, and if need be, a height override.
     * @param mode the mode in which to draw the item
     * @param size the number of columns in the row
     * @param pos the column position in the row
     * @param hei the height of the row, or 0 for the default height.
     */
    GridPosition(GridDrawingMode mode, GridJustification just, int size, int pos, int row, int hei) : gridSize(size),
                                                                                                      gridPosition(pos), gridHeight(hei), rowPosition(row), drawingMode(mode), justification(just) { }

    GridDrawingMode getDrawingMode() const { return static_cast<GridDrawingMode>(drawingMode); }
    GridJustification getJustification() const { return static_cast<GridJustification>(justification); }
    int getGridSize() const { return gridSize; }
    int getGridHeight() const { return gridHeight; }
    int getGridPosition() const { return gridPosition; }
    int getRow() const { return rowPosition; }
};

/**
 * Represents a grid position along with the menuID and also the drawable icon, if one exists. It is stored in the list
 * of drawing instructions in the base graphical renderer, and then read when a new menu is displayed in order to reorder
 * and position the items before rendering, these instructions are quite display neutral. It is able to be stored within
 * the simple BtreeList because it implements getKey returning uint16_t
 */
class GridPositionWithId {
private:
    uint16_t menuId;
    GridPosition thePosition;
public:
    GridPositionWithId() : menuId(0xffff), thePosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0) {}
    GridPositionWithId(const GridPositionWithId& other) = default;
    GridPositionWithId(uint16_t itemId, const GridPosition& pos) : menuId(itemId), thePosition(pos) {}

    const GridPosition& getPosition() { return thePosition; }
    uint16_t getKey() const { return menuId; }

    void setNewPosition(GridPosition newPosition) {
        thePosition = newPosition;
    }
};

/**
 * Represents an icon that can be presented for actionable menu items such as submenus, boolean items, and action items.
 * It can have two items states, one for selected and one for normal.
 */
class DrawableIcon {
public:
    enum IconType {
        ICON_XBITMAP, ICON_MONO, ICON_PALLETE4, ICON_NATIVE
    };
private:
    uint16_t menuId;
    Coord dimensions;
    IconType iconType;
    const uint8_t* normalIcon;
    const uint8_t* selectedIcon;

public:
    /**
     * Creates an empty drawable icon, used mainly by collection support
     */
    DrawableIcon() : menuId(0), dimensions(0,0), iconType(ICON_XBITMAP), normalIcon(nullptr), selectedIcon(nullptr) {}

    /**
     * Copy constructor to copy an existing drawable icon
     */
    DrawableIcon(const DrawableIcon& other) : menuId(other.menuId), dimensions(other.dimensions), iconType(other.iconType),
                                              normalIcon(other.normalIcon), selectedIcon(other.selectedIcon) {}

    /**
     * Create a drawable icon providing the size, icon type, and image data
     * @param id the menu id that this icon belongs to
     * @param size the size of the image, better to be in whole byte sizes
     * @param ty the type of the image, so the renderer knows what to do with it.
     * @param normal the image in the normal state.
     * @param selected the image in the selected state.
     */
    DrawableIcon(uint16_t id, const Coord& size, IconType ty, const uint8_t* normal, const uint8_t* selected = nullptr)
            : menuId(id), dimensions(size), iconType(ty), normalIcon(normal), selectedIcon(selected) { }

    /**
     * Get the icon data for the current state
     * @param selected true if the item is selected (IE pressed or ON for booleans)
     * @return the icon data, which may be in program memory on AVR and ESP.
     */
    const uint8_t* getIcon(bool selected) const {
        return (selected && selectedIcon != nullptr) ? selectedIcon : normalIcon;
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
};


#endif //TCMENU_GRAPHICALRENDERERTYPES_H
