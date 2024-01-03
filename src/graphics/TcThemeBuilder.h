/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file TcDrawableButton.h
 * @brief a theme builder that simplifies the creation of themes for GraphicsDeviceRenderer based displays.
 */

#ifndef TCCLIBSDK_TCTHEMEBUILDER_H
#define TCCLIBSDK_TCTHEMEBUILDER_H

#include "GfxMenuConfig.h"
#include <graphics/GraphicsDeviceRenderer.h>

namespace tcgfx {
    class TcThemeBuilder;

    /**
     * Represents a set of theme properties that can be applied at any level, if at the item level, this class can also
     * create grid layout. You normally don't create these directly, instead one is normally provided from builder, using
     * the various properties/override methods. This class uses builder syntax, so most methods other than `apply` can
     * be chained together as the methods return a reference to the same object.
     */
    class ThemePropertiesBuilder {
    public:
        enum ThemeLevel : uint8_t {
            THEME_GLOBAL, THEME_SUB, THEME_ITEM, THEME_ITEM_NEEDS_PROPS, THEME_ITEM_NEEDS_GRID, THEME_ITEM_NEEDS_BOTH
        };
    private:
        color_t palette[4]{};
        MenuPadding padding;
        const void* fontData{};
        GridPosition::GridJustification justification;
        MenuBorder border{};
        TcThemeBuilder *themeBuilder{};
        MenuItem* menuItem = nullptr;
        uint16_t gridHeight;
        ThemeLevel currentLevel;
        ItemDisplayProperties::ComponentType componentType;
        uint8_t fontMag{};
        uint8_t spacing{};
        GridPosition::GridDrawingMode drawingMode{};
        uint8_t row;
        uint8_t colData;
    public:
        /**
         * Sets the border to something other than the default
         * @param b the border
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withBorder(const MenuBorder& b) {
            border = b;
            needsProps();
            return *this;
        }

        /**
         * Sets the font to be an adafruit font
         * @param font the font to use
         * @param mag an optional magnification (default 1)
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withAdaFont(const GFXfont* font, int mag = 1) {
            fontData = font;
            fontMag = min(1, mag);
            needsProps();
            return *this;
        }

        /**
         * Sets the font to be a tcUnicode font
         * @param font the font to use
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withTcUnicodeFont(const UnicodeFont* font) {
            fontData = font;
            fontMag = 0;
            needsProps();
            return *this;
        }

        /**
         * Sets the font to be a native font, IE any font whatsoever
         * @param font the font to use
         * @param mag the magnification value
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withNativeFont(void* data, uint8_t mag) {
            fontData = data;
            fontMag = mag;
            needsProps();
            return *this;
        }

        /**
         * Sets the padding to a custom value
         * @param p the padding to apply
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withPadding(MenuPadding p) {
            padding = p;
            needsProps();
            return *this;
        }

        /**
         * Sets the spacing to a custom value
         * @param s the spacing to apply
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withSpacing(uint8_t s) {
            spacing = s;
            needsGrid(true);
            return *this;
        }

        /**
         * Sets the justification to be a custom value
         * @param j the justification to apply
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withJustification(GridPosition::GridJustification j) {
            justification = j;
            needsGrid(true);
            return *this;
        }

        /**
         * Sets the palette to a custom value, other than the default
         * @param p the padding to apply
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withPalette(const color_t* p) {
            needsProps();
            memcpy(palette, p, sizeof(palette));
            return *this;
        }

        /**
         * Sets the row height directly for a given menu item. Only applicable at the menu item level, and only needed
         * if you don't want to use calculated heights.
         * @param height the height to apply
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withRowHeight(uint16_t height) {
            gridHeight = height;
            needsGrid(false);
            return *this;
        }

        /**
         * For item level use only, this sets the menu item to render as an image, instead of as text.
         * @param size size of the images provided
         * @param regIcon the icon when not active / edited
         * @param selIcon the icon when selected, IE active / edited
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withImageXbmp(Coord size, const uint8_t* regIcon, const uint8_t* selIcon = nullptr);


        /**
         * Sets the drawing mode to use that overrides the default
         * @param dm the drawing mode
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withDrawingMode(GridPosition::GridDrawingMode dm) {
            drawingMode = dm;
            needsGrid(false);
            return *this;
        }

        /**
         * Sets the row, number of columns on the row, and column number. Note that each entry on the row must properly
         * set the same number of columns.
         * @param actualRow the row where the item should appear
         * @param numberOfCols the number of columns across
         * @param column the column number
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& onRowCol(uint8_t actualRow, uint8_t numberOfCols, uint8_t column) {
            row = actualRow;
            colData = (numberOfCols << 4) & column;
            return *this;
        }

        /**
         * Sets the row that an item should appear on, used when there is a single column on the row.
         * @param actualRow the row number
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& onRow(uint8_t actualRow) {
            colData = 0x10;
            row = actualRow;
            return *this;
        }

        /**
         * Call this after all other settings have been configured, this will actually put the newly created properties
         * into the factory for later use. Important: Without calling this the changes will not be applied.
         */
        void apply();

    protected:
        void initForLevel(TcThemeBuilder *b, ItemDisplayProperties::ComponentType compType, ThemeLevel level, MenuItem *item = nullptr);

        void needsProps() {
            if(currentLevel == THEME_SUB || currentLevel == THEME_GLOBAL) return;
            if(currentLevel == THEME_ITEM) currentLevel = THEME_ITEM_NEEDS_PROPS;
            else currentLevel = THEME_ITEM_NEEDS_BOTH;
        }

        void needsGrid(bool propsOk) {
            if(currentLevel == THEME_SUB || currentLevel == THEME_GLOBAL) return;
            if(currentLevel == THEME_ITEM_NEEDS_PROPS && propsOk) return;
            if(currentLevel == THEME_ITEM) currentLevel = THEME_ITEM_NEEDS_GRID;
            else currentLevel = THEME_ITEM_NEEDS_BOTH;
        }

        friend TcThemeBuilder;
    };

    /**
     * A theme builder class that is used to simply create themes for tcMenu GraphicalDisplayRenderer class. It allows
     * most graphical configurations to be applied without the complexities of directly using the item display factory.
     * This class uses a builder approach where most methods can be chained, because they return references to this
     * class.
     */
    class TcThemeBuilder {
    private:
        GraphicsDeviceRenderer &renderer;
        ConfigurableItemDisplayPropertiesFactory &factory;
        ThemePropertiesBuilder propertiesBuilder;
        color_t defaultPalette[4] = {};
        MenuPadding globalItemPadding = MenuPadding(1);
        MenuPadding globalTitlePadding = MenuPadding(2);
        const void *fontData = nullptr;
        uint8_t fontMag = 1;
        uint8_t defaultSpacing = 1;

    public:
        /**
         * Creates a theme builder by providing the renderer to use with it.
         * @param renderer the renderer to use, must be a GraphicsDeviceRenderer
         */
        explicit TcThemeBuilder(GraphicsDeviceRenderer& renderer) : renderer(renderer), factory(renderer.getGraphicsPropertiesFactory()) {
            auto *drawable = renderer.getDeviceDrawable();
            renderer.setDisplayDimensions(drawable->getDisplayDimensions().x, drawable->getDisplayDimensions().y);
        }

        /**
         * Set the colours to be used for the background and foreground when an item is selected.
         * @param bg the background color
         * @param fg the foreground color
         * @return reference to itself for chaining
         */
        TcThemeBuilder &withSelectedColors(color_t bg, color_t fg) {
            factory.setSelectedColors(bg, fg);
            return *this;
        }

        /**
         * Configure the default font as an adafruit font
         * @param font the adafruit font
         * @param mag the magnification factor - defaults to 1.
         * @return reference to itself for chaining
         */
        TcThemeBuilder& withAdaFont(const GFXfont* font, int mag = 1) {
            fontData = font;
            fontMag = min(1, mag);
            return *this;
        }

        /**
         * Configure the default font as a tcUnicode font
         * @param font the tcUnicode font
         * @return reference to itself for chaining
         */
        TcThemeBuilder& withTcUnicodeFont(const UnicodeFont* font) {
            fontData = font;
            fontMag = 0;
            return *this;
        }

        /**
         * Configure the default font as a native font
         * @param font the native font
         * @param mag the native mag value
         * @return reference to itself for chaining
         */
        TcThemeBuilder& withNativeFont(void* data, uint8_t mag) {
            fontData = data;
            fontMag = mag;
            return *this;
        }

        /**
         * Provide custom cursor icons that are used when an item is selected or edited.
         * @param size the size of the icons
         * @param editIcon the edit icon in XBM form
         * @param activeIcon the active icon in XBM form
         * @return reference to itself for chaining
         */
        TcThemeBuilder &withCursorIconsXbmp(Coord size, const uint8_t *editIcon, const uint8_t *activeIcon);

        /**
         * Use the standard lower resolution cursor icons suitable for OLED and similar resolution displays
         * @return reference to itself for chaining
         */
        TcThemeBuilder &withStandardLowResCursorIcons();

        /**
         * Use the standard medium resolution cursor icons suitable for larger TFT and higher resolution displays
         * @return reference to itself for chaining
         */
        TcThemeBuilder &withStandardMedResCursorIcons();

        /**
         * Set the standard spacing that will be used by default unless overridden
         * @param spacing the standard item spacing
         * @return reference to itself for chaining
         */
        TcThemeBuilder &withSpacing(uint8_t spacing) {
            defaultSpacing = spacing;
            return *this;
        }

        /**
         * Set the item padding that will be used by default, unless overridden
         * @param padding the padding to apply
         * @return reference to itself for chaining
         */
        TcThemeBuilder &withItemPadding(MenuPadding padding) {
            globalItemPadding = padding;
            return *this;
        }

        /**
         * Set the title spacing that will be used by default, unless overridden
         * @param padding the padding to apply
         * @return reference to itself for chaining
         */
        TcThemeBuilder &withTitlePadding(MenuPadding padding) {
            globalTitlePadding = padding;
            return *this;
        }

        /**
         * Get access to default theme properties for regular items.
         * @return a theme properties builder
         */
        ThemePropertiesBuilder &defaultItemProperties() {
            propertiesBuilder.initForLevel(this, ItemDisplayProperties::COMPTYPE_ITEM, ThemePropertiesBuilder::THEME_GLOBAL);
            return propertiesBuilder;
        }

        /**
         * Get access to default theme properties for title items.
         * @return a theme properties builder
         */
        ThemePropertiesBuilder &defaultTitleProperties() {
            propertiesBuilder.initForLevel(this, ItemDisplayProperties::COMPTYPE_TITLE, ThemePropertiesBuilder::THEME_GLOBAL);
            return propertiesBuilder;
        }

        /**
         * Get access to default theme properties for action items.
         * @return a theme properties builder
         */
        ThemePropertiesBuilder &defaultActionProperties() {
            propertiesBuilder.initForLevel(this, ItemDisplayProperties::COMPTYPE_ACTION, ThemePropertiesBuilder::THEME_GLOBAL);
            return propertiesBuilder;
        }

        /**
         * Get access to theme properties for a specific menu item, with this you can change how an item renders, and
         * even apply different grid settings.
         * @return a theme properties builder
         */
        ThemePropertiesBuilder &menuItemOverride(MenuItem &item);

        /**
         * Get access to theme properties for a sub menu, with this you can change how all items in that submenu render
         * This applies to regular items
         * @param item the sub menu item
         * @return a theme properties builder
         */
        ThemePropertiesBuilder &submenuPropertiesItemOverride(SubMenuItem &item) {
            propertiesBuilder.initForLevel(this, ItemDisplayProperties::COMPTYPE_ITEM, ThemePropertiesBuilder::THEME_SUB, &item);
            return propertiesBuilder;
        }

        /**
         * Get access to theme properties for a sub menu, with this you can change how all items in that submenu render
         * This applies to action items
         * @param item the sub menu item
         * @return a theme properties builder
         */
        ThemePropertiesBuilder &submenuPropertiesActionOverride(SubMenuItem &item) {
            propertiesBuilder.initForLevel(this, ItemDisplayProperties::COMPTYPE_ACTION, ThemePropertiesBuilder::THEME_SUB, &item);
            return propertiesBuilder;
        }

        /**
         * Get access to theme properties for a sub menu, with this you can change how all items in that submenu render
         * This applies to title items
         * @param item the sub menu item
         * @return a theme properties builder
         */
        ThemePropertiesBuilder &submenuPropertiesTitleOverride(SubMenuItem &item) {
            propertiesBuilder.initForLevel(this, ItemDisplayProperties::COMPTYPE_TITLE, ThemePropertiesBuilder::THEME_SUB, &item);
            return propertiesBuilder;
        }

        /**
         * Set the default palette that will be used unless overrridden
         * @param cols the colors to use, must be 4 entry array.
         * @return reference to itself for chaining
         */
        TcThemeBuilder& withPalette(const color_t* cols);

        /**
         * Use this to enable card layout for the root menu,  and configure the icons that will be used for left and
         * right buttons.
         * @param iconSize the size of the left and right icons
         * @param leftIcon the icon for left
         * @param rightIcon the icon for right
         * @param isMono true if mono, otherwise false
         * @return reference to itself for chaining
         */
        TcThemeBuilder& enableCardLayoutWithXbmImages(Coord iconSize, const uint8_t* leftIcon, const uint8_t* rightIcon, bool isMono);
        /**
         * Toggle card layout on and off for a given submenu.
         * @param item the sub menu to turn on/off
         * @param on the status for that submenu
         * @return reference to itself for chaining
         */
        TcThemeBuilder& setMenuAsCard(SubMenuItem& item, bool on);

        /**
         * Call after you've finished configuring your theme, this forces a refresh and ensures it presents properly.
         */
        void apply();

        ConfigurableItemDisplayPropertiesFactory& getItemFactory() { return factory; }

        MenuPadding getPaddingFor(ItemDisplayProperties::ComponentType type) {
            return type == ItemDisplayProperties::COMPTYPE_TITLE ? globalTitlePadding : globalItemPadding;
        }

        uint8_t getDefaultSpacing() const {
            return defaultSpacing;
        }

        GraphicsDeviceRenderer& getRenderer() {
            return renderer;
        }

        const color_t* getDefaultPalette() {
            return defaultPalette;
        }

        const void* getDefaultFontData() {
            return fontData;
        }

        uint8_t getDefaultFontMag() const {
            return fontMag;
        }
    };

}
#endif //TCCLIBSDK_TCTHEMEBUILDER_H
