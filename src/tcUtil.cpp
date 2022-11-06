/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "MenuItems.h"
#include "tcMenu.h"
#include "tcUtil.h"

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
