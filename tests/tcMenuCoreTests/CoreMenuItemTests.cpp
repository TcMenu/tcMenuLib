#ifndef RUNTIME_MENU_ITEM_TESTS_H
#define RUNTIME_MENU_ITEM_TESTS_H

#include <testing/SimpleTest.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <RemoteAuthentication.h>
#include "fixtures_extern.h"
#include <tcUtil.h>

RENDERING_CALLBACK_NAME_INVOKE(ipMenuItemTestCb, ipAddressRenderFn, "HelloWorld", 102, NULL)


testF(TaskManagerResettingTest, testIpAddressItem) {
	IpAddressMenuItem ipItem(ipMenuItemTestCb, 2039, NULL);
	ipItem.setIpAddress(192U, 168U, 0U, 96U);

	assertEquals(ipItem.getId(), uint16_t(2039));
	assertEquals(ipItem.getEepromPosition(), uint16_t(102));

	char sz[32];
	copyMenuItemNameAndValue(&ipItem, sz, sizeof(sz), '[');
	assertStringEquals("HelloWorld[ 192.168.0.96", sz);
	copyMenuItemValue(&ipItem, sz, sizeof(sz));
	assertStringEquals("192.168.0.96", sz);

	assertEquals(uint8_t(4), ipItem.beginMultiEdit());
	assertEquals(255, ipItem.nextPart());
	assertEquals(192, ipItem.getPartValueAsInt());

	assertEquals(255, ipItem.nextPart());
	assertEquals(168, ipItem.getPartValueAsInt());

	assertEquals(255, ipItem.nextPart());
	assertEquals(0, ipItem.getPartValueAsInt());
	ipItem.valueChanged(2);
	ipItem.copyValue(sz, sizeof(sz));
	assertStringEquals("192.168.2.96", sz);
    assertTrue(checkEditorHints(8, 9, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    assertEquals(255, ipItem.nextPart());
	ipItem.valueChanged(201);
	ipItem.copyValue(sz, sizeof(sz));
	assertStringEquals("192.168.2.201", sz);
    assertTrue(checkEditorHints(10, 13, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	assertEquals(0, ipItem.nextPart());
	ipItem.copyValue(sz, sizeof(sz));
	assertStringEquals("192.168.2.201", sz);

	assertTrue(isMenuRuntime(&ipItem));
	assertFalse(isMenuBasedOnValueItem(&ipItem));
	assertTrue(isMenuRuntimeMultiEdit(&ipItem));
}

testF(TaskManagerResettingTest, testSettingIpItemDirectly) {
	IpAddressMenuItem ipItem(ipMenuItemTestCb, 2039, NULL);
	char sz[20];

	ipItem.setIpAddress("192.168.99.22");
	ipItem.copyValue(sz, sizeof(sz));
	assertStringEquals("192.168.99.22", sz);

	ipItem.setIpAddress("255.254.12.");
	ipItem.copyValue(sz, sizeof(sz));
	assertStringEquals("255.254.12.0", sz);

	ipItem.setIpAddress("127.1.2");
	ipItem.copyValue(sz, sizeof(sz));
	assertStringEquals("127.1.2.0", sz);

	ipItem.setIpAddress("badvalue");
	ipItem.copyValue(sz, sizeof(sz));
	assertStringEquals("0.0.0.0", sz);

}

testF(TaskManagerResettingTest, testFloatType) {
	menuFloatItem.clearSendRemoteNeededAll();
	menuFloatItem.setChanged(false);
	menuFloatItem.setFloatValue(1.001);
	assertFloatNear(1.001, menuFloatItem.getFloatValue(), 0.0001);
	assertTrue(menuFloatItem.isChanged());
	assertTrue(menuFloatItem.isSendRemoteNeeded(0));
	assertEquals(4, menuFloatItem.getDecimalPlaces());
	char sz[20];

    //
    // Be very careful adding test cases for float here, float is really inaccurate, and the maximum number of digits
    // that can be represented (even relatively accurately - avoiding edge cases) is around 7 digits.
    //

    copyMenuItemNameAndValue(&menuFloatItem, sz, sizeof(sz));
	assertEquals("FloatItem: 1.0010", sz);

	menuFloatItem.setFloatValue(234.456722);
    copyMenuItemValue(&menuFloatItem, sz, sizeof(sz));
	assertEquals("234.4567", sz);

    menuFloatItem.setFloatValue(-938.4567);
    copyMenuItemValue(&menuFloatItem, sz, sizeof(sz));
    assertEquals("-938.4567", sz);

    menuFloatItem.setFloatValue(-0.001);
    copyMenuItemValue(&menuFloatItem, sz, sizeof(sz));
    assertEquals("-0.0010", sz);

    menuFloatItem.setFloatValue(-0.0);
    copyMenuItemValue(&menuFloatItem, sz, sizeof(sz));
    assertEquals("0.0000", sz);

    assertFalse(isMenuRuntime(&menuFloatItem));
	assertFalse(isMenuBasedOnValueItem(&menuFloatItem));
	assertFalse(isMenuRuntimeMultiEdit(&menuFloatItem));
}

testF(TaskManagerResettingTest, testAuthMenuItem) {
	EepromAuthenticatorManager auth;
	auth.initialise(&eeprom, 20);
	auth.addAdditionalUUIDKey("uuid1", uuid1);
	auth.addAdditionalUUIDKey("uuid2", uuid2);
    menuMgr.setAuthenticator(&auth);

	EepromAuthenticationInfoMenuItem menuItem("Authorised Keys", nullptr, 2002, nullptr);
    menuItem.init();
	RuntimeMenuItem *itm = menuItem.asParent();
	char sz[20];
	itm->copyNameToBuffer(sz, sizeof(sz));
	assertStringEquals("Authorised Keys", sz);
	itm->copyValue(sz, sizeof(sz));
	assertStringEquals("", sz);

	assertEquals(uint8_t(6), itm->getNumberOfParts());

	itm = menuItem.getChildItem(0);
	itm->copyNameToBuffer(sz, sizeof(sz));
	assertStringEquals("uuid1", sz);
	itm->copyValue(sz, sizeof(sz));
	assertStringEquals("Remove", sz);

	itm = menuItem.getChildItem(1);
	itm->copyNameToBuffer(sz, sizeof(sz));
	assertStringEquals("uuid2", sz);

	itm = menuItem.getChildItem(2);
	itm->copyNameToBuffer(sz, sizeof(sz));
	assertStringEquals("EmptyKey", sz);

	assertTrue(isMenuRuntime(&menuItem));
	assertFalse(isMenuBasedOnValueItem(&menuItem));
	assertFalse(isMenuRuntimeMultiEdit(&menuItem));

}

#endif // RUNTIME_MENU_ITEM_TESTS_H