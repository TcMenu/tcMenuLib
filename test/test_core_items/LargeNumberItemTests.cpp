#ifndef LARGE_NUMBER_ITEM_TESTS_H
#define LARGE_NUMBER_ITEM_TESTS_H

#include <unity.h>
#include <RuntimeMenuItem.h>
#include <EditableLargeNumberMenuItem.h>
#include "../tutils/fixtures_extern.h"
#include <tcMenu.h>

void dumpBuffer(LargeFixedNumber* buffer) {
	printf("Largenumber buffer: \n");
	const uint8_t* underlyingBuffer = buffer->getNumberBuffer();
	for (int i = 0; i < 6; i++) {
		printf("%d, %d, %d", i, (unsigned int)(underlyingBuffer[i] & 0x0f), underlyingBuffer[i] >>4);
	}
}

void testGettingAndSettingDigits() {
	LargeFixedNumber largeNumber(12, 4, 0, 0, false);

	// then we manually set a few digits and read them back, any unset should be 0.
	largeNumber.setDigit(0, 2);
	largeNumber.setDigit(1, 5);
	largeNumber.setDigit(3, 9);
	largeNumber.setDigit(5, 0);
	largeNumber.setDigit(6, 1);
	largeNumber.setDigit(7, 7);
	largeNumber.setDigit(8, 9);

	dumpBuffer(&largeNumber);

	TEST_ASSERT_EQUAL(largeNumber.getDigit(0), (int)2);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(1), (int)5);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(2), (int)0);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(3), (int)9);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(4), (int)0);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(5), (int)0);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(6), (int)1);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(7), (int)7);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(8), (int)9);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(9), (int)0);

	largeNumber.setValue(9239UL, 5678UL, false);

	dumpBuffer(&largeNumber);

	TEST_ASSERT_EQUAL(largeNumber.getDigit(0), (int)5);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(1), (int)6);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(2), (int)7);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(3), (int)8);

	TEST_ASSERT_EQUAL(largeNumber.getDigit(8), (int)9);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(9), (int)2);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(10), (int)3);
	TEST_ASSERT_EQUAL(largeNumber.getDigit(11), (int)9);
}

void testLargeNumberGetAndSet() {
    LargeFixedNumber largeNumber;

    // we make sure it's starts out empty to 4dp.
    largeNumber.setPrecision(4);
    TEST_ASSERT_EQUAL(largeNumber.getWhole(), (uint32_t)0);
    TEST_ASSERT_EQUAL(largeNumber.getFraction(), (uint32_t)0);
    TEST_ASSERT_FALSE(largeNumber.isNegative());

    // now we set a positive value and try to read it back.
    largeNumber.setValue(1234UL, 5678UL, false);
    TEST_ASSERT_EQUAL(largeNumber.getWhole(), (uint32_t)1234);
    TEST_ASSERT_EQUAL(largeNumber.getFraction(), (uint32_t)5678);
    TEST_ASSERT_FALSE(largeNumber.isNegative());
    TEST_ASSERT_FLOAT_WITHIN(largeNumber.getAsFloat(), 1234.5678f, 0.0001f);

    // now set a negative value and read it back
    largeNumber.setValue(123UL, 56UL, true);
	dumpBuffer(&largeNumber);

	TEST_ASSERT_EQUAL(largeNumber.getWhole(), (uint32_t)123);
    TEST_ASSERT_EQUAL(largeNumber.getFraction(), (uint32_t)56);
    TEST_ASSERT_TRUE(largeNumber.isNegative());
    TEST_ASSERT_FLOAT_WITHIN(largeNumber.getAsFloat(), -123.0056f, 0.0001f);

    // now set a really large value and try to read back
    largeNumber.setPrecision(5);
    largeNumber.setValue(9999999L, 56734L, true);
	dumpBuffer(&largeNumber);

    TEST_ASSERT_EQUAL(largeNumber.getWhole(), (uint32_t)9999999L);
    TEST_ASSERT_EQUAL(largeNumber.getFraction(), (uint32_t)56734L);
    TEST_ASSERT_TRUE(largeNumber.isNegative());

    largeNumber.setPrecision(4);
    largeNumber.setFromFloat(-0.01234F);
    dumpBuffer(&largeNumber);

    TEST_ASSERT_EQUAL(largeNumber.getWhole(), (uint32_t)0);
    TEST_ASSERT_EQUAL(largeNumber.getFraction(), (uint32_t)123);
    TEST_ASSERT_TRUE(largeNumber.isNegative());
}

bool callbackMade = false;

void largeNumCallback(int id) {
	callbackMade = true;
}

RENDERING_CALLBACK_NAME_INVOKE(largeNumTestCb, largeNumItemRenderFn, "LargeNum", 4, largeNumCallback)

void testLargeNumberEditableMenuItem() {
	EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 12, 5);
	editable.getLargeNumber()->setValue(9359946, 20395, false);

	dumpBuffer(editable.getLargeNumber());

	char sz[20];
	editable.copyNameToBuffer(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("LargeNum", sz);

	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("9359946.20395", sz);

	editable.getLargeNumber()->setValue(0, 12, false);
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("0.00012", sz);

	editable.getLargeNumber()->setValue(23452, 12343, false);
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("23452.12343", sz);

	TEST_ASSERT_EQUAL((uint8_t)13, editable.beginMultiEdit());
	TEST_ASSERT_EQUAL(1, editable.nextPart());
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("+0023452.12343", sz);
    TEST_ASSERT_TRUE(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	editable.valueChanged(1);
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("-0023452.12343", sz);
    TEST_ASSERT_TRUE(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    TEST_ASSERT_TRUE(editable.getLargeNumber()->isNegative());
	editable.valueChanged(0);
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("+0023452.12343", sz);
    TEST_ASSERT_TRUE(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    TEST_ASSERT_FALSE(editable.getLargeNumber()->isNegative());
	editable.copyValue(sz, sizeof(sz));

	TEST_ASSERT_EQUAL(9, editable.nextPart());
	editable.valueChanged(8);
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("+8023452.12343", sz);
    TEST_ASSERT_TRUE(checkEditorHints(1, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	TEST_ASSERT_EQUAL(9, editable.nextPart());
	editable.valueChanged(2);
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("+8223452.12343", sz);
    TEST_ASSERT_TRUE(checkEditorHints(2, 3, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	TEST_ASSERT_EQUAL(9, editable.previousPart());
	editable.valueChanged(3);
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("+3223452.12343", sz);
    TEST_ASSERT_TRUE(checkEditorHints(1, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	for(int i = 0;i < 8;i++) TEST_ASSERT_EQUAL(9, editable.nextPart());
	TEST_ASSERT_EQUAL(2, editable.getPartValueAsInt());

	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("+3223452.12343", sz);
    TEST_ASSERT_TRUE(checkEditorHints(10, 11, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	TEST_ASSERT_EQUAL(9, editable.nextPart());
	TEST_ASSERT_EQUAL(3, editable.getPartValueAsInt());
	editable.valueChanged(7);
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("+3223452.12743", sz);
    TEST_ASSERT_TRUE(checkEditorHints(11, 12, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	TEST_ASSERT_EQUAL(9, editable.nextPart());
	TEST_ASSERT_EQUAL(4, editable.getPartValueAsInt());
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("+3223452.12743", sz);
    TEST_ASSERT_TRUE(checkEditorHints(12, 13, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	TEST_ASSERT_EQUAL(9, editable.nextPart());
	TEST_ASSERT_EQUAL(3, editable.getPartValueAsInt());
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("+3223452.12743", sz);
    TEST_ASSERT_TRUE(checkEditorHints(13, 14, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	TEST_ASSERT_EQUAL(0, editable.nextPart());
	editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("3223452.12743", sz);
}

void testPersistLargeInteger() {
	EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 12, 5);
	editable.getLargeNumber()->setValue(10029, 20349, false);
	MockEepromAbstraction mockRom;
    menuMgr.getNavigationStore().clearNavigationListeners();
	menuMgr.setRootMenu(&editable);
	menuMgr.save(mockRom);
	dumpBuffer(editable.getLargeNumber());
	TEST_ASSERT_EQUAL(mockRom.read8(4), (uint8_t)0);
	TEST_ASSERT_EQUAL(mockRom.read8(5), (uint8_t)0x02);
	TEST_ASSERT_EQUAL(mockRom.read8(6), (uint8_t)0x43);
	TEST_ASSERT_EQUAL(mockRom.read8(7), (uint8_t)0x09);
	TEST_ASSERT_EQUAL(mockRom.read8(8), (uint8_t)0x10);
	TEST_ASSERT_EQUAL(mockRom.read8(9), (uint8_t)0x00);
	TEST_ASSERT_EQUAL(mockRom.read8(10), (uint8_t)0x92);

	menuMgr.load(mockRom);
	TEST_ASSERT_EQUAL(editable.getLargeNumber()->getWhole(), (uint32_t)10029);
	TEST_ASSERT_EQUAL(editable.getLargeNumber()->getFraction(), (uint32_t)20349);
	TEST_ASSERT_FALSE(editable.getLargeNumber()->isNegative());

	menuMgr.setRootMenu(NULL);
    menuMgr.getNavigationStore().clearNavigationListeners();
}

void testSetLargeIntFromString() {
	EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 12, 5);
	editable.setLargeNumberFromString("-10293.39482");
	TEST_ASSERT_EQUAL(editable.getLargeNumber()->getWhole(), (uint32_t)10293);
	TEST_ASSERT_EQUAL(editable.getLargeNumber()->getFraction(), (uint32_t)39482);
	TEST_ASSERT_TRUE(editable.getLargeNumber()->isNegative());

	editable.setLargeNumberFromString("0.99900");
	TEST_ASSERT_EQUAL(editable.getLargeNumber()->getWhole(), (uint32_t)0);
	TEST_ASSERT_EQUAL(editable.getLargeNumber()->getFraction(), (uint32_t)99900);
	TEST_ASSERT_FALSE(editable.getLargeNumber()->isNegative());

	editable.setLargeNumberFromString("-0.00010");
	TEST_ASSERT_EQUAL(editable.getLargeNumber()->getWhole(), (uint32_t)0);
	TEST_ASSERT_EQUAL(editable.getLargeNumber()->getFraction(), (uint32_t)10);
	TEST_ASSERT_TRUE(editable.getLargeNumber()->isNegative());

	editable.setLargeNumberFromString("9999999.99999");
	TEST_ASSERT_EQUAL(editable.getLargeNumber()->getWhole(), (uint32_t)9999999L);
	TEST_ASSERT_EQUAL(editable.getLargeNumber()->getFraction(), (uint32_t)99999L);
	TEST_ASSERT_FALSE(editable.getLargeNumber()->isNegative());
}

void testLargeNumWithNegativeNotAllowed() {
    EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 6, 0, false);

    editable.setLargeNumberFromString("15234");
    TEST_ASSERT_EQUAL(editable.getLargeNumber()->getWhole(), (uint32_t)15234);
    TEST_ASSERT_EQUAL(editable.getLargeNumber()->getFraction(), (uint32_t)0);
    TEST_ASSERT_FLOAT_WITHIN(editable.getLargeNumber()->getAsFloat(), 15234.0F, 0.00001);
    TEST_ASSERT_FALSE(editable.getLargeNumber()->isNegative());

    char sz[32];
    editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("15234", sz);

    editable.beginMultiEdit();
    TEST_ASSERT_EQUAL(9, editable.nextPart());
    TEST_ASSERT_EQUAL(0, editable.getPartValueAsInt());
    editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("015234", sz);
    TEST_ASSERT_TRUE(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    TEST_ASSERT_EQUAL(9, editable.nextPart());
    TEST_ASSERT_EQUAL(1, editable.getPartValueAsInt());
    editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("015234", sz);
    TEST_ASSERT_TRUE(checkEditorHints(1, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    TEST_ASSERT_EQUAL(9, editable.nextPart());
    TEST_ASSERT_EQUAL(5, editable.getPartValueAsInt());
    editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("015234", sz);
    TEST_ASSERT_TRUE(checkEditorHints(2, 3, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    TEST_ASSERT_EQUAL(9, editable.nextPart());
    TEST_ASSERT_EQUAL(2, editable.getPartValueAsInt());
    editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("015234", sz);
    TEST_ASSERT_TRUE(checkEditorHints(3, 4, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    TEST_ASSERT_EQUAL(9, editable.nextPart());
    TEST_ASSERT_EQUAL(3, editable.getPartValueAsInt());
    editable.valueChanged(6);
    editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("015264", sz);
    TEST_ASSERT_TRUE(checkEditorHints(4, 5, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    TEST_ASSERT_EQUAL(9, editable.nextPart());
    TEST_ASSERT_EQUAL(4, editable.getPartValueAsInt());
    editable.valueChanged(5);
    editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("015265", sz);
    TEST_ASSERT_TRUE(checkEditorHints(5, 6, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    TEST_ASSERT_EQUAL(0, editable.nextPart());

    editable.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("15265", sz);
}

bool checkMatches(LargeFixedNumber* num, uint32_t whole, uint32_t fract, float nearValue, bool isNeg) {
    if(num->getWhole() != whole || num->getFraction() != fract || num->isNegative() != isNeg) {
        serdebugF4("Mismatched number - expected ", whole, fract, isNeg);
        serdebugF4("Mismatched number - actual ", num->getWhole(), num->getFraction(), num->isNegative());
        return false;
    }
    auto diff = num->getAsFloat() - nearValue;
    return (diff > -0.00001 && diff < 0.00001);
}

void testLargeNumberWithOneDecimalPlace() {
    EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 4, 1, false);

    // conversion from String to float
    editable.setLargeNumberFromString("12.1");
    TEST_ASSERT_TRUE(checkMatches(editable.getLargeNumber(), 12, 1, 12.1F, false));

    // conversion using setFromFloat
    editable.getLargeNumber()->setFromFloat(14.1F);
    TEST_ASSERT_TRUE(checkMatches(editable.getLargeNumber(), 14, 1, 14.1F, false));
    editable.getLargeNumber()->setFromFloat(3.1F);
    TEST_ASSERT_TRUE(checkMatches(editable.getLargeNumber(), 3, 1, 3.1F, false));

    // conversion directly to fields
    editable.getLargeNumber()->setValue(13, 5, false);
    TEST_ASSERT_TRUE(checkMatches(editable.getLargeNumber(), 13, 5, 13.5F, false));

#ifdef IOA_USE_ARDUINO
    // conversion from a string object with too many decimal places -eg 3.100
    auto str = String(3.1F, 3);
    editable.setLargeNumberFromString(str.c_str());
    TEST_ASSERT_TRUE(checkMatches(editable.getLargeNumber(), 3, 1, 3.1F, false));

    // conversion right on the edge of the next value
    str = String(3.999F, 3);
    editable.setLargeNumberFromString(str.c_str());
    TEST_ASSERT_TRUE(checkMatches(editable.getLargeNumber(), 3, 9, 3.9F, false));
#endif // Arduino only test

    editable.getLargeNumber()->setFromFloat(14.9999F);
    TEST_ASSERT_TRUE(checkMatches(editable.getLargeNumber(), 14, 9, 14.9F, false));
}

#endif // LARGE_NUMBER_ITEM_TESTS_H
