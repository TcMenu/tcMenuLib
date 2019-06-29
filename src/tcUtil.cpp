/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "MenuItems.h"
#include "tcMenu.h"
#include "tcUtil.h"

void appendChar(char* str, char val, int len) {
    int i = 0;
    len -= 2;
    while(str[i] && len) {
        --len;
        ++i;
    } 
    str[i++] = val;
    str[i] = (char)0;
}

long dpToDivisor(int dp) {
    switch(dp) {
        case 8: return 100000000L;
        case 7: return 10000000L;
        case 6: return 1000000L;
        case 5: return 100000;
        case 4: return 10000;
        case 3: return 1000;
        case 2: return 100;
        case 1: return 10;
        default:
        case 9: return 1000000000L;
    }
}

void ltoaClrBuff(char* str, long val, uint8_t dp, char padChar, int len) {
    str[0]=0;
    fastltoa_mv(str, val, dpToDivisor(dp), padChar, len);
}

void fastltoa(char* str, long val, uint8_t dp, char padChar, int len) {
    fastltoa_mv(str, val, dpToDivisor(dp), padChar, len);
}

void fastltoa_mv(char* str, long val, long divisor, char padChar, int len) {
    int i=0;
    len -=2;

	if (val < 0) {
		val = abs(val);
		appendChar(str, '-', len);
	}

    val %= divisor;
    divisor /= 10;

    while(str[i] && i < len) ++i; 

    bool hadNonZeroChar = false;
    bool zeroPad = padChar != 0;

    while(divisor > 9 && i < len) {
        str[i] = (char)((val / divisor) + '0');
        hadNonZeroChar |= (str[i] != '0');
        if(zeroPad && !hadNonZeroChar) str[i] = padChar;
        if(zeroPad || hadNonZeroChar) ++i;
        val %= divisor;
        divisor /= 10;
    } 
    str[i++] = '0' + (val % 10);
    str[i] = (char)0;
}

uint8_t itemCount(MenuItem* item) {
	uint8_t count = 0;
	while (item) {
		++count;
		item = item->getNext();
	}
	return count;
}

#if defined __AVR__ || defined ESP_H
char szGlobalBuffer[16];
#endif

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
