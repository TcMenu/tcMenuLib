#ifndef RUNTIME_MENU_ITEM_TESTS_H
#define RUNTIME_MENU_ITEM_TESTS_H

#include <AUnit.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <RemoteAuthentication.h>
#include <tcm_test/testFixtures.h>
#include <tcUtil.h>

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

	// start off with an empty string
	char sz[20];
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("", sz);

	// ensure we can edit an empty string position
	assertEqual(uint8_t(10), textItem.beginMultiEdit());
	assertTrue(textItem.isEditing());
	assertEqual(255, textItem.nextPart());
	assertEqual(0, textItem.getPartValueAsInt());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[]", sz);

	// add char to empty string
	textItem.valueChanged(int('A'));
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[A]", sz);

	// add another char to empty string
	assertEqual(255, textItem.nextPart());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("A[]", sz);

	textItem.valueChanged(int('B'));
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("A[B]", sz);

	// add a last char and stop editing.
	assertEqual(255, textItem.nextPart());
	textItem.valueChanged(int('C'));
	textItem.stopMultiEdit();
	
	// check that the edit worked ok
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("ABC", sz);

	// now start editing again and clear down the string to zero terminated at position 0
	assertEqual(uint8_t(10), textItem.beginMultiEdit());
	assertEqual(255, textItem.nextPart());
	textItem.valueChanged(0);
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[]", sz);

	// now put back to a character, the text after it should come back.
	textItem.valueChanged(int('a'));
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[a]BC", sz);

	// now put back to blank.
	textItem.valueChanged(0);
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[]", sz);

	// should be empty now
	textItem.stopMultiEdit();
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("", sz);
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
	assertEqual(255, textItem.nextPart());
	assertEqual(int('G'), textItem.getPartValueAsInt());

	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[G]oodbye", sz);

	textItem.valueChanged(48);
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[0]oodbye", sz);

	assertEqual(255, textItem.nextPart());
	assertEqual(int('o'), textItem.getPartValueAsInt());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0[o]odbye", sz);

	assertEqual(255, textItem.nextPart());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0o[o]dbye", sz);

	textItem.valueChanged(49);
	assertEqual(255, textItem.nextPart());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0o1[d]bye", sz);

	assertFalse(renderActivateCalled);
	textItem.stopMultiEdit();
	assertTrue(renderActivateCalled);
	assertFalse(textItem.isEditing());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0o1dbye", sz);

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

	menuAnalog.setCurrentValue(192);
	assertEqual(192U, menuAnalog.getCurrentValue());
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
	assertStringCaseEqual("->", sz);

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
	assertStringCaseEqual("", sz);

	assertTrue(isMenuRuntime(&menuItem));
	assertFalse(isMenuBasedOnValueItem(&menuItem));
	assertFalse(isMenuRuntimeMultiEdit(&menuItem));

}

#endif // RUNTIME_MENU_ITEM_TESTS_H