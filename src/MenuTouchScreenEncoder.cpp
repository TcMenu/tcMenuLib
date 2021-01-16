/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "MenuTouchScreenEncoder.h"

MenuResistiveTouchScreen::MenuResistiveTouchScreen(AnalogDevice *device, BasicIoAbstraction *pins, pinid_t xpPin, pinid_t xnPin, pinid_t ypPin, pinid_t ynPin,
                                                   BaseGraphicalRenderer* renderer, BaseResistiveTouchScreen::TouchRotation rotation)
        : BaseResistiveTouchScreen(device, pins, xpPin, xnPin, ypPin, ynPin, rotation), renderer(renderer) {}

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
            observer->touched(TouchNotification(pEntry, raw, Coord(raw.x - localStart.x, raw.y - localSize.y), localSize, touched));
        }
        int nextExec = updateTicks == 0 ? 500 : (updateTicks < 4) ? 200 : (updateTicks < 15) ? 100 : 50;
        updateTicks++;
        return nextExec;
    }
}

void MenuResistiveTouchScreen::init(TouchObserver *obs) {
    observer = obs;
    start();
}

void MenuTouchScreenEncoder::touched(const TouchNotification &evt) {
    serdebugF4("Touch at (x,y,mode)", evt.getItemPosition().x, evt.getItemPosition().y, evt.isWithinItem())
    if(evt.isWithinItem()) {
        if(menuMgr.getCurrentEditor()) {
            if(evt.getEntry()->getMenuItem() != menuMgr.getCurrentEditor()) {
                menuMgr.stopEditingCurrentItem(false);
            }
            else {
                // deal with editing here
                return;
            }
        }
        else if(!evt.getEntry()->getMenuItem()->isActive()) {
            // if it's not active try and activate, if it fails we can't continue.
            if(!menuMgr.activateMenuItem(evt.getEntry()->getMenuItem())) return;

            auto menuType = evt.getEntry()->getMenuItem()->getMenuType();
            auto held = evt.getTouchState() == BaseResistiveTouchScreen::HELD;
            if(isItemActionable(evt.getEntry()->getMenuItem()) && !held) {
                menuMgr.onMenuSelect(false);
            }
            else if(evt.getEntry()->getPosition().getDrawingMode() == GridPosition::DRAW_INTEGER_AS_UP_DOWN) {

            }
            else if(evt.getEntry()->getPosition().getDrawingMode() == GridPosition::DRAW_INTEGER_AS_SCROLL) {
                int xStart = evt.getItemPosition().x;
                int xSize = evt.getItemStart().x;
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
