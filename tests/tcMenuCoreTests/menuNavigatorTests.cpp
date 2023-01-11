
#include <testing/SimpleTest.h>
#include <MenuHistoryNavigator.h>
#include "fixtures_extern.h"

using namespace tcnav;

test(creatingAndInitialisation) {
    MenuNavigationStore nav;
    nav.setRootItem(&menuVolume);

    assertEquals(&menuVolume, nav.getRoot());
    assertEquals(&menuVolume, nav.getCurrentRoot());
    assertEquals(nullptr, nav.getCurrentSubMenu());
}

test(navigationPushAndPop) {
    MenuNavigationStore nav;
    nav.setRootItem(&menuVolume);
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    nav.navigateTo(&menuChannel, menuStatus.getChild(), false);
    nav.navigateTo(&menuLHSTemp, menuSecondLevel.getChild(), false);
    nav.navigateTo(&menu12VStandby, menuSettings.getChild(), false);
    nav.navigateTo(&menuRHSTemp, &menuSub, false); // should not be stored
    nav.navigateTo(&menuRHSTemp, &menuSub, false);

    auto* act = nav.popNavigationGetActive();
    assertEquals(act, &menuRHSTemp);
    assertEquals(menuSettings.getChild(), nav.getCurrentRoot());
    assertEquals(&menuSettings, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    assertEquals(act, &menu12VStandby);
    assertEquals(menuSecondLevel.getChild(), nav.getCurrentRoot());
    assertEquals(&menuSecondLevel, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    assertEquals(act, &menuLHSTemp);
    assertEquals(menuStatus.getChild(), nav.getCurrentRoot());
    assertEquals(&menuStatus, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    assertEquals(act, &menuChannel);
    assertEquals(&menuVolume, nav.getCurrentRoot());
    assertEquals(nullptr, nav.getCurrentSubMenu());

    // try and over pop from array.
    act = nav.popNavigationGetActive();
    assertEquals(act, &menuVolume);
    assertEquals(&menuVolume, nav.getCurrentRoot());
    assertEquals(nullptr, nav.getCurrentSubMenu());
}

test(testRebuildingNavigation) {
    MenuNavigationStore nav;
    nav.setRootItem(&menuVolume);
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    // now add three levels of navigation
    nav.navigateTo(&menuChannel, menuStatus.getChild(), false);
    nav.navigateTo(&menuLHSTemp, menuSecondLevel.getChild(), false);
    nav.navigateTo(&menu12VStandby, menuSettings.getChild(), false);

    // now attempt to iterate through the navigation stack
    assertEquals(3, nav.getNavigationDepth());
    assertEquals(&menuChannel, nav.getActiveAt(0));
    assertEquals(&menuLHSTemp, nav.getActiveAt(1));
    assertEquals(&menu12VStandby, nav.getActiveAt(2));
    assertEquals(nav.getRoot(), nav.getRootAt(0));
    assertEquals(menuStatus.getChild(), nav.getRootAt(1));
    assertEquals(menuSecondLevel.getChild(), nav.getRootAt(2));
    assertEquals(menuSettings.getChild(), nav.getCurrentRoot());

    // now reset the stack and ensure it is empty
    nav.resetStack();
    assertEquals(0, nav.getNavigationDepth());

    // put three items back into the stack
    nav.navigateTo(&menuChannel, menuStatus.getChild(), false);
    nav.navigateTo(&menuLHSTemp, menuSecondLevel.getChild(), false);
    nav.navigateTo(&menu12VStandby, menuSettings.getChild(), false);

    // and check they are there.
    assertEquals(3, nav.getNavigationDepth());
    assertEquals(&menuChannel, nav.getActiveAt(0));
    assertEquals(&menuLHSTemp, nav.getActiveAt(1));
    assertEquals(&menu12VStandby, nav.getActiveAt(2));
}