#ifndef LARGE_NUMBER_ITEM_TESTS_H
#define LARGE_NUMBER_ITEM_TESTS_H

#include <AUnit.h>
#include <RuntimeMenuItem.h>
#include <EditableLargeNumberMenuItem.h>

void dumpBuffer(LargeFixedNumber* buffer) {
	Serial.print("Largenumber buffer: ");
	const uint8_t* underlyingBuffer = buffer->getNumberBuffer();
	for (int i = 0; i < 6; i++) {
		Serial.print((unsigned int)underlyingBuffer[i], HEX);
		Serial.print(", ");
	}
	Serial.println();
}

test(testGettingAndSettingDigits) {
	LargeFixedNumber largeNumber;

	// then we manually set a few digits and read them back, any unset should be 0.
	largeNumber.setDigit(0, 2);
	largeNumber.setDigit(1, 5);
	largeNumber.setDigit(3, 9);
	largeNumber.setDigit(5, 0);
	largeNumber.setDigit(6, 1);
	largeNumber.setDigit(7, 7);
	largeNumber.setDigit(8, 9);

	dumpBuffer(&largeNumber);

	assertEqual(largeNumber.getDigit(0), (int)2);
	assertEqual(largeNumber.getDigit(1), (int)5);
	assertEqual(largeNumber.getDigit(2), (int)0);
	assertEqual(largeNumber.getDigit(3), (int)9);
	assertEqual(largeNumber.getDigit(4), (int)0);
	assertEqual(largeNumber.getDigit(5), (int)0);
	assertEqual(largeNumber.getDigit(6), (int)1);
	assertEqual(largeNumber.getDigit(7), (int)7);
	assertEqual(largeNumber.getDigit(8), (int)9);
	assertEqual(largeNumber.getDigit(9), (int)0);

	largeNumber.setValue(9239UL, 5678UL, false);

	dumpBuffer(&largeNumber);

	assertEqual(largeNumber.getDigit(0), (int)5);
	assertEqual(largeNumber.getDigit(1), (int)6);
	assertEqual(largeNumber.getDigit(2), (int)7);
	assertEqual(largeNumber.getDigit(3), (int)8);

	assertEqual(largeNumber.getDigit(8), (int)9);
	assertEqual(largeNumber.getDigit(9), (int)2);
	assertEqual(largeNumber.getDigit(10), (int)3);
	assertEqual(largeNumber.getDigit(11), (int)9);
}

test(testLargeNumberGetAndSet) {
    LargeFixedNumber largeNumber;

    // we make sure it's starts out empty to 4dp.
    largeNumber.setPrecision(4);
    assertEqual(largeNumber.getWhole(), (uint32_t)0);
    assertEqual(largeNumber.getFraction(), (uint32_t)0);
    assertFalse(largeNumber.isNegative());

    // now we set a positive value and try to read it back.
    largeNumber.setValue(1234UL, 5678UL, false);
    assertEqual(largeNumber.getWhole(), (uint32_t)1234);
    assertEqual(largeNumber.getFraction(), (uint32_t)5678);
    assertFalse(largeNumber.isNegative());
    assertNear(largeNumber.getAsFloat(), 1234.5678f, 0.0001f);

    // now set a negative value and read it back
    largeNumber.setValue(123UL, 56UL, true);
	dumpBuffer(&largeNumber);

	assertEqual(largeNumber.getWhole(), (uint32_t)123);
    assertEqual(largeNumber.getFraction(), (uint32_t)56);
    assertTrue(largeNumber.isNegative());
    assertNear(largeNumber.getAsFloat(), -123.0056f, 0.0001f);

    // now set a really large value and try to read back
    largeNumber.setPrecision(5);
    largeNumber.setValue(9999999L, 56734L, true);
	dumpBuffer(&largeNumber);

    assertEqual(largeNumber.getWhole(), (uint32_t)9999999L);
    assertEqual(largeNumber.getFraction(), (uint32_t)56734L);
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
	assertStringCaseEqual("LargeNum", sz);

	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("9359946.20395", sz);

	editable.getLargeNumber()->setValue(23452, 12343, false);
	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("23452.12343", sz);

	assertEqual((uint8_t)13, editable.beginMultiEdit());
	assertEqual(1, editable.nextPart());
	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[+]0023452.12343", sz);

	editable.valueChanged(1);
	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[-]0023452.12343", sz);
	assertTrue(editable.getLargeNumber()->isNegative());
	editable.valueChanged(0);
	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[+]0023452.12343", sz);
	assertFalse(editable.getLargeNumber()->isNegative());
	editable.copyValue(sz, sizeof(sz));

	assertEqual(9, editable.nextPart());
	editable.valueChanged(8);
	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("+[8]023452.12343", sz);

	assertEqual(9, editable.nextPart());
	editable.valueChanged(2);
	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("+8[2]23452.12343", sz);

	assertEqual(9, editable.previousPart());
	editable.valueChanged(3);
	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("+[3]223452.12343", sz);

	for(int i = 0;i < 8;i++) assertEqual(9, editable.nextPart());
	assertEqual(2, editable.getPartValueAsInt());

	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("+3223452.1[2]343", sz);

	assertEqual(9, editable.nextPart());
	assertEqual(3, editable.getPartValueAsInt());
	editable.valueChanged(7);
	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("+3223452.12[7]43", sz);

	assertEqual(9, editable.nextPart());
	assertEqual(4, editable.getPartValueAsInt());
	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("+3223452.127[4]3", sz);

	assertEqual(9, editable.nextPart());
	assertEqual(3, editable.getPartValueAsInt());
	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("+3223452.1274[3]", sz);

	assertEqual(0, editable.nextPart());
	editable.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("3223452.12743", sz);
}

test(testPersistLargeInteger) {
	EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 12, 5);
	editable.getLargeNumber()->setValue(10029, 20349, false);
	MockEepromAbstraction mockRom;
	menuMgr.setRootMenu(&editable);
	menuMgr.save(mockRom);
	dumpBuffer(editable.getLargeNumber());
	assertEqual(mockRom.read8(4), (uint8_t)0);
	assertEqual(mockRom.read8(5), (uint8_t)0x02);
	assertEqual(mockRom.read8(6), (uint8_t)0x43);
	assertEqual(mockRom.read8(7), (uint8_t)0x09);
	assertEqual(mockRom.read8(8), (uint8_t)0x10);
	assertEqual(mockRom.read8(9), (uint8_t)0x00);
	assertEqual(mockRom.read8(10), (uint8_t)0x92);

	menuMgr.load(mockRom);
	assertEqual(editable.getLargeNumber()->getWhole(), (uint32_t)10029);
	assertEqual(editable.getLargeNumber()->getFraction(), (uint32_t)20349);
	assertFalse(editable.getLargeNumber()->isNegative());

	menuMgr.setRootMenu(NULL);
}

test(testSetLargeIntFromString) {
	EditableLargeNumberMenuItem editable(largeNumTestCb, 101, 12, 5);
	editable.setLargeNumberFromString("-10293.39482");
	assertEqual(editable.getLargeNumber()->getWhole(), (uint32_t)10293);
	assertEqual(editable.getLargeNumber()->getFraction(), (uint32_t)39482);
	assertTrue(editable.getLargeNumber()->isNegative());

	editable.setLargeNumberFromString("0.99900");
	assertEqual(editable.getLargeNumber()->getWhole(), (uint32_t)0);
	assertEqual(editable.getLargeNumber()->getFraction(), (uint32_t)99900);
	assertFalse(editable.getLargeNumber()->isNegative());

	editable.setLargeNumberFromString("-0.00010");
	assertEqual(editable.getLargeNumber()->getWhole(), (uint32_t)0);
	assertEqual(editable.getLargeNumber()->getFraction(), (uint32_t)10);
	assertTrue(editable.getLargeNumber()->isNegative());

	editable.setLargeNumberFromString("9999999.99999");
	assertEqual(editable.getLargeNumber()->getWhole(), (uint32_t)9999999L);
	assertEqual(editable.getLargeNumber()->getFraction(), (uint32_t)99999L);
	assertFalse(editable.getLargeNumber()->isNegative());
}

#endif // LARGE_NUMBER_ITEM_TESTS_H
