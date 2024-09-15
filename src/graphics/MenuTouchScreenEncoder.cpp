/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "MenuTouchScreenEncoder.h"
#include "ScrollChoiceMenuItem.h"
#include "../BaseDialog.h"
#include "DialogRuntimeEditor.h"

using namespace iotouch;
namespace tcgfx {

DialogMultiPartEditor dialogMultiPartEditor;

MenuTouchScreenManager::MenuTouchScreenManager(TouchInterrogator* interrogator, BaseGraphicalRenderer* renderer, const TouchOrientationSettings& rotation)
        : TouchScreenManager(interrogator, rotation),
          currentlySelected(nullptr), localStart(0,0), localSize(0,0), encoder(renderer), renderer(renderer),
          observer(&encoder), lastX(0.0F), lastY(0.0F), currentState(NOT_TOUCHED) {
    renderer->setHasTouchInterface(true);
}

void MenuTouchScreenManager::sendEvent(float locationX, float locationY, float touchPressure, TouchState touched) {
    // record the settings for non event users.
    lastX = locationX;
    lastY = locationY;
    currentState = touched;

    // if we are not in a touched state, there's nothing to do.
    if(touched == NOT_TOUCHED) {
        currentlySelected = nullptr;
        return;
    }

    lastCoord = Coord((int)(float(renderer->getWidth()) * locationX), (int)(float(renderer->getHeight()) * locationY));
    if(touched == TOUCHED) {
        currentlySelected = renderer->findMenuEntryAndDimensions(lastCoord, localStart, localSize);
        setUsedForScrolling(currentlySelected == nullptr || currentlySelected->getPosition().getDrawingMode() == GridPosition::DRAW_INTEGER_AS_SCROLL);
    }
    if(currentlySelected) {
        // find the local size and ensure it does not drop below 0 in either dimension!
        int locX = internal_max(0, (int)(lastCoord.x - localStart.x));
        int locY = internal_max(0, (int)(lastCoord.y - localStart.y));
        sendToObservers(TouchNotification(currentlySelected, Coord(locX, locY), localStart, localSize, touched));
    }
    else {
        sendToObservers(TouchNotification(lastCoord, touched));
    }
}

    void MenuTouchScreenManager::sendToObservers(TouchNotification notification) {
        observer->touched(notification);
        if(secondaryObserver) secondaryObserver->touched(notification);
    }

    bool isTouchActionable(MenuItem* pItem) {
    return isItemActionable(pItem) || pItem->getMenuType() == MENUTYPE_BACK_VALUE || pItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE;
}

void MenuTouchScreenEncoder::touched(const TouchNotification &evt) {
    if(evt.isWithinItem()) {
        MenuItem *theItem = evt.getEntry()->getMenuItem();
        serlogF4(SER_TCMENU_DEBUG, "Item Touched  (x,y,item)", evt.getCursorPosition().x, evt.getCursorPosition().y, theItem->getId());

        if(menuMgr.getCurrentEditor() && theItem != menuMgr.getCurrentEditor()) {
            // stop editing, selected outside of item
            menuMgr.stopEditingCurrentItem(false);
        }
        else {
            bool wasActive = renderer->getActiveItem() == theItem;
            if(!wasActive) {
                serlogF(SER_TCMENU_DEBUG, "Activating item");
                // if it's not active try and activate, if it fails we can't continue.
                uint8_t itemIdx = renderer->setActiveItem(theItem);
                if(switches.getEncoder()) {
                    switches.getEncoder()->setCurrentReading(itemIdx);
                }
            }

            auto menuType = theItem->getMenuType();
            auto held = evt.getTouchState() == HELD;
            bool showingTheList = menuMgr.getCurrentMenu() == theItem;
            if(menuType == MENUTYPE_RUNTIME_LIST && showingTheList) {
                auto* listItem = reinterpret_cast<ListRuntimeMenuItem*>(theItem);
                int row = evt.getEntry()->getPosition().getRow();
                if(row == 0) {
                    menuMgr.onMenuSelect(false);
                }
                else {
                    listItem->setActiveIndex(row - 1);
                    listItem->getChildItem(row - 1);
                    listItem->triggerCallback();
                    listItem->asParent();
                }
            }
            else if(isTouchActionable(theItem) && !held) {
                serlogF(SER_TCMENU_DEBUG, "Touch Act");
                menuMgr.onMenuSelect(false);
            }
            else if(isMenuRuntimeMultiEdit(theItem) && !theItem->isReadOnly()) {
                serlogF(SER_TCMENU_DEBUG, "TMPart");
                auto* dlg = renderer->getDialog();
                if(dlg && !dlg->isInUse()) {
                    auto* menuDlg = reinterpret_cast<MenuBasedDialog*>(dlg);
                    auto* multiItem = reinterpret_cast<EditableMultiPartMenuItem*>(theItem);
                    dialogMultiPartEditor.startEditing(menuDlg, multiItem);
                }
            }
            else if(!theItem->isReadOnly()){
                GridPosition::GridDrawingMode drawingMode = evt.getEntry()->getPosition().getDrawingMode();
                if(drawingMode == GridPosition::DRAW_INTEGER_AS_UP_DOWN && wasActive) {
                    serlogF(SER_TCMENU_DEBUG, "TUpDown");
                    if(menuMgr.getCurrentEditor() != theItem) menuMgr.onMenuSelect(false);
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
                    if(isMenuBasedOnValueItem(theItem)) {
                        auto* pValItem = reinterpret_cast<ValueMenuItem*>(theItem);
                        pValItem->setCurrentValue(pValItem->getCurrentValue() + increment);
                    }
                    else if(menuType == MENUTYPE_SCROLLER_VALUE) {
                        auto* pValItem = reinterpret_cast<ScrollChoiceMenuItem*>(theItem);
                        pValItem->setCurrentValue(pValItem->getCurrentValue() + increment);
                    }
                }
                else if(drawingMode == GridPosition::DRAW_INTEGER_AS_SCROLL && wasActive) {
                    serlogF(SER_TCMENU_DEBUG, "TScroll");
                    if(menuMgr.getCurrentEditor() != theItem) menuMgr.onMenuSelect(false);
                    auto* analog = reinterpret_cast<AnalogMenuItem*>(theItem);
                    float correction =  float(analog->getMaximumValue()) / float(evt.getItemSize().x);
                    float percentage = evt.getCursorPosition().x * correction;
                    analog->setCurrentValue(percentage);
                }
                else {
                    serlogF(SER_TCMENU_DEBUG, "Touch Ignored");
                }
            }
        }
    }
    else if(menuMgr.getCurrentEditor()) {
        // touched outside of the item, stop editing.
        menuMgr.stopEditingCurrentItem(false);
        serlogF(SER_TCMENU_DEBUG, "TEndEdit");
    }
    else {
        // deal with click completely outside of item area
        serlogF3(SER_TCMENU_DEBUG, "TOut (x,y)", evt.getCursorPosition().x, evt.getCursorPosition().y);
    }
}

} // namespace tcgfx
