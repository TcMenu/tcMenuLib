//
// Created by Dave Cherry on 17/10/2026.
//

#include "TcTouchKeyboardController.h"
#include "stockIcons/directionalIcons.h"
#include "tcMenu.h"
#include "MenuItems.h"
#include "EditableLargeNumberMenuItem.h"

using namespace tcgfx;

DrawableIcon drawUpArrowLgeBitmap(65000U, Coord(16, 20), DrawableIcon::ICON_XBITMAP, tcMenuUpArrowLgeBitmap, nullptr);
DrawableIcon drawDownArrowLgeBitmap(65001U, Coord(16, 20), DrawableIcon::ICON_XBITMAP, tcMenuDownArrowLgeBitmap, nullptr);


TcTouchKeyboardController::TcTouchKeyboardController(TouchKeyboardLayout* defKeyboard, GraphicsDeviceRenderer& renderer, ItemDisplayProperties& properties) :
    upButton(&drawUpArrowLgeBitmap), downButton(&drawDownArrowLgeBitmap), keyboardLayout(nullptr),
    touchRenderer(renderer), properties(properties), defKeyboard(defKeyboard)  {
}

void TcTouchKeyboardController::initialiseScrollbar(int titleHeight) {
    // work out where on the screen the buttons should be
    int btnWid = drawUpArrowLgeBitmap.getDimensions().x + 6;
    auto xLoc = touchRenderer.getWidth() - btnWid;
    auto yHei = touchRenderer.getHeight() - titleHeight;
    // configure the up and down buttons both positionally and color
    upButton.setPositionAndSize(Coord(xLoc, titleHeight), Coord(btnWid, yHei / 2));
    upButton.setColors(properties.getColor(ItemDisplayProperties::TEXT), properties.getColor(ItemDisplayProperties::BACKGROUND), properties.getColor(ItemDisplayProperties::HIGHLIGHT1));
    downButton.setPositionAndSize(Coord(xLoc, titleHeight + yHei / 2), Coord(btnWid, yHei / 2));
    downButton.setColors(properties.getColor(ItemDisplayProperties::TEXT), properties.getColor(ItemDisplayProperties::BACKGROUND), properties.getColor(ItemDisplayProperties::HIGHLIGHT1));
}

void TcTouchKeyboardController::startEditing(MenuItem* menuItem, KeyboardTouchEditor* actEditor) {
    if (menuItem == nullptr) {
        serlogF(SER_WARNING, "TchEd-null");
        return;
    } else if (actEditor) {
        itemEditor = actEditor;
    } else if (menuItem->getMenuType() == MENUTYPE_TEXT_VALUE || menuItem->getMenuType() == MENUTYPE_LARGENUM_VALUE) {
        itemEditor = perCharEditor.initFor(menuItem);
    } else if (menuItem->getMenuType() == MENUTYPE_DATE || menuItem->getMenuType() == MENUTYPE_IPADDRESS || menuItem->getMenuType() == MENUTYPE_COLOR_VALUE || menuItem->getMenuType() == MENUTYPE_TIME) {
        itemEditor = summingEditor.initFor(menuItem);
    }
    // the keyboard starts hidden.
    keyboardLayout = defKeyboard;
}

void TcTouchKeyboardController::drawKeyboardController(DeviceDrawable* drawable, const Coord& displayDims, int titleHeight) {
    if (keyboardLayout) {
        keyboardLayout->drawRows(touchRenderer, 0);
    } else {
        const auto yHei = displayDims.y - titleHeight;
        const auto xLoc = displayDims.x - TC_KEYBOARD_BUTTON_SIZE;
        upButton.setPositionAndSize(Coord(xLoc, titleHeight), Coord(TC_KEYBOARD_BUTTON_SIZE, yHei / 2));
        upButton.setColors(properties.getColor(ItemDisplayProperties::TEXT), properties.getColor(ItemDisplayProperties::BACKGROUND), properties.getColor(ItemDisplayProperties::HIGHLIGHT1));
        upButton.paintButton(drawable);
        downButton.setPositionAndSize(Coord(xLoc, titleHeight + yHei / 2), Coord(TC_KEYBOARD_BUTTON_SIZE, yHei / 2));
        downButton.setColors(properties.getColor(ItemDisplayProperties::TEXT), properties.getColor(ItemDisplayProperties::BACKGROUND), properties.getColor(ItemDisplayProperties::HIGHLIGHT1));
        downButton.paintButton(drawable);
    }
}

void TcTouchKeyboardController::reportTouch(const Coord& touch) const {
    if (keyboardLayout) {
        keyboardLayout->touchInRow(touch);
    } else if (upButton.touchInBounds(touch)) {
        //perform up action
        switches.getEncoder()->increment(-1);
    } else if (downButton.touchInBounds(touch)) {
        // perform down action
        switches.getEncoder()->increment(+1);
    }
}

void TcTouchKeyboardController::stopEditing() {
    keyboardLayout = nullptr;
    touchRenderer.redrawRequirement(MENUDRAW_COMPLETE_REDRAW);
}

TcTouchKeyboardController* tcgfx::defaultTouchKeyboardController(const Coord& size, GraphicsDeviceRenderer& renderer, ItemDisplayProperties& properties) {
    auto* layout = new TouchKeyboardLayout(size);
    layout->withRow("789>", 4)->withRow("456<", 4)->withRow("123X", 4)->withRow(".0+", 4);
    auto* touchKeyboard = new TcTouchKeyboardController(layout, renderer, properties);
    return touchKeyboard;
}
