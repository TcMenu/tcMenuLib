#line 2 "tcMenuCoreTests.ino"

#include <AUnit.h>
#include <tcMenu.h>
#include "tcMenuFixtures.h"
#include <BaseRenderers.h>
#include <MockEepromAbstraction.h>
#include <MockIoAbstraction.h>
#include <MenuIterator.h>

#include "menuManagerTests.h"
#include "authenticationTests.h"
#include "menuItemTests.h"
#include "baseDialogTests.h"
//#include "baseRemoteTests.h"
#include <tcm_test/testFixtures.h>

using namespace aunit;

MockedIoAbstraction mockIo;
NoRenderer noRenderer; 
MockEepromAbstraction eeprom(400);
char szData[10] = { "123456789" };
const char PROGMEM pgmMyName[]  = "UnitTest";
int counter = 0;

void setup() {
    Serial.begin(115200);
    while(!Serial);

    menuMgr.initWithoutInput(&noRenderer, &menuVolume);
}

void loop() {
    TestRunner::run();
}


test(testTcUtilIntegerConversions) {
    char szBuffer[20];
    
    // first check the basic cases for the version that always starts at pos 0
    strcpy(szBuffer, "abc");
    ltoaClrBuff(szBuffer, 1234, 4, ' ', sizeof(szBuffer));
    assertEqual(szBuffer, "1234");
    ltoaClrBuff(szBuffer, 907, 4, ' ', sizeof(szBuffer));
    assertEqual(szBuffer, " 907");
    ltoaClrBuff(szBuffer, 22, 4, '0', sizeof(szBuffer));
    assertEqual(szBuffer, "0022");
	ltoaClrBuff(szBuffer, -22, 4, '0', sizeof(szBuffer));
	assertEqual(szBuffer, "-0022");
	ltoaClrBuff(szBuffer, -93, 2, NOT_PADDED, sizeof(szBuffer));
	assertEqual(szBuffer, "-93");

    // and now test the appending version with zero padding
    strcpy(szBuffer, "val = ");
    fastltoa(szBuffer, 22, 4, '0', sizeof(szBuffer));
    assertEqual(szBuffer, "val = 0022");

    // and now test the appending version with an absolute divisor.
    strcpy(szBuffer, "val = ");
    fastltoa_mv(szBuffer, 22, 1000, '0', sizeof(szBuffer));
    assertEqual(szBuffer, "val = 022");

    // and lasty try the divisor version without 0.
    strcpy(szBuffer, "val = ");
    fastltoa_mv(szBuffer, 22, 10000, NOT_PADDED, sizeof(szBuffer));

    // and now try something bigger than the divisor
    strcpy(szBuffer, "val = ");
    fastltoa_mv(szBuffer, 222222, 10000, NOT_PADDED, sizeof(szBuffer));
    assertEqual(szBuffer, "val = 2222");
}

void printMenuItem(MenuItem* menuItem) {
    if(menuItem == NULL) {
        Serial.print("NULL");
    }
    else {
        char buffer[20];
        menuItem->copyNameToBuffer(buffer, sizeof buffer);
        Serial.print(menuItem->getId());Serial.print(',');Serial.print(menuItem->getMenuType());Serial.print(',');Serial.print(buffer);
    }
}

class MenuItemIteratorFixture : public TestOnce {
public:
    void assertMenuItem(MenuItem* actual, MenuItem* expected) {
        if(expected != actual) {
            Serial.print("Menu items are not equal: expected=");
            printMenuItem(expected);
            Serial.print(". Actual=");
            printMenuItem(actual);
            Serial.println();
        }
        assertTrue(actual == expected);
    }
};


testF(MenuItemIteratorFixture, testTcUtilGetParentAndVisit) {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    assertMenuItem(getParentRoot(NULL), &menuVolume);
    assertMenuItem(getParentRoot(&menuVolume), &menuVolume);
    assertMenuItem(getParentRoot(&menuStatus), &menuVolume);
    assertMenuItem(getParentRoot(&menuBackSettings), &menuVolume);
    assertMenuItem(getParentRoot(&menuBackSecondLevel), &menuBackStatus);

    counter = 0;
    assertMenuItem(getParentRootAndVisit(&menuBackSecondLevel, [](MenuItem* item) { 
        counter++;
		// below is for debugging
        //Serial.print("Visited");printMenuItem(item);Serial.println();
    }), &menuBackStatus);
    assertEqual(counter, 15);
}

testF(MenuItemIteratorFixture, testGetItemById) {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    assertTrue(getMenuItemById(0) == NULL);
    assertMenuItem(getMenuItemById(1), &menuVolume);
    assertMenuItem(getMenuItemById(menuBackStatus.getId()), &menuBackStatus);
    assertMenuItem(getMenuItemById(101), &menuPressMe);
    assertMenuItem(getMenuItemById(2), &menuChannel);
    assertMenuItem(getMenuItemById(7), &menuLHSTemp);
    assertMenuItem(getMenuItemById(103), &menuCaseTemp);
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
        Serial.print("Type Predicate item iteration ");Serial.println(i);

        assertMenuItem(iterator.nextItem(), &menuSettings);
        assertMenuItem(iterator.nextItem(), &menuStatus);
        assertMenuItem(iterator.nextItem(), &menuSecondLevel);
        assertTrue(iterator.nextItem() == NULL);
    }

    RemoteNoMenuItemPredicate remotePredicate(0);
    iterator.setPredicate(&remotePredicate);

    // this predicate looks for a remote needing to be set.
    menuVolume.setSendRemoteNeededAll();
    menuPressMe.setSendRemoteNeededAll();

    // prevent menu volume from being sent as it is local only
    menuVolume.setLocalOnly(true);
    
    assertMenuItem(iterator.nextItem(), &menuPressMe);
    assertTrue(iterator.nextItem() == NULL);

    clearAllChangeStatus();

    assertTrue(iterator.nextItem() == NULL);
}


testF(MenuItemIteratorFixture, testIteratorTypePredicateLocalOnly) {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    clearAllChangeStatus();

    MenuItemTypePredicate intPredicate(MENUTYPE_INT_VALUE, TM_REGULAR_LOCAL_ONLY);
    MenuItemIterator iterator;
    iterator.setPredicate(&intPredicate);

    menuVolume.setLocalOnly(true);
    menuContrast.setLocalOnly(true);

    for(int i=0;i<3;i++) {
        Serial.print("Local Int Predicate item iteration ");Serial.println(i);

        assertMenuItem(iterator.nextItem(), &menuLHSTemp);
        assertMenuItem(iterator.nextItem(), &menuRHSTemp);
        assertMenuItem(iterator.nextItem(), &menuCaseTemp);
        assertTrue(iterator.nextItem() == NULL);
    }
}

class NothingMatchingMenuPredicate : public MenuItemPredicate {
    bool matches(MenuItem* /*ignored*/) override {
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
        assertTrue(iterator.nextItem() == NULL);
    }
}

testF(MenuItemIteratorFixture, testIterationOverAllMenuItems) {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    MenuItemIterator iterator;
    iterator.setPredicate(NULL);
    iterator.reset();

    // iterators should be completely repeatable
    for(int i=0;i<3;i++) {
        Serial.print("All item iteration ");Serial.println(i);

        // this should be the list of items in the text fixture exactly as they
        // are laid out in the file, IE depth first ordering.

        // first we get the volume and channel
        assertMenuItem(iterator.nextItem(),&menuVolume);
        assertMenuItem(iterator.nextItem(), &menuChannel);
        assertTrue(iterator.currentParent() == NULL);

        // then traverse into the settings menu (parent is volume)
        assertMenuItem(iterator.nextItem(), &menuSettings);
        assertMenuItem(iterator.currentParent(), NULL);
        assertMenuItem(iterator.nextItem(), &menuBackSettings);
        assertMenuItem(iterator.currentParent(), &menuSettings);
        assertMenuItem(iterator.nextItem(), &menu12VStandby);
        assertMenuItem(iterator.nextItem(), &menuContrast);

        // and then into the status menu, which has a nested submenu
        assertMenuItem(iterator.nextItem(), &menuStatus);
        assertMenuItem(iterator.currentParent(), NULL);
        assertMenuItem(iterator.nextItem(), &menuBackStatus);
        assertMenuItem(iterator.currentParent(), &menuStatus);
        assertMenuItem(iterator.nextItem(), &menuLHSTemp);
        assertMenuItem(iterator.currentParent(), &menuStatus);
        assertMenuItem(iterator.nextItem(), &menuRHSTemp);
        // nested sub menu of status here
        assertMenuItem(iterator.nextItem(), &menuSecondLevel);
        assertMenuItem(iterator.currentParent(), &menuStatus);
        assertMenuItem(iterator.nextItem(), &menuBackSecondLevel);
        assertMenuItem(iterator.currentParent(), &menuSecondLevel);
        assertMenuItem(iterator.nextItem(), &menuPressMe);
        assertMenuItem(iterator.nextItem(), &menuFloatItem);
        assertMenuItem(iterator.currentParent(), &menuSecondLevel);
        // exit of nested submenu here.
        assertMenuItem(iterator.nextItem(), &menuCaseTemp);
        assertMenuItem(iterator.currentParent(), &menuStatus);
        assertTrue(iterator.nextItem() == NULL);
    }
}

testF(MenuItemIteratorFixture, testIterationOnSimpleMenu) {
    menuMgr.initWithoutInput(&noRenderer, &menuSimple1);

    MenuItemIterator iterator;
    iterator.setPredicate(NULL);
    iterator.reset();

    for(int i=0;i<3;i++) {
        assertMenuItem(iterator.nextItem(), &menuSimple1);
        assertMenuItem(iterator.nextItem(), &menuSimple2);
        assertTrue(iterator.nextItem() == NULL);
    }
}