// This example shows a simple menu structure with a card layout for the root menu on a TFT. Very simple case of
// Adafruit GFX ST7735 driver for the display, encoder directly connected to device pins.
// We have connected the following way, you can adjust as needed in code generator / sketch
// TFT_DC: 7     MISO: 37      PIN_A:  3
// TFT_CS: 34    MOSI: 35      PIN_B:  4
// TFT_RST: 6    SCLK: 36      PIN_OK: 5
//
// Getting started: https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/tcmenu-overview-quick-start/

#include "generated/esp32s3TftEncoder_menu.h"
#include "stockIcons/directionalIcons.h"
#include "app_icondata.h"
#include "tcMenuVersion.h"
#include <IoLogging.h>
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <graphics/TcThemeBuilder.h>
#include <SPI.h>

//
// We use a card layout to present the items, here we demonstrate how to set it up and prepare custom menu items that
// have different layouts and fonts.
//
// START card layout and custom layout code

TitleWidget titleWidget(iconsEthernetConnection, 2, 16, 12);

//
// This example uses card layout for the main menu, this means that one item shows at a time, as we scroll through
// the items, the item is replaced on display. We also override the menu items to render as icons using XBMP icons.
// For each item in the card layout, we override how it is drawn and what row it is drawn on. See:
// https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/themes/rendering-with-themes-icons-grids/
//
void setupGridLayoutForCardView() {
    const Coord iconSize(APPICONS_WIDTH, APPICONS_HEIGHT);
    TcThemeBuilder themeBuilder(renderer);
    themeBuilder.enableCardLayoutWithXbmImages(Coord(11, 22), ArrowHoriz11x22BitmapLeft, ArrowHoriz11x22BitmapRight, false)
            .addingTitleWidget(titleWidget)
            .manualDimensions(128, 128);

    themeBuilder.menuItemOverride(menuSettings)
            .withImageXbmp(iconSize, settingsIcon40Bits)
            .withDrawingMode(tcgfx::GridPosition::DRAW_AS_ICON_TEXT)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .onRow(0)
            .apply();

    themeBuilder.menuItemOverride(menuMute)
            .withImageXbmp(iconSize, muteOffIcon40Bits, muteOnIcon40Bits)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .onRow(1)
            .apply();

    themeBuilder.menuItemOverride(menuBattery)
            .withImageXbmp(Coord(BATTERY_WIDTH, BATTERY_HEIGHT), batteryIcon40Bits)
            .withDrawingMode(tcgfx::GridPosition::DRAW_AS_ICON_TEXT)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .onRow(2)
            .apply();

    // after doing anything with theme builder always call apply.
    themeBuilder.apply();
}

// END card / custom layouts

void setup() {
    setupMenu();
    // here we set up the devices and services that we are going to use.
    Serial.begin(115200);
    serEnableLevel(SER_TCMENU_DEBUG, true);
    SPI.begin(36, 37, 35);

    // Now we enable the card layout mode for the main menu by first enabling support, then adding the root menu.
    setupGridLayoutForCardView();

    // We can set a callback for when the title item is pressed on the main menu, here we show the app version
    setTitlePressedCallback([](int) {
        showVersionDialog(&applicationInfo);
    });

    // every second we simulate updating the battery condition indicators.
    taskManager.schedule(repeatSeconds(1), [] {
        menuBatteryCharge.setCurrentValue(rand() % 100);
        menuBatteryCondition.setCurrentValue(rand() % 3);
    });

    taskManager.schedule(repeatSeconds(30), [] {
        titleWidget.setCurrentState(rand() % 2);
    });
}

void loop() {
    taskManager.runLoop();
}
