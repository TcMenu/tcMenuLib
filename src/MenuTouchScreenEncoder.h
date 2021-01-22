/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_MENUTOUCHSCREENENCODER_H
#define TCMENU_MENUTOUCHSCREENENCODER_H

#include "PlatformDetermination.h"
#include <IoAbstraction.h>
#include "BaseGraphicalRenderer.h"
#include <AnalogDeviceAbstraction.h>
#include <ResistiveTouchScreen.h>

/**
 * A notification event sent by a touch screen implementation that provides if the event is raw (outside of menu item)
 * or within a menu item (local to the item). If it is local the coordinates are corrected to the item, otherwise they
 * are in terms of the screen. This also indicates the type of event too.
 */
class TouchNotification {
private:
    GridPositionRowCacheEntry* pEntry;
    Coord cursorPosition;
    Coord itemSize;
    bool withinItem;
    BaseResistiveTouchScreen::TouchState touchState;
public:
    TouchNotification(const Coord& rawCoords,  BaseResistiveTouchScreen::TouchState touchState)
            : pEntry(nullptr), cursorPosition(rawCoords), itemSize(0, 0), withinItem(false), touchState(touchState) {}

    TouchNotification(GridPositionRowCacheEntry* ent, const Coord& local, const Coord& localStart, const Coord& localSize, BaseResistiveTouchScreen::TouchState touchState)
            : pEntry(ent), cursorPosition(local), itemSize(localSize), withinItem(true), touchState(touchState) {}

    GridPositionRowCacheEntry* getEntry() const {
        return pEntry;
    }

    const Coord &getCursorPosition() const {
        return cursorPosition;
    }

    const Coord &getItemSize() const {
        return itemSize;
    }

    bool isWithinItem() const {
        return withinItem;
    }

    BaseResistiveTouchScreen::TouchState getTouchState() const {
        return touchState;
    }
};

class TouchObserver {
public:
    virtual void touched(const TouchNotification& notification)=0;
};

class MenuTouchScreenEncoder : public TouchObserver {
private:
    BaseGraphicalRenderer *renderer;
public:
    MenuTouchScreenEncoder(BaseGraphicalRenderer* rend) {
        renderer = rend;
    }

    void touched(const TouchNotification& notification) override;
};

class MenuResistiveTouchScreen : public BaseResistiveTouchScreen {
private:
    MenuTouchScreenEncoder encoder;
    BaseGraphicalRenderer* renderer;
    TouchObserver* observer;
    int updateTicks;
public:
    MenuResistiveTouchScreen(AnalogDevice *device, BasicIoAbstraction *pins, pinid_t xpPin, pinid_t xnPin,
                             pinid_t ypPin, pinid_t ynPin, BaseGraphicalRenderer* renderer, TouchRotation rotation);

    uint32_t sendEvent(float locationX, float locationY, float touchPressure, TouchState touched) override;
};

#endif //TCMENU_MENUTOUCHSCREENENCODER_H
