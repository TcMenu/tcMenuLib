#ifndef RUNTIME_MENU_ITEM_TESTS_H
#define RUNTIME_MENU_ITEM_TESTS_H

#include <AUnit.h>
#include <RuntimeMenuItem.h>
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
	case RENDERFN_INVOKE:
		renderActivateCalled = true;
		break;
	}
	return true;
}

test(testBasicRuntimeMenuItem) {
	RuntimeMenuItem item(MENUTYPE_RUNTIME_VALUE, 22, 44, testBasicRuntimeFn, 222, NULL);

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
	RuntimeMenuItem* parent = item.asParent();
	assertEqual(parent->getId(), uint16_t(22));
	assertEqual(parent->getEepromPosition(), uint16_t(0xffff));

	// check the name and test on the "top level" or parent item
	char sz[20];
	parent->copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("hello", sz);
	parent->copyValue(sz, sizeof(sz));
	assertStringCaseEqual("255", sz);

	// ensure there are two parts
	assertEqual(uint8_t(2), item.getNumberOfParts());

	RuntimeMenuItem* child = item.getChildItem(0);
	child->copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("name0", sz);
	child->copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0", sz);

	child = item.getChildItem(1);
	child->copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("name1", sz);
	child->copyValue(sz, sizeof(sz));
	assertStringCaseEqual("1", sz);
}

void renderCallback(int id) {
	renderActivateCalled = true;
}

const char helloWorldPgm[] PROGMEM = "HelloWorld";

RENDERING_CALLBACK_NAME_INVOKE(textMenuItemTestCb, textItemRenderFn, helloWorldPgm, renderCallback)

test(testTextRuntimeItem) {
	TextMenuItem textItem(textMenuItemTestCb, 33, 99, 10, NULL);
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

RENDERING_CALLBACK_NAME_INVOKE(ipMenuItemTestCb, ipAddressRenderFn, helloWorldPgm, NULL)

test(testIpAddressItem) {
	IpAddressMenuItem ipItem(ipMenuItemTestCb, 2039, 102, NULL);
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
	assertStringCaseEqual("192.168=2.96", sz);
	assertEqual(255, ipItem.nextPart());
	ipItem.valueChanged(201);
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("192.168.2=201", sz);
	assertTrue(ipItem.isEditing());

	assertEqual(0, ipItem.nextPart());
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("192.168.2.201", sz);
	assertFalse(ipItem.isEditing());
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

#endif // RUNTIME_MENU_ITEM_TESTS_H