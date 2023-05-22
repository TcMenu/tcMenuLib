/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "MenuItems.h"
#include "tcMenu.h"
#include "tcUtil.h"
#include "BaseDialog.h"
#include "tcMenuVersion.h"
#include "MenuItems.h"

#if defined __AVR__ || defined ESP_H
char szGlobalBuffer[16];
#endif

// only if we are implementing the serial number storage
#ifndef TC_MANUAL_SERIAL_NO_IMPL

// if not defined give a developer one of one short of a billion
#ifndef TC_BOARD_SERIAL_NO
#define TC_BOARD_SERIAL_NO 999999999L
#endif //!TC_BOARD_SERIAL_NO

// and implement the function
const long glTcSerialNumber PROGMEM = TC_BOARD_SERIAL_NO;
uint32_t getBoardSerialNumber() {
    return pgm_read_dword(&glTcSerialNumber);
}
#endif // !TC_MANUAL_SERIAL_NO_IMPL

uint8_t safeProgCpy(char* dst, const char* pgmSrc, uint8_t size) {
    uint8_t pos = 0;
    char nm = get_info_char(pgmSrc);
    while (nm && pos < (size - 1)) {
		dst[pos] = nm;
		++pgmSrc;
        ++pos;
        nm = get_info_char(pgmSrc);
    }
    dst[pos] = 0;
    return pos;
}

void showVersionDialog(const ConnectorLocalInfo *localInfo) {
    static const ConnectorLocalInfo *localInfoStatic = localInfo;
    withMenuDialogIfAvailable([](MenuBasedDialog *dialog) {
        dialog->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
        dialog->show(localInfoStatic->name, false, nullptr);
        char sz[25];
        tccore::copyTcMenuVersion(sz, sizeof(sz));
        strncat(sz, " S/N:", sizeof(sz) - strlen(sz) - 1);
        fastltoa(sz, (long)getBoardSerialNumber(), 9, NOT_PADDED, sizeof sz);
        dialog->copyIntoBuffer(sz);
    });
}

void populateCore(AnyMenuInfo* ptr, const char* n, bool pgm, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, uint16_t max) {
    ptr->callback = cb;
    ptr->maxValue = max;
    ptr->eepromAddr = eeprom;
    ptr->id = id;
    if(pgm) {
        safeProgCpy(ptr->name, n, sizeof(ptr->name));
    } else {
        strncpy(ptr->name, n, sizeof(ptr->name));
    }
}

AnyMenuInfo *newAnyMenuInfoP(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, uint16_t max) {
    auto ptr = new AnyMenuInfo();
    populateCore(ptr, name, INFO_LOCATION_PGM, id, eeprom, cb, max);
    return ptr;
}

AnyMenuInfo *newAnyMenuInfo(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, uint16_t max) {
    auto ptr = new AnyMenuInfo();
    populateCore(ptr, name, INFO_LOCATION_RAM, id, eeprom, cb, max);
    return ptr;
}

AnalogMenuInfo* newAnalogMenuInfo(const char* name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, uint16_t max, uint16_t offset, uint16_t divisor, const char* unit) {
    auto ptr = new AnalogMenuInfo();
    populateCore((AnyMenuInfo*)ptr, name, INFO_LOCATION_RAM, id, eeprom, cb, max);
    ptr->offset = offset;
    ptr->divisor = divisor;
    strncpy(ptr->unitName, unit, sizeof(ptr->unitName));
    return ptr;
}

AnalogMenuInfo* newAnalogMenuInfoP(const char* name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, uint16_t max, uint16_t offset, uint16_t divisor, const char* unit) {
    auto ptr = new AnalogMenuInfo();
    populateCore((AnyMenuInfo*)ptr, name, INFO_LOCATION_PGM, id, eeprom, cb, max);
    ptr->offset = offset;
    ptr->divisor = divisor;
    safeProgCpy(ptr->unitName, unit, sizeof(ptr->unitName));
    return ptr;
}

BooleanMenuInfo *newBooleanMenuInfoP(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, BooleanNaming naming) {
    auto ptr = new BooleanMenuInfo();
    populateCore((AnyMenuInfo*)ptr, name, INFO_LOCATION_PGM, id, eeprom, cb, 1);
    ptr->naming = naming;
    return ptr;
}

BooleanMenuInfo *newBooleanMenuInfo(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, BooleanNaming naming) {
    auto ptr = new BooleanMenuInfo();
    populateCore((AnyMenuInfo*)ptr, name, INFO_LOCATION_RAM, id, eeprom, cb, 1);
    ptr->naming = naming;
    return ptr;
}

FloatMenuInfo *newFloatMenuInfoP(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, int decimalPlaces) {
    auto ptr = new FloatMenuInfo();
    populateCore((AnyMenuInfo*)ptr, name, INFO_LOCATION_PGM, id, eeprom, cb, decimalPlaces);
    return ptr;
}

FloatMenuInfo *newFloatMenuInfo(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, int decimalPlaces) {
    auto ptr = new FloatMenuInfo();
    populateCore((AnyMenuInfo*)ptr, name, INFO_LOCATION_RAM, id, eeprom, cb, decimalPlaces);
    return ptr;
}
