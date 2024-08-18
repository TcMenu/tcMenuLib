
#include <unity.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <RemoteAuthentication.h>
#include "../tutils/fixtures_extern.h"
#include <tcUtil.h>

RENDERING_CALLBACK_NAME_INVOKE(ipMenuItemTestCb, ipAddressRenderFn, "HelloWorld", 102, NULL)


void testIpAddressItem() {
    taskManager.reset();
	IpAddressMenuItem ipItem(ipMenuItemTestCb, 2039, NULL);
	ipItem.setIpAddress(192U, 168U, 0U, 96U);

	TEST_ASSERT_EQUAL(ipItem.getId(), uint16_t(2039));
	TEST_ASSERT_EQUAL(ipItem.getEepromPosition(), uint16_t(102));

	char sz[32];
	copyMenuItemNameAndValue(&ipItem, sz, sizeof(sz), '[');
	TEST_ASSERT_EQUAL_STRING("HelloWorld[ 192.168.0.96", sz);
	copyMenuItemValue(&ipItem, sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("192.168.0.96", sz);

	TEST_ASSERT_EQUAL(uint8_t(4), ipItem.beginMultiEdit());
	TEST_ASSERT_EQUAL(255, ipItem.nextPart());
	TEST_ASSERT_EQUAL(192, ipItem.getPartValueAsInt());

	TEST_ASSERT_EQUAL(255, ipItem.nextPart());
	TEST_ASSERT_EQUAL(168, ipItem.getPartValueAsInt());

	TEST_ASSERT_EQUAL(255, ipItem.nextPart());
	TEST_ASSERT_EQUAL(0, ipItem.getPartValueAsInt());
	ipItem.valueChanged(2);
	ipItem.copyValue(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("192.168.2.96", sz);
    TEST_ASSERT_TRUE(checkEditorHints(8, 9, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    TEST_ASSERT_EQUAL(255, ipItem.nextPart());
	ipItem.valueChanged(201);
	ipItem.copyValue(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("192.168.2.201", sz);
    TEST_ASSERT_TRUE(checkEditorHints(10, 13, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

	TEST_ASSERT_EQUAL(0, ipItem.nextPart());
	ipItem.copyValue(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("192.168.2.201", sz);

	TEST_ASSERT_TRUE(isMenuRuntime(&ipItem));
	TEST_ASSERT_FALSE(isMenuBasedOnValueItem(&ipItem));
	TEST_ASSERT_TRUE(isMenuRuntimeMultiEdit(&ipItem));
}

void testSettingIpItemDirectly() {
    taskManager.reset();
	IpAddressMenuItem ipItem(ipMenuItemTestCb, 2039, NULL);
	char sz[20];

	ipItem.setIpAddress("192.168.99.22");
	ipItem.copyValue(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("192.168.99.22", sz);

	ipItem.setIpAddress("255.254.12.");
	ipItem.copyValue(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("255.254.12.0", sz);

	ipItem.setIpAddress("127.1.2");
	ipItem.copyValue(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("127.1.2.0", sz);

	ipItem.setIpAddress("badvalue");
	ipItem.copyValue(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("0.0.0.0", sz);

}

void testFloatType() {
    taskManager.reset();
	menuFloatItem.clearSendRemoteNeededAll();
	menuFloatItem.setChanged(false);
	menuFloatItem.setFloatValue(1.001);
	TEST_ASSERT_FLOAT_WITHIN(1.001, menuFloatItem.getFloatValue(), 0.0001);
	TEST_ASSERT_TRUE(menuFloatItem.isChanged());
	TEST_ASSERT_TRUE(menuFloatItem.isSendRemoteNeeded(0));
	TEST_ASSERT_EQUAL(4, menuFloatItem.getDecimalPlaces());
	char sz[20];

    //
    // Be very careful adding test cases for float here, float is really inaccurate, and the maximum number of digits
    // that can be represented (even relatively accurately - avoiding edge cases) is around 7 digits.
    //

    copyMenuItemNameAndValue(&menuFloatItem, sz, sizeof(sz));
	TEST_ASSERT_EQUAL("FloatItem: 1.0010", sz);

	menuFloatItem.setFloatValue(234.456722);
    copyMenuItemValue(&menuFloatItem, sz, sizeof(sz));
	TEST_ASSERT_EQUAL("234.4567", sz);

    menuFloatItem.setFloatValue(-938.4567);
    copyMenuItemValue(&menuFloatItem, sz, sizeof(sz));
    TEST_ASSERT_EQUAL("-938.4567", sz);

    menuFloatItem.setFloatValue(-0.001);
    copyMenuItemValue(&menuFloatItem, sz, sizeof(sz));
    TEST_ASSERT_EQUAL("-0.0010", sz);

    menuFloatItem.setFloatValue(-0.0);
    copyMenuItemValue(&menuFloatItem, sz, sizeof(sz));
    TEST_ASSERT_EQUAL("0.0000", sz);

    TEST_ASSERT_FALSE(isMenuRuntime(&menuFloatItem));
	TEST_ASSERT_FALSE(isMenuBasedOnValueItem(&menuFloatItem));
	TEST_ASSERT_FALSE(isMenuRuntimeMultiEdit(&menuFloatItem));
}

void testAuthMenuItem() {
    taskManager.reset();

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
	TEST_ASSERT_EQUAL_STRING("Authorised Keys", sz);
	itm->copyValue(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("", sz);

	TEST_ASSERT_EQUAL(uint8_t(6), itm->getNumberOfParts());

	itm = menuItem.getChildItem(0);
	itm->copyNameToBuffer(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("uuid1", sz);
	itm->copyValue(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("Remove", sz);

	itm = menuItem.getChildItem(1);
	itm->copyNameToBuffer(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("uuid2", sz);

	itm = menuItem.getChildItem(2);
	itm->copyNameToBuffer(sz, sizeof(sz));
	TEST_ASSERT_EQUAL_STRING("EmptyKey", sz);

	TEST_ASSERT_TRUE(isMenuRuntime(&menuItem));
	TEST_ASSERT_FALSE(isMenuBasedOnValueItem(&menuItem));
	TEST_ASSERT_FALSE(isMenuRuntimeMultiEdit(&menuItem));

}
