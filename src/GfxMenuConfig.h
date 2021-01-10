#ifndef _GFX_MENU_CONFIG_H_
#define _GFX_MENU_CONFIG_H_

#include <tcUtil.h>
#include <SimpleCollections.h>
#include "MenuItems.h"
#include "BaseGraphicalRendererTypes.h"

/**
 * @file GfxMenuConfig.h
 * 
 * This file contains the base drawing configuration structures and helper methods for
 * drawing onto graphical screens, be it mono or colour. Also there's some additional
 * structures for describing colours, coordinates and padding.
 */

/**
 * Defines a basic color type that can be used with RGB() macro
 * regardless of the color depth.
 */
typedef uint16_t color_t;

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
    uint8_t fontMagnification: 4;
    uint8_t defaultJustification: 4;
    uint8_t spaceAfter;
    uint8_t requiredHeight;
public:
    ItemDisplayProperties() : propsKey(0), colors{}, padding(), fontData(nullptr), fontMagnification(1), defaultJustification(0), spaceAfter(0), requiredHeight(0) {}
    ItemDisplayProperties(uint32_t key, color_t* palette, const MenuPadding& pad, const void* font, uint8_t mag, uint8_t spacing,
                          uint8_t height, GridPosition::GridJustification defaultJustification)
                          : propsKey(key), padding{pad}, fontData(font), fontMagnification(mag), defaultJustification(defaultJustification),
                            spaceAfter(spacing), requiredHeight(height) {
        memcpy(colors, palette, sizeof colors);
    }
    ItemDisplayProperties(const ItemDisplayProperties& other) : padding{other.padding}, fontData(other.fontData),
                            fontMagnification(other.fontMagnification), defaultJustification(other.defaultJustification),
                            spaceAfter(other.spaceAfter), requiredHeight(other.requiredHeight) {
        memcpy(colors, other.colors, sizeof colors);
    }

    uint32_t getKey() const { return propsKey; }

    GridPosition::GridJustification getDefaultJustification() { return (GridPosition::GridJustification)defaultJustification; }

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

    void setColors(color_t* palette) {
        memcpy(colors, palette, sizeof colors);
    }

    const MenuPadding& getPadding() const {
        return padding;
    }

    void setPadding(const MenuPadding& pad) {
        padding = pad;
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
};

class NullItemDisplayPropertiesFactory : public ItemDisplayPropertiesFactory {
private:
    ItemDisplayProperties props = ItemDisplayProperties(0, {}, MenuPadding(0), nullptr, 1, 0, 1, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT);
    BtreeList<uint16_t, GridPositionWithId> gridByItem;
public:
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

    void addGridPosition(MenuItem* pItem, const GridPosition& position) {
        if(!pItem) return;
        gridByItem.add(GridPositionWithId(pItem->getId(), position));
    }
};

#define MENUID_NOTSET 0xffff

inline uint32_t MakePropsKey(uint16_t menuId, bool parentKey, ItemDisplayProperties::ComponentType ty) {
    return (uint32_t)menuId | (parentKey ? 0x10000UL : 0UL) | ((uint32_t)ty << 18UL);
}

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

    void addGridPosition(MenuItem* pItem, const GridPosition& position) {
        if(!pItem) return;
        gridByItem.add(GridPositionWithId(pItem->getId(), position));
    }

    void addImageToCache(const DrawableIcon& toAdd) {
        iconsByItem.add(toAdd);
    }

    void setDrawingPropertiesDefault(ItemDisplayProperties::ComponentType drawing, color_t* palette, MenuPadding pad, const void *font,
                                     uint8_t mag,uint8_t spacing, uint8_t requiredHeight, GridPosition::GridJustification defaultJustification) {
        setDrawingProperties(MakePropsKey(MENUID_NOTSET, false, drawing), palette, pad, font, mag, spacing, requiredHeight, defaultJustification);
    }

    void setDrawingPropertiesForItem(ItemDisplayProperties::ComponentType drawing, uint16_t id, color_t* palette, MenuPadding pad, const void *font,
                                     uint8_t mag,uint8_t spacing, uint8_t requiredHeight, GridPosition::GridJustification defaultJustification) {
        setDrawingProperties(MakePropsKey(id, false, drawing), palette, pad, font, mag, spacing, requiredHeight, defaultJustification);
    }

    void setDrawingPropertiesAllInSub(ItemDisplayProperties::ComponentType drawing, uint16_t id, color_t* palette, MenuPadding pad, const void *font,
                                      uint8_t mag, uint8_t spacing, uint8_t requiredHeight, GridPosition::GridJustification defaultJustification) {
        setDrawingProperties(MakePropsKey(id, true, drawing), palette, pad, font, mag, spacing, requiredHeight, defaultJustification);
    }

    void setDrawingProperties(uint32_t key, color_t* palette, MenuPadding pad, const void* font, uint8_t mag, uint8_t spacing,
                              uint8_t requiredHeight, GridPosition::GridJustification defaultJustification);

    void setSelectedColors(color_t background, color_t text) {
        selectedBackgroundColor = background;
        selectedTextColor = text;
    }
};

/**
 * This is an internal method, used by display specific plugins. Prefer to use the
 * mehtod shipped with the plugin. This is for higher resolution colour displays.
 */
void prepareDefaultGfxConfig(ColorGfxMenuConfig<void*>* config);

/**
 * The default editing icon for approx 100-150 dpi resolution displays 
 */
extern const unsigned char PROGMEM loResEditingIcon[];

/**
 * The default active icon for approx 100-150 dpi resolution displays 
 */
extern const unsigned char PROGMEM loResActiveIcon[];

/**
 * The low resolution icon for indicating active status
 */
extern const unsigned char PROGMEM defActiveIcon[];

/**
 * The low resolution icon for editing status
 */
extern const unsigned char PROGMEM defEditingIcon[];

#endif // _GFX_MENU_CONFIG_H_
