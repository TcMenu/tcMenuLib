
#include <AUnit.h>
#include <MenuHistoryNavigator.h>
#include "fixtures_extern.h"

using namespace tcnav;

test(creatingAndInitialisation) {
    MenuNavigationStore nav;
    nav.setRootItem(&menuVolume);

    assertEqual(&menuVolume, nav.getRoot());
    assertEqual(&menuVolume, nav.getCurrentRoot());
    assertEqual(nullptr, nav.getCurrentSubMenu());
}

test(isWithinRootIsWorkingProperly) {
    MenuNavigationStore nav;
    nav.setRootItem(&menuVolume);

    assertTrue(nav.isItemWithinRoot(&menuVolume, &menuVolume));     // top level items
    assertTrue(nav.isItemWithinRoot(&menuVolume, &menuSettings));
    assertTrue(nav.isItemWithinRoot(&menuVolume, &menuSecondLevel));// two test on nested items
    assertTrue(nav.isItemWithinRoot(&menuVolume, &menuPressMe));
    assertTrue(nav.isItemWithinRoot(&menuVolume, nullptr));         // null returns true
    assertFalse(nav.isItemWithinRoot(&menuVolume, &menuNumTwoDp));  // from a different tree
}

test(navigationPushAndPop) {
    MenuNavigationStore nav;
    nav.setRootItem(&menuVolume);
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    nav.navigateTo(&menuChannel, menuStatus.getChild());
    nav.navigateTo(&menuLHSTemp, menuSecondLevel.getChild());
    nav.navigateTo(&menu12VStandby, menuSettings.getChild());
    nav.navigateTo(&menuRHSTemp, &menuSub); // should not be stored
    nav.navigateTo(&menuRHSTemp, &menuSub);

    auto* act = nav.popNavigationGetActive();
    assertEqual(act, &menuRHSTemp);
    assertEqual(menuSettings.getChild(), nav.getCurrentRoot());
    assertEqual(&menuSettings, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    assertEqual(act, &menu12VStandby);
    assertEqual(menuSecondLevel.getChild(), nav.getCurrentRoot());
    assertEqual(&menuSecondLevel, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    assertEqual(act, &menuLHSTemp);
    assertEqual(menuStatus.getChild(), nav.getCurrentRoot());
    assertEqual(&menuStatus, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    assertEqual(act, &menuChannel);
    assertEqual(&menuVolume, nav.getCurrentRoot());
    assertEqual(nullptr, nav.getCurrentSubMenu());

    // try and over pop from array.
    act = nav.popNavigationGetActive();
    assertEqual(act, &menuVolume);
    assertEqual(&menuVolume, nav.getCurrentRoot());
    assertEqual(nullptr, nav.getCurrentSubMenu());
}
