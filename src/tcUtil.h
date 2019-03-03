/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _TCUTIL_H
#define _TCUTIL_H

#include <Arduino.h>
#include "RemoteTypes.h"

/**
 * @file tcUtil.h
 * 
 * A series of utilities that used thoughout tcMenu
 */

/**
 * appends a character at the end of the string, if there is space according to len
 */
void appendChar(char* str, char val, int len);

/**
 * A fast int to ascii function with some limitations. Supports zero padding and number of places to start. only
 * if there's space according to len.
 */
void fastltoa_mv(char* str, long val, long divisor, bool zeroPad, int len);
void fastltoa(char* str, long val, uint8_t dp, bool zeroPad, int len);

class MenuItem; // forward reference.

/**
 * returns the number of items in the current menu described by itemCount
 */
uint8_t itemCount(MenuItem* item);

/**
 * converts decimal places into a suitable divisor, eg: 2 -> 100, 4 -> 10000
 */
long dpToDivisor(int dp);

// There now follows pretty much internal code for dealing with program memory
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

#ifdef __AVR__
#include <avr/pgmspace.h>
#define PGM_TCM PROGMEM
extern char szGlobalBuffer[];
inline char* potentialProgramMemory(const char *x) {
    safeProgCpy(szGlobalBuffer, x, 16);
    return szGlobalBuffer;
}
inline void copy_info_ptr_ptr_array(char* buffer, const char* const** ptr, int size, int idx) {
    char** itemPtr = ((char**)pgm_read_ptr_near(ptr) + idx);
    char* itemLoc = (char *)pgm_read_ptr_near(itemPtr);
    safeProgCpy(buffer, itemLoc, size);
}
inline int get_info_len_ptr_ptr_array(const char* const** ptr, int idx) {
    char** itemPtr = ((char**)pgm_read_ptr_near(ptr) + idx);
    char* itemLoc = (char *)pgm_read_ptr_near(itemPtr);
    return strlen_P(itemLoc);
}
#define get_info_char(x) ((char) pgm_read_byte_near(x)) 
#define get_info_int(x) ((int)pgm_read_word_near(x))
#define get_info_uint(x) ((unsigned int)pgm_read_word_near(x))
#define get_info_callback(x) ((MenuCallbackFn)pgm_read_ptr_near(x))
#define safeProgStrLen(x) (strlen_P(x))
#define TCMENU_DEFINED_PLATFORM PLATFORM_ARDUINO_8BIT
#else 
#define TCMENU_DEFINED_PLATFORM PLATFORM_ARDUINO_32BIT
#define PGM_TCM
#define potentialProgramMemory(x) (x)
#define get_info_char(x) ((char)(*x)) 
#define get_info_int(x) ((int)(*x))
#define get_info_uint(x) ((unsigned int)(*x))
#define get_info_callback(x) ((MenuCallbackFn)(*x))
#define safeProgStrLen(x) (strlen(x))
#endif

#endif
