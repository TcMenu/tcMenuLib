/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _TCUTIL_H
#define _TCUTIL_H

#include <Arduino.h>

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

#endif