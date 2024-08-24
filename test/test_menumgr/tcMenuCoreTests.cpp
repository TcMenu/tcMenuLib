#include <unity.h>
#include <tcMenu.h>
#include <BaseRenderers.h>
#include <MockEepromAbstraction.h>
#include <MockIoAbstraction.h>
#include <MenuIterator.h>
#include "../tutils/fixtures_extern.h"

// here we set the pressMe menu item callback to our standard action callback.
void myActionCb(int id);
#define PRESSMECALLBACK myActionCb

const char *uuid1 = "07cd8bc6-734d-43da-84e7-6084990becfc";
const char *uuid2 = "07cd8bc6-734d-43da-84e7-6084990becfd";
const char *uuid3 = "07cd8bc6-734d-43da-84e7-6084990becfe";

MockedIoAbstraction mockIo;
NoRenderer noRenderer; 
MockEepromAbstraction eeprom(400);
char szData[10] = { "123456789" };
const char PROGMEM pgmMyName[]  = "UnitTest";
int counter = 0;
const PROGMEM ConnectorLocalInfo applicationInfo = { "DfRobot", "2ba37227-a412-40b7-94e7-42caf9bb0ff4" };

bool checkMenuItem(MenuItem* actual, MenuItem* expected) {
    if(expected != actual) {
        serdebugF("Menu items are not equal: expected=");
        printMenuItem(expected);
        serdebugF(". Actual=");
        printMenuItem(actual);
    }
    return (actual == expected);
}


void testTcUtilGetParentAndVisit() {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    TEST_ASSERT_TRUE(checkMenuItem(getParentRoot(nullptr), &menuVolume));
    TEST_ASSERT_TRUE(checkMenuItem(getParentRoot(&menuVolume), &menuVolume));
    TEST_ASSERT_TRUE(checkMenuItem(getParentRoot(&menuStatus), &menuVolume));
    TEST_ASSERT_TRUE(checkMenuItem(getParentRoot(&menuBackSettings), &menuVolume));
    TEST_ASSERT_TRUE(checkMenuItem(getParentRoot(&menuBackSecondLevel), &menuBackStatus));

    counter = 0;
    TEST_ASSERT_TRUE(checkMenuItem(getParentRootAndVisit(&menuBackSecondLevel, [](MenuItem* item) {
        counter++;
		// below is for debugging
        //Serial.print("Visited");printMenuItem(item);Serial.println();
    }), &menuBackStatus));
    TEST_ASSERT_EQUAL(counter, 15);
}

void testIteratorGetSubMenu() {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);
    // passing in null returns null
    TEST_ASSERT_TRUE(getSubMenuFor(nullptr) == nullptr);
    // root is presented as null
    TEST_ASSERT_TRUE(getSubMenuFor(&menuVolume) == nullptr);
    // now check both menu levels including providing a submenu within a submenu
    TEST_ASSERT_TRUE(checkMenuItem(getSubMenuFor(&menuPressMe), &menuSecondLevel));
    TEST_ASSERT_TRUE(checkMenuItem(getSubMenuFor(&menuSecondLevel), &menuStatus));
    TEST_ASSERT_TRUE(checkMenuItem(getSubMenuFor(&menuBackSettings), &menuSettings));
}

void testGetItemById() {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    TEST_ASSERT_TRUE(getMenuItemById(0) == nullptr);
    TEST_ASSERT_TRUE(checkMenuItem(getMenuItemById(1), &menuVolume));
    TEST_ASSERT_TRUE(checkMenuItem(getMenuItemById(menuBackStatus.getId()), &menuBackStatus));
    TEST_ASSERT_TRUE(checkMenuItem(getMenuItemById(5), &menuStatus));
    TEST_ASSERT_TRUE(checkMenuItem(getMenuItemById(101), &menuPressMe));
    TEST_ASSERT_TRUE(checkMenuItem(getMenuItemById(2), &menuChannel));
    TEST_ASSERT_TRUE(checkMenuItem(getMenuItemById(7), &menuLHSTemp));
    TEST_ASSERT_TRUE(checkMenuItem(getMenuItemById(103), &menuCaseTemp));

    // now test a few again for the 2nd time to ensure caching works as expected
    TEST_ASSERT_TRUE(checkMenuItem(getMenuItemById(5), &menuStatus));
    TEST_ASSERT_TRUE(checkMenuItem(getMenuItemById(101), &menuPressMe));
    TEST_ASSERT_TRUE(checkMenuItem(getMenuItemById(7), &menuLHSTemp));
}

void clearAllChangeStatus() {
    getParentRootAndVisit(&menuVolume, [](MenuItem* item) {
        item->clearSendRemoteNeededAll();
        item->setChanged(false);
    });
}

void testIterationWithPredicate() {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    clearAllChangeStatus();

    MenuItemTypePredicate subPredicate(MENUTYPE_SUB_VALUE);
    MenuItemIterator iterator;
    iterator.setPredicate(&subPredicate);

    for(int i=0;i<3;i++) {
        serdebugF2("Type Predicate item iteration ", i);

        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuSettings));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuStatus));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuSecondLevel));
        TEST_ASSERT_TRUE(iterator.nextItem() == nullptr);
    }

    RemoteNoMenuItemPredicate remotePredicate(0);
    iterator.setPredicate(&remotePredicate);

    // this predicate looks for a remote needing to be set, but we need to check that if the local only flag is set
    // on the item (or a parent submenu), we never send regardless of this state.
    menuVolume.setSendRemoteNeededAll();
    menuPressMe.setSendRemoteNeededAll();

    // prevent status and secondLevel (as under status) and volume from being sent.
    menuStatus.setLocalOnly(true);
    menuVolume.setLocalOnly(true);

    // now we set an item to remote send that is actually able to send remotely
    menu12VStandby.setSendRemoteNeededAll();

    TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuSettings));
    TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menu12VStandby));
    TEST_ASSERT_TRUE(iterator.nextItem() == nullptr);

    clearAllChangeStatus();

    TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuSettings));
    TEST_ASSERT_TRUE(iterator.nextItem() == nullptr);
}


void testIteratorTypePredicateLocalOnly() {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    clearAllChangeStatus();

    MenuItemTypePredicate intPredicate(MENUTYPE_INT_VALUE, TM_REGULAR_LOCAL_ONLY | TM_EXTRA_INCLUDE_SUBMENUS);
    MenuItemIterator iterator;
    iterator.setPredicate(&intPredicate);

    menuVolume.setLocalOnly(false);
    menuContrast.setLocalOnly(true);
    menuStatus.setLocalOnly(true);

    for(int i=0;i<3;i++) {
        serdebugF2("Local Int Predicate item iteration ", i);

        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuVolume));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuSettings));
        TEST_ASSERT_TRUE(iterator.nextItem() == nullptr);
    }

    menuVolume.setLocalOnly(true);
    menuContrast.setLocalOnly(false);
    menuStatus.setLocalOnly(false);

    for(int i=0;i<3;i++) {
        serdebugF2("Local Int Predicate (secondLevel local only)", i);

        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuSettings));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuContrast));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuStatus));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuLHSTemp));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuRHSTemp));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuSecondLevel));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuCaseTemp));
        TEST_ASSERT_TRUE(iterator.nextItem() == nullptr);
    }
}

class NothingMatchingMenuPredicate : public MenuItemPredicate {
    bool matches(MenuItem* ) override {
        return false;
    }
};

void testIteratorNothingMatchesPredicate() {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    clearAllChangeStatus();

    NothingMatchingMenuPredicate noMatch;
    MenuItemIterator iterator;
    iterator.setPredicate(&noMatch);
    iterator.reset();

    // should be repeatedly null, nothing matches.
    for(int i=0;i<10;i++) {
        TEST_ASSERT_TRUE(iterator.nextItem() == nullptr);
    }
}

void testIterationOverAllMenuItems() {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    MenuItemIterator iterator;
    iterator.setPredicate(nullptr);
    iterator.reset();

    // iterators should be completely repeatable
    for(int i=0;i<3;i++) {
        serdebugF2("All item iteration ", i);

        // this should be the list of items in the text fixture exactly as they
        // are laid out in the file, IE depth first ordering.

        // first we get the volume and channel
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(),&menuVolume));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuChannel));
        TEST_ASSERT_TRUE(iterator.currentParent() == nullptr);

        // then traverse into the settings menu (parent is volume)
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuSettings));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.currentParent(), nullptr));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuBackSettings));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.currentParent(), &menuSettings));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menu12VStandby));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuContrast));

        // and then into the status menu, which has a nested submenu
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuStatus));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.currentParent(), nullptr));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuBackStatus));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.currentParent(), &menuStatus));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuLHSTemp));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.currentParent(), &menuStatus));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuRHSTemp));
        // nested sub menu of status here
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuSecondLevel));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.currentParent(), &menuStatus));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuBackSecondLevel));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.currentParent(), &menuSecondLevel));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuPressMe));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuFloatItem));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.currentParent(), &menuSecondLevel));
        // exit of nested submenu here.
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuCaseTemp));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.currentParent(), &menuStatus));
        TEST_ASSERT_TRUE(iterator.nextItem() == nullptr);
    }
}

void testIterationOnSimpleMenu() {
    menuMgr.initWithoutInput(&noRenderer, &menuSimple1);

    MenuItemIterator iterator;
    iterator.setPredicate(nullptr);
    iterator.reset();

    for(int i=0;i<3;i++) {
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuSimple1));
        TEST_ASSERT_TRUE(checkMenuItem(iterator.nextItem(), &menuSimple2));
        TEST_ASSERT_TRUE(iterator.nextItem() == nullptr);
    }
}


