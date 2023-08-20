/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "CardLayoutPanel.h"


CardLayoutPane::CardLayoutPane(const DrawableIcon *left, const DrawableIcon *right, MenuTouchScreenManager* optionalTouch, bool mono)
        : leftButton(left), rightButton(right), iconLeft(left), touchScreenManager(optionalTouch) {
    usingCardLayout.add(SubMenuUsingCardLayout(0, true));
    if(mono) {
        leftButton.setButtonOnMonoDisplay(true);
        rightButton.setButtonOnMonoDisplay(true);
    }
}

bool CardLayoutPane::isSubMenuCardLayout(MenuItem *item) {
    if(item == nullptr) return false; // it is an unmanaged menu, so cannot be card layout.
    SubMenuUsingCardLayout* sc = usingCardLayout.getByKey(item->getId());
    return (sc && sc->getState());
}

void CardLayoutPane::setEnablementForSub(MenuItem *item, bool onOrOff) {
    if(item == nullptr) return;

    SubMenuUsingCardLayout* existing = usingCardLayout.getByKey(item->getId());
    if(existing != nullptr) {
        existing->setState(onOrOff);
    } else {
        usingCardLayout.add(SubMenuUsingCardLayout(item->getId(), onOrOff));
    }
}

void CardLayoutPane::forMenu(ItemDisplayProperties* titleProps, ItemDisplayProperties* buttonProps, GraphicsDeviceRenderer* gfxRenderer, bool titleNeeded) {
    serlogF(SER_TCMENU_INFO, "Card Layout init");
    theRenderer = gfxRenderer;
    inUse = true;
    if(touchScreenManager) {
        touchScreenManager->setSecondaryObserver(this);
    }

    auto rootDrawable = gfxRenderer->getDeviceDrawable();

    leftButton.setColors(buttonProps->getColor(ItemDisplayProperties::BACKGROUND), buttonProps->getColor(ItemDisplayProperties::TEXT), buttonProps->getColor(ItemDisplayProperties::HIGHLIGHT2));
    rightButton.setColors(buttonProps->getColor(ItemDisplayProperties::BACKGROUND), buttonProps->getColor(ItemDisplayProperties::TEXT), buttonProps->getColor(ItemDisplayProperties::HIGHLIGHT2));
    int titleEndY = 0;
    int remainingHeight = rootDrawable->getDisplayDimensions().y;
    if(titleNeeded) {
        titleEndY = titleProps->getRequiredHeight();
        remainingHeight -= titleEndY;
    }

    Coord buttonSize(iconLeft->getDimensions().x + (buttonProps->getPadding().left * 2), remainingHeight);
    leftButton.setPositionAndSize(Coord(0, titleEndY + buttonProps->getSpaceAfter()), buttonSize);
    rightButton.setPositionAndSize(Coord(rootDrawable->getDisplayDimensions().x - buttonSize.x, titleEndY + buttonProps->getSpaceAfter()), buttonSize);

    menuItemLocation = Coord(buttonSize.x, titleEndY + titleProps->getSpaceAfter());
    menuItemSize = Coord(rootDrawable->getDisplayDimensions().x - (buttonSize.x * 2), remainingHeight);
    titleSize = Coord(rootDrawable->getDisplayDimensions().x, titleEndY);
    theRenderer->setEditStatusIconsEnabled(false);
}

void CardLayoutPane::notInUse() {
    if(!inUse) return;

    if(touchScreenManager != nullptr) {
        touchScreenManager->clearSecondaryObserver();
    }
    theRenderer->setEditStatusIconsEnabled(true);

    inUse = false;
}

CardLayoutPane::CardLayoutDir CardLayoutPane::prepareAndPaintButtons(GraphicsDeviceRenderer* renderer, int active, int countOfItems, bool titleActive) {
    CardLayoutDir tempStatus = dirStatus;
    dirStatus = DOING_NOTHING;

    // work out if we are at the bounds for this menu - IE first or last item and grey out buttons as needed.
    leftButton.setButtonDrawingMode(((titleActive && active <= 1) || (!titleActive && active == 0)) ? TcDrawableButton::NOT_SELECTABLE : TcDrawableButton::NORMAL);
    rightButton.setButtonDrawingMode(active >= (countOfItems - 1) ? TcDrawableButton::NOT_SELECTABLE : TcDrawableButton::NORMAL);

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
