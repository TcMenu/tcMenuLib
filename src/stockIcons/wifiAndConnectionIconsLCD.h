/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _WIFI_STOCK_CONNECTION_LCD_H
#define _WIFI_STOCK_CONNECTION_LCD_H

const uint8_t iconWifiNotConnected[8] PROGMEM = {
    0b00000,
    0b01110,
    0b10001,
    0b10001,
    0b01010,
    0b01010,
    0b00100,
    0    
};

const uint8_t iconWifiLowSignal[8] PROGMEM = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00100,
    0    
};

const uint8_t iconWifiMedSignal[8] PROGMEM = {
    0b00000,
    0b00000,
    0b00000,
    0b00100,
    0b01010,
    0b00000,
    0b00100,
    0    
};

const uint8_t iconWifiStrongSignal[8] PROGMEM = {
    0b00000,
    0b01110,
    0b10001,
    0b00100,
    0b01010,
    0b00000,
    0b00100,
    0    
};

const uint8_t iconWifiBestSignal[8] PROGMEM = {
    0b01110,
    0b10001,
    0b01110,
    0b10001,
    0b00100,
    0b01010,
    0b00100,
    0    
};

const uint8_t iconConnected[8] PROGMEM = {
    0b00001110,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00010001,
    0b00011111,
    0b00001110,
    0
};

const uint8_t iconDisconnected[8] PROGMEM = {
    0b00011111,
    0b00010011,
    0b00010011,
    0b00010101,
    0b00010101,
    0b00011001,
    0b00011111,
    0
};

const uint8_t* const iconsWifi[] PROGMEM = { iconWifiNotConnected, iconWifiLowSignal, iconWifiMedSignal, iconWifiStrongSignal, iconWifiBestSignal };

const uint8_t* const iconsConnection[] PROGMEM = { iconDisconnected, iconConnected };

#endif //_WIFI_STOCK_CONNECTION_LCD_H