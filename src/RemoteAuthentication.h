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

#include <EepromAbstraction.h>

#define UUID_KEY_SIZE 40
#define CLIENT_DESC_SIZE 16
#define TOTAL_KEY_SIZE (UUID_KEY_SIZE + CLIENT_DESC_SIZE)

/**
 * This is the external interface of authentication when using the menu library. It
 * provides the support for checking if a connection is authenticated and also for
 * adding new keys. There are two implementations of it so far, and more may follow
 * so always try and use the interface rather than a given impl.
 */
class AuthenticationManager {
public:    
    /**
     * Adds an additional name, key mapping to the authentication manager, if there is space
     * left in internal storage. If there is not enough space, it will return false and you
     * need to reset the storage to proceed, as it's probably full.
     * 
     * @param connectionName the name of the remote
     * @param uuid the key of the API / UI 
     * @return true if successful otherwise false.
     */
    virtual bool addAdditionalUUIDKey(const char* connectionName, const char* uuid)=0;

    /**
     * Checks if a set of connection parameters is allowed to connect.
     * @param connectionName the name of the remote
     * @param authResponse the key from the remote
     * @return true if authenticated otherwise false.
     */
    virtual bool isAuthenticated(const char* connectionName, const char* authResponse)=0;
};

/**
 * An implementation of AuthenticationManager that stores it's values in EEPROM.
 * It stores up to KEY_STORAGE_SIZE (default 6) key value pairs in EEPROM and
 * checks directly against them without any buffering.
 */
class EepromAuthenticatorManager : public AuthenticationManager {
private:
    EepromAbstraction *eeprom;
    EepromPosition romStart;
    uint16_t magicKey;
	uint8_t  numberOfEntries;
public:
    EepromAuthenticatorManager(uint8_t numOfEntries = 6) {
        eeprom = NULL;
        romStart = 0;
        this->magicKey = 0;
		this->numberOfEntries = numOfEntries;
    }

	/**
	 * Initialises the authenticator with an eeprom object, start position in the rom and optional magic key.
	 * the magic key is used to determine if the rom contains anything reasonable on startup. The default will
	 * probably be OK
	 * @param eeprom any eeprom abstraction from the IoAbstraction library
	 * @param start position in the rom to start writing at
	 * @param magicKey the value that is checked on load to see if stored data is trustworthy.
	 */
    void initialise(EepromAbstraction* eeprom, EepromPosition start, uint16_t magicKey = 0x9078);

    /**
     * Reset all keys in this authenticator, such that all connections would need to run
     * through the normal joining behaviour.
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
    bool addAdditionalUUIDKey(const char* connectionName, const char* uuid) override;

    /**
     * Check if the connectionName and authResponse match the one on record.
     * @param connectionName the name of the connection
     * @param authResponse the key associated with it.
     */
    bool isAuthenticated(const char* connectionName, const char* authResponse) override;

	/**
	 * @return the number of spaces for entries in the eeprom
	 */
	int getNumberOfEntries() {
		return numberOfEntries;
	}
private:
    // finds the slot (or an empty slot) or if neither are found returns -1
    int findSlotFor(const char* name);

    // helper to calculate the eeprom position from an index.
    EepromPosition eepromOffset(int i) {
        return romStart + 2 + (i * TOTAL_KEY_SIZE);
    }
};

/**
 * Should you not wish to use the security features, and there's little benefit memory wise to be honest
 * then this version of the class does nothing and always allows connections.
 */
class NoAuthenticationManager : public AuthenticationManager {
public:
    NoAuthenticationManager() { }

    /** prize every time with the No Authentication impl, everyone is always admitted. */
    bool addAdditionalUUIDKey(const char* /*connectionName*/, const char* /*uuid*/) override { return true; }

    /** prize every time with the No Authentication impl, everyone is always admitted. */
    bool isAuthenticated(const char* /*connectionName*/, const char* /*authResponse*/) override { return true; }
};

/**
 * When using the read only authenticator, you create these structures in read only memory up front at compile
 * time using const and PROGMEM.
 */
struct AuthBlock {
    char name[CLIENT_DESC_SIZE];
    char uuid[UUID_KEY_SIZE];
};

/**
 * Should you wish to permit only a small number of pre-known keys to connect, then this version
 * will authenticate against an array of AuthBlock's stored in PROGMEM / constant memory.
 */
class ReadOnlyAuthenticationManager : public AuthenticationManager {
private:
    const AuthBlock* authBlocksPgm;
    int numberOfEntries;
public:
    /**
     * Initialise with an array of AuthBlock structures in program memory and the number of entries
     * @param authBlocksPgm the authorisation blocks in const / program memory
     * @param numberOfEntries the number of blocks in the array.
     */
    ReadOnlyAuthenticationManager(const AuthBlock* authBlocksPgm, int numberOfEntries) {
        this->authBlocksPgm = authBlocksPgm;
        this->numberOfEntries = numberOfEntries;
    }

    /** Does not do anything in this variant - it is read only */
    bool addAdditionalUUIDKey(const char* /*connectionName*/, const char* /*uuid*/) override { return false; }

    /**
     * Checks the list of AuthBlocks to see if any contain our credentials.
     * @param connectionName the name of the remote
     * @param authResponse the key provided in the join message
     */
    bool isAuthenticated(const char* connectionName, const char* authResponse) override;
};

#endif //_REMOTE_AUTHENTICATION_H_
