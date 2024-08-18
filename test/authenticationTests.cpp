
#include <unity.h>
#include <MockEepromAbstraction.h>
#include "RemoteAuthentication.h"
#include "fixtures_extern.h"


void printAllAuthenticatorEntries(EepromAuthenticatorManager& authenticator, const char* why) {
	serdebug2(why, ". entries : ");
	for (int i = 0; i < authenticator.getNumberOfEntries(); i++) {
		char sz[16];
		authenticator.copyKeyNameToBuffer(i, sz, sizeof(sz));
		if (sz[0] != 0) {
			serdebug3(sz, ' ', i);
		}
	}
}

void authenticationTest() {
	EepromAuthenticatorManager authenticator;
	authenticator.initialise(&eeprom, 10);

    TEST_ASSERT_EQUAL(eeprom.read16(10), uint16_t(0x9B32));

    // we should be in an out the box state, nothing should authenticate.
    TEST_ASSERT_FALSE(authenticator.isAuthenticated("uuid1", uuid1));
    TEST_ASSERT_FALSE(authenticator.isAuthenticated("uuid2", uuid2));
    TEST_ASSERT_FALSE(authenticator.isAuthenticated("uuid3", uuid3));

    // add two keys and ensure we can authenticate with them
    authenticator.addAdditionalUUIDKey("uuid1", uuid1);
    authenticator.addAdditionalUUIDKey("uuid2", uuid2);
    printAllAuthenticatorEntries(authenticator, "After add");

	// now check what we've added
    TEST_ASSERT_TRUE(authenticator.isAuthenticated("uuid1", uuid1));
    TEST_ASSERT_TRUE(authenticator.isAuthenticated("uuid2", uuid2));
    TEST_ASSERT_FALSE(authenticator.isAuthenticated("uuid3", uuid1));

    // re-add uuid1 as a different ID and check the old key doesn't work
    authenticator.addAdditionalUUIDKey("uuid1", uuid3);
    printAllAuthenticatorEntries(authenticator, "After replace");
    TEST_ASSERT_FALSE(authenticator.isAuthenticated("uuid1", uuid1));
    TEST_ASSERT_TRUE(authenticator.isAuthenticated("uuid1", uuid3));

    // re-initialise, should be from eeprom without clearing it
    authenticator.initialise(&eeprom, 10);
    printAllAuthenticatorEntries(authenticator ,"After load");

    // check the state was loaded back properly
    TEST_ASSERT_TRUE(authenticator.isAuthenticated("uuid1", uuid3));
    TEST_ASSERT_TRUE(authenticator.isAuthenticated("uuid2", uuid2));
    TEST_ASSERT_FALSE(authenticator.isAuthenticated("uuid3", uuid2));

	// test clearing down everything, basically wipe eeprom
    authenticator.resetAllKeys();

	// everything should now be gone.
    TEST_ASSERT_FALSE(authenticator.isAuthenticated("uuid1", uuid1));
    TEST_ASSERT_FALSE(authenticator.isAuthenticated("uuid2", uuid1));
    TEST_ASSERT_FALSE(authenticator.isAuthenticated("uuid3", uuid1));
}

void testNoAuthenicatorMode() {
	NoAuthenticationManager noAuth;

	// does nothing but returns true to fulfil interface.
	TEST_ASSERT_TRUE(noAuth.addAdditionalUUIDKey("uuid22", uuid2));
		
	// does nothing but returns true in all cases.
	TEST_ASSERT_TRUE(noAuth.isAuthenticated("uuid1", uuid1));
	TEST_ASSERT_TRUE(noAuth.isAuthenticated("anything", uuid2));
	TEST_ASSERT_TRUE(noAuth.isAuthenticated("...", uuid3));
}

const AuthBlock authBlocks[] PROGMEM = {
	{ "uuid1", "07cd8bc6-734d-43da-84e7-6084990becfc" },  // UUID1
	{ "uuid2", "07cd8bc6-734d-43da-84e7-6084990becfd" }   // UUID2
};

const char pgmPassword[] PROGMEM = "1234";

void testProgmemAuthenicatorMode() {
	ReadOnlyAuthenticationManager roAuth(authBlocks, 2, pgmPassword);

	// check the ones we know should work
	TEST_ASSERT_TRUE(roAuth.isAuthenticated("uuid1", uuid1));
	TEST_ASSERT_TRUE(roAuth.isAuthenticated("uuid2", uuid2));

	// now try combinations of ones that should not.
	TEST_ASSERT_FALSE(roAuth.isAuthenticated("anything", uuid1));
	TEST_ASSERT_FALSE(roAuth.isAuthenticated("uuid2", uuid1));
	TEST_ASSERT_FALSE(roAuth.isAuthenticated("uuid1", uuid2));
	
	// it is not possible to add keys to this manager.
	TEST_ASSERT_FALSE(roAuth.addAdditionalUUIDKey("anyKey", uuid3));
}
