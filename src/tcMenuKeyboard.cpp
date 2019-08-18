/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "BaseRenderers.h"
#include "tcMenuKeyboard.h"
#include <IoLogging.h>

/**
 * this makes the index based selections feel more natural
 */
int fromKeyToIndex(char ch) {
	if (ch == '0') return 9;
	return ch - '1';
}

void MenuEditingKeyListener::keyPressed(char key, bool held) {
    // we must have a locally available base renderer.
    if(menuMgr.getRenderer()->getRendererType() != RENDERER_TYPE_BASE) return;
    BaseMenuRenderer* renderer = reinterpret_cast<BaseMenuRenderer*>(menuMgr.getRenderer());

	// no matter what this always resets the state.
	if (key == '#' && held) {
		clearState();
		renderer->resetToDefault();
	}

	MenuItem* editor = renderer->getCurrentEditor();
	if (editor != NULL) {
		// we are editing, attempt to manipulate the item using the keypress
		if (editor->getMenuType() == MENUTYPE_ENUM_VALUE || editor->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
			processSimpleValueKeyPress(reinterpret_cast<ValueMenuItem*>(editor), key);
		}
		else if (editor->getMenuType() == MENUTYPE_INT_VALUE) {
			processAnalogKeyPress(reinterpret_cast<AnalogMenuItem*>(editor), key);
		}
		else if (editor->getMenuType() == MENUTYPE_TEXT_VALUE) {
			processMultiEditKeyPress(reinterpret_cast<TextMenuItem*>(editor), key);
		}
		else if (isMenuRuntimeMultiEdit(editor)) {
			processIntegerMultiEdit(reinterpret_cast<EditableMultiPartMenuItem<byte[4]>*>(editor), key);
		}
	}
	else if(isdigit(key)) {
		clearState();
		// we are not editing, attempt to select an item using 0-9
		menuMgr.valueChanged(fromKeyToIndex(key));
	}
	else if (key == '*') {
		clearState();
		menuMgr.onMenuSelect(held);
	}
}

void MenuEditingKeyListener::keyReleased(char key) {
	// presently ignored.
}

void MenuEditingKeyListener::processSimpleValueKeyPress(ValueMenuItem* item, char key) {
	clearState();
	if (isdigit(key)) {
		int val = key - '0';
		if (val > item->getMaximumValue()) val = item->getMaximumValue();
		item->setCurrentValue(val);
	}
}

void MenuEditingKeyListener::processIntegerMultiEdit(EditableMultiPartMenuItem<byte[4]>* item, char key) {
	if (mode == KEYEDIT_NONE || item != currentEditor) {
		mode = KEYEDIT_MULTIEDIT_INT_START;
		currentEditor = item;
		item->valueChanged(0);
	}

	if (key == '*') {
		int range = item->nextPart();
		if (range == 0) {
			serdebugF("Finished with multiedit");
			clearState();
			return;
		}
		serdebugF2("Next editable part: ", range);
		switches.changeEncoderPrecision(range, 0);
		item->valueChanged(0);
	}
	else if (key >= '0' && key <= '9') {
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

void MenuEditingKeyListener::processAnalogKeyPress(AnalogMenuItem* item, char key) {
	if (mode == KEYEDIT_NONE || item != currentEditor) {
		// we cannot edit on a keyboard items that are not factors of 10 or less than 10.
		if (item->getDivisor() > 10 && item->getDivisor() != 100 && item->getDivisor() != 1000) return;
		mode = KEYEDIT_ANALOG_EDIT_WHOLE;
		currentEditor = item;
		currentValue.whole = 0;
		currentValue.fraction = 0;
		serdebugF("Starting analog edit");
	}

	// special handling.
	if (key < '0' || key > '9') {
		if (mode == KEYEDIT_ANALOG_EDIT_WHOLE && (key == '#' || key  == '-')) {
			currentValue.whole = currentValue.whole * -1;
			serdebugF2("Negate to ", currentValue.whole);
			item->setFromWholeAndFraction(currentValue);
		}
		else if (mode == KEYEDIT_ANALOG_EDIT_WHOLE && item->getDivisor() > 1) {
			mode = KEYEDIT_ANALOG_EDIT_FRACT;
			serdebugF("Start fraction edit");
		}
		else {
			item->setEditing(false);
			clearState();
		}

		// we must not enter the next block, as it's not a digit
		return;
	}

	// numeric handling
	int num = (key - '0');
	if (mode == KEYEDIT_ANALOG_EDIT_WHOLE) {
		currentValue.whole = (currentValue.whole * 10) + num;
		serdebugF2("New digit ", currentValue.whole);
	}
	else if (mode == KEYEDIT_ANALOG_EDIT_FRACT) {
		if (item->getDivisor() <= 10) {
			currentValue.fraction = num;
			serdebugF2("New fraction digit ", currentValue.fraction);
		}
		else {
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
	serdebugF3("Setting to ", currentValue.whole, currentValue.fraction);
	item->setFromWholeAndFraction(currentValue);
}

void MenuEditingKeyListener::processMultiEditKeyPress(TextMenuItem* item, char key) {
	item->valueChanged(findPositionInEditorSet(key));
	if (!item->nextPart()) {
		clearState();
		item->setEditing(false);
	}
}

void MenuEditingKeyListener::clearState() {
	menuMgr.getRenderer()->onSelectPressed(NULL);
	currentEditor = NULL;
	mode = KEYEDIT_NONE;
}
