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


// here we provide two title widgets, for ethernet connection, and client connection
TitleWidget widgetConnection(iconsConnection, 2, 16, 12, nullptr);
TitleWidget widgetEthernet(iconsEthernetConnection, 2, 16, 12, &widgetConnection);


//
// We use a card layout to present the items, here we demonstrate how to set it up and prepare custom menu items that
// have different layouts and fonts.
//
// START card layout and custom layout code

// first we need to define both a left and right button, we use the ones from stockIcons/directionalIcons.h
DrawableIcon iconLeft(-1, Coord(11, 22), tcgfx::DrawableIcon::ICON_XBITMAP, ArrowHoriz11x22BitmapLeft, nullptr);
DrawableIcon iconRight(-1, Coord(11, 22), tcgfx::DrawableIcon::ICON_XBITMAP, ArrowHoriz11x22BitmapRight, nullptr);

color_t defaultCardPalette[] = {1, 0, 1, 1};

void setupGridLayoutForCardView() {
    auto & factory = renderer.getGraphicsPropertiesFactory();

    // Now we define the grid layouts for the 33, 45 and 78 action items, each one has custom font size and therefore a
    // custom height as well. It avoids the use of icons for the speed selectors.
    factory.setDrawingPropertiesForItem(tcgfx::ItemDisplayProperties::COMPTYPE_ACTION, menu33.getId(), defaultCardPalette,
                                        MenuPadding(2), u8g2_font_inr33_mn, 1, 2, 44, tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE, MenuBorder());
    factory.setDrawingPropertiesForItem(tcgfx::ItemDisplayProperties::COMPTYPE_ACTION, menu45.getId(), defaultCardPalette,
                                        MenuPadding(2), u8g2_font_inr33_mn, 1, 2, 44, tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE, MenuBorder());
    factory.setDrawingPropertiesForItem(tcgfx::ItemDisplayProperties::COMPTYPE_ACTION, menu78.getId(), defaultCardPalette,
                                        MenuPadding(2), u8g2_font_inr33_mn, 1, 2, 44, tcgfx::GridPosition::JUSTIFY_CENTER_NO_VALUE, MenuBorder());

    // now we make the two settings and status menus use icons instead of regular drawing.
    const Coord iconSize(APPICONS_WIDTH, APPICONS_HEIGHT);
    factory.addImageToCache(DrawableIcon(menuSettings.getId(), iconSize, DrawableIcon::ICON_XBITMAP, settingsIcon40Bits));
    factory.addImageToCache(DrawableIcon(menuStatus.getId(), iconSize, DrawableIcon::ICON_XBITMAP, statusIcon40Bits));
    factory.addGridPosition(&menuSettings, GridPosition(GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_NO_VALUE, 4, 40));
    factory.addGridPosition(&menuStatus, GridPosition(GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_NO_VALUE, 5, 40));

    // after adding things to the drawing properties, we must refresh it.
    tcgfx::ConfigurableItemDisplayPropertiesFactory::refreshCache();

    // and now we set the title widgets that appear on the top right, for the link status we check the ethernet library
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

    // lastly, we set the card layout for the main "root" menu. The last parameter is an optional touch screen interface
    // that the card layout will interact with, to "flip" between cards. Set to null when no touch screen available.
    renderer.enableCardLayout(iconLeft, iconRight, nullptr, true);

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
    serlogF2(SER_DEBUG, "Decimal Step now ", stepVal);
}
