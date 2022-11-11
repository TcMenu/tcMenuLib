#include <testing/SimpleTest.h>
#include <tcMenu.h>
#include "tcMenuFixtures.h"
#include <BaseRenderers.h>
#include <MockEepromAbstraction.h>
#include <MockIoAbstraction.h>
#include <MenuIterator.h>

// here we set the pressMe menu item callback to our standard action callback.
void myActionCb(int id);
#define PRESSMECALLBACK myActionCb

#include <tcm_test/testFixtures.h>

using namespace SimpleTest;

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

IOLOG_MBED_PORT_IF_NEEDED(USBTX, USBRX)

void setup() {
    IOLOG_START_SERIAL
    serEnableLevel(SER_IOA_DEBUG, true);
    startTesting();

    menuMgr.initWithoutInput(&noRenderer, &menuVolume);
}

DEFAULT_TEST_RUNLOOP

void printMenuItem(MenuItem* menuItem) {
    if(menuItem == nullptr) {
        serdebugF("NULL");
    }
    else {
        char buffer[20];
        menuItem->copyNameToBuffer(buffer, sizeof buffer);
        serdebug3(menuItem->getId(), menuItem->getMenuType(),buffer);
    }
}

class MenuItemIteratorFixture : public UnitTestExecutor {
public:
    bool checkMenuItem(MenuItem* actual, MenuItem* expected) {
        if(expected != actual) {
            serdebugF("Menu items are not equal: expected=");
            printMenuItem(expected);
            serdebugF(". Actual=");
            printMenuItem(actual);
        }
        return (actual == expected);
    }
};

testF(MenuItemIteratorFixture, testTcUtilGetParentAndVisit) {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    assertTrue(checkMenuItem(getParentRoot(nullptr), &menuVolume));
    assertTrue(checkMenuItem(getParentRoot(&menuVolume), &menuVolume));
    assertTrue(checkMenuItem(getParentRoot(&menuStatus), &menuVolume));
    assertTrue(checkMenuItem(getParentRoot(&menuBackSettings), &menuVolume));
    assertTrue(checkMenuItem(getParentRoot(&menuBackSecondLevel), &menuBackStatus));

    counter = 0;
    assertTrue(checkMenuItem(getParentRootAndVisit(&menuBackSecondLevel, [](MenuItem* item) {
        counter++;
		// below is for debugging
        //Serial.print("Visited");printMenuItem(item);Serial.println();
    }), &menuBackStatus));
    assertEquals(counter, 15);
}

testF(MenuItemIteratorFixture, testIteratorGetSubMenu) {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);
    // passing in null returns null
    assertTrue(getSubMenuFor(nullptr) == nullptr);
    // root is presented as null
    assertTrue(getSubMenuFor(&menuVolume) == nullptr);
    // now check both menu levels including providing a submenu within a submenu
    assertTrue(checkMenuItem(getSubMenuFor(&menuPressMe), &menuSecondLevel));
    assertTrue(checkMenuItem(getSubMenuFor(&menuSecondLevel), &menuStatus));
    assertTrue(checkMenuItem(getSubMenuFor(&menuBackSettings), &menuSettings));
}

testF(MenuItemIteratorFixture, testGetItemById) {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    assertTrue(getMenuItemById(0) == nullptr);
    assertTrue(checkMenuItem(getMenuItemById(1), &menuVolume));
    assertTrue(checkMenuItem(getMenuItemById(menuBackStatus.getId()), &menuBackStatus));
    assertTrue(checkMenuItem(getMenuItemById(5), &menuStatus));
    assertTrue(checkMenuItem(getMenuItemById(101), &menuPressMe));
    assertTrue(checkMenuItem(getMenuItemById(2), &menuChannel));
    assertTrue(checkMenuItem(getMenuItemById(7), &menuLHSTemp));
    assertTrue(checkMenuItem(getMenuItemById(103), &menuCaseTemp));

    // now test a few again for the 2nd time to ensure caching works as expected
    assertTrue(checkMenuItem(getMenuItemById(5), &menuStatus));
    assertTrue(checkMenuItem(getMenuItemById(101), &menuPressMe));
    assertTrue(checkMenuItem(getMenuItemById(7), &menuLHSTemp));
}

void clearAllChangeStatus() {
    getParentRootAndVisit(&menuVolume, [](MenuItem* item) {
        item->clearSendRemoteNeededAll();
        item->setChanged(false);
    });
}

testF(MenuItemIteratorFixture, testIterationWithPredicate) {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    clearAllChangeStatus();

    MenuItemTypePredicate subPredicate(MENUTYPE_SUB_VALUE);
    MenuItemIterator iterator;
    iterator.setPredicate(&subPredicate);

    for(int i=0;i<3;i++) {
        serdebugF2("Type Predicate item iteration ", i);

        assertTrue(checkMenuItem(iterator.nextItem(), &menuSettings));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuStatus));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuSecondLevel));
        assertTrue(iterator.nextItem() == nullptr);
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

    assertTrue(checkMenuItem(iterator.nextItem(), &menuSettings));
    assertTrue(checkMenuItem(iterator.nextItem(), &menu12VStandby));
    assertTrue(iterator.nextItem() == nullptr);

    clearAllChangeStatus();

    assertTrue(checkMenuItem(iterator.nextItem(), &menuSettings));
    assertTrue(iterator.nextItem() == nullptr);
}


testF(MenuItemIteratorFixture, testIteratorTypePredicateLocalOnly) {
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

        assertTrue(checkMenuItem(iterator.nextItem(), &menuVolume));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuSettings));
        assertTrue(iterator.nextItem() == nullptr);
    }

    menuVolume.setLocalOnly(true);
    menuContrast.setLocalOnly(false);
    menuStatus.setLocalOnly(false);

    for(int i=0;i<3;i++) {
        serdebugF2("Local Int Predicate (secondLevel local only)", i);

        assertTrue(checkMenuItem(iterator.nextItem(), &menuSettings));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuContrast));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuStatus));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuLHSTemp));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuRHSTemp));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuSecondLevel));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuCaseTemp));
        assertTrue(iterator.nextItem() == nullptr);
    }
}

class NothingMatchingMenuPredicate : public MenuItemPredicate {
    bool matches(MenuItem* ) override {
        return false;
    }
};

testF(MenuItemIteratorFixture, testIteratorNothingMatchesPredicate) {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    clearAllChangeStatus();

    NothingMatchingMenuPredicate noMatch;
    MenuItemIterator iterator;
    iterator.setPredicate(&noMatch);
    iterator.reset();

    // should be repeatedly null, nothing matches.
    for(int i=0;i<10;i++) {
        assertTrue(iterator.nextItem() == nullptr);
    }
}

testF(MenuItemIteratorFixture, testIterationOverAllMenuItems) {
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
        assertTrue(checkMenuItem(iterator.nextItem(),&menuVolume));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuChannel));
        assertTrue(iterator.currentParent() == nullptr);

        // then traverse into the settings menu (parent is volume)
        assertTrue(checkMenuItem(iterator.nextItem(), &menuSettings));
        assertTrue(checkMenuItem(iterator.currentParent(), nullptr));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuBackSettings));
        assertTrue(checkMenuItem(iterator.currentParent(), &menuSettings));
        assertTrue(checkMenuItem(iterator.nextItem(), &menu12VStandby));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuContrast));

        // and then into the status menu, which has a nested submenu
        assertTrue(checkMenuItem(iterator.nextItem(), &menuStatus));
        assertTrue(checkMenuItem(iterator.currentParent(), nullptr));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuBackStatus));
        assertTrue(checkMenuItem(iterator.currentParent(), &menuStatus));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuLHSTemp));
        assertTrue(checkMenuItem(iterator.currentParent(), &menuStatus));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuRHSTemp));
        // nested sub menu of status here
        assertTrue(checkMenuItem(iterator.nextItem(), &menuSecondLevel));
        assertTrue(checkMenuItem(iterator.currentParent(), &menuStatus));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuBackSecondLevel));
        assertTrue(checkMenuItem(iterator.currentParent(), &menuSecondLevel));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuPressMe));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuFloatItem));
        assertTrue(checkMenuItem(iterator.currentParent(), &menuSecondLevel));
        // exit of nested submenu here.
        assertTrue(checkMenuItem(iterator.nextItem(), &menuCaseTemp));
        assertTrue(checkMenuItem(iterator.currentParent(), &menuStatus));
        assertTrue(iterator.nextItem() == nullptr);
    }
}

testF(MenuItemIteratorFixture, testIterationOnSimpleMenu) {
    menuMgr.initWithoutInput(&noRenderer, &menuSimple1);

    MenuItemIterator iterator;
    iterator.setPredicate(nullptr);
    iterator.reset();

    for(int i=0;i<3;i++) {
        assertTrue(checkMenuItem(iterator.nextItem(), &menuSimple1));
        assertTrue(checkMenuItem(iterator.nextItem(), &menuSimple2));
        assertTrue(iterator.nextItem() == nullptr);
    }
}
