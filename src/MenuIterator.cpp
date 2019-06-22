/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "MenuItems.h"
#include "tcMenu.h"
#include "tcUtil.h"
#include "MenuIterator.h"

MenuItem* recursiveFindParentRootVisit(MenuItem* currentMenu, MenuItem* toFind, MenuItem* parentRoot, MenuVisitorFn fn) {
    MenuItem* parent = NULL;
	MenuItem* currentRoot = currentMenu;

    while(currentMenu != NULL) {
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
    if(current == NULL) current = menuMgr.getRoot();
    return  recursiveFindParentRootVisit(menuMgr.getRoot(), current, menuMgr.getRoot(), visitor);
}

/**
 * Find an item by it's ID by traversing through the menu structure looking
 * for the ID and short circuiting on the first match.
 */
MenuItem* recursiveFindById(MenuItem* current, uint16_t id) {
    while(current != NULL) {
        if(current->getMenuType() == MENUTYPE_SUB_VALUE) {
            SubMenuItem* sub = reinterpret_cast<SubMenuItem*>(current);
            MenuItem* ret = recursiveFindById(sub->getChild(), id);
            if(ret != NULL) {
                return ret;
            }
        }
        else if(current->getId() == id) {
            return current;
        } 

        current = current->getNext();
    }
    return NULL;
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
    
    MenuItem* toReturn = NULL;
    while(!toReturn) {
        // if there's no current item we have to either find one or exit with NULL.
        if(currentItem == NULL) {
            if(level == 0) {
                // exhausted all options, clear down and exit.
                reset();
                return NULL;
            }
            else {
                // we are in the menu structure still, pop something off stack.
                currentItem = parentItems[--level]->getNext();
            }
        }

        while(currentItem && !toReturn) {
            if(currentItem->getMenuType() == MENUTYPE_SUB_VALUE) {

                // check if there's a predicate match on the submenu.
                bool predicateMatches = predicate == NULL || predicate->matches(currentItem);

                // if we have to report the submenu back to the callee, then we need to wait until
                // the next go to do that, otherwise we overwrite the parent too early.
                if(!processingSubMenu && predicateMatches) {
                    processingSubMenu = true;
                    return currentItem;
                }
                else processingSubMenu = true;
                
                // we always follow submenus even if we don't report them. But we sometimes need
                // to do it in two iterations.
                if(processingSubMenu) {
                    processingSubMenu = false;
                    parentItems[level++] = currentItem;
                    currentItem = ((SubMenuItem*)currentItem)->getChild();
                }
            }
            
            if(predicate == NULL || predicate->matches(currentItem)) toReturn = currentItem;

            // now try and find the next item, or reset if completely finished.
            currentItem = currentItem->getNext();
        }
    }
    return toReturn;
}

MenuItem* MenuItemIterator::currentParent() {
    if(level == 0) return NULL;
    else return parentItems[level - 1];
}

bool RemoteNoMenuItemPredicate::matches(MenuItem* item) { 
    return item->isSendRemoteNeeded(remoteNo) && !item->isLocalOnly(); 
}

bool MenuItemTypePredicate::matches(MenuItem* item) {
    if(bitRead(mode, 3) && item->isLocalOnly()) return false;

    if(bitRead(mode, 0))
        return item->getMenuType() != filterType;
    else
        return item->getMenuType() == filterType;
}