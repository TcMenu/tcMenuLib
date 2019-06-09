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

const char helloWorldPgm[] PROGMEM = "HelloWorld";

test(testTextRuntimeItem) {
	TextMenuItem textItem(helloWorldPgm, 33, 99, 10, NULL);
	textItem.setTextValue("Goodbye");

	assertEqual(textItem.getId(), uint16_t(33));
	assertEqual(textItem.getEepromPosition(), uint16_t(99));

	// check the name and test on the "top level" or parent item
	char sz[20];
	textItem.copyNameToBuffer(sz, sizeof(sz));
	assertStringCaseEqual("HelloWorld", sz);
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("Goodbye", sz);

	assertEqual(uint8_t(10), textItem.beginMultiEdit());
	assertEqual(255, textItem.nextPart());
	
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[G]oodbye", sz);

	textItem.valueChanged(48);
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("[0]oodbye", sz);

	assertEqual(255, textItem.nextPart());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0[o]odbye", sz);

	assertEqual(255, textItem.nextPart());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0o[o]dbye", sz);

	textItem.valueChanged(49);
	assertEqual(255, textItem.nextPart());
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0o1[d]bye", sz);
	textItem.stopMultiEdit();
	textItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("0o1dbye", sz);
}

test(testIpAddressItem) {
	IpAddressMenuItem ipItem(helloWorldPgm, 2039, 102, NULL);
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
	assertEqual(255, ipItem.nextPart());
	assertEqual(255, ipItem.nextPart());
	ipItem.valueChanged(2);
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("192.168=2.96", sz);
	assertEqual(255, ipItem.nextPart());
	ipItem.valueChanged(201);
	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("192.168.2=201", sz);
	assertEqual(0, ipItem.nextPart());

	ipItem.copyValue(sz, sizeof(sz));
	assertStringCaseEqual("192.168.2.201", sz);
}


#endif // RUNTIME_MENU_ITEM_TESTS_H