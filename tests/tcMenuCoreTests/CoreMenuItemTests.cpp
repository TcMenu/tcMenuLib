#ifndef RUNTIME_MENU_ITEM_TESTS_H
#define RUNTIME_MENU_ITEM_TESTS_H

#include <AUnit.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <RemoteAuthentication.h>
#include "fixtures_extern.h"
#include <tcUtil.h>

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