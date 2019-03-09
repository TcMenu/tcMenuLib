
#include <tcMenu.h>
#include <Adafruit_ILI9341.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "tcMenuAdaFruitGfx.h"

Adafruit_ILI9341 gfx(6, 7);
AdaColorGfxMenuConfig config;

AdaFruitGfxMenuRenderer renderer(&gfx, &config, 320, 240);
const char applicationName[] = "Graphics Test";

void prepareGfxConfig() {
	makePadding(config.titlePadding, 5, 5, 20, 5);
	makePadding(config.itemPadding, 5, 5, 3, 5);
	makePadding(config.widgetPadding, 5, 10, 0, 5);

	config.bgTitleColor = RGB(255, 255, 0);
	config.fgTitleColor = RGB(0, 0, 0);
	config.titleFont = &FreeSans18pt7b;
	config.titleBottomMargin = 10;

	config.bgItemColor = RGB(0, 0, 0);
	config.fgItemColor = RGB(222, 222, 222);
	config.itemFont = &FreeSans9pt7b;

	config.bgSelectColor = RGB(0, 0, 200);
	config.fgSelectColor = RGB(255, 255, 255);
	config.widgetColor = RGB(30, 30, 30);

	config.titleFontMagnification = 1;
	config.itemFontMagnification = 1;
}
const uint8_t iconWifiNotConnected[] PROGMEM = {
	0b00000001, 0b10000000,
	0b00000110, 0b01100000,
	0b00111000, 0b00011100,
	0b11000000, 0b00000011,
	0b01000000, 0b00000010,
	0b00100000, 0b00000100,
	0b00010000, 0b00001000,
	0b00001000, 0b00010000,
	0b00000100, 0b00100000,
	0b00000011, 0b11000000,
};

const uint8_t iconWifiOneBar[] PROGMEM = {
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000001, 0b10000000,
	0b00000001, 0b10000000
};

const uint8_t iconWifiTwoBar[] PROGMEM = {
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000111, 0b11100000,
	0b00001100, 0b00110000,
	0b00000001, 0b10000000,
	0b00000001, 0b10000000
};

const uint8_t iconWifiThreeBar[] PROGMEM = {
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000000, 0b00000000,
	0b00000111, 0b11100000,
	0b00011100, 0b00111000,
	0b00110000, 0b00001100,
	0b00000111, 0b11100000,
	0b00001100, 0b00110000,
	0b00000001, 0b10000000,
	0b00000001, 0b10000000
};

const uint8_t iconWifiFourBar[] PROGMEM = {
	0b00000011, 0b11000000,
	0b00001111, 0b11110000,
	0b01110000, 0b00001110,
	0b11000111, 0b11100011,
	0b00011100, 0b00111000,
	0b00110000, 0b00001100,
	0b00000111, 0b11100000,
	0b00001100, 0b00110000,
	0b00000001, 0b10000000,
	0b00000001, 0b10000000
};

const uint8_t iconConnectionNone[] PROGMEM = {
	0b01111111, 0b11111110,
	0b10110000, 0b00000001,
	0b10001100, 0b00000001,
	0b10000110, 0b00000001,
	0b10000011, 0b00000001,
	0b10000000, 0b11000001,
	0b10000001, 0b00110001,
	0b10000000, 0b00001101,
	0b10000001, 0b10000011,
	0b01111111, 0b11111110,
};

const uint8_t iconConnected[] PROGMEM = {
	0b01111111, 0b11111110,
	0b11000000, 0b00000011,
	0b11011101, 0b11101111,
	0b11000000, 0b00000011,
	0b11011011, 0b00110011,
	0b11000000, 0b00000011,
	0b11000000, 0b00000011,
	0b11000000, 0b00000011,
	0b11000001, 0b10000011,
	0b01111111, 0b11111110,
};

const uint8_t* iconsWifi[] PROGMEM = { iconWifiNotConnected, iconWifiOneBar, iconWifiTwoBar, iconWifiThreeBar, iconWifiFourBar };
const uint8_t* iconsConnection[] PROGMEM = { iconConnectionNone, iconConnected };

TitleWidget connectedWidget(iconsConnection, 2, 16, 10);
TitleWidget wifiWidget(iconsWifi, 5, 16, 10, &connectedWidget);

const AnalogMenuInfo minfoRHSTemp = { "R HS Temp", 8, 0xffff, 255, NO_CALLBACK, 0, 2, "C" };
AnalogMenuItem menuRHSTemp(&minfoRHSTemp, 0, NULL);
const AnalogMenuInfo minfoLHSTemp = { "L HS Temp", 7, 0xffff, 255, NO_CALLBACK, 0, 2, "C" };
AnalogMenuItem menuLHSTemp(&minfoLHSTemp, 0, &menuRHSTemp);
const SubMenuInfo minfoStatus = { "Status", 5, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackStatus(&menuLHSTemp, (const AnyMenuInfo*)&minfoStatus);
SubMenuItem menuStatus(&minfoStatus, &menuBackStatus, NULL);
const AnalogMenuInfo minfoContrast = { "Contrast", 10, 6, 255, NO_CALLBACK, 0, 2, "" };
AnalogMenuItem menuContrast(&minfoContrast, 0, NULL);
const BooleanMenuInfo minfo12VStandby = { "12V Standby", 4, 0xffff, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menu12VStandby(&minfo12VStandby, false, &menuContrast);
const SubMenuInfo minfoSettings = { "Settings", 3, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackSettings(&menu12VStandby, (const AnyMenuInfo*)&minfoSettings);
SubMenuItem menuSettings(&minfoSettings, &menuBackSettings, &menuStatus);
const char enumStrChannel_0[] = "CD Player";
const char enumStrChannel_1[] = "Turntable";
const char enumStrChannel_2[] = "Computer";
const char* const enumStrChannel[] = { enumStrChannel_0, enumStrChannel_1, enumStrChannel_2 };
const EnumMenuInfo minfoChannel = { "Channel", 2, 4, 2, NO_CALLBACK, enumStrChannel };
EnumMenuItem menuChannel(&minfoChannel, 0, &menuSettings);
const AnalogMenuInfo minfoVolume = { "Volume", 1, 2, 255, NO_CALLBACK, -190, 2, "dB" };
AnalogMenuItem menuVolume(&minfoVolume, 0, &menuChannel);


void setup() {
	Serial.print("Testing adafruit driver");
	Serial.println(applicationName);
	prepareGfxConfig();
	gfx.begin();
	gfx.setRotation(3);
	menuMgr.initWithoutInput(&renderer, &menuVolume);
	menuVolume.setActive(true);
	renderer.setFirstWidget(&wifiWidget);

	taskManager.scheduleFixedRate(5, [] {
		renderer.onSelectPressed(&menuVolume);

		int wifiState = wifiWidget.getCurrentState() + 1;
		if (wifiState == 5) wifiState = 0;
		wifiWidget.setCurrentState(wifiState);

		connectedWidget.setCurrentState(!connectedWidget.getCurrentState());
	}, TIME_SECONDS);
}

void loop() {
	taskManager.runLoop();
}
