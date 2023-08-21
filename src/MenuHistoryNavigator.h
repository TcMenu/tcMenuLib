/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file MenuHistoryNavigator.h
 * @brief contains the functionality to do with menu item navigation, including a stack of
 * previous navigations
 */

#ifndef TCMENU_MENUHISTORYNAVIGATOR_H
#define TCMENU_MENUHISTORYNAVIGATOR_H

#include "MenuItems.h"

#ifndef NAV_ITEM_ARRAY_SIZE
#define NAV_ITEM_ARRAY_SIZE 4
#endif

namespace tcnav {
    class NavigationListener {
        NavigationListener* next = nullptr;
    public:
        virtual void navigationHasChanged(MenuItem* newItem, bool completelyReset)=0;
        NavigationListener* getNext() { return next; }
        void setNext(NavigationListener* nxt) { next = nxt; }
    };

    class MenuNavigationStore {
    private:
        MenuItem* root;
        MenuItem* currentRoot;
        MenuItem* currentSub;
        MenuItem* navItems[NAV_ITEM_ARRAY_SIZE];
        MenuItem* activeItems[NAV_ITEM_ARRAY_SIZE];
        NavigationListener* navigationLister = nullptr;
        uint8_t navIdx;
        bool currentIsCustom;
    public:
        MenuNavigationStore() = default;

        void addNavigationListener(NavigationListener* newListener);
        void clearNavigationListeners() { navigationLister = nullptr; }

        void triggerNavigationListener(bool completeReset);

        /**
         * @return the absolute root of all menu items
         */
        MenuItem* getRoot() { return root; }
        /**
         * @return the currently selected root (or sub menu)
         */
        MenuItem* getCurrentRoot() { return currentRoot; }
        /**
         * Get the submenu for the current item, WARNING null is returned on the root menu as it has no root
         * @return the current submenu or null for ROOT
         */
        MenuItem* getCurrentSubMenu() { return currentSub; }

        /**
         * Call during initialisation of a complete menu structure, it sets the root, and resets navigation.
         * @param item the new root
         */
        void setRootItem(MenuItem* item);

        /**
         * Navigates to a new menu, remembering the history of all items that are
         * contained within the current root.
         * @param activeItem the item that was selected
         * @param newRoot the new root that will be displayed
         */
        void navigateTo(MenuItem* activeItem, MenuItem* newRoot, bool custom);

        /**
         * Pops the last navigation item, or in the worst case goes back to root.
         * @return the item that should be selected after this item is popped
         */
        MenuItem* popNavigationGetActive();

        bool isShowingRoot();

        /**
         * @return the depth of the navigation stack that is being managed.
         */
        int getNavigationDepth() const { return navIdx; }

        /**
         * The active item at a given zero based position in the stack or nullptr if out of range
         * @param i the index
         * @return the item or nullptr
         */
        MenuItem* getActiveAt(uint8_t i) { return i < navIdx ? activeItems[i] : nullptr; }

        /**
         * Get the root menu item for the given zero based position in the stack or nullptr if out of range.
         * @param i the index
         * @return the item or nullptr
         */
        MenuItem* getRootAt(uint8_t i) { return i < navIdx ? navItems[i] : nullptr; }

        /** Completely reset the navigation back to the initial state where root is on display */
        void resetStack();
    };
}

#endif //TCMENU_MENUHISTORYNAVIGATOR_H
