/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _TCUTIL_H
#define _TCUTIL_H

#include <PlatformDetermination.h>
#include <TextUtilities.h>

#include <BasicIoAbstraction.h>
#include "RemoteTypes.h"

// forward reference.
class MenuItem;

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
#endif

#endif
