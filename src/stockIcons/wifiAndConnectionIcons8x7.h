#ifndef _WIFI_AND_CONNECTION_ICONS_8X7
#define _WIFI_AND_CONNECTION_ICONS_8X7

const uint8_t iconConnectionNone[] PROGMEM = {
	0b01111111,
	0b01100011,
	0b01010101,
	0b01001001,
	0b01010101,
	0b01100011,
	0b01111111,
};

const uint8_t iconConnected[] PROGMEM = {
	0b01111111,
	0b01000001,
	0b01000001,
	0b01000001,
	0b01000001,
	0b01001001,
	0b01111111,
};

const uint8_t iconWifiNone[] PROGMEM = {
	0b01111110,
	0b10000001,
	0b01000010,
	0b01000010,
	0b00100100,
	0b00100100,
	0b00011000
};

const uint8_t iconWifiLow[] PROGMEM = {
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00011000
};


const uint8_t iconWifiMed[] PROGMEM = {
	0b00000000,
	0b00000000,
	0b00000000,
	0b00000000,
	0b00111100,
	0b01000010,
	0b00011000
};

const uint8_t iconWifiGood[] PROGMEM = {
	0b00000000,
	0b00000000,
	0b01111110,
	0b10000001,
	0b00111100,
	0b01000010,
	0b00011000
};

const uint8_t iconWifiStrong[] PROGMEM = {
	0b11111111,
	0b10000001,
	0b01111110,
	0b10000001,
	0b00111100,
	0b01000010,
	0b00011000
};


// here is the definition of the actual widget, where we assign the above bitmaps to the widget.
const uint8_t* const iconsWifi[] PROGMEM = { iconWifiNone, iconWifiLow, iconWifiMed, iconWifiGood, iconWifiStrong };

const uint8_t* const iconsConnection[] PROGMEM = { iconConnectionNone, iconConnected };

#endif //_WIFI_AND_CONNECTION_ICONS_8X7