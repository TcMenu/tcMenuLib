#ifndef _BASE_REMOTE_TESTS_H
#define _BASE_REMOTE_TESTS_H

#include <AUnit.h>
#include <MockEepromAbstraction.h>
#include "RemoteAuthentication.h"

using namespace aunit;

const char *uuid1 = "07cd8bc6-734d-43da-84e7-6084990becfc"; 
const char *uuid2 = "07cd8bc6-734d-43da-84e7-6084990becfd";
const char *uuid3 = "07cd8bc6-734d-43da-84e7-6084990becfe";

void printAllAuthenticatorEntries(const char* why) {
    Serial.print(why);
    Serial.print(". entries : ");
    for(int i=0;i<KEY_STORAGE_SIZE;i++) {
        char sz[16];
        authenticator.copyKeyNameToBuffer(i, sz, sizeof(sz));
        if(sz[0]!=0) {
            Serial.print(sz); 
            Serial.print('('); 
            Serial.print(i); 
            Serial.print(") ");
        }
    }
    Serial.println();
}

test(AuthenticationFixture, authenticationTest) {

    authenticator.initialise(&eeprom, 10);
    assertEqual(eeprom.read16(10), 0x9078);

    // we should be in an out the box state, nothing should authenticate.
    assertFalse(authenticator.isAuthenticated("uuid1", uuid1));
    assertFalse(authenticator.isAuthenticated("uuid2", uuid2));
    assertFalse(authenticator.isAuthenticated("uuid3", uuid3));

    // add two keys and ensure we can authenticate with them
    authenticator.addAdditionalUUIDKey("uuid1", uuid1);
    authenticator.addAdditionalUUIDKey("uuid2", uuid2);
    printAllAuthenticatorEntries("After add");


    assertTrue(authenticator.isAuthenticated("uuid1", uuid1));
    assertTrue(authenticator.isAuthenticated("uuid2", uuid2));
    assertFalse(authenticator.isAuthenticated("uuid3", uuid1));

    // re-add uuid1 as a different ID and check the old key doesn't work
    authenticator.addAdditionalUUIDKey("uuid1", uuid3);
    printAllAuthenticatorEntries("After replace");
    assertFalse(authenticator.isAuthenticated("uuid1", uuid1));
    assertTrue(authenticator.isAuthenticated("uuid1", uuid3));


    // re-initialise, should be from eeprom without clearing it
    authenticator.initialise(&eeprom, 10);
    printAllAuthenticatorEntries("After load");


    // check the state was loaded back properly
    assertTrue(authenticator.isAuthenticated("uuid1", uuid3));
    assertTrue(authenticator.isAuthenticated("uuid2", uuid2));
    assertFalse(authenticator.isAuthenticated("uuid3", uuid2));

    authenticator.resetAllKeys();

    assertFalse(authenticator.isAuthenticated("uuid1", uuid1));
    assertFalse(authenticator.isAuthenticated("uuid2", uuid1));
    assertFalse(authenticator.isAuthenticated("uuid3", uuid1));
}

#endif