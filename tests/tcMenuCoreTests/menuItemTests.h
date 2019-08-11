#ifndef RUNTIME_MENU_ITEM_TESTS_H
#define RUNTIME_MENU_ITEM_TESTS_H

#include <AUnit.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <RemoteAuthentication.h>
#include <tcm_test/testFixtures.h>
#include <tcUtil.h>

bool renderActivateCalled = false;

// forward reference
void printMenuItem(MenuItem* menuItem);

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

test(testBasicRuntimeMenuItem) {
	RuntimeMenuItem item(MENUTYPE_RUNTIME_VALUE, 22, testBasicRuntimeFn, 222, NULL);

	assertEqual(item.getId(), uint16_t(22));
	assertEqual(item.getEepromPosition(), uint16_t(44));
	char sz[20];
	item.copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("hello", sz);
	item.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("222", sz);
	
	renderActivateCalled = false;
	item.runCallback();
	assertTrue(renderActivateCalled);
}

test(testListRuntimeItem) {
	ListRuntimeMenuItem item(22, 2, testBasicRuntimeFn, NULL);

	// check the name and test on the "top level" or parent item
	char sz[20];

	// ensure there are two parts
	assertEqual(uint8_t(2), item.getNumberOfParts());

	RuntimeMenuItem* child = item.getChildItem(0);
	assertEqual(MENUTYPE_RUNTIME_LIST, child->getMenuType());
	child->copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("name0", sz);
	child->copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0", sz);

	child = item.getChildItem(1);
	assertEqual(MENUTYPE_RUNTIME_LIST, child->getMenuType());
	child->copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("name1", sz);
	child->copyValue(sz, sizeof(sz));
	assertStringCaseEqual("1", sz);

	RuntimeMenuItem* back = item.asBackMenu();
	assertEqual(MENUTYPE_BACK_VALUE, back->getMenuType());

	RuntimeMenuItem* parent = item.asParent();
	assertEqual(MENUTYPE_RUNTIME_LIST, back->getMenuType());
	assertEqual(parent->getId(), uint16_t(22));
	assertEqual(parent->getEepromPosition(), uint16_t(44));
	parent->copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("hello", sz);
	item.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("255", sz);
}

void renderCallback(int id) {
	renderActivateCalled = true;
}

RENDERING_CALLBACK_NAME_INVOKE(textMenuItemTestCb, textItemRenderFn, "HelloWorld", 99, renderCallback)

test(testTextMenuItemFromEmpty) {
	TextMenuItem textItem(textMenuItemTestCb, 33, 10, NULL);
	
	// first simulate eeprom loading back from storage.
	uint8_t* data = (uint8_t*)textItem.getTextValue();
	data[0] = 0;
	data[1] = 'Z';
	data[2] = 'Y';
	data[3] = 'X';
	data[4] = '[';
	data[5] = ']';
	textItem.cleanUpArray();

	// start off with an empty string
	char sz[20];
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("", sz);

	// ensure we can edit an empty string position
	assertEqual(uint8_t(10), textItem.beginMultiEdit());
	assertTrue(textItem.isEditing());
	assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
	assertEqual(0, textItem.getPartValueAsInt());

	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[]", sz);

	// add char to empty string
	textItem.valueChanged(findPositionInEditorSet('N'));
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[N]", sz);

	// add another char to empty string
	assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
	textItem.valueChanged(findPositionInEditorSet('E'));
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("N[E]", sz);

	assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
	textItem.valueChanged(findPositionInEditorSet('T'));
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("NE[T]", sz);

	assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
	textItem.valueChanged(findPositionInEditorSet('_'));
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("NET[_]", sz);

	assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());

	// check that the edit worked ok
	textItem.stopMultiEdit();
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("NET_", sz);

	// now start editing again and clear down the string to zero terminated at position 0
	assertEqual(uint8_t(10), textItem.beginMultiEdit());
	assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
	textItem.valueChanged(0);
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[]", sz);

	// should be empty now
	textItem.stopMultiEdit();
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("", sz);

	// check every byte of buffer is 0.
	for (int i = 0; i < textItem.textLength(); i++) assertEqual(int(data[i]), 0);
}

test(testFindEditorSetFunction) {
	assertEqual(13, findPositionInEditorSet('9'));
	assertEqual(24, findPositionInEditorSet('K'));
	assertEqual(94, findPositionInEditorSet('~'));
	assertEqual(1, findPositionInEditorSet(' '));
	assertEqual(2, findPositionInEditorSet('.'));
	assertEqual(0, findPositionInEditorSet(0));
}

test(testTextRuntimeItem) {
	TextMenuItem textItem(textMenuItemTestCb, 33, 10, NULL);
	textItem.setTextValue("Goodbye");

	assertEqual(textItem.getId(), uint16_t(33));
	assertEqual(textItem.getEepromPosition(), uint16_t(99));

	// check the name and test on the "top level" or parent item
	char sz[20];
	textItem.copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("HelloWorld", sz);
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("Goodbye", sz);

	renderActivateCalled = false;

	assertEqual(uint8_t(10), textItem.beginMultiEdit());
	assertTrue(textItem.isEditing());
	assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
	assertEqual(findPositionInEditorSet('G'), textItem.getPartValueAsInt());

	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[G]oodbye", sz);

	textItem.valueChanged(findPositionInEditorSet('0'));
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[0]oodbye", sz);

	assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
	assertEqual(findPositionInEditorSet('o'), textItem.getPartValueAsInt());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0[o]odbye", sz);

	assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0o[o]dbye", sz);

	textItem.valueChanged(findPositionInEditorSet('1'));
	assertEqual(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0o1[d]bye", sz);

	assertFalse(renderActivateCalled);
	textItem.stopMultiEdit();
	assertTrue(renderActivateCalled);
	assertFalse(textItem.isEditing());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0o1dbye", sz);

}

RENDERING_CALLBACK_NAME_INVOKE(timeMenuItemTestCb, timeItemRenderFn, "Time", 103, NULL)

test(testTimeMenuItem12Hr) {
    TimeFormattedMenuItem timeItem24(timeMenuItemTestCb, 111, EDITMODE_TIME_12H);

	char sz[20];
    timeItem24.setTime(TimeStorage(12, 20, 30));
	timeItem24.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("12:20:30PM", sz);

    timeItem24.setTime(TimeStorage(0, 10, 30));
	timeItem24.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("12:10:30AM", sz);

    timeItem24.setTime(TimeStorage(11, 59, 30));
	timeItem24.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("11:59:30AM", sz);

    timeItem24.setTime(TimeStorage(23, 59, 30));
	timeItem24.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("11:59:30PM", sz);
}


test(testTimeMenuItem24Hr) {
    TimeFormattedMenuItem timeItem24(timeMenuItemTestCb, 111, EDITMODE_TIME_HUNDREDS_24H);

	char sz[20];
    timeItem24.setTime(TimeStorage(20, 39, 30, 93));
	timeItem24.copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("Time", sz);
	timeItem24.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("20:39:30.93", sz);

	assertEqual(uint8_t(4), timeItem24.beginMultiEdit());
	assertEqual(23, timeItem24.nextPart());
	assertEqual(20, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(18);

	assertEqual(59, timeItem24.nextPart());
	assertEqual(39, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(30);

	timeItem24.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("18:[30]:30.93", sz);

	assertEqual(59, timeItem24.nextPart());
	assertEqual(30, timeItem24.getPartValueAsInt());

	assertEqual(99, timeItem24.nextPart());
	assertEqual(93, timeItem24.getPartValueAsInt());
    timeItem24.valueChanged(10);

	timeItem24.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("18:30:30.[10]", sz);
    timeItem24.stopMultiEdit();

    timeItem24.setTimeFromString("23:44:00.33");
	timeItem24.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("23:44:00.33", sz);

    timeItem24.setTimeFromString("8:32");
	timeItem24.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("08:32:00.00", sz);
}

RENDERING_CALLBACK_NAME_INVOKE(ipMenuItemTestCb, ipAddressRenderFn, "HelloWorld", 102, NULL)

test(testIpAddressItem) {
	IpAddressMenuItem ipItem(ipMenuItemTestCb, 2039, NULL);
	ipItem.setIpAddress(192U, 168U, 0U, 96U);

	assertEqual(ipItem.getId(), uint16_t(2039));
	assertEqual(ipItem.getEepromPosition(), uint16_t(102));

	char sz[20];
	ipItem.copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("HelloWorld", sz);
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("192.168.0.96", sz);

	assertEqual(uint8_t(4), ipItem.beginMultiEdit());
	assertEqual(255, ipItem.nextPart());
	assertEqual(192, ipItem.getPartValueAsInt());

	assertEqual(255, ipItem.nextPart());
	assertEqual(168, ipItem.getPartValueAsInt());

	assertEqual(255, ipItem.nextPart());
	assertEqual(0, ipItem.getPartValueAsInt());
	ipItem.valueChanged(2);
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("192.168.[2].96", sz);
	assertEqual(255, ipItem.nextPart());
	ipItem.valueChanged(201);
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("192.168.2.[201]", sz);
	assertTrue(ipItem.isEditing());

	assertEqual(0, ipItem.nextPart());
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("192.168.2.201", sz);
	assertFalse(ipItem.isEditing());

	assertTrue(isMenuRuntime(&ipItem));
	assertFalse(isMenuBasedOnValueItem(&ipItem));
	assertTrue(isMenuRuntimeMultiEdit(&ipItem));
}

test(testSettingIpItemDirectly) {
	IpAddressMenuItem ipItem(ipMenuItemTestCb, 2039, NULL);
	char sz[20];

	ipItem.setIpAddress("192.168.99.22");
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("192.168.99.22", sz);

	ipItem.setIpAddress("255.254.12.");
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("255.254.12.0", sz);

	ipItem.setIpAddress("127.1.2");
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("127.1.2.0", sz);

	ipItem.setIpAddress("badvalue");
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0.0.0.0", sz);

}

test(testCoreAndBooleanMenuItem) {
	// these may seem overkill but the setters are bitwise so quite complex.
	boolItem1.setActive(true);
	assertTrue(boolItem1.isActive());
	boolItem1.setActive(false);
	assertFalse(boolItem1.isActive());
	assertEqual(MENUTYPE_BOOLEAN_VALUE, boolItem1.getMenuType());

	boolItem1.setChanged(true);
	assertTrue(boolItem1.isChanged());
	boolItem1.setChanged(false);
	assertFalse(boolItem1.isChanged());

	boolItem1.setReadOnly(true);
	assertTrue(boolItem1.isReadOnly());
	boolItem1.setReadOnly(false);
	assertFalse(boolItem1.isReadOnly());

	boolItem1.setLocalOnly(true);
	assertTrue(boolItem1.isLocalOnly());
	boolItem1.setLocalOnly(false);
	assertFalse(boolItem1.isLocalOnly());

	boolItem1.setSendRemoteNeededAll();
	assertTrue(boolItem1.isSendRemoteNeeded(0));
	assertTrue(boolItem1.isSendRemoteNeeded(1));
	assertTrue(boolItem1.isSendRemoteNeeded(2));

	boolItem1.setSendRemoteNeeded(1, false);
	assertTrue(boolItem1.isSendRemoteNeeded(0));
	assertFalse(boolItem1.isSendRemoteNeeded(1));
	assertTrue(boolItem1.isSendRemoteNeeded(2));

	assertEqual(4U, boolItem1.getId());
	assertEqual(8U, boolItem1.getEepromPosition());
	assertEqual(1U, boolItem1.getMaximumValue());

	char sz[4];
	boolItem1.copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual(sz, "Boo");

	idOfCallback = 0;
	boolItem1.triggerCallback();
	assertEqual(4, idOfCallback);

	boolItem1.setBoolean(false);
	assertFalse(boolItem1.getBoolean());
	boolItem1.setBoolean(true);
	assertTrue(boolItem1.getBoolean());
}

bool checkWholeFraction(AnalogMenuItem* item, int16_t whole, int16_t fract) {
	WholeAndFraction wf = item->getWholeAndFraction();
	if (wf.fraction != fract || wf.whole != whole) {
		printMenuItem(item);
		serdebugF3("Mismatch in whole fraction expected: ", whole, fract);
		serdebugF3("Actual values: ", wf.whole, wf.fraction);
		return false;
	}
	return true;
}

test(testAnalogEnumMenuItem) {
	assertEqual(MENUTYPE_ENUM_VALUE, menuEnum1.getMenuType());
	assertEqual(MENUTYPE_INT_VALUE, menuAnalog.getMenuType());

	char sz[10];
	// try getting all the strings.
	menuEnum1.copyEnumStrToBuffer(sz, sizeof(sz), 0);
	assertStringCaseEqual(sz, "ITEM1");
	menuEnum1.copyEnumStrToBuffer(sz, sizeof(sz), 1);
	assertStringCaseEqual(sz, "ITEM2");
	menuEnum1.copyEnumStrToBuffer(sz, sizeof(sz), 2);
	assertStringCaseEqual(sz, "ITEM3");

	// try with limited string buffer and ensure properly terminated
	menuEnum1.copyEnumStrToBuffer(sz, 4, 2);
	assertStringCaseEqual(sz, "ITE");

	// verify the others.
	assertEqual(5, menuEnum1.getLengthOfEnumStr(0));
	assertEqual(5, menuEnum1.getLengthOfEnumStr(1));
	assertEqual(2U, menuEnum1.getMaximumValue());

	assertEqual(255U, menuAnalog.getMaximumValue());
	assertEqual(0, menuAnalog.getOffset());
	assertEqual(1U, menuAnalog.getDivisor());
	assertEqual(2, menuAnalog.unitNameLength());
	menuAnalog.copyUnitToBuffer(sz);
	assertStringCaseEqual("AB", sz);

	assertEqual(uint8_t(0), menuAnalog.getDecimalPlacesForDivisor());

	menuAnalog.setCurrentValue(192);
	assertEqual(192U, menuAnalog.getCurrentValue());
	assertTrue(checkWholeFraction(&menuAnalog, 192, 0));
	assertEqual(192U, menuAnalog.getCurrentValue());
	assertNear(float(192.0), menuAnalog.getAsFloatingPointValue(), float(0.0001));
	menuAnalog.setCurrentValue(0);
	assertTrue(checkWholeFraction(&menuAnalog, 0, 0));
	assertNear(float(0.0), menuAnalog.getAsFloatingPointValue(), float(0.0001));
	menuAnalog.setFromFloatingPointValue(21.3);
	assertNear(float(21.0), menuAnalog.getAsFloatingPointValue(), float(0.0001));
	assertTrue(checkWholeFraction(&menuAnalog, 21, 0));
	menuAnalog.copyValue(sz, sizeof sz);
	assertStringCaseEqual("21AB", sz);

}

test(testAnalogValuesWithFractions) {
	char sz[20];
	assertEqual(uint8_t(1), menuHalvesOffs.getDecimalPlacesForDivisor());
	menuHalvesOffs.setCurrentValue(21);
	assertTrue(checkWholeFraction(&menuHalvesOffs, -39, 5));
	assertNear(float(-39.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));
	menuHalvesOffs.copyValue(sz, sizeof sz);
	assertStringCaseEqual("-39.5dB", sz);

	menuHalvesOffs.setCurrentValue(103);
	assertTrue(checkWholeFraction(&menuHalvesOffs, 1, 5));
	assertNear(float(1.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));

	menuHalvesOffs.setFromFloatingPointValue(50.5);
	assertTrue(checkWholeFraction(&menuHalvesOffs, 50, 5));
	assertNear(float(50.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));
	assertEqual(uint16_t(201), menuHalvesOffs.getCurrentValue());
	menuHalvesOffs.copyValue(sz, sizeof sz);
	assertStringCaseEqual("50.5dB", sz);

	menuHalvesOffs.setFromWholeAndFraction(WholeAndFraction(10, 5));
	assertEqual(uint16_t(121), menuHalvesOffs.getCurrentValue());
	assertTrue(checkWholeFraction(&menuHalvesOffs, 10, 5));
	assertNear(float(10.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));
	menuHalvesOffs.copyValue(sz, sizeof sz);
	assertStringCaseEqual("10.5dB", sz);

	assertEqual(uint8_t(2), menuNumTwoDp.getDecimalPlacesForDivisor());
	menuNumTwoDp.setFromFloatingPointValue(98.234);
	assertNear(float(98.23), menuNumTwoDp.getAsFloatingPointValue(), float(0.0001));
	assertEqual(uint16_t(9823), menuNumTwoDp.getCurrentValue());
	assertTrue(checkWholeFraction(&menuNumTwoDp, 98, 23));

	menuNumTwoDp.copyValue(sz, sizeof sz);
	assertStringCaseEqual("98.23", sz);

	menuNumTwoDp.setFromWholeAndFraction(WholeAndFraction(22, 99));
	assertNear(float(22.99), menuNumTwoDp.getAsFloatingPointValue(), float(0.0001));
	assertEqual(uint16_t(2299), menuNumTwoDp.getCurrentValue());
	assertTrue(checkWholeFraction(&menuNumTwoDp, 22, 99));

	menuNumTwoDp.copyValue(sz, sizeof sz);
	assertStringCaseEqual("22.99", sz);
}

test(testFloatAndActionTypes) {
	menuFloatItem.clearSendRemoteNeededAll();
	menuFloatItem.setChanged(false);
	menuFloatItem.setFloatValue(1.001);
	assertNear(1.001, menuFloatItem.getFloatValue(), 0.0001);
	assertTrue(menuFloatItem.isChanged());
	assertTrue(menuFloatItem.isSendRemoteNeeded(0));
	assertEqual(4, menuFloatItem.getDecimalPlaces());

	assertFalse(isMenuRuntime(&menuFloatItem));
	assertFalse(isMenuBasedOnValueItem(&menuFloatItem));
	assertFalse(isMenuRuntimeMultiEdit(&menuFloatItem));
}

test(testAuthMenuItem) {
	EepromAuthenticatorManager auth;
	auth.initialise(&eeprom, 20);
	auth.addAdditionalUUIDKey("uuid1", uuid1);
	auth.addAdditionalUUIDKey("uuid2", uuid2);

	EepromAuthenicationInfoMenuItem menuItem(2002, &auth, NULL);
	RuntimeMenuItem *itm = menuItem.asParent();
	char sz[20];
	itm->copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("Authorised Keys", sz);
	itm->copyValue(sz, sizeof(sz));
	assertStringCaseEqual("", sz);

	assertEqual(uint8_t(6), itm->getNumberOfParts());

	itm = menuItem.getChildItem(0);
	itm->copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("uuid1", sz);
	itm->copyValue(sz, sizeof(sz));
	assertStringCaseEqual("Remove", sz);

	itm = menuItem.getChildItem(1);
	itm->copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("uuid2", sz);

	itm = menuItem.getChildItem(2);
	itm->copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("EmptyKey", sz);

	assertTrue(isMenuRuntime(&menuItem));
	assertFalse(isMenuBasedOnValueItem(&menuItem));
	assertFalse(isMenuRuntimeMultiEdit(&menuItem));

}

#endif // RUNTIME_MENU_ITEM_TESTS_H