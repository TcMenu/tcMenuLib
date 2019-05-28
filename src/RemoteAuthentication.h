/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file RemoteAuthentication.h
 * Contains the base functionality for communication between the menu library and remote APIs.
 */

#ifndef _REMOTE_AUTHENTICATION_H_
#define _REMOTE_AUTHENTICATION_H_

// START user adjustable

// if you don't want to use security with your connections you can save a few bytes by turning it off here.
// To turn off, comment out the line below to undefine REMOTE_SECURITY_REQUIRED
#define REMOTE_SECURITY_REQUIRED

// END  user adjustable


#ifdef REMOTE_SECURITY_REQUIRED

#include <EepromAbstraction.h>
#include "MenuItems.h"

#define UUID_KEY_SIZE 40
#define CLIENT_DESC_SIZE 16
#define TOTAL_KEY_SIZE (UUID_KEY_SIZE + CLIENT_DESC_SIZE)
#define KEY_STORAGE_SIZE 6
#define MENU_ID_RANGE_AUTH_START 30000

/**
 * This is the external interface of authentication when using the menu library. It
 * provides the support for checking if a connection is authenticated and also for
 * adding new keys.
 */
class AuthenticationManager {
private:
    EepromAbstraction *eeprom;
    EepromPosition romStart;
    uint16_t magicKey;

public:
    AuthenticationManager() {
        eeprom = NULL;
        romStart = 0;
        this->magicKey = 0;
    }

    void initialise(EepromAbstraction* eeprom, EepromPosition start, uint16_t magicKey = 0x9078);

    /**
     * Clear down the whole authentication structure to start again.
     */
    void resetAllKeys();

    /**
     * Copy the name of the item at position idx into the provided buffer.
     * @param idx the index to get the name of
     * @param buffer the buffer to copy into
     * @param bufSize size of buffer usually obtained from sizeof
     */
    void copyKeyNameToBuffer(int idx, char* buffer, int bufSize);

    /**
     * Attempt to add an additional UUID to key mapping, may fail if there is not enough
     * space to add another mapping.
     * @param connectionName the name of the connection
     * @param uuid the key associated with it.
     */ 
    bool addAdditionalUUIDKey(const char* connectionName, const char* uuid);

    /**
     * Check if the connectionName and authResponse match the one on record.
     * @param connectionName the name of the connection
     * @param authResponse the key associated with it.
     */
    bool isAuthenticated(const char* connectionName, const char* authResponse);
private:
    // finds the slot (or an empty slot) or if neither are found returns -1
    int findSlotFor(const char* name);

    // helper to calculate the eeprom position from an index.
    EepromPosition eepromOffset(int i) {
        return romStart + 2 + (i * TOTAL_KEY_SIZE);
    }
};

#else // REMOTE_SECURITY_REQUIRED

/**
 * Should you not wish to use the security features, and there's little benefit memory wise to be honest
 * then this version of the class does nothing and always allows connections.
 */
class AuthenticationManager {
public:
    AuthenticationManager() { }

    void addAdditionalUUIDKey(const char* connectionName, const char* uuid) { }

    bool isAuthenticated(const char* connectionName, const char* /*authResponse*/) { return true; }
};

#endif // REMOTE_SECURITY_REQUIRED

extern AuthenticationManager authenticator;

#endif //_REMOTE_AUTHENTICATION_H_