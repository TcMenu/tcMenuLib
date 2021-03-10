//
// Created by David Cherry on 09/03/2021.
//

#ifndef TCMENU_MENUHISTORYNAVIGATOR_H
#define TCMENU_MENUHISTORYNAVIGATOR_H

#include "MenuItems.h"

#ifndef NAV_ITEM_ARRAY_SIZE
#define NAV_ITEM_ARRAY_SIZE 4
#endif

namespace tcnav {

    class MenuNavigationStore {
    private:
        MenuItem* root;
        MenuItem* currentRoot;
        MenuItem* currentSub;
        MenuItem* navItems[NAV_ITEM_ARRAY_SIZE];
        MenuItem* activeItems[NAV_ITEM_ARRAY_SIZE];
        uint8_t navIdx;
    public:
        MenuNavigationStore() = default;

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
         * checks if the item is within the root menu, traversing any sub menus.
         * @param root the root to start from
         * @param item the item to find
         * @return true if found, otherwise false
         */
        bool isItemWithinRoot(MenuItem* toCheck, MenuItem* item);

        /**
         * Navigates to a new menu, remembering the history of all items that are
         * contained within the current root.
         * @param activeItem the item that was selected
         * @param newRoot the new root that will be displayed
         */
        void navigateTo(MenuItem* activeItem, MenuItem* newRoot);

        /**
         * Pops the last navigation item, or in the worst case goes back to root.
         * @return the item that should be selected after this item is popped
         */
        MenuItem* popNavigationGetActive();

    };
}

#endif //TCMENU_MENUHISTORYNAVIGATOR_H
