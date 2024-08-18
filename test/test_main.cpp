#include <unity.h>
#include <tcMenu.h>
#include "fixtures_extern.h"

// core tests
void testTcUtilGetParentAndVisit();
void testIteratorGetSubMenu();
void testGetItemById();
void testIterationWithPredicate();
void testIteratorTypePredicateLocalOnly();
void testIteratorNothingMatchesPredicate();
void testIterationOverAllMenuItems();
void testIterationOnSimpleMenu();

// core menu item tests
void testIpAddressItem();
void testSettingIpItemDirectly();
void testFloatType();
void testAuthMenuItem();

// authentication tests
void authenticationTest();
void testNoAuthenicatorMode();
void testProgmemAuthenicatorMode();

// dialog tests
void testBaseDialogInfo();
void testBaseDialogQuestion();

// core renderer tests
void testEmptyItemPropertiesFactory();
void testDefaultItemPropertiesFactory();
void testSubAndItemSelectionPropertiesFactory();
void testIconStorageAndRetrival();
void testGridPositionStorageAndRetrival();
void testWidgetFunctionality();
void testBaseRendererWithDefaults();
void testScrollingWithMoreThanOneItemOnRow();
void testTakeOverDisplay();
void testListRendering();

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

// menu manager
void testSaveAndLoadFromMenuSized();
void testAddingItemsAndMenuCallbacks();
void testCreatingAndInitialisation();
void testNavigationPushAndPop();
void testRebuildingNavigation();

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

    /* core */
    RUN_TEST(testTcUtilGetParentAndVisit);
    RUN_TEST(testIteratorGetSubMenu);
    RUN_TEST(testGetItemById);
    RUN_TEST(testIterationWithPredicate);
    RUN_TEST(testIteratorTypePredicateLocalOnly);
    RUN_TEST(testIteratorNothingMatchesPredicate);
    RUN_TEST(testIterationOverAllMenuItems);
    RUN_TEST(testIterationOnSimpleMenu);

    /* base dialog */
    RUN_TEST(testBaseDialogInfo);
    RUN_TEST(testBaseDialogQuestion);

    /* Authentication tests file */
    RUN_TEST(authenticationTest);
    RUN_TEST(testNoAuthenicatorMode);
    RUN_TEST(testProgmemAuthenicatorMode);

    /* menu mgr and navigator */
    RUN_TEST(testSaveAndLoadFromMenuSized);
    RUN_TEST(testAddingItemsAndMenuCallbacks);
    RUN_TEST(testCreatingAndInitialisation);
    RUN_TEST(testNavigationPushAndPop);
    RUN_TEST(testRebuildingNavigation);

    /* core item tests - keep last */
    RUN_TEST(testIpAddressItem);
    RUN_TEST(testSettingIpItemDirectly);
    RUN_TEST(testFloatType);
    RUN_TEST(testAuthMenuItem);


    /* core renderer - keep last */
    RUN_TEST(testEmptyItemPropertiesFactory);
    RUN_TEST(testDefaultItemPropertiesFactory);
    RUN_TEST(testSubAndItemSelectionPropertiesFactory);
    RUN_TEST(testIconStorageAndRetrival);
    RUN_TEST(testGridPositionStorageAndRetrival);
    RUN_TEST(testWidgetFunctionality);
    RUN_TEST(testBaseRendererWithDefaults);
    RUN_TEST(testScrollingWithMoreThanOneItemOnRow);
    RUN_TEST(testTakeOverDisplay);
    RUN_TEST(testListRendering);

    UNITY_END();
}

void loop() {
}