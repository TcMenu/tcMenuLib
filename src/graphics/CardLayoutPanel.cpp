/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "CardLayoutPanel.h"


CardLayoutPane::CardLayoutPane(const DrawableIcon *left, const DrawableIcon *right, MenuTouchScreenManager* optionalTouch)
        : leftButton(left), rightButton(right), iconLeft(left), touchScreenManager(optionalTouch) {
    usingCardLayout.add(SubMenuUsingCardLayout(0, true));
}

bool CardLayoutPane::isSubMenuCardLayout(MenuItem *item) {
    menuid_t itemId = item != nullptr ? item->getId() : 0;
    SubMenuUsingCardLayout* sc = usingCardLayout.getByKey(itemId);
    return (sc && sc->getState());
}

void CardLayoutPane::setSubMenuState(MenuItem *item, bool onOrOff) {
    menuid_t itemId = item != nullptr ? item->getId() : 0;
    SubMenuUsingCardLayout* existing = usingCardLayout.getByKey(itemId);
    if(existing != nullptr) {
        existing->setState(onOrOff);
    } else {
        usingCardLayout.add(SubMenuUsingCardLayout(itemId, onOrOff));
    }
}

void CardLayoutPane::forMenu(ItemDisplayProperties* titleProps, DeviceDrawable* rootDrawable, bool titleNeeded) {
    inUse = true;
    if(touchScreenManager) {
        touchScreenManager->setSecondaryObserver(this);
    }

    leftButton.setColors(titleProps->getColor(ItemDisplayProperties::BACKGROUND), titleProps->getColor(ItemDisplayProperties::TEXT), titleProps->getColor(ItemDisplayProperties::HIGHLIGHT2));
    rightButton.setColors(titleProps->getColor(ItemDisplayProperties::BACKGROUND), titleProps->getColor(ItemDisplayProperties::TEXT), titleProps->getColor(ItemDisplayProperties::HIGHLIGHT2));
    int titleEndY = 0;
    int remainingHeight = rootDrawable->getDisplayDimensions().y;
    if(titleNeeded) {
        titleEndY = titleProps->getRequiredHeight();
        remainingHeight -= titleEndY;
    }

    Coord buttonSize(iconLeft->getDimensions().x + (titleProps->getPadding().left * 2), remainingHeight);
    leftButton.setPositionAndSize(Coord(0, titleEndY), buttonSize);
    rightButton.setPositionAndSize(Coord(rootDrawable->getDisplayDimensions().x - buttonSize.x, titleEndY), buttonSize);

    menuItemLocation = Coord(buttonSize.x, titleEndY + titleProps->getSpaceAfter());
    menuItemSize = Coord(rootDrawable->getDisplayDimensions().x - (buttonSize.x * 2), remainingHeight);
    titleSize = Coord(rootDrawable->getDisplayDimensions().x, titleEndY);
}

void CardLayoutPane::setTouchManager(MenuTouchScreenManager *manager) {
    touchScreenManager = manager;
}

void CardLayoutPane::notInUse() {
    if(inUse && touchScreenManager != nullptr) {
        touchScreenManager->clearSecondaryObserver();
    }
    inUse = false;
}

CardLayoutPane::CardLayoutDir CardLayoutPane::prepareAndPaintButtons(GraphicsDeviceRenderer* renderer, int active, int countOfItems) {
    CardLayoutDir tempStatus = dirStatus;
    dirStatus = DOING_NOTHING;

    leftButton.paintButton(renderer->getDeviceDrawable());
    rightButton.paintButton(renderer->getDeviceDrawable());

    return tempStatus;
}

void CardLayoutPane::touched(const TouchNotification& notification) {
    if(!notification.isWithinItem()) {
        bool leftTouched = leftButton.touchInBounds(notification.getCursorPosition());
        bool rightTouched = rightButton.touchInBounds(notification.getCursorPosition());
        leftButton.setButtonDrawingMode(leftTouched ? TcDrawableButton::SELECTED : TcDrawableButton::NORMAL);
        rightButton.setButtonDrawingMode(rightTouched ? TcDrawableButton::SELECTED : TcDrawableButton::NORMAL);
        dirStatus = DOING_NOTHING;
        if(leftTouched) {
            dirStatus = GOING_LEFT;
        }
        else if(rightTouched) {
            dirStatus = GOING_RIGHT;
        }
    }
}
