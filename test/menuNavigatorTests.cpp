
#include <unity.h>
#include <MenuHistoryNavigator.h>
#include "fixtures_extern.h"

using namespace tcnav;

void testCreatingAndInitialisation() {
    MenuNavigationStore nav;
    nav.setRootItem(&menuVolume);

    TEST_ASSERT_EQUAL(&menuVolume, nav.getRoot());
    TEST_ASSERT_EQUAL(&menuVolume, nav.getCurrentRoot());
    TEST_ASSERT_EQUAL(nullptr, nav.getCurrentSubMenu());
}

void testNavigationPushAndPop() {
    MenuNavigationStore nav;
    nav.setRootItem(&menuVolume);
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    nav.navigateTo(&menuChannel, menuStatus.getChild(), false);
    nav.navigateTo(&menuLHSTemp, menuSecondLevel.getChild(), false);
    nav.navigateTo(&menu12VStandby, menuSettings.getChild(), false);
    nav.navigateTo(&menuRHSTemp, &menuSub, false); // should not be stored
    nav.navigateTo(&menuRHSTemp, &menuSub, false);

    auto* act = nav.popNavigationGetActive();
    TEST_ASSERT_EQUAL(act, &menuRHSTemp);
    TEST_ASSERT_EQUAL(menuSettings.getChild(), nav.getCurrentRoot());
    TEST_ASSERT_EQUAL(&menuSettings, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    TEST_ASSERT_EQUAL(act, &menu12VStandby);
    TEST_ASSERT_EQUAL(menuSecondLevel.getChild(), nav.getCurrentRoot());
    TEST_ASSERT_EQUAL(&menuSecondLevel, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    TEST_ASSERT_EQUAL(act, &menuLHSTemp);
    TEST_ASSERT_EQUAL(menuStatus.getChild(), nav.getCurrentRoot());
    TEST_ASSERT_EQUAL(&menuStatus, nav.getCurrentSubMenu());

    act = nav.popNavigationGetActive();
    TEST_ASSERT_EQUAL(act, &menuChannel);
    TEST_ASSERT_EQUAL(&menuVolume, nav.getCurrentRoot());
    TEST_ASSERT_EQUAL(nullptr, nav.getCurrentSubMenu());

    // try and over pop from array.
    act = nav.popNavigationGetActive();
    TEST_ASSERT_EQUAL(act, &menuVolume);
    TEST_ASSERT_EQUAL(&menuVolume, nav.getCurrentRoot());
    TEST_ASSERT_EQUAL(nullptr, nav.getCurrentSubMenu());
}

void testRebuildingNavigation() {
    MenuNavigationStore nav;
    nav.setRootItem(&menuVolume);
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    // now add three levels of navigation
    nav.navigateTo(&menuChannel, menuStatus.getChild(), false);
    nav.navigateTo(&menuLHSTemp, menuSecondLevel.getChild(), false);
    nav.navigateTo(&menu12VStandby, menuSettings.getChild(), false);

    // now attempt to iterate through the navigation stack
    TEST_ASSERT_EQUAL(3, nav.getNavigationDepth());
    TEST_ASSERT_EQUAL(&menuChannel, nav.getActiveAt(0));
    TEST_ASSERT_EQUAL(&menuLHSTemp, nav.getActiveAt(1));
    TEST_ASSERT_EQUAL(&menu12VStandby, nav.getActiveAt(2));
    TEST_ASSERT_EQUAL(nav.getRoot(), nav.getRootAt(0));
    TEST_ASSERT_EQUAL(menuStatus.getChild(), nav.getRootAt(1));
    TEST_ASSERT_EQUAL(menuSecondLevel.getChild(), nav.getRootAt(2));
    TEST_ASSERT_EQUAL(menuSettings.getChild(), nav.getCurrentRoot());

    // now reset the stack and ensure it is empty
    nav.resetStack();
    TEST_ASSERT_EQUAL(0, nav.getNavigationDepth());

    // put three items back into the stack
    nav.navigateTo(&menuChannel, menuStatus.getChild(), false);
    nav.navigateTo(&menuLHSTemp, menuSecondLevel.getChild(), false);
    nav.navigateTo(&menu12VStandby, menuSettings.getChild(), false);

    // and check they are there.
    TEST_ASSERT_EQUAL(3, nav.getNavigationDepth());
    TEST_ASSERT_EQUAL(&menuChannel, nav.getActiveAt(0));
    TEST_ASSERT_EQUAL(&menuLHSTemp, nav.getActiveAt(1));
    TEST_ASSERT_EQUAL(&menu12VStandby, nav.getActiveAt(2));
}