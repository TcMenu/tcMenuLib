/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _GFX_MENU_CONFIG_H_
#define _GFX_MENU_CONFIG_H_

#include <tcUtil.h>
#include <SimpleCollections.h>
#include "MenuItems.h"
#include "DrawingPrimitives.h"

/**
 * @file GfxMenuConfig.h
 * 
 * This file contains the base drawing configuration structures and helper methods for
 * drawing onto graphical screens, be it mono or colour. Also there's some additional
 * structures for describing colours, coordinates and padding.
 */

namespace tcgfx {

#define SPECIAL_ID_EDIT_ICON 0xfffe
#define SPECIAL_ID_ACTIVE_ICON 0xfffd

    /**
     * Holds the graphical configuration of how to render a menu onto a both mono and colour displays. If you don't intend
     * to override this initially just call the factory method provided with your renderer.
     */
    template<typename FONTPTR> struct ColorGfxMenuConfig {
        color_t bgTitleColor;
        color_t fgTitleColor;
        MenuPadding titlePadding;
        FONTPTR titleFont;

        color_t bgItemColor;
        color_t fgItemColor;
        MenuPadding itemPadding;
        FONTPTR itemFont;

        color_t bgSelectColor;
        color_t fgSelectColor;
        color_t widgetColor;
        MenuPadding widgetPadding;

        const uint8_t* activeIcon;
        const uint8_t* editIcon;
        uint8_t editIconWidth;
        uint8_t editIconHeight;

        uint8_t titleBottomMargin;
        uint8_t titleFontMagnification;
        uint8_t itemFontMagnification;
    };

    /**
     * @deprecated do not use in new designs use drawing properties instead, may be removed in a future release
     * @param config the config to be filled in.
     */
    void prepareDefaultGfxConfig(ColorGfxMenuConfig<void*>* config);


    /**
    * Provides a platform independent means of identifying where on the screen a particular menu item resides using a
    * simple grid layout
    */
    class GridPosition {
    public:
        /**
         * Represents how the item in this position should be drawn.
         */
        enum GridDrawingMode : uint8_t {
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

        enum GridJustification : uint8_t {
            CORE_JUSTIFY_LEFT = 1,
            CORE_JUSTIFY_RIGHT = 2,
            CORE_JUSTIFY_CENTER = 3,

            CORE_JUSTIFY_VALUE_REQUIRED = 0b1000,
            CORE_JUSTIFY_NAME_REQUIRED  = 0b0100,

            JUSTIFY_TITLE_LEFT_VALUE_RIGHT = 0,
            JUSTIFY_TITLE_LEFT_WITH_VALUE = CORE_JUSTIFY_LEFT + CORE_JUSTIFY_NAME_REQUIRED + CORE_JUSTIFY_VALUE_REQUIRED,
            JUSTIFY_CENTER_WITH_VALUE = CORE_JUSTIFY_CENTER + CORE_JUSTIFY_NAME_REQUIRED + CORE_JUSTIFY_VALUE_REQUIRED,
            JUSTIFY_RIGHT_WITH_VALUE = CORE_JUSTIFY_RIGHT + CORE_JUSTIFY_NAME_REQUIRED + CORE_JUSTIFY_VALUE_REQUIRED,
            JUSTIFY_LEFT_NO_VALUE = CORE_JUSTIFY_LEFT + CORE_JUSTIFY_NAME_REQUIRED,
            JUSTIFY_CENTER_NO_VALUE = CORE_JUSTIFY_CENTER + CORE_JUSTIFY_NAME_REQUIRED,
            JUSTIFY_RIGHT_NO_VALUE = CORE_JUSTIFY_RIGHT + CORE_JUSTIFY_NAME_REQUIRED,
            JUSTIFY_LEFT_VALUE_ONLY= CORE_JUSTIFY_LEFT + CORE_JUSTIFY_VALUE_REQUIRED,
            JUSTIFY_CENTER_VALUE_ONLY = CORE_JUSTIFY_CENTER + CORE_JUSTIFY_VALUE_REQUIRED,
            JUSTIFY_RIGHT_VALUE_ONLY = CORE_JUSTIFY_RIGHT + CORE_JUSTIFY_VALUE_REQUIRED
        };
    private:
        /** the number of columns in the grid */
        uint32_t gridSize: 3;
        /** the position in the columns */
        uint32_t gridPosition: 3;
        /** not yet implemented in any renderers, the number of rows this item takes up. */
        uint32_t gridRowSpan: 2;
        /** the height or 0 if not overridden. Between 0..255*/
        uint32_t gridHeight: 9;
        /** the row ordering for the item */
        uint32_t rowPosition: 7;
        /** the drawing mode that should be used */
        uint32_t drawingMode: 4;
        /** the text justification that should be used */
        uint32_t justification: 4;
    public:
        GridPosition() : gridSize(0), gridPosition(0), gridHeight(0), rowPosition(0), drawingMode(0),
                         justification(JUSTIFY_TITLE_LEFT_VALUE_RIGHT) {}

        GridPosition(const GridPosition &other) = default;
        GridPosition& operator=(const GridPosition &other) = default;

        /**
         * Create a simple grid position that represents a row with a single column with optional override of the row height
         * @param mode the mode in which to draw the item
         * @param height the height of the item or leave blank for default
         */
        GridPosition(GridDrawingMode mode, GridJustification justification, int row, int height = 0)
                : gridSize(1), gridPosition(1), gridHeight(height),rowPosition(row), drawingMode(mode), justification(justification) { }

        /**
         * Create a more complex multi column grid with height, this represents a single row with one or more columns,
         * a position in the columns, and if need be, a height override.
         * @param mode the mode in which to draw the item
         * @param size the number of columns in the row
         * @param pos the column position in the row
         * @param hei the height of the row, or 0 for the default height.
         */
        GridPosition(GridDrawingMode mode, GridJustification just, int size, int pos, int row, int hei)
                : gridSize(size), gridPosition(pos), gridHeight(hei), rowPosition(row), drawingMode(mode), justification(just) { }

        GridDrawingMode getDrawingMode() const { return static_cast<GridDrawingMode>(drawingMode); }

        GridJustification getJustification() const { return static_cast<GridJustification>(justification); }

        int getGridSize() const { return gridSize; }

        int getGridHeight() const { return gridHeight; }

        int getGridPosition() const { return gridPosition; }

        int getRow() const { return rowPosition; }
    };

    inline bool itemNeedsValue(GridPosition::GridJustification justification) {
        return (justification & GridPosition::CORE_JUSTIFY_VALUE_REQUIRED) != 0;
    }

    inline bool itemNeedsName(GridPosition::GridJustification justification) {
        return (justification & GridPosition::CORE_JUSTIFY_NAME_REQUIRED) != 0;
    }

    inline GridPosition::GridJustification coreJustification(GridPosition::GridJustification j) {
        return static_cast<GridPosition::GridJustification>(j & 0b11);
    }

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
        GridPositionWithId() : menuId(0xffff), thePosition(GridPosition::DRAW_TEXTUAL_ITEM,
                                                           GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0) {}

        GridPositionWithId(const GridPositionWithId &other) = default;
        GridPositionWithId& operator=(const GridPositionWithId &other) = default;

        GridPositionWithId(uint16_t itemId, const GridPosition &pos) : menuId(itemId), thePosition(pos) {}

        const GridPosition &getPosition() { return thePosition; }

        uint16_t getKey() const { return menuId; }

        void setNewPosition(GridPosition newPosition) {
            thePosition = newPosition;
        }
    };

    /**
     * Represents the display properties for a menu item, submenu or default, it stores this the properties key which
     * can key at each of those levels. It stores the color palette, padding, row spacing, font information and default
     * justification for rendering.
     */
    class ItemDisplayProperties {
    public:
        /**
         * The sub-component that is being drawn that we need the formatting rules for.
         */
        enum ColorType: uint8_t {
            TEXT,
            BACKGROUND,
            HIGHLIGHT1,
            HIGHLIGHT2,
            SIZEOF_COLOR_ARRAY
        };
        /**
         * The overall component type being rendered
         */
        enum ComponentType {
            COMPTYPE_TITLE,
            COMPTYPE_ITEM,
            COMPTYPE_ACTION,
        };
    private:
        uint32_t propsKey;
        color_t colors[SIZEOF_COLOR_ARRAY];
        MenuPadding padding;
        const void* fontData;
        MenuBorder borderWidths;
        uint8_t fontMagnification: 4;
        uint8_t defaultJustification: 4;
        uint8_t spaceAfter;
        uint8_t requiredHeight;
    public:
        ItemDisplayProperties() : propsKey(0), colors{}, padding(), fontData(nullptr), borderWidths(), fontMagnification(1), defaultJustification(0), spaceAfter(0), requiredHeight(0) {}
        ItemDisplayProperties(uint32_t key, const color_t* palette, const MenuPadding& pad, const void* font, uint8_t mag, uint8_t spacing,
                              uint8_t height, GridPosition::GridJustification defaultJustification, MenuBorder borderWidths)
                              : propsKey(key), padding{pad}, fontData(font), borderWidths(borderWidths), fontMagnification(mag), defaultJustification(defaultJustification),
                                spaceAfter(spacing), requiredHeight(height) {
            memcpy(colors, palette, sizeof colors);
        }
        ItemDisplayProperties(const ItemDisplayProperties& other) : propsKey(other.propsKey), padding{other.padding}, fontData(other.fontData),
                                borderWidths(other.borderWidths), fontMagnification(other.fontMagnification), defaultJustification(other.defaultJustification),
                                spaceAfter(other.spaceAfter), requiredHeight(other.requiredHeight) {
            memcpy(colors, other.colors, sizeof colors);
        }
        ItemDisplayProperties& operator=(const ItemDisplayProperties& other) {
            if(&other == this) return *this;
            propsKey = other.propsKey;
            padding = other.padding;
            fontData = other.fontData;
            borderWidths = other.borderWidths;
            fontMagnification = other.fontMagnification;
            defaultJustification = other.defaultJustification;
            spaceAfter = other.spaceAfter;
            requiredHeight = other.requiredHeight;
            memcpy(colors, other.colors, sizeof colors);
            return *this;
        }

        uint32_t getKey() const { return propsKey; }

        GridPosition::GridJustification getDefaultJustification() const { return (GridPosition::GridJustification)defaultJustification; }

        void setDefaultJustification(GridPosition::GridJustification justification) { defaultJustification = justification; }

        uint8_t getSpaceAfter() const {return spaceAfter; }

        void setSpaceAfter(uint8_t space) { spaceAfter = space; }

        uint8_t getRequiredHeight() const { return requiredHeight; }

        void setRequiredHeight(uint8_t newHeight) { requiredHeight = newHeight; }

        color_t getColor(ColorType color) const {
            return (color < SIZEOF_COLOR_ARRAY) ? colors[color] : RGB(0,0,0);
        }

        void setColor(ColorType color, color_t value) {
            if(color >= SIZEOF_COLOR_ARRAY) return;
            colors[color] = value;
        }

        void setColors(const color_t* palette) {
            memcpy(colors, palette, sizeof colors);
        }

        const MenuPadding& getPadding() const {
            return padding;
        }

        void setPadding(const MenuPadding& pad) {
            padding = pad;
        }

        MenuBorder getBorder() const {
            return borderWidths;
        }

        void setBorder(MenuBorder border) {
            borderWidths = border;
        }

        uint8_t getFontMagnification() const {
            return fontMagnification;
        }

        void setFontInfo(const void* font, uint8_t mag) {
            fontMagnification = mag;
            fontData = font;
        }

        const void* getFont() {
            return fontData;
        }

        color_t* getPalette() {
            return colors;
        }
    };

    /**
     * This factory is responsible for generating all the display configuration settings for the main display, it provides
     * all the grid setting overrides, icons, fonts, padding and colors. This class also provides sensible defaults for
     * when no overrides are present. For GridSettings, it returns null if there is no override, for colors, fonts and padding
     * it checks first by ID, then gets the default if no override exists.
     * be easy to extend by the end user, in order to add additional drawing rules easily. This class will slowly replace
     * the current GfxConfig objects which are quite inflexible.
     */
    class ItemDisplayPropertiesFactory {
    public:
        virtual ItemDisplayProperties* configFor(MenuItem* pItem, ItemDisplayProperties::ComponentType compType) = 0;
        virtual DrawableIcon* iconForMenuItem(uint16_t id) = 0;
        virtual GridPositionWithId* gridPositionForItem(MenuItem* pItem) = 0;
        virtual color_t getSelectedColor(ItemDisplayProperties::ColorType colorType)  = 0;
        virtual void addGridPosition(MenuItem* item, const GridPosition& position)  = 0;
    };

    /**
     * Used by the BaseGraphicalRenderer class to work out how to draw an item, which grid position it is in etc. This
     * version does not implement icons, nor does it implement overriding of display properties, it is intended for LCDs.
     *
     * This is used by non graphical displays to allow grid configurations while not providing the complex support for
     * display properties at different levels. This makes the class much simpler and should use far less memory on devices.
     */
    class NullItemDisplayPropertiesFactory : public ItemDisplayPropertiesFactory {
    private:
        ItemDisplayProperties props = ItemDisplayProperties(0, {}, MenuPadding(0), nullptr, 1, 0, 1, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder());
        BtreeList<uint16_t, GridPositionWithId> gridByItem;
    public:
        NullItemDisplayPropertiesFactory() : gridByItem(4) {}

        ItemDisplayProperties* configFor(MenuItem* pItem, ItemDisplayProperties::ComponentType compType) override {
            return &props;
        }

        DrawableIcon* iconForMenuItem(uint16_t id) override{
            return nullptr;
        }

        color_t getSelectedColor(ItemDisplayProperties::ColorType colorType) override {
            return 0;
        }

        GridPositionWithId* gridPositionForItem(MenuItem* pItem) override {
            if(!pItem) return nullptr;
            return gridByItem.getByKey(pItem->getId());
        }

        void addGridPosition(MenuItem* pItem, const GridPosition& position) override {
            if(!pItem) return;
            auto* grid = gridByItem.getByKey(pItem->getId());
            if(grid) {
                grid->setNewPosition(position);
            }
            else {
                serdebugF2("Adding grid ", pItem->getId());
                gridByItem.add(GridPositionWithId(pItem->getId(), position));
            }
        }
    };

#define MENUID_NOTSET 0xffff

    inline uint32_t MakePropsKey(uint16_t menuId, bool parentKey, ItemDisplayProperties::ComponentType ty) {
        return (uint32_t)menuId | (parentKey ? 0x10000UL : 0UL) | ((uint32_t)ty << 18UL);
    }

    /**
     * Provides full support for configurability of menu items, in terms of the their grid position and also any associated
     * icons and drawing color / font / padding overrides. Each time the renderer sets up a new menu, it calls into here
     * for each item to find out what settings to use for drawing. It is therefore possible to adjust settings either globally,
     * by sub menu, or for a given menu item. This class also stores icon definitions by menu item, so any items that are
     * set to draw as icons will look in that cache.
     */
    class ConfigurableItemDisplayPropertiesFactory : public ItemDisplayPropertiesFactory {
    private:
        BtreeList<uint32_t, ItemDisplayProperties> displayProperties;
        BtreeList<uint16_t, DrawableIcon> iconsByItem;
        BtreeList<uint16_t, GridPositionWithId> gridByItem;
        color_t selectedTextColor = RGB(255,255,255);
        color_t selectedBackgroundColor = RGB(0, 0, 255);
    public:
        ConfigurableItemDisplayPropertiesFactory()
                : displayProperties(5, GROW_BY_5),
                  iconsByItem(6, GROW_BY_5)
                  { }
        DrawableIcon* iconForMenuItem(uint16_t id) override {
            return iconsByItem.getByKey(id);
        }

        GridPositionWithId* gridPositionForItem(MenuItem* pItem) override {
            if(!pItem) return nullptr;
            return gridByItem.getByKey(pItem->getId());
        }

        ItemDisplayProperties* configFor(MenuItem* pItem, ItemDisplayProperties::ComponentType compType)  override;

        color_t getSelectedColor(ItemDisplayProperties::ColorType colorType) override {
            return colorType == ItemDisplayProperties::BACKGROUND ? selectedBackgroundColor : selectedTextColor;
        }

        void addGridPosition(MenuItem* pItem, const GridPosition& position) override {
            if(!pItem) return;
            auto* grid = gridByItem.getByKey(pItem->getId());
            if(grid) {
                grid->setNewPosition(position);
            }
            else {
                gridByItem.add(GridPositionWithId(pItem->getId(), position));
            }
        }

        void addImageToCache(const DrawableIcon& toAdd) {
            auto* current = iconsByItem.getByKey(toAdd.getKey());
            if(current) {
                current->setFromValues(toAdd.getDimensions(), toAdd.getIconType(), toAdd.getIcon(false), toAdd.getIcon(true));
            }
            else {
                iconsByItem.add(toAdd);
            }
        }

        void setDrawingPropertiesDefault(ItemDisplayProperties::ComponentType drawing, const color_t* palette, MenuPadding pad, const void *font, uint8_t mag,
                                         uint8_t spacing, uint8_t requiredHeight, GridPosition::GridJustification defaultJustification, MenuBorder border) {
            setDrawingProperties(MakePropsKey(MENUID_NOTSET, false, drawing), palette, pad, font, mag, spacing, requiredHeight, defaultJustification, border);
        }

        void setDrawingPropertiesForItem(ItemDisplayProperties::ComponentType drawing, uint16_t id, const color_t* palette, MenuPadding pad, const void *font, uint8_t mag,
                                         uint8_t spacing, uint8_t requiredHeight, GridPosition::GridJustification defaultJustification, MenuBorder border) {
            setDrawingProperties(MakePropsKey(id, false, drawing), palette, pad, font, mag, spacing, requiredHeight, defaultJustification, border);
        }

        void setDrawingPropertiesAllInSub(ItemDisplayProperties::ComponentType drawing, uint16_t id, const color_t* palette, MenuPadding pad, const void *font, uint8_t mag,
                                          uint8_t spacing, uint8_t requiredHeight, GridPosition::GridJustification defaultJustification, MenuBorder border) {
            setDrawingProperties(MakePropsKey(id, true, drawing), palette, pad, font, mag, spacing, requiredHeight, defaultJustification, border);
        }

        void setDrawingProperties(uint32_t key, const color_t* palette, MenuPadding pad, const void* font, uint8_t mag, uint8_t spacing,
                                  uint8_t requiredHeight, GridPosition::GridJustification defaultJustification, MenuBorder border);

        void setSelectedColors(color_t background, color_t text) {
            selectedBackgroundColor = background;
            selectedTextColor = text;
        }

        static void refreshCache();
    };

} // namespace tcgfx

/**
 * The default editing icon for approx 100-150 dpi resolution displays
 */
extern const uint8_t PROGMEM loResEditingIcon[];

/**
 * The default active icon for approx 100-150 dpi resolution displays
 */
extern const uint8_t PROGMEM loResActiveIcon[];

/**
 * The low resolution icon for indicating active status
 */
extern const uint8_t PROGMEM defActiveIcon[];

/**
 * The low resolution icon for editing status
 */
extern const uint8_t PROGMEM defEditingIcon[];

#endif // _GFX_MENU_CONFIG_H_
