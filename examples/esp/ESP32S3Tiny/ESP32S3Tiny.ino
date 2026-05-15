// This example shows a simple menu structure with a card layout for the root menu on a TFT. Very simple case of
// Adafruit GFX ST7735 driver for the display, encoder directly connected to device pins.
// We have connected the following way, you can adjust as needed in code generator / sketch
// TFT_DC: 7     MISO: 37      PIN_A:  3
// TFT_CS: 34    MOSI: 35      PIN_B:  4
// TFT_RST: 6    SCLK: 36      PIN_OK: 5
//
// To build your own menu: https://designer.thecoderscorner.com/
// Getting started: https://www.thecoderscorner.com/products/apps/tcmenu-designer/
// Documentation: https://www.thecoderscorner.com/products/arduino-libraries/

#include "ESP32S3Tiny_menu.h"
#include <graphics/TcThemeBuilder.h>
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <stockIcons/directionalIcons.h>
#include "app_icondata.h"

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
// https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/themes/rendering-with-themes-icons-grids/
//
void setupGridLayoutForCardView() {
    const Coord iconSize(APPICONS_WIDTH, APPICONS_HEIGHT);
    TcThemeBuilder themeBuilder(renderer);
    themeBuilder.enableCardLayoutWithXbmImages(Coord(11, 22), ArrowHoriz11x22BitmapLeft, ArrowHoriz11x22BitmapRight, false)
            .addingTitleWidget(titleWidget)
            .manualDimensions(128, 128);

    themeBuilder.menuItemOverride(getMenuSettings())
            .withImageXbmp(iconSize, settingsIcon40Bits)
            .withDrawingMode(tcgfx::GridPosition::DRAW_AS_ICON_TEXT)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .onRow(0)
            .apply();

    themeBuilder.menuItemOverride(getMenuMute())
            .withImageXbmp(iconSize, muteOffIcon40Bits, muteOnIcon40Bits)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .onRow(1)
            .apply();

    themeBuilder.menuItemOverride(getMenuBattery())
            .withImageXbmp(Coord(BATTERY_WIDTH, BATTERY_HEIGHT), batteryIcon40Bits)
            .withDrawingMode(tcgfx::GridPosition::DRAW_AS_ICON_TEXT)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .onRow(2)
            .apply();

    // after doing anything with theme builder always call apply.
    themeBuilder.apply();
}

// END card / custom layouts


// Declaring any arrays used by enum/list items
const char* BatteryConditionEnumEntries[] = { "Bad", "Normal", "Good" };
const char* FoodsEnumEntries[] = { "Pizza", "Pasta", "Salad" };

void buildMenu(TcMenuBuilder& builder) {
    builder        .subMenu(MENU_SETTINGS_ID, "Settings", NoMenuFlags, nullptr)
            .rgb32Item(MENU_NEW_SUB_MENU_COLOR_ID, "Color", DONT_SAVE, false, NoMenuFlags, RgbColor32(0, 0, 0), nullptr)
            .textItem(MENU_NEW_SUB_MENU_TEXT_ID, "Text", DONT_SAVE, 5, NoMenuFlags, "", nullptr)
            .dateItem(MENU_NEW_SUB_MENU_DATE_ID, "Date", DONT_SAVE, NoMenuFlags, DateStorage(1, 1, 2020), nullptr)
            .analogBuilder(MENU_PERCENT1_ID, "Percent1", DONT_SAVE, NoMenuFlags, 0, nullptr)
                .offset(0).divisor(1).step(1).maxValue(100).unit("%").endItem()
            .analogBuilder(MENU_TENTHS1_ID, "Tenths1", DONT_SAVE, NoMenuFlags, 0, nullptr)
                .offset(0).divisor(10).step(1).maxValue(1000).unit("").endItem()
            .largeNumberItem(MENU_LGE_NUM1_ID, "Lge Num1", DONT_SAVE, LargeFixedNumber(6, 4, 0U, 0U, false), true, NoMenuFlags, nullptr)
            .endSub()
        .boolItem(MENU_MUTE_ID, "Mute", DONT_SAVE, NAMING_TRUE_FALSE, NoMenuFlags, false, nullptr)
        .subMenu(MENU_BATTERY_ID, "Battery", NoMenuFlags, nullptr)
            .analogBuilder(MENU_BATTERY_CHARGE_ID, "Charge", DONT_SAVE, MenuFlags().readOnly(), 0, nullptr)
                .offset(0).divisor(1).step(1).maxValue(100).unit("%").endItem()
            .enumItem(MENU_BATTERY_CONDITION_ID, "Condition", DONT_SAVE, BatteryConditionEnumEntries, 3, MenuFlags().readOnly(), 0, nullptr)
            .endSub()
        .enumItem(MENU_FOODS_ID, "Foods", DONT_SAVE, FoodsEnumEntries, 3, NoMenuFlags, 0, nullptr)
        .boolItem(MENU_TO_GO_ID, "To go", DONT_SAVE, NAMING_YES_NO, NoMenuFlags, false, nullptr);
}



void setup() {
    // here we set up the devices and services that we are going to use.
    Serial.begin(115200);
    serEnableLevel(SER_TCMENU_DEBUG, true);
    SPI.begin(36, 37, 35);

    setupMenu();

    // Now we enable the card layout mode for the main menu by first enabling support, then adding the root menu.
    setupGridLayoutForCardView();

    // We can set a callback for when the title item is pressed on the main menu, here we show the app version
    setTitlePressedCallback([](int) {
        showVersionDialog(&applicationInfo);
    });

    // every second we simulate updating the battery condition indicators.
    taskManager.schedule(repeatSeconds(1), [] {
        getMenuCharge().setCurrentValue(rand() % 100);
        getMenuCondition().setCurrentValue(rand() % 3);
    });

    taskManager.schedule(repeatSeconds(30), [] {
        titleWidget.setCurrentState(rand() % 2);
    });
}

void loop() {
    taskManager.runLoop();

}
