#include <unity.h>
#include <tcMenu.h>
#include "../tutils/fixtures_extern.h"
#include "../tutils/tcMenuFixturesExtra.h"
#include <tcm_test/testFixtures.h>

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
void testTakeOverDisplay();
void testListRendering();

void setup() {
    menuMgr.initWithoutInput(&noRenderer, &menuVolume);
    Serial.begin(115200);

    UNITY_BEGIN();

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
    RUN_TEST(testTakeOverDisplay);
    RUN_TEST(testListRendering);

    UNITY_END();
}

void loop() {
}

bool renderActivateCalled = false;

int testBasicRuntimeFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    switch (mode) {
        case RENDERFN_NAME: {
            if (row < 10) {
                strcpy(buffer, "name");
                fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
            }
            else {
                strcpy(buffer, "hello");
            }
            break;
        }
        case RENDERFN_VALUE:
            ltoaClrBuff(buffer, row, row, NOT_PADDED, bufferSize);
            break;
        case RENDERFN_EEPROM_POS:
            return 44;
        case RENDERFN_INVOKE:
            renderActivateCalled = true;
            break;
        default: break;
    }
    return true;
}

NoRenderer noRenderer;