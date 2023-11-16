/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _TCUTIL_H
#define _TCUTIL_H

#include <PlatformDetermination.h>
#include <TextUtilities.h>
#include <MenuItems.h>

#include <BasicIoAbstraction.h>
#include "RemoteTypes.h"

/**
 * @file tcUtil.h
 * @brief A series of utilities that used throughout tcMenu
 */

/**
 * This structure is created in program memory and passed to all RemoteConnector instances.
 * It contains the name that this device should be identified by and it's UUID.
 */
struct ConnectorLocalInfo {
    char name[30];
    char uuid[38];
};

/**
 * Provides the serial number for the board, it can be displayed and also used in remote JOIN messages to identify
 * the board uniquiely by UUID and serial number.
 *
 * If you want to use the default implementation you can define flag TC_BOARD_SERIAL_NO to a long int value that will
 * be stored as a constant in memory.
 *
 * If you want to have a custom implementation, define TC_MANUAL_SERIAL_NO_IMPL and then you must implement this method
 * yourself instead. This function should not take undue time if implemented by you as it could block networking.
 * @return the serial number
 */
uint32_t getBoardSerialNumber();

/**
 * Show the TcMenu version in a dialog that can be dismissed
 * @param localInfo the local app information
 */
void showVersionDialog(const ConnectorLocalInfo* localInfo);

/**
 * Create a menu info structure using the new operator for the given parameters, max value is optional
 * @param name the name to copy from RAM not PGM
 * @param id the ID to give the info block
 * @param eeprom the eerprom position or -1
 * @param cb the callback to apply or nullptr
 * @param max optional the max value
 */
AnyMenuInfo *newAnyMenuInfoP(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, uint16_t max = 0);

/**
 * Create a menu info structure using the new operator for the given parameters, max value is optional
 * @param name the name to copy from RAM
 * @param id the ID to give the info block
 * @param eeprom the eerprom position or -1
 * @param cb the callback to apply or nullptr
 * @param max optional the max value
 */
AnyMenuInfo *newAnyMenuInfo(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, uint16_t max = 0);

/**
 * Create a menu info structure using the new operator for the given parameters, it populates all the fields for an
 * analog item dynamically
 * @param name the name to copy from PGM
 * @param id the ID to give the info block
 * @param eeprom the eerprom position or -1
 * @param cb the callback to apply or nullptr
 * @param max optional the max value
 * @param offset the offset to apply
 * @param divisor the divisor to apply
 * @param unit the unit to copy from PGM
 */
AnalogMenuInfo* newAnalogMenuInfoP(const char* name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, uint16_t max, uint16_t offset, uint16_t divisor, const char* unit);

/**
 * Create a menu info structure using the new operator for the given parameters, it populates all the fields for an
 * analog item dynamically
 * @param name the name to copy from RAM
 * @param id the ID to give the info block
 * @param eeprom the eerprom position or -1
 * @param cb the callback to apply or nullptr
 * @param max optional the max value
 * @param offset the offset to apply
 * @param divisor the divisor to apply
 * @param unit the unit to copy from RAM
 */
AnalogMenuInfo* newAnalogMenuInfo(const char* name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, uint16_t max, uint16_t offset, uint16_t divisor, const char* unit);

/**
 * Create a menu info structure using the new operator for the given parameters, it populates all the fields for a
 * boolean item dynamically
 * @param name the name to copy from PGM
 * @param id the ID to give the info block
 * @param eeprom the eerprom position or -1
 * @param cb the callback to apply or nullptr
 * @param naming the naming type to use
 */
BooleanMenuInfo *newBooleanMenuInfoP(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, BooleanNaming naming);

/**
 * Create a menu info structure using the new operator for the given parameters, it populates all the fields for a
 * boolean item dynamically
 * @param name the name to copy from RAM
 * @param id the ID to give the info block
 * @param eeprom the eerprom position or -1
 * @param cb the callback to apply or nullptr
 * @param naming the naming type to use
 */
BooleanMenuInfo *newBooleanMenuInfo(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, BooleanNaming naming);

/**
 * Create a menu info structure using the new operator for the given parameters, it populates all the fields for a
 * float item dynamically
 * @param name the name to copy from PGM
 * @param id the ID to give the info block
 * @param eeprom the eerprom position or -1
 * @param cb the callback to apply or nullptr
 * @param decimalPlaces the number of DP to display
 */
FloatMenuInfo *newFloatMenuInfoP(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, int decimalPlaces);

/**
 * Create a menu info structure using the new operator for the given parameters, it populates all the fields for a
 * float item dynamically
 * @param name the name to copy from RAM
 * @param id the ID to give the info block
 * @param eeprom the eerprom position or -1
 * @param cb the callback to apply or nullptr
 * @param decimalPlaces the number of DP to display
 */
FloatMenuInfo *newFloatMenuInfo(const char *name, menuid_t id, uint16_t eeprom, MenuCallbackFn cb, int decimalPlaces);


// There now follows pretty much internal code, boolean negative for dealing with program memory
// never use direct program memory commands, always prefer these, it allows us
// to compile it out much easier.

/**
 * This is always safe to call, if there's a string that's in program mem on AVR
 * but not on other boards, this always does the right thing.
 * @param dst the destination buffer
 * @param pgmSrc the source to be copied (program mem on AVR)
 * @param size the size of dst.
 */
uint8_t safeProgCpy(char* dst, const char* pgmSrc, uint8_t size);

// for AVR only definitions
#ifdef __AVR__
#include <avr/pgmspace.h>
#define get_info_callback(x) ((MenuCallbackFn)pgm_read_ptr_near(x))
#define TCMENU_DEFINED_PLATFORM PLATFORM_ARDUINO_8BIT
#endif

// for ESP only definitions
#ifdef ESP_H
#include <pgmspace.h>
#define get_info_callback(x) ((MenuCallbackFn)(*x))
#define TCMENU_DEFINED_PLATFORM PLATFORM_ARDUINO_32BIT
#endif

// for things that are the same between AVR and ESP
#if (defined __AVR__ || defined ESP_H) && !defined __MBED__
#define PGM_TCM PROGMEM
extern char szGlobalBuffer[];
inline char* potentialProgramMemory(const char *x) {
    safeProgCpy(szGlobalBuffer, x, 16);
    return szGlobalBuffer;
}
#define get_info_char(x) ((char) pgm_read_byte_near(x)) 
#define get_info_int(x) ((int16_t)pgm_read_word_near(x))
#define get_info_uint(x) ((unsigned int)pgm_read_word_near(x))
#define safeProgStrLen(x) (strlen_P(x))
#else
#define TCMENU_DEFINED_PLATFORM PLATFORM_ARDUINO_32BIT
#define PGM_TCM
#define potentialProgramMemory(x) (x)
#define get_info_char(x) ((char)(*x)) 
#define get_info_int(x) ((int16_t)(*x))
#define get_info_uint(x) ((unsigned int)(*x))
#define get_info_callback(x) ((MenuCallbackFn)(*x))
#define safeProgStrLen(x) (strlen(x))

#if !defined(pgm_read_dword) && (defined(__MBED__) || defined(BUILD_FOR_PICO_CMAKE))
# define pgm_read_byte(addr) (*(const unsigned char *)(addr))
# define pgm_read_word(addr) (*(const unsigned short *)(addr))
# define pgm_read_dword(addr) (*(const unsigned long *)(addr))
# define pgm_read_float(addr) (*(const float *)(addr))
# define pgm_read_ptr(addr) (*(addr))
# define memcpy_P memcpy
#endif // pgm_read_byte
#endif

#endif
