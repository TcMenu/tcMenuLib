#include <tcMenu.h>
#include "../tutils/tcMenuFixturesExtra.h"
#include <tcm_test/testFixtures.h>
#include "../tutils/fixtures_extern.h"
#include <unity.h>

void setUp() {}
void tearDown() {}

// core tests
void testTcUtilGetParentAndVisit();
void testIteratorGetSubMenu();
void testGetItemById();
void testIterationWithPredicate();
void testIteratorTypePredicateLocalOnly();
void testIteratorNothingMatchesPredicate();
void testIterationOverAllMenuItems();
void testIterationOnSimpleMenu();

// authentication tests
void authenticationTest();
void testNoAuthenicatorMode();
void testProgmemAuthenicatorMode();

// menu manager
void testSaveAndLoadFromMenuSized();
void testAddingItemsAndMenuCallbacks();
void testCreatingAndInitialisation();
void testNavigationPushAndPop();
void testRebuildingNavigation();

int main(int argc, char** argv) {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);

    UNITY_BEGIN();

    /* core */
    RUN_TEST(testTcUtilGetParentAndVisit);
    RUN_TEST(testIteratorGetSubMenu);
    RUN_TEST(testGetItemById);
    RUN_TEST(testIterationWithPredicate);
    RUN_TEST(testIteratorTypePredicateLocalOnly);
    RUN_TEST(testIteratorNothingMatchesPredicate);
    RUN_TEST(testIterationOverAllMenuItems);
    RUN_TEST(testIterationOnSimpleMenu);

    /* Authentication tests file */
    RUN_TEST(authenticationTest);
    RUN_TEST(testNoAuthenicatorMode);
    RUN_TEST(testProgmemAuthenicatorMode);

    /* menu mgr and navigator */
    RUN_TEST(testSaveAndLoadFromMenuSized);
    RUN_TEST(testAddingItemsAndMenuCallbacks);
    RUN_TEST(testCreatingAndInitialisation);
    //IGNORED RUN_TEST(testNavigationPushAndPop);
    //IGNORED RUN_TEST(testRebuildingNavigation);

    return UNITY_END();
}

void loop() {
}