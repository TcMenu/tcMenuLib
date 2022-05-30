/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "BaseRenderers.h"
#include "tcMenuKeyboard.h"
#include <IoLogging.h>
#include "EditableLargeNumberMenuItem.h"
#include "ScrollChoiceMenuItem.h"

/**
 * this makes the index based selections feel more natural
 */
int fromKeyToIndex(char ch) {
    if (ch == '0') return 9;
    return ch - '1';
}

void MenuEditingKeyListener::keyPressed(char key, bool held) {
    MenuItem *editor = menuMgr.getCurrentEditor();

    // holding delete always resets the menu.
    if (key == deleteKey && held) {
        clearState();
        menuMgr.resetMenu(true);
    }

    if (editor != nullptr) {
        // we are editing, attempt to manipulate the item using the keypress
        MenuType type = editor->getMenuType();
        if (type == MENUTYPE_ENUM_VALUE || type == MENUTYPE_BOOLEAN_VALUE) {
            processSimpleValueKeyPress(reinterpret_cast<ValueMenuItem *>(editor), key);
        } else if(type == MENUTYPE_SCROLLER_VALUE) {
            processScrollValueKeyPress(reinterpret_cast<ScrollChoiceMenuItem*>(editor), key);
        } else if(type == MENUTYPE_RUNTIME_LIST) {
            processListMenuSelection(reinterpret_cast<ListRuntimeMenuItem*>(editor), key);
        } else if (type == MENUTYPE_INT_VALUE) {
            processAnalogKeyPress(reinterpret_cast<AnalogMenuItem *>(editor), key);
        } else if (type == MENUTYPE_TEXT_VALUE) {
            processMultiEditKeyPress(reinterpret_cast<TextMenuItem *>(editor), key);
        } else if (type == MENUTYPE_LARGENUM_VALUE) {
            processLargeNumberPress(reinterpret_cast<EditableLargeNumberMenuItem *>(editor), key);

        } else if (isMenuRuntimeMultiEdit(editor)) {
            processIntegerMultiEdit(reinterpret_cast<EditableMultiPartMenuItem*>(editor), key);
        }
    } else if (isdigit(key)) {
        clearState();
        // we are not editing, attempt to select an item using 0-9
        menuMgr.valueChanged(fromKeyToIndex(key));
    } else if (key == backKey || key == nextKey) {
        int dir = (key == backKey)  ? -1 : 1;

        MenuItem* currentActive = menuMgr.findCurrentActive();
        uint16_t indexOfActive = offsetOfCurrentActive(currentActive) + dir;
        uint8_t numItems = itemCount(menuMgr.getCurrentMenu(), false);
        serdebugF4("act, items", indexOfActive, numItems, dir);
        if(indexOfActive > numItems) return;
        MenuItem* newActive = getItemAtPosition(menuMgr.getCurrentMenu(), indexOfActive);
        if(currentActive && newActive) {
            currentActive->setActive(false);
            newActive->setActive(true);
            serdebugF2("activate item ", newActive->getId());
        }
    } else if (key == deleteKey) {
        clearState();
        menuMgr.resetMenu(held);
    } else if (key == enterKey) {
        clearState();
        menuMgr.onMenuSelect(held);
    }

}

void MenuEditingKeyListener::keyReleased(char key) {
    // presently ignored.
}

void MenuEditingKeyListener::processDirectionalIndexItem(MenuItem *item, uint16_t currVal, char key, DirectionalItemCallback callback) {
    if (isdigit(key)) {
        int val = key - '0';
        if (uint16_t(val) > item->getMaximumValue()) val = int(item->getMaximumValue());
        callback(item, val);
        clearState();
    } else if (key == backKey) {
        uint16_t value = currVal;
        value--;
        if (value <= item->getMaximumValue()) {
            callback(item, value);
        }
    } else if (key == nextKey) {
        uint16_t value = currVal;
        value++;
        if (value <= item->getMaximumValue()) {
            callback(item, value);
        }
    } else if (key == enterKey || key == backKey) {
        clearState();
    }
}

void MenuEditingKeyListener::processScrollValueKeyPress(ScrollChoiceMenuItem *item, char key) {
    processDirectionalIndexItem(item, item->getCurrentValue(), key, [](MenuItem* itm, uint16_t newVal) {
        reinterpret_cast<ScrollChoiceMenuItem*>(itm)->setCurrentValue(int(newVal));
    });
}


void MenuEditingKeyListener::processListMenuSelection(ListRuntimeMenuItem *item, char key) {
    processDirectionalIndexItem(item, item->getActiveIndex(), key, [](MenuItem* itm, uint16_t newVal) {
        reinterpret_cast<ListRuntimeMenuItem*>(itm)->setActiveIndex(newVal);
    });
}

void MenuEditingKeyListener::processSimpleValueKeyPress(ValueMenuItem *item, char key) {
    processDirectionalIndexItem(item, item->getCurrentValue(), key, [](MenuItem* itm, uint16_t newVal) {
        reinterpret_cast<ValueMenuItem*>(itm)->setCurrentValue(newVal);
    });
}

void MenuEditingKeyListener::processIntegerMultiEdit(EditableMultiPartMenuItem *item, char key) {
    if (mode == KEYEDIT_NONE || item != currentEditor) {
        mode = KEYEDIT_MULTIEDIT_INT_START;
        currentEditor = item;
        item->valueChanged(0);
    }

    if (key == enterKey) {
        int range = item->nextPart();
        if (range == 0) {
            serdebugF("Finished with multi-edit");
            clearState();
            return;
        }
        serdebugF2("Next editable part: ", range);
        switches.changeEncoderPrecision(range, 0);
        item->valueChanged(0);
    } else if (key >= '0' && key <= '9') {
        int partVal = item->getPartValueAsInt();
        partVal = (partVal * 10) + (key - '0');
        if (partVal > item->getCurrentRange()) {
            serdebugF3("Edited multi overvalue: ", partVal, item->getCurrentRange());
            clearState();
            return;
        }

        serdebugF2("Edited multi: ", partVal);
        item->valueChanged(partVal);
    }
}

void MenuEditingKeyListener::processAnalogKeyPress(AnalogMenuItem *item, char key) {
    if (mode == KEYEDIT_NONE || item != currentEditor) {
        // we cannot edit on a keyboard items that are not factors of 10 or less than 10.
        if (item->getDivisor() > 10 && item->getDivisor() != 100 && item->getDivisor() != 1000) return;
        mode = KEYEDIT_ANALOG_EDIT_WHOLE;
        currentEditor = item;
        currentValue.whole = 0;
        currentValue.fraction = 0;
        serdebugF("Starting analog edit");
    }

    if (mode == KEYEDIT_ANALOG_EDIT_WHOLE && (key == deleteKey || key == '-')) {
            currentValue.negative = !currentValue.negative;
            serdebugF2("Negate to ", currentValue.whole);
            item->setFromWholeAndFraction(currentValue);
    } else if (key == enterKey) {
        if(mode == KEYEDIT_ANALOG_EDIT_WHOLE && item->getDivisor() > 1) {
            mode = KEYEDIT_ANALOG_EDIT_FRACT;
            serdebugF("Start fraction edit");
        }
        else {
            clearState();
            return;
        }
    } else if((key >= '0' && key <= '9')) {
        int num = (key - '0');
        // numeric handling
        if (mode == KEYEDIT_ANALOG_EDIT_WHOLE) {
            currentValue.whole = (currentValue.whole * 10) + num;
            serdebugF2("New digit ", currentValue.whole);
        } else if (mode == KEYEDIT_ANALOG_EDIT_FRACT) {
            if (item->getDivisor() <= 10) {
                currentValue.fraction = num;
                serdebugF2("New fraction digit ", currentValue.fraction);
            } else {
                unsigned int frac = (currentValue.fraction * 10) + num;
                if (frac > item->getDivisor()) {
                    // the number entered is too big, exit.
                    serdebugF2("Number too large ", frac);
                    item->setEditing(false);
                    clearState();
                    return;
                }
                currentValue.fraction = frac;
            }
        }
    }
    serdebugF3("Setting to ", currentValue.whole, currentValue.fraction);
    item->setFromWholeAndFraction(currentValue);
}

void MenuEditingKeyListener::processLargeNumberPress(EditableLargeNumberMenuItem *item, char key) {
    if (key >= '0' && key <= '9') {
        item->valueChanged(key - '0');
        if (!item->nextPart()) {
            clearState();
            item->setEditing(false);
        }
    }
    else if(key == enterKey) {
        clearState();
    }

}

void MenuEditingKeyListener::processMultiEditKeyPress(TextMenuItem *item, char key) {
    if(key == enterKey) {
        clearState();
    } else {
        item->valueChanged(findPositionInEditorSet(key));
        if (!item->nextPart()) {
            clearState();
            item->setEditing(false);
        }
    }
}

void MenuEditingKeyListener::clearState() {
    menuMgr.stopEditingCurrentItem(true);
    currentEditor = nullptr;
    mode = KEYEDIT_NONE;
}
