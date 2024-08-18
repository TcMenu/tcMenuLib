#include <unity.h>
#include <tcMenu.h>
#include "../tutils/fixtures_extern.h"
#include "../tutils/tcMenuFixtures.h"

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

void setup() {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);


    /* base dialog */
    RUN_TEST(testBaseDialogInfo);
    RUN_TEST(testBaseDialogQuestion);

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