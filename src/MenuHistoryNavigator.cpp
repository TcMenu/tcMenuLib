
#include <IoLogging.h>
#include "MenuHistoryNavigator.h"
#include "MenuIterator.h"
#include "RuntimeMenuItem.h"

void tcnav::MenuNavigationStore::setRootItem(MenuItem *item) {
    root = item;
    currentRoot = root;
    currentSub = nullptr;
    navIdx = 0;
}

bool tcnav::MenuNavigationStore::isItemWithinRoot(MenuItem *toCheck, MenuItem *item) {
    if(item == nullptr) return true;
    while(toCheck) {
        if(item == toCheck) return true;
        if(toCheck->getMenuType() == MENUTYPE_SUB_VALUE) {
            if(isItemWithinRoot(reinterpret_cast<SubMenuItem*>(toCheck)->getChild(), item)) return true;
        }
        toCheck = toCheck->getNext();
    }
    return false;
}

void tcnav::MenuNavigationStore::navigateTo(MenuItem *activeItem, MenuItem *newRoot) {
    // we only stack when
    if(navIdx < NAV_ITEM_ARRAY_SIZE && isItemWithinRoot(root, currentSub)) {
        navItems[navIdx] = currentRoot;
        activeItems[navIdx] = activeItem ? activeItem : currentRoot;
        serdebugF4("NavigateTo pushed ", navIdx, currentRoot->getId(), activeItems[navIdx]->getId());
        navIdx++;
    }
    else {
        serdebugF("Nav exceeded");
    }
    currentRoot = newRoot;
    currentSub = getSubMenuFor(newRoot);
}

MenuItem *tcnav::MenuNavigationStore::popNavigationGetActive() {
    if(navIdx == 0) {
        serdebugF("Nav pop root");
        currentSub = nullptr;
        currentRoot = root;
        return root;
    } else {
        navIdx--;
        currentRoot = navItems[navIdx];
        currentSub = getSubMenuFor(currentRoot);
        serdebugF3("Nav pop ", navIdx, currentRoot->getId());
        return activeItems[navIdx];
    }
}
