/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "MenuItems.h"
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
    return (dp==7) ? 10000000L : (dp == 6) ? 1000000L : (dp == 5) ? 100000 : (dp == 4) ? 10000 : (dp == 3) ? 1000 : (dp == 2) ? 100 : 10;
}

void fastltoa(char* str, long val, uint8_t dp, bool zeroPad, int len) {
    fastltoa_mv(str, val, dpToDivisor(dp), zeroPad, len);
}

void fastltoa_mv(char* str, long val, long divisor, bool zeroPad, int len) {
    int i=0;
    len -=2;
    divisor /= 10;

    while(str[i] && i < len) ++i; 

    bool hadNonZeroChar = false;

    while(divisor > 9 && i < len) {
        str[i] = (char)((val / divisor) + '0');
        hadNonZeroChar |= (str[i] != '0');
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
