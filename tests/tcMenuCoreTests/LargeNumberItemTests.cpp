#ifndef LARGE_NUMBER_ITEM_TESTS_H
#define LARGE_NUMBER_ITEM_TESTS_H

#include <testing/SimpleTest.h>
#include <RuntimeMenuItem.h>
#include <EditableLargeNumberMenuItem.h>
#include "fixtures_extern.h"
#include <tcMenu.h>

void dumpBuffer(LargeFixedNumber* buffer) {
	serdebugF("Largenumber buffer: ");
	const uint8_t* underlyingBuffer = buffer->getNumberBuffer();
	for (int i = 0; i < 6; i++) {
		serdebug3(i, (unsigned int)(underlyingBuffer[i] & 0x0f), underlyingBuffer[i] >>4);
	}
}

test(testGettingAndSettingDigits) {
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

	assertEquals(largeNumber.getDigit(0), (int)2);
	assertEquals(largeNumber.getDigit(1), (int)5);
	assertEquals(largeNumber.getDigit(2), (int)0);
	assertEquals(largeNumber.getDigit(3), (int)9);
	assertEquals(largeNumber.getDigit(4), (int)0);
	assertEquals(largeNumber.getDigit(5), (int)0);
	assertEquals(largeNumber.getDigit(6), (int)1);
	assertEquals(largeNumber.getDigit(7), (int)7);
	assertEquals(largeNumber.getDigit(8), (int)9);
	assertEquals(largeNumber.getDigit(9), (int)0);

	largeNumber.setValue(9239UL, 5678UL, false);

	dumpBuffer(&largeNumber);

	assertEquals(largeNumber.getDigit(0), (int)5);
	assertEquals(largeNumber.getDigit(1), (int)6);
	assertEquals(largeNumber.getDigit(2), (int)7);
	assertEquals(largeNumber.getDigit(3), (int)8);

	assertEquals(largeNumber.getDigit(8), (int)9);
	assertEquals(largeNumber.getDigit(9), (int)2);
	assertEquals(largeNumber.getDigit(10), (int)3);
	assertEquals(largeNumber.getDigit(11), (int)9);
}

test(testLargeNumberGetAndSet) {
    LargeFixedNumber largeNumber;

    // we make sure it's starts out empty to 4dp.
    largeNumber.setPrecision(4);
    assertEquals(largeNumber.getWhole(), (uint32_t)0);
    assertEquals(largeNumber.getFraction(), (uint32_t)0);
    assertFalse(largeNumber.isNegative());

    // now we set a positive value and try to read it back.
    largeNumber.setValue(1234UL, 5678UL, false);
    assertEquals(largeNumber.getWhole(), (uint32_t)1234);
    assertEquals(largeNumber.getFraction(), (uint32_t)5678);
    assertFalse(largeNumber.isNegative());
    assertFloatNear(largeNumber.getAsFloat(), 1234.5678f, 0.0001f);

    // now set a negative value and read it back
    largeNumber.setValue(123UL, 56UL, true);
	dumpBuffer(&largeNumber);

	assertEquals(largeNumber.getWhole(), (uint32_t)123);
    assertEquals(largeNumber.getFraction(), (uint32_t)56);
    assertTrue(largeNumber.isNegative());
    assertFloatNear(largeNumber.getAsFloat(), -123.0056f, 0.0001f);

    // now set a really large value and try to read back
    largeNumber.setPrecision(5);
    largeNumber.setValue(9999999L, 56734L, true);
	dumpBuffer(&largeNumber);

    assertEquals(largeNumber.getWhole(), (uint32_t)9999999L);
    assertEquals(largeNumber.getFraction(), (uint32_t)56734L);
    assertTrue(largeNumber.isNegative());

    largeNumber.setPrecision(4);
    largeNumber.setFromFloat(-0.01234F);
    dumpBuffer(&largeNumber);

    assertEquals(largeNumber.getWhole(), (uint32_t)0);
    assertEquals(largeNumber.getFraction(), (uint32_t)123);
    assertTrue(largeNumber.isNegative());
}

bool callbackMade = false;

void largeNumCallback(int id) {
	callbackMade = true;
}

RENDERING_CALLBACK_NAME_INVOKE(largeNumTestCb, largeNumItemRenderFn, "LargeNum", 4, largeNumCallback)

test(testLargeNumberEditableMenuItem) {
	EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 12, 5);
	editable.getLargeNumber()->setValue(9359946, 20395, false);

	dumpBuffer(editable.getLargeNumber());

	char sz[20];
	editable.copyNameToBuffer(sz, sizeof(sz));
	assertStringEquals("LargeNum", sz);

	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("9359946.20395", sz);

	editable.getLargeNumber()->setValue(0, 12, false);
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("0.00012", sz);

	editable.getLargeNumber()->setValue(23452, 12343, false);
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("23452.12343", sz);

	assertEquals((uint8_t)13, editable.beginMultiEdit());
	assertEquals(1, editable.nextPart());
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("+0023452.12343", sz);
    assertTrue(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	editable.valueChanged(1);
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("-0023452.12343", sz);
    assertTrue(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertTrue(editable.getLargeNumber()->isNegative());
	editable.valueChanged(0);
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("+0023452.12343", sz);
    assertTrue(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    assertFalse(editable.getLargeNumber()->isNegative());
	editable.copyValue(sz, sizeof(sz));

	assertEquals(9, editable.nextPart());
	editable.valueChanged(8);
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("+8023452.12343", sz);
    assertTrue(checkEditorHints(1, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	assertEquals(9, editable.nextPart());
	editable.valueChanged(2);
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("+8223452.12343", sz);
    assertTrue(checkEditorHints(2, 3, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	assertEquals(9, editable.previousPart());
	editable.valueChanged(3);
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("+3223452.12343", sz);
    assertTrue(checkEditorHints(1, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	for(int i = 0;i < 8;i++) assertEquals(9, editable.nextPart());
	assertEquals(2, editable.getPartValueAsInt());

	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("+3223452.12343", sz);
    assertTrue(checkEditorHints(10, 11, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	assertEquals(9, editable.nextPart());
	assertEquals(3, editable.getPartValueAsInt());
	editable.valueChanged(7);
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("+3223452.12743", sz);
    assertTrue(checkEditorHints(11, 12, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	assertEquals(9, editable.nextPart());
	assertEquals(4, editable.getPartValueAsInt());
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("+3223452.12743", sz);
    assertTrue(checkEditorHints(12, 13, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	assertEquals(9, editable.nextPart());
	assertEquals(3, editable.getPartValueAsInt());
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("+3223452.12743", sz);
    assertTrue(checkEditorHints(13, 14, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	assertEquals(0, editable.nextPart());
	editable.copyValue(sz, sizeof(sz));
	assertStringEquals("3223452.12743", sz);
}

test(testPersistLargeInteger) {
	EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 12, 5);
	editable.getLargeNumber()->setValue(10029, 20349, false);
	MockEepromAbstraction mockRom;
    menuMgr.getNavigationStore().clearNavigationListeners();
	menuMgr.setRootMenu(&editable);
	menuMgr.save(mockRom);
	dumpBuffer(editable.getLargeNumber());
	assertEquals(mockRom.read8(4), (uint8_t)0);
	assertEquals(mockRom.read8(5), (uint8_t)0x02);
	assertEquals(mockRom.read8(6), (uint8_t)0x43);
	assertEquals(mockRom.read8(7), (uint8_t)0x09);
	assertEquals(mockRom.read8(8), (uint8_t)0x10);
	assertEquals(mockRom.read8(9), (uint8_t)0x00);
	assertEquals(mockRom.read8(10), (uint8_t)0x92);

	menuMgr.load(mockRom);
	assertEquals(editable.getLargeNumber()->getWhole(), (uint32_t)10029);
	assertEquals(editable.getLargeNumber()->getFraction(), (uint32_t)20349);
	assertFalse(editable.getLargeNumber()->isNegative());

	menuMgr.setRootMenu(NULL);
    menuMgr.getNavigationStore().clearNavigationListeners();
}

test(testSetLargeIntFromString) {
	EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 12, 5);
	editable.setLargeNumberFromString("-10293.39482");
	assertEquals(editable.getLargeNumber()->getWhole(), (uint32_t)10293);
	assertEquals(editable.getLargeNumber()->getFraction(), (uint32_t)39482);
	assertTrue(editable.getLargeNumber()->isNegative());

	editable.setLargeNumberFromString("0.99900");
	assertEquals(editable.getLargeNumber()->getWhole(), (uint32_t)0);
	assertEquals(editable.getLargeNumber()->getFraction(), (uint32_t)99900);
	assertFalse(editable.getLargeNumber()->isNegative());

	editable.setLargeNumberFromString("-0.00010");
	assertEquals(editable.getLargeNumber()->getWhole(), (uint32_t)0);
	assertEquals(editable.getLargeNumber()->getFraction(), (uint32_t)10);
	assertTrue(editable.getLargeNumber()->isNegative());

	editable.setLargeNumberFromString("9999999.99999");
	assertEquals(editable.getLargeNumber()->getWhole(), (uint32_t)9999999L);
	assertEquals(editable.getLargeNumber()->getFraction(), (uint32_t)99999L);
	assertFalse(editable.getLargeNumber()->isNegative());
}

test(LargeNumWithNegativeNotAllowed) {
    EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 6, 0, false);

    editable.setLargeNumberFromString("15234");
    assertEquals(editable.getLargeNumber()->getWhole(), (uint32_t)15234);
    assertEquals(editable.getLargeNumber()->getFraction(), (uint32_t)0);
    assertFloatNear(editable.getLargeNumber()->getAsFloat(), 15234.0F, 0.00001);
    assertFalse(editable.getLargeNumber()->isNegative());

    char sz[32];
    editable.copyValue(sz, sizeof(sz));
    assertStringEquals("15234", sz);

    editable.beginMultiEdit();
    assertEquals(9, editable.nextPart());
    assertEquals(0, editable.getPartValueAsInt());
    editable.copyValue(sz, sizeof(sz));
    assertStringEquals("015234", sz);
    assertTrue(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(9, editable.nextPart());
    assertEquals(1, editable.getPartValueAsInt());
    editable.copyValue(sz, sizeof(sz));
    assertStringEquals("015234", sz);
    assertTrue(checkEditorHints(1, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(9, editable.nextPart());
    assertEquals(5, editable.getPartValueAsInt());
    editable.copyValue(sz, sizeof(sz));
    assertStringEquals("015234", sz);
    assertTrue(checkEditorHints(2, 3, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(9, editable.nextPart());
    assertEquals(2, editable.getPartValueAsInt());
    editable.copyValue(sz, sizeof(sz));
    assertStringEquals("015234", sz);
    assertTrue(checkEditorHints(3, 4, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(9, editable.nextPart());
    assertEquals(3, editable.getPartValueAsInt());
    editable.valueChanged(6);
    editable.copyValue(sz, sizeof(sz));
    assertStringEquals("015264", sz);
    assertTrue(checkEditorHints(4, 5, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(9, editable.nextPart());
    assertEquals(4, editable.getPartValueAsInt());
    editable.valueChanged(5);
    editable.copyValue(sz, sizeof(sz));
    assertStringEquals("015265", sz);
    assertTrue(checkEditorHints(5, 6, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    assertEquals(0, editable.nextPart());

    editable.copyValue(sz, sizeof(sz));
    assertStringEquals("15265", sz);
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

test(LargeNumberWithOneDecimalPlace) {
    EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 4, 1, false);

    // conversion from String to float
    editable.setLargeNumberFromString("12.1");
    assertTrue(checkMatches(editable.getLargeNumber(), 12, 1, 12.1F, false));

    // conversion using setFromFloat
    editable.getLargeNumber()->setFromFloat(14.1F);
    assertTrue(checkMatches(editable.getLargeNumber(), 14, 1, 14.1F, false));
    editable.getLargeNumber()->setFromFloat(3.1F);
    assertTrue(checkMatches(editable.getLargeNumber(), 3, 1, 3.1F, false));

    // conversion directly to fields
    editable.getLargeNumber()->setValue(13, 5, false);
    assertTrue(checkMatches(editable.getLargeNumber(), 13, 5, 13.5F, false));

#ifdef IOA_USE_ARDUINO
    // conversion from a string object with too many decimal places -eg 3.100
    auto str = String(3.1F, 3);
    editable.setLargeNumberFromString(str.c_str());
    assertTrue(checkMatches(editable.getLargeNumber(), 3, 1, 3.1F, false));

    // conversion right on the edge of the next value
    str = String(3.999F, 3);
    editable.setLargeNumberFromString(str.c_str());
    assertTrue(checkMatches(editable.getLargeNumber(), 3, 9, 3.9F, false));
#endif // Arduino only test

    editable.getLargeNumber()->setFromFloat(14.9999F);
    assertTrue(checkMatches(editable.getLargeNumber(), 14, 9, 14.9F, false));
}

#endif // LARGE_NUMBER_ITEM_TESTS_H
