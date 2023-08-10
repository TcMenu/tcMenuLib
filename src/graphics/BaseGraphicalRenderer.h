/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file BaseGraphicalRenderer.h
 * @brief Contains the base functionality for all graphical renderers.
 */

#ifndef TCMENU_BASEGRAPHICALRENDERER_H
#define TCMENU_BASEGRAPHICALRENDERER_H

#include <BaseRenderers.h>
#include "GfxMenuConfig.h"
#include "RuntimeTitleMenuItem.h"

#define GFX_LAST_ROW_FIT_FLAG 0
#define GFX_USING_RAW_TOUCH 1
#define GFX_USING_TOUCH_INTERFACE 2
#define GFX_SLIDER_FOR_ANALOG 3
#define GFX_TITLE_ON_DISPLAY 4
#define GFX_EDIT_STATUS_ICONS_ENABLED 5

namespace tcgfx {

    /**
     * An internal method used to calculate the row*col index that is used during rendering to locate items quickly
     * @param row the row
     * @param col the column
     * @return a key based on row and column.
     */
    inline uint16_t rowCol(int row, int col) {
        return (row * 100) + col;
    }

    class DrawingFlags {
    private:
        uint16_t flags;
    public:
        DrawingFlags(bool drawAll, bool active, bool editing) : flags(0) {
            bitWrite(flags, 0, drawAll);
            bitWrite(flags, 1, active);
            bitWrite(flags, 2, editing);
        }
        DrawingFlags(const DrawingFlags& other) = default;
        DrawingFlags& operator=(const DrawingFlags& other) = default;

        bool isDrawingAll() const { return bitRead(flags, 0); }
        bool isActive() const { return bitRead(flags, 1); }
        bool isEditing() const { return bitRead(flags, 2); }
    };

    /**
     * Represents a grid position along with the menuID and also the drawable icon, if one exists. It is stored in the list
     * of drawing instructions in the base graphical renderer, and then read when a new menu is displayed in order to reorder
     * and position the items before rendering, these instructions are quite display neutral. It is able to be stored within
     * the simple BtreeList because it implements getKey returning uint16_t
     */
    class GridPositionRowCacheEntry {
    private:
        MenuItem *menuItem;
        GridPosition thePosition;
        ItemDisplayProperties *properties;
    public:
        GridPositionRowCacheEntry() : menuItem(nullptr), thePosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0), properties(nullptr) {}

        GridPositionRowCacheEntry(const GridPositionRowCacheEntry &other) = default;
        GridPositionRowCacheEntry& operator=(const GridPositionRowCacheEntry &other) = default;

        GridPositionRowCacheEntry(MenuItem *item, const GridPosition &pos, ItemDisplayProperties *props) : menuItem(item), thePosition(pos), properties(props) {}

        const GridPosition &getPosition() { return thePosition; }

        ItemDisplayProperties *getDisplayProperties() { return properties; }

        uint16_t getKey() const { return rowCol(thePosition.getRow(), thePosition.getGridPosition()); }

        uint16_t getHeight() {
            return thePosition.getGridHeight() != 0 ? thePosition.getGridHeight() : properties->getRequiredHeight();
        }

        MenuItem *getMenuItem() { return menuItem; }
    };

    /**
     * Indicates the layout mode, IE how the items will be rendered onto the display
     */
    enum LayoutMode: uint8_t {
        /** The items will be laid out using default vertical rendering */
        LAYOUT_VERTICAL_DEFAULT,
        /** The items will layout in card view horizontally */
        LAYOUT_CARD_SIDEWAYS
    };

    class BaseGraphicalRenderer;

    /** Used to tie together the navigation change events with the renderer without multiple inheritance */
    class RenderingNavigationListener : public tcnav::NavigationListener {
    private:
        BaseGraphicalRenderer* renderer;
    public:
        explicit RenderingNavigationListener(BaseGraphicalRenderer* r);
        void navigationHasChanged(MenuItem *newItem, bool completelyReset) override;
    };

    class CachedDrawingLocation {
    private:
        uint16_t startY;
        uint8_t currentOffset;
    public:
        CachedDrawingLocation() = default;
        CachedDrawingLocation(uint16_t startY, uint8_t currentOffset) : startY(startY), currentOffset(currentOffset) {}
        CachedDrawingLocation(const CachedDrawingLocation& other) = default;
        CachedDrawingLocation& operator=(const CachedDrawingLocation& other) = default;

        uint16_t getStartY() const {
            return startY;
        }

        uint8_t getCurrentOffset() const {
            return currentOffset;
        }
    };

    /**
     * This is the base class for all simpler renderer classes where the height of a row is equal for all entries,
     * and there is always exactly one item on a row. This takes away much of the work to row allocation for simple
     * renderers. Examples of this are the LiquidCrystal renderer
     */
    class BaseGraphicalRenderer : public BaseMenuRenderer {
    public:
        /**
         * This provides the mode in which the renderer will draw the title, either not at all, always, or as the first row.
         */
        enum TitleMode : uint8_t {
            /** never draw the title */
            NO_TITLE,
            /** the title will only appear when row 0 is selected */
            TITLE_FIRST_ROW,
            /** the title will always be shown regardless of index position */
            TITLE_ALWAYS
        };
        /**
         * Represents the possible drawing commands that the base renderer sends to the leaf class
         */
        enum RenderDrawingCommand {
            /** this indicates the screen should be cleared */
            DRAW_COMMAND_CLEAR,
            /** this indicates that the drawing is about to start */
            DRAW_COMMAND_START,
            /** this indicates that the drawing is ending */
            DRAW_COMMAND_ENDED
        };

        /**
         * The current menu from the renderers perspective
         * @return the current menu that the rendering layer is drawing for
         */
        MenuItem *getCurrentRendererRoot() { return currentRootMenu; }

    private:
        RenderingNavigationListener navigationListener;
        MenuItem *currentRootMenu;
        const char *pgmTitle;
        GridPositionRowCacheEntry cachedEntryItem;
    protected:
        BtreeList<uint16_t, GridPositionRowCacheEntry> itemOrderByRow;
        TitleMode titleMode = TITLE_FIRST_ROW;
        uint16_t width, height;
        CachedDrawingLocation drawingLocation;
        uint8_t flags;
    public:
        BaseGraphicalRenderer(int bufferSize, int wid, int hei, bool lastRowExact, const char *appTitle);
        void initialise() override;

        void setTitleMode(TitleMode mode);

        /**
         * TcMenu supports more than one display now, (presently 2) so you set the display number here and it
         * will be passed to the isChanged function to ensure rendering works for both items.
         * @param displayNum the display number.
         */
        void setDisplayNumber(uint8_t displayNum) { this->displayNumber = displayNum; }

        /**
         * @return the display number as set using setDisplayNumber
         */
        uint8_t getDisplayNumber() { return this->displayNumber; }

        /**
         * set the use of sliders by default for all integer items
         * @param useSlider true to use sliders
         */
        void setUseSliderForAnalog(bool useSlider) { bitWrite(flags, GFX_SLIDER_FOR_ANALOG, useSlider); }
        /**
         * Enable touch support within the renderer
         * @param hasTouch true to enable touch
         */
        void setHasTouchInterface(bool hasTouch) { bitWrite(flags, GFX_USING_TOUCH_INTERFACE, hasTouch); }
        /**
         * Set the title as on, so it appears on the display, somewhat internal to the library, control from the theme
         * @param titleOn true to turn on
         */
        void setTitleOnDisplay(bool titleOn) { bitWrite(flags, GFX_TITLE_ON_DISPLAY, titleOn); }
        /**
         * Set that the last row has to fit exactly, this is for LCD cases and where a gap at the bottom is deemed as
         * better than half rendering.
         * @param exact true for LCD and cases where a gap is better, otherwise false.
         */
        void setLastRowExactFit(bool exact) { bitWrite(flags, GFX_LAST_ROW_FIT_FLAG, exact); }
        /**
         * When this is on, the touch will not look up items, helpful for where you need complete control of the touch
         * interface for a short time.
         */
        void setRawTouchMode(bool rawTouch) { bitWrite(flags, GFX_USING_RAW_TOUCH, rawTouch); }
        /**
         * Turn off editing icons and editor indications of editing for a short time, for example during special layouts
         * such as card layout. This is an override that can force editing icons OFF, it cannot force them ON if no icons
         * were registered.
         * @param ena true to enable (Default)
         */
        void setEditStatusIconsEnabled(bool ena) { bitWrite(flags, GFX_EDIT_STATUS_ICONS_ENABLED, ena); }
        /**
         * @return if using sliders by default for analog items
         */
        bool isUseSliderForAnalog() const { return bitRead(flags, GFX_SLIDER_FOR_ANALOG); }
        /**
         * @return if there is a touch interface configured
         */
        bool isHasTouchInterface() const { return bitRead(flags, GFX_USING_TOUCH_INTERFACE); }
        /**
         * @return if the title is on display (somewhat internal)
         */
        bool isTitleOnDisplay() const { return bitRead(flags, GFX_TITLE_ON_DISPLAY); }
        /**
         * @return if the last row should fit on the display exactly
         */
        bool isLastRowExactFit() const { return bitRead(flags, GFX_LAST_ROW_FIT_FLAG); }
        /**
         * @return if raw touch mode is enabled where it will not recognise item spaces
         */
        bool isRawTouchMode() const { return bitRead(flags, GFX_USING_RAW_TOUCH); }
        /**
         * @return if the edit icons and other indications are being shown
         */
        bool isEditStatusIconEnabled() const { return bitRead(flags, GFX_EDIT_STATUS_ICONS_ENABLED); }

        void render() override;

        /**
         * Usually called during the initialisation of the display internally to set the width and height.
         * @param w display width in current rotation
         * @param h display height in current rotation
         */
        void setDisplayDimensions(int w, int h) {
            serlogF3(SER_TCMENU_INFO, "Set dimensions: ", w, h);
            width = w;
            height = h;
        }

        /**
         * @return the layout mode that would be applied for a given root menu item, see the enum.
         */
        virtual LayoutMode getLayoutMode(MenuItem* rootItem) { return LAYOUT_VERTICAL_DEFAULT; }

        /**
         * Draw a widget into the title area at the position indicated, the background will need to be cleared before
         * performing the operation.
         * @param where the position on the screen to draw the widget
         * @param widget the widget to be drawn.
         * @param colorFg the foreground color
         * @param colorBg the background color
         */
        virtual void drawWidget(Coord where, TitleWidget *widget, color_t colorFg, color_t colorBg) = 0;

        /**
         * Draw a menu item onto the display using the instructions
         * @param theItem the item to be rendered
         * @param mode the suggested mode in which to draw
         * @param where the position on the display to render at
         * @param areaSize the size of the area where it should be rendered
         */
        virtual void drawMenuItem(GridPositionRowCacheEntry *entry, Coord where, Coord areaSize, const DrawingFlags& drawFlags) = 0;

        /**
         * This sends general purpose commands that can be implemened by the leaf class as needed.
         */
        virtual void drawingCommand(RenderDrawingCommand command) = 0;

        /**
         * This indicates to the renderer leaf class that the background color should be filled from
         * Y end point to the end of the screen.
         * @param endPoint the last drawing point in the Y location
         */
        virtual void fillWithBackgroundTo(int endPoint) = 0;

        /**
         * Gets the item display factory that provides the formatting information for this renderer, it holds the font,
         * color, padding and grid information. For all bitmapped renderers (EG: AdaFruit_GFX, U8G2) you can safely up
         * cast to the ConfigurableItemDisplayPropertiesFactory
         * @return the display properties factory.
         */
        virtual ItemDisplayPropertiesFactory &getDisplayPropertiesFactory() = 0;

        /**
         * Find an item's offset in a given root, safely returns 0.
         * @param root the root item
         * @param toFind the item within that root
         * @return the index if found, otherwise 0.
         */
        int findItemIndex(MenuItem *root, MenuItem *toFind) override;

        /**
         * @return the total number of items in the current menu
         */
        uint8_t itemCount(MenuItem* root, bool) override;

        /**
         * Provides the menu item grid position and dimensions of it, given a screen position. Usually used by touch screen
         * implementations. Note that this will return nullptr if no entry is found.
         * @param screenPos the raw screen position
         * @param localStart the local menu item start position
         * @param localSize the local menu item size
         * @return the grid cache entry or nullptr if no item was found.
         */
        GridPositionRowCacheEntry* findMenuEntryAndDimensions(const Coord &screenPos, Coord &localStart, Coord &localSize);

        /**
         * Gets the menu item at a given index, which may be different to the order in the tree.
         */
        MenuItem *getMenuItemAtIndex(MenuItem* currentRoot, uint8_t idx) override;

        /**
         * All base graphical renderers use a dialog based on menu item, this provides the greatest flexibility and
         * also reduces the amount of dialog code. This means nearly every display we support uses menu based dialogs
         * @return a shared menu based dialog instance
         */
        BaseDialog *getDialog() override;

        /**
         * @return width of the display in current rotation
         */
        int getWidth() const { return width; }

        /**
         * @return height of the display in current rotation
         */
        int getHeight() const { return height; }

        /**
         * Force the renderer to completely recalculate the display parameters next time it's drawn.
         */
        void displayPropertiesHaveChanged();

        /**
         * This is generally called by the navigation listener when the root item has changed due to a new menu being
         * displayed, or display reset event. It will force an immediate recalculation of all items.
         * @param newItem the new root item
         */
        void rootHasChanged(MenuItem* newItem);

        /**
         * Sets the active item to be the menu item selected, also this recalculates the offset required to present
         * that item.
         * @param item the new active item
         * @return the index of the item
         */
        uint8_t setActiveItem(MenuItem *item) override;
    protected:
        /**
         * This is responsible for redrawing a series of menu items onto the screen, it can be overridden as needed
         * in extension classes, such that different rendering can be achieved. The default rendering is vertical,
         * with the items scrolling downward if needed. It is virtual to allow for such extensions.
         * @param rootItem the first item in the linked list of children
         * @param locRedrawMode the drawing mdoe, a reference, will be updated.
         * @param forceDrawWidgets reference to widgets force update flag, will be updated.
         */
        virtual void subMenuRender(MenuItem* rootItem, uint8_t& locRedrawMode, bool& forceDrawWidgets);
        int heightOfRow(int row, bool includeSpace=false);
    private:
        bool drawTheMenuItems(int startRow, int startY, bool drawEveryLine);

        void renderList();

        void recalculateDisplayOrder(MenuItem *pItem, bool safeMode);

        void redrawAllWidgets(bool forceRedraw);

        bool areRowsOutOfOrder();

        int calculateHeightTo(int index, MenuItem *pItem);

        ItemDisplayProperties::ComponentType toComponentType(GridPosition::GridDrawingMode mode, MenuItem *pMenuItem);
    };

    /**
     * This is a helper function for analog items that converts the range of an analog item over the width available
     * in a menu item. It's mainly used by scrolling components.
     * @param item the item to determine the current and max value from
     * @param screenWidth the width available for the full range
     * @return the point on the screen which represents current value
     */
    inline int analogRangeToScreen(AnalogMenuItem *item, int screenWidth) {
        float ratio = (float) screenWidth / (float) item->getMaximumValue();
        return int((float) item->getCurrentValue() * ratio);
    }

    /**
     * This method takes an existing graphics configuration and converts it into the new display properties format, its
     * designer as a bridge between the old config object method and the new more supportable properties definitions.
     * @param factory the properties factory that we wish to populate
     * @param gfxConfig the graphics configuration to convert
     * @param titleHeight the height of the title
     * @param itemHeight the height of a standard item
     */
    void preparePropertiesFromConfig(ConfigurableItemDisplayPropertiesFactory &factory,
                                     const ColorGfxMenuConfig<const void *> *gfxConfig, int titleHeight,
                                     int itemHeight);
} // namespace tcgfx

#endif //TCMENU_BASEGRAPHICALRENDERER_H
