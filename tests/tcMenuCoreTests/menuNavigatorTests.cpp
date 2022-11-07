
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
