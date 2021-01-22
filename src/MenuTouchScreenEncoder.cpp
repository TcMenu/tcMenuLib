/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "MenuTouchScreenEncoder.h"
#include "ScrollChoiceMenuItem.h"

MenuResistiveTouchScreen::MenuResistiveTouchScreen(AnalogDevice *device, BasicIoAbstraction *pins, pinid_t xpPin, pinid_t xnPin, pinid_t ypPin, pinid_t ynPin,
                                                   BaseGraphicalRenderer* renderer, BaseResistiveTouchScreen::TouchRotation rotation)
        : BaseResistiveTouchScreen(device, pins, xpPin, xnPin, ypPin, ynPin, rotation),
          encoder(renderer), renderer(renderer), observer(&encoder), updateTicks(0) {
    renderer->setHasTouchInterface(true);
}

uint32_t MenuResistiveTouchScreen::sendEvent(float locationX, float locationY, float touchPressure,
                                             BaseResistiveTouchScreen::TouchState touched) {
    if(touched != TOUCHED && touched != HELD) {
        updateTicks = 0;
        return 100;
    }
    else if(observer) {
        Coord raw = Coord(float(renderer->getWidth()) * locationX, float(renderer->getHeight()) * locationY);
        Coord localStart(0,0), localSize(0,0);
        GridPositionRowCacheEntry* pEntry = renderer->findMenuEntryAndDimensions(raw, localStart, localSize);
        if(!pEntry) {
            observer->touched(TouchNotification(raw, touched));
        }
        else {
            observer->touched(TouchNotification(pEntry, Coord(raw.x - localStart.x, raw.y - localStart.y), localStart, localSize, touched));
        }
        int nextExec = updateTicks == 0 ? 500 : (updateTicks < 4) ? 200 : (updateTicks < 15) ? 100 : 50;
        updateTicks++;
        return nextExec;
    }
}

bool isTouchActionable(MenuItem* pItem) {
    return isItemActionable(pItem) ||
            pItem->getMenuType() == MENUTYPE_BACK_VALUE ||
            pItem->getMenuType() == MENUTYPE_TITLE_ITEM ||
            pItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE;
}

void MenuTouchScreenEncoder::touched(const TouchNotification &evt) {
    serdebugF4("Touch at (x,y,mode)", evt.getCursorPosition().x, evt.getCursorPosition().y, evt.isWithinItem())
    if(evt.isWithinItem()) {
        if(menuMgr.getCurrentEditor() && evt.getEntry()->getMenuItem() != menuMgr.getCurrentEditor()) {
            // stop editing, selected outside of item
            menuMgr.stopEditingCurrentItem(false);
        }
        else {
            bool wasActive = evt.getEntry()->getMenuItem()->isActive();
            if(!wasActive) {
                // if it's not active try and activate, if it fails we can't continue.
                if (!menuMgr.activateMenuItem(evt.getEntry()->getMenuItem())) return;
            }

            auto menuType = evt.getEntry()->getMenuItem()->getMenuType();
            auto held = evt.getTouchState() == BaseResistiveTouchScreen::HELD;
            if(isTouchActionable(evt.getEntry()->getMenuItem()) && !held) {
                menuMgr.onMenuSelect(false);
            }
            else {
                GridPosition::GridDrawingMode drawingMode = evt.getEntry()->getPosition().getDrawingMode();
                if(drawingMode == GridPosition::DRAW_INTEGER_AS_UP_DOWN && wasActive) {
                    if(!evt.getEntry()->getMenuItem()->isEditing()) menuMgr.onMenuSelect(false);
                    int increment = 0;
                    auto xPos = evt.getCursorPosition().x;
                    auto buttonSize = evt.getItemSize().y;
                    if(xPos > 0 && xPos < buttonSize) {
                        // down button pressed
                        increment = -1;
                    }
                    else if(xPos > (evt.getItemSize().x - buttonSize)) {
                        // up button pressed
                        increment = 1;
                    }
                    if(isMenuBasedOnValueItem(evt.getEntry()->getMenuItem())) {
                        auto* pValItem = reinterpret_cast<ValueMenuItem*>(evt.getEntry()->getMenuItem());
                        pValItem->setCurrentValue(pValItem->getCurrentValue() + increment);
                    }
                    else if(menuType == MENUTYPE_SCROLLER_VALUE) {
                        auto* pValItem = reinterpret_cast<ScrollChoiceMenuItem*>(evt.getEntry()->getMenuItem());
                        pValItem->setCurrentValue(pValItem->getCurrentValue() + increment);
                    }
                }
                else if(drawingMode == GridPosition::DRAW_INTEGER_AS_SCROLL && wasActive) {
                    if(!evt.getEntry()->getMenuItem()->isEditing()) menuMgr.onMenuSelect(false);
                    auto* analog = reinterpret_cast<AnalogMenuItem*>(evt.getEntry()->getMenuItem());
                    float correction =  float(analog->getMaximumValue()) / float(evt.getItemSize().x);
                    float percentage = evt.getCursorPosition().x * correction;
                    analog->setCurrentValue(percentage);
                }
            }
        }
    }
    else if(menuMgr.getCurrentEditor()) {
        // touched outside of the item, stop editing.
        menuMgr.stopEditingCurrentItem(false);
    }
    else {
        // deal with click completely outside of item area
    }
}
