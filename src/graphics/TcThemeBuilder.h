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

#define COL_COUNT_FLAG_BULK_MODE 0x80

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
        // do not reorder these fields, they are ordered this way to reduce the size, IE the packing overhead.
        TcThemeBuilder *themeBuilder;
        color_t palette[4]{};
        const void* fontData{};
        MenuItem* menuItem = nullptr;
        MenuPadding padding {};
        MenuBorder border{};
        uint16_t gridHeight {};
        ItemDisplayProperties::ComponentType componentType = ItemDisplayProperties::COMPTYPE_ITEM;
        ThemeLevel currentLevel = THEME_GLOBAL;
        uint8_t fontMag{};
        uint8_t spacing{};
        GridPosition::GridDrawingMode drawingMode{};
        GridPosition::GridJustification justification = GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT;
        uint8_t row {};
        uint8_t colPos {};
        uint8_t colCount {};
    public:
        explicit ThemePropertiesBuilder(TcThemeBuilder *themeBuilder): themeBuilder(themeBuilder) {}

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
         * A somewhat internal method that allows for all withImage.. calls to be simplified.
         */
        ThemePropertiesBuilder& withImageOfType(Coord size, DrawableIcon::IconType iconType, const uint8_t* regIcon,
                                                const uint8_t* selIcon, const color_t* palette = nullptr);

        /**
         * For item level use only, this sets the menu item to render as an image, instead of as text. Xbmp format is a
         * byte packed array in LSB first order. IE (0,0) is MSB of byte 1.
         * @param size size of the images provided
         * @param regIcon the icon when not active / edited
         * @param selIcon the icon when selected, IE active / edited
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withImageXbmp(Coord size, const uint8_t* regIcon, const uint8_t* selIcon = nullptr) {
            withImageOfType(size, DrawableIcon::ICON_XBITMAP, regIcon, selIcon);
            return *this;
        }

        /**
         * For item level use only, this sets the menu item to render as an image with 2 bits per pixel, that is the
         * each color index defined by each 2 bits maps to en entry in the palette.
         * @param size size of the images provided
         * @param palette the palette for each color index, must be color_t array of size 4
         * @param regIcon the icon when not active / edited
         * @param selIcon the icon when selected, IE active / edited
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withImage2bpp(Coord size, const color_t* imgPalette, const uint8_t* regIcon, const uint8_t* selIcon = nullptr) {
            withImageOfType(size, DrawableIcon::ICON_PALLETE_2BPP, regIcon, selIcon, imgPalette);
            return *this;
        }

        /**
         * For item level use only, this sets the menu item to render as an image with 4 bits per pixel, that is the
         * each color index defined by each 4 bits maps to en entry in the palette.
         * @param size size of the images provided
         * @param palette the palette for each color index, must be color_t array of size 16
         * @param regIcon the icon when not active / edited
         * @param selIcon the icon when selected, IE active / edited
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withImage4bpp(Coord size, const color_t* imgPalette, const uint8_t* regIcon, const uint8_t* selIcon = nullptr) {
            withImageOfType(size, DrawableIcon::ICON_PALLETE_4BPP, regIcon, selIcon, imgPalette);
            return *this;
        }

        /**
         * For item level use only, this sets the menu item to render as a mono bitmap, instead of as text. Mono bitmaps
         * are byte packed arrays in MSB first order. IE (0,0) is LSB of byte 1.
         * @param size size of the images provided
         * @param regIcon the icon when not active / edited
         * @param selIcon the icon when selected, IE active / edited
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& withMonoBitmap(Coord size, const uint8_t* regIcon, const uint8_t* selIcon = nullptr) {
            withImageOfType(size, DrawableIcon::ICON_MONO, regIcon, selIcon);
            return *this;
        }


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
         * Sets the column index and number of columns on the row. Note that each entry on the row must properly
         * set the same number of columns. If you don't need to specify the column, prefer using onRow(n) instead.
         * @param actualRow the row where the item should appear
         * @param numberOfCols the number of columns across
         * @param column the column number
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& onRowCol(uint8_t actRow, uint8_t column, uint8_t numberOfCols) {
            row = actRow;
            colPos = column;
            colCount = numberOfCols;
            return *this;
        }

        /**
         * Sets the row that an item should appear on, used when there is a single column on the row. Use this when
         * there is only one item on the row as the column will be defaulted to column 1 spanning the row.
         * @param actualRow the row number
         * @return reference to itself for chaining
         */
        ThemePropertiesBuilder& onRow(uint8_t actualRow) {
            colPos = 1;
            colCount = 1;
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
        explicit TcThemeBuilder(GraphicsDeviceRenderer& renderer) : renderer(renderer), factory(renderer.getGraphicsPropertiesFactory()),
                                                                    propertiesBuilder(this) {
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
         * Configure the default font as an adafruit font, normally for use either when using an Adafruit_GFX based
         * library, or tcUnicode which also supports Adafruit font rendering.
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
         * Configure the default font as a tcUnicode font, this is only supported when tcUnicode is enabled.
         * @param font the tcUnicode font
         * @return reference to itself for chaining
         */
        TcThemeBuilder& withTcUnicodeFont(const UnicodeFont* font) {
            fontData = font;
            fontMag = 0;
            return *this;
        }

        /**
         * Configure the default font as a native font consulting the plugin documentation for the chosen display
         * and library.
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
         * Turn on tcUnicode drawing support if it is not already turned on. This means that instead of using native
         * fonts, the rendering engine will use TcUnicode for all text drawing. This is seamless from the rendering
         * perspective, but requires that you use fonts supported by TcUnicode, these are Adafruit or TcUnicode fonts.
         * @return reference to itself for chaining
         */
        TcThemeBuilder & enablingTcUnicode();

        /**
         * Provide custom cursor icons that are used when an item is selected or edited. Use this when you don't want
         * to use the standard cursor icons. Note that the two icons must be the same size.
         * Cursor icons are the visual cues, IE the active arrow and the editing icon usually on the left of an item.
         * @param size the size of the icons
         * @param editIcon the edit icon in XBM form
         * @param activeIcon the active icon in XBM form
         * @return reference to itself for chaining
         */
        TcThemeBuilder &withCursorIconsXbmp(Coord size, const uint8_t *editIcon, const uint8_t *activeIcon);

        /**
         * Use the standard lower resolution cursor icons suitable for OLED and similar resolution displays.
         * Cursor icons are the visual cues, IE the active arrow and the editing icon usually on the left of an item.
         * @return reference to itself for chaining
         */
        TcThemeBuilder &withStandardLowResCursorIcons();

        /**
         * Use the standard medium resolution cursor icons suitable for larger TFT and higher resolution displays
         * Cursor icons are the visual cues, IE the active arrow and the editing icon usually on the left of an item.
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
         * Set up the core rendering settings, that is the way the title should be drawn, and also if analog sliders
         * should be used to represent analog items.
         * @param mode how the title should be drawing, one of BaseGraphicalRenderer::TitleMode enum values.
         * @param useAnalogSliders true to use sliders, otherwise false
         * @return reference to itself for chaining
         */
        TcThemeBuilder& withRenderingSettings(BaseGraphicalRenderer::TitleMode mode, bool useAnalogSliders) {
            renderer.setTitleMode(mode);
            renderer.setUseSliderForAnalog(useAnalogSliders);
            return *this;
        }

        /**
         * Apply the dimensions from the device drawable to the renderer, this takes the device specific size and applies
         * it to the renderer.
         * @return reference to itself for chaining
         */
        TcThemeBuilder& dimensionsFromRenderer();

        /**
         * Manually set the dimensions of the display, for cases where `dimensionsFromRenderer` does not work.
         * @param x the width
         * @param y the height
         * @return reference to itself for chaining
         */
        TcThemeBuilder& manualDimensions(int x, int y);

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
