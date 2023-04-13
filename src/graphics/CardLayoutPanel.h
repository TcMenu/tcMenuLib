/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "MenuTouchScreenEncoder.h"
#include "DrawingPrimitives.h"
#include "GfxMenuConfig.h"
#include "TcDrawableButton.h"

/**
 * @file CardLayoutPanel.h
 * @brief a panel that helps the renderer draw items in a card layout.
 */

#ifndef TCMENU_CARDLAYOUTPANEL_H
#define TCMENU_CARDLAYOUTPANEL_H

namespace tcgfx {

    class SubMenuUsingCardLayout {
    private:
        uint16_t key;
        bool state;
    public:
        SubMenuUsingCardLayout() : key(-1), state(false) {}

        SubMenuUsingCardLayout(const SubMenuUsingCardLayout &) = default;

        SubMenuUsingCardLayout &operator=(const SubMenuUsingCardLayout &) = default;

        SubMenuUsingCardLayout(uint16_t key, bool state) : key(key), state(state) {}

        uint16_t getKey() const { return key; }

        uint16_t getState() const { return state; }

        void setState(bool onOrOff) { state = onOrOff; }
    };

    class GraphicsDeviceRenderer;

    /**
     * Card layout is a layout where a single item (and possibly a title) are shown on the display with left and
     * right indicators (or buttons on a touch screen).
     * This class does most of the layout level calculations to make some attempt at taking new layouts outside
     * of the core. If more new layouts are created, we'll formalize this into some kind of interface and tidy up
     * the code in the renderer. On touch screens the buttons will work with touch.
     *
     * @param left the icon for the left button
     * @param right the icon for the right button
     * @param optionalTouch if using touch, provide the touch manager, otherwise null.
     */
    class CardLayoutPane : public tcgfx::TouchObserver {
    public:
        enum CardLayoutDir {
            DOING_NOTHING, GOING_LEFT, GOING_RIGHT
        };
    private:
        TcDrawableButton leftButton;
        TcDrawableButton rightButton;
        const DrawableIcon *iconLeft;
        BtreeList<uint16_t, SubMenuUsingCardLayout> usingCardLayout;
        Coord menuItemLocation;
        Coord menuItemSize;
        Coord titleSize;
        CardLayoutDir dirStatus = DOING_NOTHING;
        bool inUse = false;
        MenuTouchScreenManager *touchScreenManager = nullptr;
        GraphicsDeviceRenderer* theRenderer;
    public:
        /**
         * Creates a card layout with the left and right buttons, ready for use later when a card layout is presented.
         * This class does most of the layout level calculations to make some attempt at taking new layouts outside
         * of the core. If more new layouts are created, we'll formalize this into some kind of interface and tidy up
         * the code in the renderer. On touch screens the buttons will work with touch.
         *
         * @param left the icon for the left button
         * @param right the icon for the right button
         * @param optionalTouch if using touch, provide the touch manager, otherwise null.
         * @param monoDisplay if the display is mono
         */
        CardLayoutPane(const DrawableIcon *left, const DrawableIcon *right, MenuTouchScreenManager *optionalTouch, bool monoDisplay);

        /**
         * When a menu is presented in card layout this will be called when full re-draw is requested to set up the
         * layout and prepare the available sizes.
         * @param titleProps the display properties that the buttons should use
         * @param gfxRenderer the renderer used to draw with
         * @param titleNeeded if the title is needed or not.
         */
        void forMenu(ItemDisplayProperties *titleProps, ItemDisplayProperties *itemProps, GraphicsDeviceRenderer* gfxRenderer, bool titleNeeded);

        /**
         * When a card layout menu is going off display, this is called to reset anything adjust for the card layout.
         */
        void notInUse();

        /**
         * Use this to add or remove submenus from the card layout
         * @param item the item to check (or null for root)
         * @param onOrOff if it should use card layout
         */
        void setEnablementForSub(MenuItem *item, bool onOrOff);

        /**
         * Check if a given menu item is using card layout.
         * @param item the item to check (or null for root)
         * @return true if card layout, otherwise false
         */
        bool isSubMenuCardLayout(MenuItem *item);

        /**
         * Call this every time the renderer paint runs, it makes sure the buttons are up to date and also returns if
         * any buttons have been pressed.
         * @param renderer the renderer
         * @param active the currently active item index
         * @param countOfItems the number of items
         * @return the current button press mode.
         */
        CardLayoutDir prepareAndPaintButtons(GraphicsDeviceRenderer *renderer, int active, int countOfItems, bool titleActive);

        /**
         * Part of the touch observer interface, called when there is a touch event .
         * @param notification the touch notification
         */
        void touched(const TouchNotification &notification);

        /**
         * @return the location of the start of the item to be presented
         */
        const Coord &getMenuLocation() { return menuItemLocation; }

        /**
         * @return the size of the item to be presented
         */
        const Coord &getMenuSize() { return menuItemSize; }

        /**
         * @return the size of the title area
         */
        const Coord &getTitleSize() { return titleSize; }
    };

}

#endif //TCMENU_CARDLAYOUTPANEL_H
