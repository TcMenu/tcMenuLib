#include <unity.h>
#include <tcMenu.h>
#include "../tutils/fixtures_extern.h"
#include <tcm_test/testFixtures.h>
#include "../tutils/tcMenuFixturesExtra.h"

const char *uuid1 = "07cd8bc6-734d-43da-84e7-6084990becfc";
const char *uuid2 = "07cd8bc6-734d-43da-84e7-6084990becfd";
const char *uuid3 = "07cd8bc6-734d-43da-84e7-6084990becfe";

NoRenderer noRenderer;
MockEepromAbstraction eeprom(400);

// core menu item tests
void testIpAddressItem();
void testSettingIpItemDirectly();
void testFloatType();
void testAuthMenuItem();

// Date time tests
void testTimeMenuItem12Hr();
void testTimeMenuItem24Hr();
void testTimeMenuItemDuration();
void testTimeMenuItem24HrEditing();
void testDateFormattedMenuItem();
void testDateLeapYearAndMonthSizes();

// large number
void testGettingAndSettingDigits();
void testLargeNumberGetAndSet();
void testLargeNumberEditableMenuItem();
void testPersistLargeInteger();
void testSetLargeIntFromString();
void testLargeNumWithNegativeNotAllowed();
void testLargeNumberWithOneDecimalPlace();

// scroll choice
void testValueAtPositionAndGetters();
void testValueAtPositionEeeprom();
void testValueAtPositionCustom();
void testColorMenuItemNoAlpha();
void testColorMenuItemWithAlphaAndFn();
void testColor32Struct();


// value item cases
void testCoreAndBooleanMenuItem();
void testEnumMenuItem();
void testAnalogMenuItem();
void testAnalogItemNegativeInteger();
void testGetIntValueIncudingOffset();
void testAnalogValuesWithFractions();
void testAnalogValueItemInMemory();
void testBooleanItemInMemory();
void testFloatItemInMemory();

// runtime menu item
void testBasicRuntimeMenuItem();
void testListRuntimeItem();
void testTextMenuItemFromEmpty();
void testFindEditorSetFunction();
void testTextPasswordItem();
void testTextRuntimeItem();
void testSubMenuItem();
void testActionMenuItem();

void setup() {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    UNITY_BEGIN();
    /* runtime item */
    RUN_TEST(testBasicRuntimeMenuItem);
    RUN_TEST(testListRuntimeItem);
    RUN_TEST(testTextMenuItemFromEmpty);
    RUN_TEST(testFindEditorSetFunction);
    RUN_TEST(testTextPasswordItem);
    RUN_TEST(testTextRuntimeItem);
    RUN_TEST(testSubMenuItem);
    RUN_TEST(testActionMenuItem);

    /* value item */
    RUN_TEST(testCoreAndBooleanMenuItem);
    RUN_TEST(testEnumMenuItem);
    RUN_TEST(testAnalogMenuItem);
    RUN_TEST(testAnalogItemNegativeInteger);
    RUN_TEST(testGetIntValueIncudingOffset);
    RUN_TEST(testAnalogValuesWithFractions);
    RUN_TEST(testAnalogValueItemInMemory);
    RUN_TEST(testBooleanItemInMemory);
    RUN_TEST(testFloatItemInMemory);

    /* scroll choice */
    RUN_TEST(testValueAtPositionAndGetters);
    RUN_TEST(testValueAtPositionEeeprom);
    RUN_TEST(testValueAtPositionCustom);
    RUN_TEST(testColorMenuItemNoAlpha);
    RUN_TEST(testColorMenuItemWithAlphaAndFn);
    RUN_TEST(testColor32Struct);

    /* large number */
    RUN_TEST(testGettingAndSettingDigits);
    RUN_TEST(testLargeNumberGetAndSet);
    RUN_TEST(testLargeNumberEditableMenuItem);
    RUN_TEST(testPersistLargeInteger);
    RUN_TEST(testSetLargeIntFromString);
    RUN_TEST(testLargeNumWithNegativeNotAllowed);
    RUN_TEST(testLargeNumberWithOneDecimalPlace);

    /* date time */
    RUN_TEST(testTimeMenuItem12Hr);
    RUN_TEST(testTimeMenuItem24Hr);
    RUN_TEST(testTimeMenuItemDuration);
    RUN_TEST(testTimeMenuItem24HrEditing);
    RUN_TEST(testDateFormattedMenuItem);
    RUN_TEST(testDateLeapYearAndMonthSizes);

    /* core item tests - keep last */
    RUN_TEST(testIpAddressItem);
    RUN_TEST(testSettingIpItemDirectly);
    RUN_TEST(testFloatType);
    RUN_TEST(testAuthMenuItem);

    UNITY_END();
}

void loop() {
}