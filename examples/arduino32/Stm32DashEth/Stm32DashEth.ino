/**
 * This is a simple demo application for Stm32Duino based boards. It just showcases many of the types of editor that
 * are available. By default it is setup for an OLED screen and a rotary encoder, although it could be moved to use
 * many other different display and input technologies.
 *
 * It demonstrates the use of the card menu layout, where items are shown one at a time as you scroll through them,
 * configuring some items to use larger than usual fonts, and other items to render as icons. It also has an ethernet
 * network remote included too.
 *
 * To build your own menu: https://designer.thecoderscorner.com/
 * Getting started: https://www.thecoderscorner.com/products/apps/tcmenu-designer/
 * Documentation: https://www.thecoderscorner.com/products/arduino-libraries/
 */

// this is the menu project wiring file, always needs to be included
#include "Stm32DashEth_menu.h"
// now we include the libraries that we're using.
#include <STM32Ethernet.h>
#include <SPI.h>
#include <TaskManagerIO.h>
#include <graphics/TcThemeBuilder.h>
#include <IoLogging.h>
#include <stockIcons/directionalIcons.h>
#include <stockIcons/wifiAndConnectionIcons16x12.h>
// and then we wire in the custom drawing that we demo in this sketch
#include "RawCustomDrawing.h"
#include "app_icondata.h"

// We added a RAM based scroll choice item, and this references a fixed width array variable.
// This variable is the RAM data for scroll choice item Scroll
// https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
char ramDataSet[] = "1\0        2\0        3\0        4\0        5\0        ~";

//
// These settings here simply set up the regular Stm32Ethernet library later on
//
const uint8_t myManualIp[] = { 192, 168, 0, 202 };
const uint8_t myManualMac[] = { 0xde, 0xed, 0xbe, 0xef, 0xfe, 0xed };
const uint8_t standardNetMask[] = { 255, 255, 255, 0 };

//
// We use a card layout to present the items, card layout means that only one item will be displayed at once on the
// display with a left and right icon on the edges showing in which direction you can move between items. Below is a
// somewhat rough ASCII-art example of how it may look on the display.
//
//  /    __    \  the left and right icons show when you can move in each direction
//  |   |  |   |  one item (whatever is actively selected) is drawn in the middle
//  \    --    /  it is recommended that the title be disabled during card-layout menus
//
// This demonstrates how to set up card layout for both the root menu and also an additional sub menu, preparing
// custom drawing using icons or larger fonts for those items.
//
// START card layout and custom layout code

// Some helpful guides for working with card layouts and theme builder:
// 1. https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/creating-and-using-bitmaps-menu/
// 2. https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/themes/rendering-with-themes-icons-grids/

// here we provide two title widgets, for ethernet connection, and client connection
TitleWidget widgetConnection(iconsConnection, 2, 16, 12);
TitleWidget widgetEthernet(iconsEthernetConnection, 2, 16, 12);

color_t defaultCardPalette[] = {1, 0, 1, 1};


// Declaring any arrays used by enum/list items
const char* SettingsDefaultEnumEntries[] = { "33", "45", "78" };
const char* MoreItemsOptionsEnumEntries[] = { "Pizza", "Pasta", "Salad" };
const char* DecimalStepEnumEntries[] = { "1x", "2x", "4x" };

void buildMenu(TcMenuBuilder& builder) {
    builder        .actionItem(MENU_33_ID, "33", NoMenuFlags, nullptr)
        .actionItem(MENU_45_ID, "45", NoMenuFlags, nullptr)
        .actionItem(MENU_78_ID, "78", NoMenuFlags, nullptr)
        .subMenu(MENU_STATUS_CARDS_ID, "Cards", NoMenuFlags, nullptr)
            .actionItem(MENU_STATUS_CARDS_ACE_ID, "Ace", NoMenuFlags, nullptr)
            .actionItem(MENU_STATUS_CARDS_CLUB_ID, "Club", NoMenuFlags, nullptr)
            .actionItem(MENU_STATUS_CARDS_HRTS_ID, "Hrts", NoMenuFlags, nullptr)
            .actionItem(MENU_STATUS_CARDS_DMND_ID, "Dmnd", NoMenuFlags, nullptr)
            .endSub()
        .subMenu(MENU_STATUS_ID, "Status", NoMenuFlags, nullptr)
            .analogBuilder(MENU_STATUS_CURRENT_ID, "Actual", DONT_SAVE, MenuFlags().readOnly(), 0, nullptr)
                .offset(0).divisor(1).step(1).maxValue(7800).unit("RPM").endItem()
            .analogBuilder(MENU_STATUS_MOTOR_ID, "Motor", DONT_SAVE, MenuFlags().readOnly(), 0, nullptr)
                .offset(0).divisor(10).step(1).maxValue(300).unit("V").endItem()
            .analogBuilder(MENU_STATUS_WO_W_ID, "WoW", DONT_SAVE, MenuFlags().readOnly(), 15000, nullptr)
                .offset(-15000).divisor(5000).step(1).maxValue(30000).unit("%").endItem()
            .endSub()
        .subMenu(MENU_SETTINGS_ID, "Settings", NoMenuFlags, nullptr)
            .analogBuilder(MENU_SETTINGS_OFST33_ID, "Ofst33", 25, NoMenuFlags, 5000, nullptr)
                .offset(-5000).divisor(1000).step(1).maxValue(10000).unit("%").endItem()
            .analogBuilder(MENU_SETTINGS_OFST45_ID, "Ofst45", 27, NoMenuFlags, 5000, nullptr)
                .offset(-5000).divisor(1000).step(1).maxValue(10000).unit("%").endItem()
            .analogBuilder(MENU_SETTINGS_OFST78_ID, "Ofst78", 29, NoMenuFlags, 5000, nullptr)
                .offset(-5000).divisor(1000).step(1).maxValue(10000).unit("%").endItem()
            .enumItem(MENU_SETTINGS_DEFAULT_ID, "Default", DONT_SAVE, SettingsDefaultEnumEntries, 3, NoMenuFlags, 0, nullptr)
            .actionItem(MENU_SETTINGS_SAVE_NOW_ID, "Save Now", NoMenuFlags, nullptr)
            .endSub()
        .subMenu(MENU_MORE_ITEMS_ID, "More Items", NoMenuFlags, nullptr)
            .enumItem(MENU_MORE_ITEMS_OPTIONS_ID, "Options", 14, MoreItemsOptionsEnumEntries, 3, NoMenuFlags, 0, nullptr)
            .boolItem(MENU_MORE_ITEMS_TOPPINGS_ID, "Toppings", 16, NAMING_YES_NO, NoMenuFlags, false, nullptr)
            .boolItem(MENU_MORE_ITEMS_POWER_ID, "Power", 17, NAMING_ON_OFF, NoMenuFlags, false, nullptr)
            .actionItem(MENU_MORE_ITEMS_PRESS_ME_ID, "Save", NoMenuFlags, saveWasPressed)
            .floatItem(MENU_MORE_ITEMS_NUMBER_ID, "Number", DONT_SAVE, 1, NoMenuFlags, 0.0, nullptr)
            .scrollChoiceBuilder(MENU_MORE_ITEMS_SCROLL_ID, "Scroll", DONT_SAVE, NoMenuFlags, 0, nullptr).fromRamChoices(ramDataSet, 5, 10).endItem()
            .endSub()
        .subMenu(MENU_RUNTIMES_ID, "Runtimes", NoMenuFlags, nullptr)
            .textItem(MENU_RUNTIMES_TEXT_ID, "Text", 18, 5, NoMenuFlags, "", nullptr)
            .listItemRtCustom(MENU_RUNTIMES_CUSTOM_LIST_ID, "Custom List", 0, fnRuntimesCustomListRtCall, NoMenuFlags, nullptr)
            .remoteConnectivityMonitor(MENU_IO_T_MONITOR_ID, "IoT Monitor", NoMenuFlags)
            .eepromAuthenticationItem(MENU_AUTHENTICATOR_ID, "Authenticator", NoMenuFlags, nullptr)
            .analogBuilder(MENU_HALVES1_ID, "Halves1", DONT_SAVE, NoMenuFlags, 0, nullptr)
                .offset(0).divisor(2).step(1).maxValue(255).unit("dB").endItem()
            .largeNumberItem(MENU_LGE_NUM1_ID, "Lge Num1", DONT_SAVE, LargeFixedNumber(9, 3, 0U, 0U, false), true, NoMenuFlags, largeNumDidChange)
            .endSub()
        .analogBuilder(MENU_DECIMAL_ID, "Decimal", 2, NoMenuFlags, 0, decimalDidChange)
            .offset(0).divisor(10).step(1).maxValue(1000).unit("d").endItem()
        .enumItem(MENU_DECIMAL_STEP_ID, "Decimal Step", 23, DecimalStepEnumEntries, 3, NoMenuFlags, 0, onDecimalStepChange);
}

void overrideDrawingForMainMenu(TcThemeBuilder& themeBuilder) {
    // now we make the two settings and status menus use icons instead of regular drawing, and the numbered menu items
    // 33,45,78 to use a larger font so it takes the entire screen.

    // we're going to use this a few times so declare once
    const Coord iconSize(APPICONS_WIDTH, APPICONS_HEIGHT);

    // override menu33 to draw text centered in a large font with more padding
    themeBuilder.menuItemOverride(getMenu33())
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .withNativeFont(u8g2_font_inr33_mn, 1)
            .withPadding(MenuPadding(2))
            .onRow(0)
            .apply();

    // override menu45 to draw text centered in a large font with more padding
    themeBuilder.menuItemOverride(getMenu45())
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .withNativeFont(u8g2_font_inr33_mn, 1)
            .withPadding(MenuPadding(2))
            .onRow(1)
            .apply();

    // override menu78 to draw text centered in a large font with more padding
    themeBuilder.menuItemOverride(getMenu78())
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .withNativeFont(u8g2_font_inr33_mn, 1)
            .withPadding(MenuPadding(2))
            .onRow(2)
            .apply();

    // override settings to draw as an icon only
    themeBuilder.menuItemOverride(getMenuSettings())
            .withImageXbmp(iconSize, settingsIcon40Bits)
            .withPalette(defaultCardPalette)
            .onRow(3)
            .withDrawingMode(tcgfx::GridPosition::DRAW_AS_ICON_ONLY).apply();

    // override status to draw as an icon only
    themeBuilder.menuItemOverride(getMenuStatus())
            .withImageXbmp(iconSize, statusIcon40Bits)
            .withPalette(defaultCardPalette)
            .onRow(4)
            .withDrawingMode(tcgfx::GridPosition::DRAW_AS_ICON_ONLY).apply();

    themeBuilder.menuItemOverride(getMenuCards())
            .withImageXbmp(Coord(32, 32), cardIconBitmap)
            .onRow(5)
            .withPalette(defaultCardPalette)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .apply();
}

void overrideDrawingForCardMenu(TcThemeBuilder& themeBuilder) {
    // override  back button on the card menu to be the default 32x32 back icon
    themeBuilder.menuItemOverride(*(getMenuCards().getChild()))
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_WITH_VALUE)
            .withPadding(MenuPadding(2))
            .withPalette(defaultCardPalette)
            .withImageXbmp(Coord(32, 32), defaultBackIconBitmap)
            .onRow(0) // <== if you prefer that the back item is not first, you can change its order here
            .apply();

    // override every single item on the card sub menu to have larger font and different padding/justification
    themeBuilder.submenuPropertiesActionOverride(getMenuCards())
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .withNativeFont(u8g2_font_inb16_mf, 1)
            .withPadding(MenuPadding(2))
            .apply();
}

void setupCardLayoutAndWidgets() {
    // create a theme builder to help us configure how to draw.
    TcThemeBuilder themeBuilder(renderer);

    // enable card layout providing the left and right icons. This enables for root menu
    themeBuilder.enableCardLayoutWithXbmImages(Coord(11, 22), ArrowHoriz11x22BitmapLeft, ArrowHoriz11x22BitmapRight, true)
        .setMenuAsCard(getMenuCards(), true);

    // these two functions, defined directly above this one configure the icons and special text arrangements for
    // the items in the card layouts.
    overrideDrawingForMainMenu(themeBuilder);
    overrideDrawingForCardMenu(themeBuilder);

    themeBuilder.addingTitleWidget(widgetEthernet)
            .addingTitleWidget(widgetConnection);

    // You must always call apply after doing anything with theme builder.
    themeBuilder.apply();

    // now we set the title widgets that appear on the top right, for the link status we check the ethernet library
    // status every half second and update the widget to represent that status.
    taskManager.scheduleFixedRate(500, [] {
        widgetEthernet.setCurrentState(Ethernet.linkStatus() == LinkON ? 1 : 0);
    });

    // for the connectivity icon, we use the IoT monitors notification pass through. It tells us of any changes
    // for all incoming connections in one place.
    getMenuIoTMonitor().registerCommsNotification([](CommunicationInfo ci) {
        widgetConnection.setCurrentState(ci.connected ? 1 : 0);
    });
}

// END card / custom layouts


void setup() {
    // This example logs using IoLogging, see the following guide to enable
    // https://www.thecoderscorner.com/products/arduino-libraries//io-abstraction/arduino-logging-with-io-logging/
    IOLOG_START_SERIAL
    serEnableLevel(SER_NETWORK_DEBUG, true);
    serEnableLevel(SER_TCMENU_DEBUG, true);

    // Start up serial and prepare the correct SPI, your pins or method of doing this may differ
    SPI.setMISO(PB4);
    SPI.setMOSI(PB5);
    SPI.setSCLK(PB3);

    // Here we start up the Stm32Ethernet library for STM32 boards/chips with built-in ethernet, again you could
    // easily change this for your own boards network arrangements
    Ethernet.begin();
    Serial.print("My IP address is ");
    Ethernet.localIP().printTo(Serial);
    Serial.println();

    setupMenu();

    // now load back values from EEPROM, but only when we can read the confirmatory magic key, see EEPROM docs:
    // https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-eeprom-integrations/
    menuMgr.load(0xd00d, [] {
        // this gets called when the menu hasn't been saved before, to initialise the first time.
        getMenuDecimal().setCurrentValue(4);
    });

    // here we register the custom drawing created in `RawCustomDrawing.h`
    myCustomDrawing.registerWithRenderer();

    // We can set a callback for when the title item is pressed on the main menu, here we just take over the display
    setTitlePressedCallback([](int) {
        renderer.takeOverDisplay();
    });

    // set the list to have 10 rows, for each row the custom callback further down will be called to get the value.
    getMenuCustomList().setNumberOfRows(10);

    // now we set up the layouts to make the card view look right.
    setupCardLayoutAndWidgets();
}

void loop() {
    taskManager.runLoop();

}


void CALLBACK_FUNCTION saveWasPressed(int id) {
    auto bspBackupRam = reinterpret_cast<HalStm32EepromAbstraction*>(menuMgr.getEepromAbstraction());
    menuMgr.save(0xd00d);
    bspBackupRam->commit();}

// This callback needs to be implemented by you, see the below docs:
//  1. List Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
//  2. ScrollChoice Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
int CALLBACK_FUNCTION fnRuntimesCustomListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    if(mode == RENDERFN_VALUE && row != LIST_PARENT_ITEM_POS) {
        strncpy(buffer, "Val", bufferSize);
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    }
    return defaultRtListCallback(item, row, mode, buffer, bufferSize);
}

void CALLBACK_FUNCTION onDecimalStepChange(int id) {
    //
    // Analog menu items support the concept of step, that is the number of ticks forward that one detent of the
    // encoder represents, here we read the step enum item to get the current value, and then call setStep on the
    // analog item to set how much one tick moves the current value.
    //
    int stepChoice = getMenuDecimalStep().getCurrentValue();
    int stepVal;
    switch (stepChoice) {
    case 0:
    default:
        stepVal = 1;
        break;
    case 1:
        stepVal = 2;
        break;
    case 2:
        stepVal = 4;
        break;
    }
    getMenuDecimal().setStep(stepVal);
    getMenuDecimal().setChanged(true);
    serlogF2(SER_DEBUG, "Decimal Step now ", stepVal);
}


void CALLBACK_FUNCTION decimalDidChange(int id) {
    char sz[20];
    getMenuDecimal().copyValue(sz, sizeof sz);
    serlogF2(SER_DEBUG, "Value decimal ", sz);
}


void CALLBACK_FUNCTION largeNumDidChange(int id) {
}
