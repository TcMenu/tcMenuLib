/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <PlatformDetermination.h>
#include "MenuItems.h"
#include "tcMenu.h"
#include "MenuIterator.h"

MenuItem* recursiveFindParentRootVisit(MenuItem* currentMenu, MenuItem* toFind, MenuItem* parentRoot, MenuVisitorFn fn) {
    MenuItem* parent = nullptr;
	MenuItem* currentRoot = currentMenu;

    while(currentMenu != nullptr) {
        if(currentMenu->getId() == toFind->getId()) {
            parent = parentRoot;
        }

		if(currentMenu->getMenuType() == MENUTYPE_SUB_VALUE) {
			MenuItem *par = recursiveFindParentRootVisit(((SubMenuItem*)currentMenu)->getChild(), toFind, currentRoot, fn);
            // if the recursive operation found the parent.
            if(par) parent = par;
		}

        if(fn) {
            // when there's a visitor we must visit every node with the function
            fn(currentMenu);
        }
        else {
            // otherwise, if there's no visitor we can bail out on finding the
            // parent menuitem..
            if(parent) return parent;
        }

		currentMenu = currentMenu->getNext();
	}
    return parent;
}

MenuItem* getParentRootAndVisit(MenuItem* current, MenuVisitorFn visitor) {
    if(current == nullptr) current = menuMgr.getRoot();
    return  recursiveFindParentRootVisit(menuMgr.getRoot(), current, menuMgr.getRoot(), visitor);
}

/**
 * Find an item by it's ID by traversing through the menu structure looking
 * for the ID and short circuiting on the first match.
 */
MenuItem* recursiveFindById(MenuItem* current, uint16_t id) {
    while(current != nullptr) {
        if(current->getMenuType() == MENUTYPE_SUB_VALUE) {
            auto sub = reinterpret_cast<SubMenuItem*>(current);
            MenuItem* ret = recursiveFindById(sub->getChild(), id);
            if(ret != nullptr) {
                return ret;
            }
        }
        else if(current->getId() == id) {
            return current;
        } 

        current = current->getNext();
    }
    return nullptr;
}

MenuItem* getMenuItemById(int id) {
    return recursiveFindById(menuMgr.getRoot(), id);
}

void MenuItemIterator::reset() {
    currentItem = menuMgr.getRoot();
    processingSubMenu = false;
    level = 0;
}

MenuItem* MenuItemIterator::nextItem() {
    
    MenuItem* toReturn = nullptr;
    while(!toReturn) {
        // if there's no current item we have to either find one or exit with NULL.
        if(currentItem == nullptr) {
            if(level == 0) {
                // exhausted all options, clear down and exit.
                reset();
                return nullptr;
            }
            else {
                // we are in the menu structure still, pop something off stack.
                currentItem = parentItems[--level]->getNext();
            }
        }

        while(currentItem && !toReturn) {
            if(currentItem->getMenuType() == MENUTYPE_SUB_VALUE) {

                // check if there's a predicate match on the submenu.
                bool predicateMatches = predicate == nullptr || predicate->matches(currentItem);

                // if we have to report the submenu back to the callee, then we need to wait until
                // the next go to do that, otherwise we overwrite the parent too early.
                if(!processingSubMenu && predicateMatches) {
                    processingSubMenu = true;
                    return currentItem;
                }
                else processingSubMenu = predicateMatches;
                
                // We should most certainly not follow a sub menu that does not match, because it's
                // highly unlikely to be useful and will probably cause problems in the remote side.
                if(processingSubMenu) {
                    processingSubMenu = false;
                    parentItems[level++] = currentItem;
                    currentItem = ((SubMenuItem*)currentItem)->getChild();
                }
            }
            
            if(predicate == nullptr || predicate->matches(currentItem)) toReturn = currentItem;

            // now try and find the next item, or reset if completely finished.
            currentItem = currentItem->getNext();
        }
    }
    return toReturn;
}

MenuItem* MenuItemIterator::currentParent() {
    if(level == 0) return nullptr;
    else return parentItems[level - 1];
}

bool RemoteNoMenuItemPredicate::matches(MenuItem* item) {
    if(item->getMenuType() == MENUTYPE_SUB_VALUE) {
        return !item->isLocalOnly();
    }
    else {
        return item->isSendRemoteNeeded(remoteNo) && !item->isLocalOnly();
    }
}

bool MenuItemTypePredicate::matches(MenuItem* item) {
    if(bitRead(mode, TM_BIT_LOCAL_ONLY) && item->isLocalOnly()) return false;
    if(bitRead(mode, TM_BIT_INCLUDE_SUBMENU) && item->getMenuType() == MENUTYPE_SUB_VALUE) return true;

    if(bitRead(mode, TM_BIT_INVERT))
        return item->getMenuType() != filterType;
    else
        return item->getMenuType() == filterType;
}

MenuItem* getItemAtPosition(MenuItem* root, uint8_t pos) {
	uint8_t i = 0;
	MenuItem* itm = root;

	while (itm != nullptr) {
        if(itm->isVisible())
        {
            if (i == pos) {
                return itm;
            }
            i++;
        }
		itm = itm->getNext();
	}

	return root;
}

int offsetOfCurrentActive(MenuItem* root) {
	uint8_t i = 0;
	MenuItem* itm = root;
	while (itm != nullptr) {
        if(itm->isVisible()) {
            if (itm->isActive() || itm->isEditing()) {
                return i;
            }
            i++;
        }
		itm = itm->getNext();
	}

	return 0;
}

uint8_t itemCount(MenuItem* item, bool includeNonVisible) {
	uint8_t count = 0;
	while (item) {
        if(includeNonVisible || item->isVisible()) ++count;
		item = item->getNext();
	}
	return count;
}
