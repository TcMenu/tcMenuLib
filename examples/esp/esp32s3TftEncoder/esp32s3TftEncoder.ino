// This example shows a simple menu structure with a card layout for the root menu on a TFT. Very simple case of
// TFT_eSPI for the display, encoder directly connected to device pins.
//
// Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/

#include "generated/esp32s3TftEncoder_menu.h"
#include "stockIcons/directionalIcons.h"
#include "app_icondata.h"
#include "tcMenuVersion.h"
#include <IoLogging.h>
#include <stockIcons/wifiAndConnectionIcons16x12.h>

//
// We use a card layout to present the items, here we demonstrate how to set it up and prepare custom menu items that
// have different layouts and fonts.
//
// START card layout and custom layout code

// first we need to define both a left and right button, we use the ones from stockIcons/directionalIcons.h
DrawableIcon iconLeft(-1, Coord(11, 22), tcgfx::DrawableIcon::ICON_XBITMAP, ArrowHoriz11x22BitmapLeft, nullptr);
DrawableIcon iconRight(-1, Coord(11, 22), tcgfx::DrawableIcon::ICON_XBITMAP, ArrowHoriz11x22BitmapRight, nullptr);

TitleWidget titleWidget(iconsEthernetConnection, 2, 16, 12);

void setupGridLayoutForCardView() {
    auto & factory = renderer.getGraphicsPropertiesFactory();

    // now we make the two settings and status menus use icons instead of regular drawing.
    const Coord iconSize(APPICONS_WIDTH, APPICONS_HEIGHT);
    factory.addImageToCache(DrawableIcon(menuSettings.getId(), iconSize, DrawableIcon::ICON_XBITMAP, settingsIcon40Bits));
    factory.addImageToCache(DrawableIcon(menuMute.getId(), iconSize, DrawableIcon::ICON_XBITMAP, muteOffIcon40Bits, muteOnIcon40Bits));
    const Coord iconBatSize(BATTERY_WIDTH, BATTERY_HEIGHT);
    factory.addImageToCache(DrawableIcon(menuBattery.getId(), iconBatSize, DrawableIcon::ICON_XBITMAP, batteryIcon40Bits));

    factory.addGridPosition(&menuSettings, GridPosition(GridPosition::DRAW_AS_ICON_TEXT, GridPosition::JUSTIFY_CENTER_NO_VALUE, 0, 60));
    factory.addGridPosition(&menuMute, GridPosition(GridPosition::DRAW_AS_ICON_TEXT, GridPosition::JUSTIFY_CENTER_NO_VALUE, 1, 60));
    factory.addGridPosition(&menuBattery, GridPosition(GridPosition::DRAW_AS_ICON_TEXT, GridPosition::JUSTIFY_CENTER_NO_VALUE, 2, 60));

    // after adding things to the drawing properties, we must refresh it.
    tcgfx::ConfigurableItemDisplayPropertiesFactory::refreshCache();
}

// END card / custom layouts

void setup() {
    Serial.begin(115200);

    serEnableLevel(SER_TCMENU_DEBUG, true);
    renderer.setFirstWidget(&titleWidget);
    setupMenu();

    // Here we enable the card layout mode for the main menu by first enabling support, then adding the root menu.
    // We also set up the item layout for card view by calling our setup function, defined above.
    renderer.enableCardLayout(iconLeft, iconRight, nullptr, false);
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
