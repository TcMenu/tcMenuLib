/**
 * This is a simple demo application for Stm32Duino based boards. It just showcases many of the types of editor that
 * are available. By default it is setup for an OLED screen and a rotary encoder, although it could be moved to use
 * many other different display and input technologies.
 *
 * Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/
 */

#include "generated/stm32DuinoDemo_menu.h"
#include <STM32Ethernet.h>
#include <PlatformDetermination.h>
#include <SPI.h>
#include <TaskManagerIO.h>
#include <IoLogging.h>
#include <stockIcons/directionalIcons.h>
#include "RawCustomDrawing.h"
#include "app_icondata.h"
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <graphics/TcThemeBuilder.h>

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
// 1. https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/creating-and-using-bitmaps-menu/
// 2. https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/themes/rendering-with-themes-icons-grids/

// here we provide two title widgets, for ethernet connection, and client connection
TitleWidget widgetConnection(iconsConnection, 2, 16, 12, nullptr);
TitleWidget widgetEthernet(iconsEthernetConnection, 2, 16, 12, &widgetConnection);

color_t defaultCardPalette[] = {1, 0, 1, 1};

void overrideDrawingForMainMenu(TcThemeBuilder& themeBuilder) {
    // we're going to use this a few times so declare once
    const Coord iconSize(APPICONS_WIDTH, APPICONS_HEIGHT);

    // override menu33 to draw text centered in a large font with more padding
    themeBuilder.menuItemOverride(menu33)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .withNativeFont(u8g2_font_inr33_mn, 1)
            .withPadding(MenuPadding(2))
            .onRow(0)
            .apply();

    // override menu45 to draw text centered in a large font with more padding
    themeBuilder.menuItemOverride(menu45)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .withNativeFont(u8g2_font_inr33_mn, 1)
            .withPadding(MenuPadding(2))
            .onRow(1)
            .apply();

    // override menu78 to draw text centered in a large font with more padding
    themeBuilder.menuItemOverride(menu78)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .withNativeFont(u8g2_font_inr33_mn, 1)
            .withPadding(MenuPadding(2))
            .onRow(2)
            .apply();

    // override settings to draw as an icon only
    themeBuilder.menuItemOverride(menuSettings)
            .withImageXbmp(iconSize, settingsIcon40Bits)
            .withPalette(defaultCardPalette)
            .onRow(3)
            .withDrawingMode(tcgfx::GridPosition::DRAW_AS_ICON_ONLY).apply();

    // override status to draw as an icon only
    themeBuilder.menuItemOverride(menuStatus)
            .withImageXbmp(iconSize, statusIcon40Bits)
            .withPalette(defaultCardPalette)
            .onRow(4)
            .withDrawingMode(tcgfx::GridPosition::DRAW_AS_ICON_ONLY).apply();

    themeBuilder.menuItemOverride(menuStatusCards)
            .withImageXbmp(Coord(32, 32), cardIconBitmap)
            .onRow(5)
            .withPalette(defaultCardPalette)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .apply();
}

void overrideDrawingForCardMenu(TcThemeBuilder& themeBuilder) {
    // override every single item on the card menu to have larger font and different padding/justification
    themeBuilder.submenuPropertiesActionOverride(menuStatusCards)
            .withJustification(tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE)
            .withNativeFont(u8g2_font_inr33_mn, 1)
            .withPadding(MenuPadding(2))
            .apply();
}

void setupGridLayoutForCardView() {
    // create a theme builder to help us configure how to draw.
    TcThemeBuilder themeBuilder(renderer);

    // enable card layout providing the left and right icons. This enables for root menu
    themeBuilder.enableCardLayoutWithXbmImages(Coord(11, 22), ArrowHoriz11x22BitmapLeft, ArrowHoriz11x22BitmapRight, true)
        .setMenuAsCard(menuStatusCards, true);

    // see the method above where we override drawing for all items that are in card layout.
    overrideDrawingForMainMenu(themeBuilder);
    overrideDrawingForCardMenu(themeBuilder);

    // now we make the two settings and status menus use icons instead of regular drawing.
    themeBuilder.apply();

    // now we set the title widgets that appear on the top right, for the link status we check the ethernet library
    // status every half second and update the widget to represent that status.
    renderer.setFirstWidget(&widgetEthernet);
    taskManager.scheduleFixedRate(500, [] {
        widgetEthernet.setCurrentState(Ethernet.linkStatus() == LinkON ? 1 : 0);
    });

    // for the connectivity icon, we use the IoT monitors notification pass through. It tells us of any changes
    // for all incoming connections in one place.
    menuRuntimesIoTMonitor.registerCommsNotification([](CommunicationInfo ci) {
        widgetConnection.setCurrentState(ci.connected ? 1 : 0);
    });
}

// END card / custom layouts

using namespace tcremote;

void setup() {
    // This example logs using IoLogging, see the following guide to enable
    // https://www.thecoderscorner.com/products/arduino-libraries/io-abstraction/arduino-logging-with-io-logging/
    IOLOG_START_SERIAL
    serEnableLevel(SER_NETWORK_DEBUG, true);

    // Start up serial and prepare the correct SPI, your pins may differ
    SPI.setMISO(PB4);
    SPI.setMOSI(PB5);
    SPI.setSCLK(PB3);

    //
    // Here we start up the Stm32Ethernet library for STM32 boards/chips with built-in ethernet
    Ethernet.begin();
    Serial.print("My IP address is ");
    Ethernet.localIP().printTo(Serial);
    Serial.println();

    // and then run the menu setup
    setupMenu();

    // now load back values from EEPROM, but only when we can read the confirmatory magic key, see EEPROM loading in the docs
    menuMgr.load(0xd00d, [] {
        // this gets called when the menu hasn't been saved before, to initialise the first time.
        menuDecimal.setCurrentValue(4);
    });

    // here we register the custom drawing we created earlier with the renderer
    myCustomDrawing.registerWithRenderer();

    // We can set a callback for when the title item is pressed on the main menu, here we just take over the display
    setTitlePressedCallback([](int) {
        renderer.takeOverDisplay();
    });

    // set the list to have 10 rows, for each row the custom callback further down will be called to get the value.
    menuRuntimesCustomList.setNumberOfRows(10);

    // now we set up the layouts to make the card view look right.
    setupGridLayoutForCardView();
}

void loop() {
    taskManager.runLoop();
}

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

void CALLBACK_FUNCTION decimalDidChange(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION saveWasPressed(int id) {
     auto bspBackupRam = reinterpret_cast<HalStm32EepromAbstraction*>(menuMgr.getEepromAbstraction());
     menuMgr.save(0xd00d);
     bspBackupRam->commit();
}


void CALLBACK_FUNCTION largeNumDidChange(int id) {
    // TODO - your menu change code
}


void CALLBACK_FUNCTION onDecimalStepChange(int id) {
    //
    // Analog menu items support the concept of step, that is the number of ticks forward that one detent of the
    // encoder represents, here we read the step enum item to get the current value, and then call setStep on the
    // analog item to set how much one tick moves the current value.
    //
    int stepChoice = menuDecimalStep.getCurrentValue();
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
    menuDecimal.setStep(stepVal);
    menuDecimal.setChanged(true);
    serlogF2(SER_DEBUG, "Decimal Step now ", stepVal);
}
