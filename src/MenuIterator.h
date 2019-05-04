/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>

/**
 * @file MenuIterator.h
 * 
 * Provides a number of utility functions for the processing of menu item structures.
 */

#ifndef _MENUITERATOR_H
#define _MENUITERATOR_H

#define MAX_MENU_DEPTH 4

// forward reference of menu item
class MenuItem;

/**
 * defines a function used when needing to visit all menu items 
 */
typedef void (*MenuVisitorFn)(MenuItem* item);

/**
 * Finds the parent root menu item to the item that's passed in, that is the root item that contains
 * this menu item. Normally used by protocol and display layers when there's a need to traverse the
 * menu structure. This version is also able to provide a function that will visit each element in
 * the tree. It will always visit every item.
 * 
 * @param current the menu item that is currently menu root
 * @return the parent menu item to the present menu item
 */
MenuItem* getParentRootAndVisit(MenuItem* current, MenuVisitorFn visitor);

/**
 * Finds the parent root menu item to the item that's passed in, that is the root item that contains
 * this menu item. This version will short circuit out of the traversal as soon as the item is found.
 * 
 * @param current the menu item that is currently menu root
 * @return the parent menu item to the present menu item
 */
inline MenuItem* getParentRoot(MenuItem* current) { return getParentRootAndVisit(current, NULL); }


/**
 * A predicate that can match upon a menu item, the match is generally performed by calling the
 * match method, which returns true for a match.
 */
class MenuItemPredicate {
public:
    /**
     * This method is used to determine if a given menuitem given by item matches the predicate
     * @param item the item to be checked for a match.
     * @return true if there is a match, false otherwise.
     */
    virtual bool matches(MenuItem* item)=0;
};

/**
 * A specialisation of the MenuItemPredicate that matches on a given remote number.
 */
class RemoteNoMenuItemPredicate : public MenuItemPredicate {
private:
    uint8_t remoteNo;
public:
    /**
     * Constructs the predicate with the remote number we are interested in.
     * @param the number of the remote we want to check for.
     */
    RemoteNoMenuItemPredicate(int remoteNo) { this->remoteNo = remoteNo; }

    /**
     * Matches if the remote number is marked as changed on the item. Remote number from the constructor.
     * @param item the menu item to be checked
     * @return true if the remote has changed, false otherwise
     */
    bool matches(MenuItem* item) override;
};

/**
 * A specialisation of the MenuItemPredicate that matches on a given MenuType. For example sub menus or 
 * boolean menu items.
 */
class MenuItemTypePredicate : public MenuItemPredicate {
private:
    MenuType filterType;
    bool inverted;
public:
    /**
     * Construct the predicate indicating the type of item to filter on
     * @param filterType the type to filter for
     */
    MenuItemTypePredicate(MenuType filterType, bool inverted = false) { 
        this->filterType = filterType; 
        this->inverted = inverted;
    }

    /**
     * This predicate checks if the item matches the type in the constructor.
     * @param item the item to be checked
     * @return true if menu type is of filterType provided in the constructor
     */
    bool matches(MenuItem* item) override;
};

/**
 * This provides a way to non recursively iterate through the entire menu structure. Each call to
 * nextItem() finds the next item that matches the predicate, traversing through submenus if needed.
 * This has a limitation of traversing only up to `MAX_MENU_DEPTH` levels of menus. Adjust this value
 * in MenuIterator.h to support more menu levels.
 */
class MenuItemIterator {
private:
    MenuItem* currentItem;
    MenuItemPredicate* predicate;
    MenuItem* parentItems[MAX_MENU_DEPTH];
    uint8_t level;
    bool processingSubMenu;
public:
    MenuItemIterator() { 
        reset(); 
        predicate = NULL;
    }

    /**
     * Set the predicate that will be used on subsequent calls
     * @param predicate a predicate that will filter returned results.
     */
    void setPredicate(MenuItemPredicate* predicate) { this->predicate = predicate; }

    /**
     * Reset the state so that the next call to nextItem() returns the first 
     */
    void reset();

    /**
     * Gets the next menu item in this iterators order, filtered by the current predicate.
     * When the last item is passed this iterator returns NULL and then calls reset().
     * @return the next item or NULL when the end is reached.
     */
    MenuItem* nextItem();

    /**
     * Returns the parent of the item that will be returned by nextItem(), always call AFTER
     * calling nextItem() as next item will possibly alter this value. Result will be NULL when
     * there is no parent (root). The result when not null will be the nearest submenu.
     * 
     * @return the parent of the next item or NULL for root, must be called before nextItem().
     */
    MenuItem* currentParent();
};

#endif
