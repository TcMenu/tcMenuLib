/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "RemoteAuthentication.h"
#include <IoLogging.h>

void EepromAuthenticatorManager::initialise(EepromAbstraction* eeprom, EepromPosition start, uint16_t magicKey) {
    this->romStart = start;
    this->eeprom = eeprom;
    this->magicKey = magicKey;
    if(eeprom->read16(romStart) != magicKey) {
        // we need a clean start, the magic key has not been written at position 0
        resetAllKeys();
    }
}

bool EepromAuthenticatorManager::addAdditionalUUIDKey(const char* connectionName, const char* uuid) {
    if(eeprom == NULL) {
        serdebugF("EEPROM Auth not initialised!!");
        return false;
    }

    // find a space for the key
    int insertAt = findSlotFor(connectionName);
    if(insertAt == -1) {
        serdebugF2("Add Key failure ", connectionName);
        return false; // no spaces left
    } 

    // temp space to store the strings and ensure zero terminated.
    char buffer[UUID_KEY_SIZE];
    
    // buffer and then write out the name
    strncpy(buffer, connectionName, sizeof(buffer));
    buffer[CLIENT_DESC_SIZE-1] = 0;
    eeprom->writeArrayToRom(eepromOffset(insertAt), reinterpret_cast<const uint8_t*>(buffer), CLIENT_DESC_SIZE);
    
    // buffer and then write out the uuid
    strncpy(buffer, uuid, sizeof(buffer));
    buffer[UUID_KEY_SIZE-1] = 0;
    eeprom->writeArrayToRom(eepromOffset(insertAt) + CLIENT_DESC_SIZE, reinterpret_cast<const uint8_t*>(buffer), UUID_KEY_SIZE);

    serdebugF2("Add Key success ", connectionName);
    return true;
}

bool EepromAuthenticatorManager::isAuthenticated(const char* connectionName, const char* authResponse) {
    if(eeprom == NULL) {
        serdebugF("EEPROM Auth not initialised!!");
        return false;
    }
    char buffer[UUID_KEY_SIZE];
    int i = findSlotFor(connectionName);
    if(i != -1) {
        eeprom->readIntoMemArray(reinterpret_cast<uint8_t*>(buffer), eepromOffset(i) + CLIENT_DESC_SIZE, sizeof(buffer));
        buffer[UUID_KEY_SIZE-1] = 0;
        serdebugF3("uuid rom, mem ", buffer, authResponse)
        if(strcmp(buffer, authResponse) == 0) {
            serdebugF2("Authenticated ", connectionName);
            return true;
        }
        else {
             serdebugF2("Invalid Key ", connectionName);
        }
    }
    serdebugF2("Not found ", connectionName);
    return false;
}

void EepromAuthenticatorManager::copyKeyNameToBuffer(int idx, char* buffer, int bufSize) {
    if(idx < 0 || idx >= numberOfEntries) {
        buffer[0]=0;
        return;
    }

    eeprom->readIntoMemArray(reinterpret_cast<uint8_t*>(buffer), eepromOffset(idx), min(bufSize, CLIENT_DESC_SIZE));
    buffer[bufSize-1]=0;
}

void EepromAuthenticatorManager::resetAllKeys() {
    serdebugF("Resetting auth store");
    eeprom->write16(romStart, magicKey);
    for(int i=0; i<numberOfEntries;i++) {
        // we just zero the name and UUID first character, to clear it.
        eeprom->write8(eepromOffset(i), 0);
        eeprom->write8(eepromOffset(i) + CLIENT_DESC_SIZE, 0);
    }
}

int EepromAuthenticatorManager::findSlotFor(const char* name) {
    int emptySlot = -1;
    for(int i=0;i<numberOfEntries;i++) {
        uint8_t val = eeprom->read8(eepromOffset(i));
        if(emptySlot == -1 && val == 0) {
            // if there's an empty slot and we haven't got one, then record it.
            emptySlot = i;
        }
        else if(val != 0) {
            // however if there's an existing slot for the same name this takes priority
            char buffer[CLIENT_DESC_SIZE];
            eeprom->readIntoMemArray(reinterpret_cast<uint8_t*>(buffer), eepromOffset(i), CLIENT_DESC_SIZE);
            buffer[CLIENT_DESC_SIZE-1]=0;
            if(strcmp(buffer, name)==0) {
                // existing name found, so we give this index back.
                return i;
            }
        }
    }
    return emptySlot;
}

bool ReadOnlyAuthenticationManager::isAuthenticated(const char* connectionName, const char* authResponse) { 
    for(int i=0;i<numberOfEntries;i++) {
        if(strcmp_P(connectionName, authBlocksPgm[i].name) == 0) {                
            bool keyMatch = strcmp_P(authResponse, authBlocksPgm[i].uuid) == 0;
            serdebugF3("AuthBlock found (name, match) ", connectionName, keyMatch);
            return keyMatch;
        }
    }
    serdebugF2("AuthBlock not found for ", connectionName);
    return false;
}
