/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file GraphicsDeviceRenderer.h
 * @brief the interface that all graphics devices should implement to do the actual graphics rendering.
 */

#ifndef TCLIBRARYDEV_GRAPHICSDEVICERENDERER_H
#define TCLIBRARYDEV_GRAPHICSDEVICERENDERER_H

#include <PlatformDetermination.h>
#include "../tcMenu.h"
#include "BaseGraphicalRenderer.h"
#include "GfxMenuConfig.h"
#include "DeviceDrawable.h"
#include "DeviceDrawableHelper.h"
#include "TcDrawableButton.h"
#include "MenuTouchScreenEncoder.h"

#ifndef MINIMUM_CURSOR_SIZE
#define MINIMUM_CURSOR_SIZE 6
#endif // MINIMUM_CURSOR_SIZE

namespace tcgfx {

    class CardLayoutPane;

    /**
     * This class implements TcUnicode's plot pipeline for tcMenu renderers, as a last resort way of drawing when we
     * don't have a direct implementation for the hardware.
     */
    class DrawableTextPlotPipeline : public TextPlotPipeline {
    private:
        DeviceDrawable *drawable;
        Coord cursor;
    public:
        explicit DrawableTextPlotPipeline(DeviceDrawable *drawable) : drawable(drawable) {}
        void drawPixel(uint16_t x, uint16_t y, uint32_t color) override {
            drawable->setDrawColor(color);
            drawable->drawPixel(x, y);
        }
        void setCursor(const Coord& where) override { cursor = where; }
        Coord getCursor() override { return cursor; }
        Coord getDimensions() override { return drawable->getDisplayDimensions(); }
    };

    /**
     * This class contains all the drawing code that is used for most graphical displays, it relies on an instance of
     * device drawable to do the drawing. It can also use sub drawing if the drawing device supports it, and it is enabled.
     * This means that on supported devices it is possible to do flicker free rendering.
     *
     * To support a new display, do not touch this class unless something is amiss or there is a bug, instead just implement
     * the above DeviceDrawable for that display. Take a look at the other rendering classes we already have for an example
     * of how.
     */
    class GraphicsDeviceRenderer : public BaseGraphicalRenderer {
    private:
        DeviceDrawable* rootDrawable;
        DeviceDrawableHelper helper;
        ConfigurableItemDisplayPropertiesFactory propertiesFactory;
        CardLayoutPane* cardLayoutPane = nullptr;
        bool redrawNeeded = false;
    public:
        GraphicsDeviceRenderer(int bufferSize, const char *appTitle, DeviceDrawable *drawable);

        void drawWidget(Coord where, TitleWidget *widget, color_t colorFg, color_t colorBg) override;
        void drawMenuItem(GridPositionRowCacheEntry *entry, Coord where, Coord areaSize, const DrawingFlags& drawingFlags) override;
        void drawingCommand(RenderDrawingCommand command) override;

        void fillWithBackgroundTo(int endPoint) override;

        /**
         * Get the height for the font and add the descent to the bottom padding.
         * @param font the font to measure
         * @param mag any magnification to apply - if supported
         * @param padding the padding for the item, bottom will be adjusted
         * @return the height of the item.
         */
        int heightForFontPadding(const void *font, int mag, MenuPadding &padding);

        /**
         * Set up the display based on the legacy graphics configuration. This is deprecated and you should move to
         * using prepareDisplay. New code should NOT use this as it will be removed in a future build.
         * @deprecated use prepareDisplay with displayProperties overrides instead as per all examples.
         * @param gfxConfig the legacy graphics configuration
         */
        void setGraphicsConfiguration(void* gfxConfig);

        /**
         * Set up the display using a basic configuration. Setting factories with default colours and sizes.
         * @param monoPalette true if the display is monochrome, otherwise false.
         * @param itemFont the font to use for items
         * @param titleFont the font to use for the title
         * @param needEditingIcons true if editing icons should be prepared, otherwise false.
         */
        void prepareDisplay(bool monoPalette, const void *itemFont, int magItem, const void *titleFont, int magTitle, bool needEditingIcons);

        /**
         * Gets the abstract display properties factory, used internally to get the factory regardless of what actual
         * type it is, in user code that is using graphics properties factory, use getGraphicsPropertiesFactory
         * @return the properties factory
         */
        ItemDisplayPropertiesFactory &getDisplayPropertiesFactory() override { return propertiesFactory; }

        /**
         * Gets the graphical display properties factory, so that you can add graphics configuration easily.
         * @return the properties factory
         */
        ConfigurableItemDisplayPropertiesFactory &getGraphicsPropertiesFactory() { return propertiesFactory; }

        /**
         * Gets the underlying device drawable so that you can render to the screen in a device independent way.
         * On most systems this is a very thin wrapper on the library and performs very well for all but the most
         * intensive of drawing operations.
         * @return the underlying device drawable.
         */
        DeviceDrawable* getDeviceDrawable() { return rootDrawable; }

        /**
         * Enables TcUnicode as the default font processor for all operations, and ensures that the unicode helper can
         * be accessed too.
         */
        void enableTcUnicode() { rootDrawable->enableTcUnicode(); }

        /**
         * Enables the card layout system for the items provided, a card layout shows a single item at once, with left
         * and right images present that work somewhat like buttons. Rotating the encoder (or up/down buttons) will result
         * in switching between the menu items. This works particularly well when combined with using drawable icons
         * for each of the menu items. Note that after enabling, you must call `setCardLayoutStatusForSubMenu` to enable
         * menus, see the method docs for details.
         * @param left the icon to use for left, (takes its colors from the title)
         * @param right the icon to use for right, (takes its colors from the title)
         * @param touchManager optionally provide the touch manager to make the buttons work with a touch screen
         * @param monoDisplay for mono displays the direction buttons completely disappears instead of greying out
         */
        void enableCardLayout(const DrawableIcon& left, const DrawableIcon& right, MenuTouchScreenManager* touchManager, bool monoDisplay);

        /**
         * Allows the card layout mode to be enabled for a root item, this is the first item in the linked list, for
         * submenus you can obtain this by calling `menuSub.getChild()` on a submenu, or for root call `rootMenuItem()`
         * to get the first item of the root menu.
         * @param root the root menu item to change status
         * @param onOrOff true if on, otherwise false.
         */
        void setCardLayoutStatusForSubMenu(MenuItem* root, bool onOrOff);

        LayoutMode getLayoutMode(MenuItem* rootItem) override;
    protected:
        /**
         * Overrides the default implementation to allow for card based layouts, if this is not enabled for the submenu
         * then regular rendering is used instead.
         * @param rootItem the first item in the linked list for this submenu
         * @param locRedrawMode reference to the local redraw mode. May be updated
         * @param forceDrawWidgets reference to if the widgets are to be force drawn, may be updated
         */
        void subMenuRender(MenuItem* rootItem, uint8_t& locRedrawMode, bool& forceDrawWidgets) override;
        bool isActiveOrEditing(MenuItem* pItem, const DrawingFlags& drawingFlags);
    private:
        int calculateSpaceBetween(const void* font, uint8_t mag, const char* buffer, int start, int end);
        void internalDrawText(GridPositionRowCacheEntry* pEntry, const Coord& where, const Coord& size, const DrawingFlags& drawingFlags);
        void drawCoreLineItem(GridPositionRowCacheEntry* entry, DrawableIcon* icon, Coord &where, Coord &size,
                              const DrawingFlags& drawingFlags, bool drawBg);
        void drawTextualItem(GridPositionRowCacheEntry* entry, Coord& where, Coord& size, const DrawingFlags& drawingFlags);
        void drawCheckbox(GridPositionRowCacheEntry *entry, Coord& where, Coord& size, const DrawingFlags& drawingFlags);
        void drawSlider(GridPositionRowCacheEntry* entry, AnalogMenuItem* pItem, Coord& where, Coord& size, const DrawingFlags& drawingFlags);
        void drawUpDownItem(GridPositionRowCacheEntry* entry, Coord& where, Coord& size, const DrawingFlags& drawingFlags);
        void drawIconItem(GridPositionRowCacheEntry *pEntry, Coord& where, Coord& size, const DrawingFlags& drawingFlags);
        void drawBorderAndAdjustSize(Coord &where, Coord &size, MenuBorder &border);

        DrawableIcon *getStateIndicatorIcon(GridPositionRowCacheEntry *entry);
    };
} // namespace tcgfx

#endif //TCLIBRARYDEV_GRAPHICSDEVICERENDERER_H
