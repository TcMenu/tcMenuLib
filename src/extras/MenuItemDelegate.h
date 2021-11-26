/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_MENUITEMDELEGATE_H
#define TCMENU_MENUITEMDELEGATE_H

/**
 * @file MenuItemDelegate.h contains a delegate class that allows some menu item operations on more than one item at once
 */

#include <MenuItems.h>

namespace tccore {

    /**
     * This delegate allows menu item operations to take place on more than once item at once. You provide an array of menu
     * item references in the constructor, and then calling the delegated methods applies that action to all menu items.
     *
     * An example is:
     * ```
     * MenuItem*[] items = { &item1, &item2 };
     * MenuItemDelegate delegate(items, 2);
     * delegate.setReadOnly(true); // makes item1 and item2 read only.
     * ```
     */
    class MenuItemDelegate {
    private:
        MenuItem** itemArray;
        int numberOfItems;
        bool internalFlag;
    public:
        enum AnyOrAll { ANY, ALL };
        typedef bool (*ItemDelegateFn)(MenuItem* item, bool internalFlag);

        MenuItemDelegate(MenuItem** itemArray, int items) : itemArray(itemArray), numberOfItems(items), internalFlag(false) {}

        void setReadOnly(bool readOnly);
        void setLocalOnly(bool localOnly);
        void setVisible(bool visible);
        void setChangedAndRemoteSend();
        void setChangedOnly();

        bool isReadOnly(AnyOrAll mode);
        bool isLocalOnly(AnyOrAll mode);
        bool isVisible(AnyOrAll mode);
        bool isChanged(AnyOrAll mode);

        bool onEachItem(ItemDelegateFn itemDelegateFn, AnyOrAll modeAny);
    };

}

#endif //TCMENU_MENUITEMDELEGATE_H
