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
    public:
        CardLayoutPane(const DrawableIcon *left, const DrawableIcon *right, MenuTouchScreenManager *optionalTouch);

        void setTouchManager(MenuTouchScreenManager *manager);

        void forMenu(ItemDisplayProperties *titleProps, DeviceDrawable *rootDrawable, bool titleNeeded);

        void notInUse();

        void setSubMenuState(MenuItem *item, bool onOrOff);

        bool isSubMenuCardLayout(MenuItem *item);

        tcgfx::TcDrawableButton &getLeftButton() { return leftButton; }

        tcgfx::TcDrawableButton &getRightButton() { return rightButton; }

        CardLayoutDir prepareAndPaintButtons(GraphicsDeviceRenderer *renderer, int active, int countOfItems);

        void touched(const TouchNotification &notification);

        const Coord &getMenuLocation() { return menuItemLocation; }

        const Coord &getMenuSize() { return menuItemSize; }

        const Coord &getTitleSize() { return titleSize; }
    };

}

#endif //TCMENU_CARDLAYOUTPANEL_H
