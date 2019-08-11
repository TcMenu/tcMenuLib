/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include "tcUtil.h"
#include "MenuIterator.h"
#include "RemoteMenuItem.h"
#include "BaseRenderers.h"
#include "BaseDialog.h"

MenuRenderer* MenuRenderer::theInstance = NULL;

BaseMenuRenderer::BaseMenuRenderer(int bufferSize) : MenuRenderer(RENDERER_TYPE_BASE, bufferSize) {
	ticksToReset = 0;
    lastOffset = 0;
    resetValInTicks = 30 * SECONDS_IN_TICKS;
	renderCallback = NULL;
    resetCallback = NULL;
	redrawMode = MENUDRAW_COMPLETE_REDRAW;
	this->currentEditor = NULL;
	this->currentRoot = NULL;
	this->lastOffset = 0;
    this->firstWidget = NULL;
    this->dialog = NULL;
    MenuRenderer::theInstance = this;
}

void BaseMenuRenderer::initialise() {
	ticksToReset = MAX_TICKS;
	renderCallback = NULL;
    currentRoot = menuMgr.getRoot();
	redrawMode = MENUDRAW_COMPLETE_REDRAW;

	resetToDefault();

	taskManager.scheduleFixedRate(SCREEN_DRAW_INTERVAL, this);
}

void BaseMenuRenderer::exec() {
	if(dialog!=NULL && dialog->isInUse()) {
        RotaryEncoder* encoder = switches.getEncoder();
		dialog->dialogRendering((encoder != NULL) ? encoder->getCurrentReading() : 0, false);
    }
    else if(getRenderingCallback()) {
        RotaryEncoder* encoder = switches.getEncoder();
		renderCallback((encoder != NULL) ? encoder->getCurrentReading() : 0, false);
	}
	else {
		render();
	}
}

void BaseMenuRenderer::activeIndexChanged(uint8_t index) {
    // we only change the active index / edit state if we own the display
	if (!getRenderingCallback()) {
		if (currentRoot->getMenuType() == MENUTYPE_RUNTIME_LIST) {
			reinterpret_cast<ListRuntimeMenuItem*>(currentRoot)->setActiveIndex(index);
		}
		else {
			MenuItem* currentActive = menuMgr.findCurrentActive();
			currentActive->setActive(false);
			currentActive = getItemAtPosition(index);
			currentActive->setActive(true);
			menuAltered();
		}
	}
}

void BaseMenuRenderer::resetToDefault() {
    serdebugF2("Display reset - timeout ticks: ", resetValInTicks);
	currentEditor = NULL;
    getParentAndReset();
	prepareNewSubmenu(menuMgr.getRoot());
	ticksToReset = MAX_TICKS;

    // once the menu has been reset, if the reset callback is present
    // then we call it.
    if(resetCallback) resetCallback();
}

void BaseMenuRenderer::countdownToDefaulting() {
	if (ticksToReset == 0) {
		resetToDefault();
		ticksToReset = resetValInTicks;
	}
	else if (ticksToReset != MAX_TICKS) {
		--ticksToReset;
	}
}

void BaseMenuRenderer::menuValueToText(MenuItem* item,	MenuDrawJustification justification) {
	switch (item->getMenuType()) {
	case MENUTYPE_INT_VALUE:
		menuValueAnalog((AnalogMenuItem*)item, justification);
		break;
	case MENUTYPE_ENUM_VALUE:
		menuValueEnum((EnumMenuItem*)item, justification);
		break;
	case MENUTYPE_BOOLEAN_VALUE:
		menuValueBool((BooleanMenuItem*)item, justification);
		break;
	case MENUTYPE_SUB_VALUE:
	case MENUTYPE_ACTION_VALUE:
		if (justification == JUSTIFY_TEXT_LEFT) buffer[0] = 0;
		break;
	case MENUTYPE_BACK_VALUE:
	case MENUTYPE_TEXT_VALUE:
	case MENUTYPE_IPADDRESS:
    case MENUTYPE_TIME:
	case MENUTYPE_RUNTIME_LIST:
	case MENUTYPE_RUNTIME_VALUE:
		menuValueRuntime(reinterpret_cast<RuntimeMenuItem*>(item), justification);
		break;
	case MENUTYPE_FLOAT_VALUE:
		menuValueFloat((FloatMenuItem*)item, justification);
		break;
	default:
		strcpy(buffer, "???");
		break;
	}

}

void BaseMenuRenderer::menuValueAnalog(AnalogMenuItem* item, MenuDrawJustification justification) {
	char itoaBuf[12];
	item->copyValue(itoaBuf, sizeof(itoaBuf));

	if(justification == JUSTIFY_TEXT_LEFT) {
		strcpy(buffer, itoaBuf);
	}
	else {
		uint8_t numLen = strlen(itoaBuf);
		uint8_t startPlace = bufferSize - numLen;
		strcpy(buffer + startPlace, itoaBuf);
	}
}

void BaseMenuRenderer::menuValueEnum(EnumMenuItem* item, MenuDrawJustification justification) {
	if(justification == JUSTIFY_TEXT_LEFT) {
		item->copyEnumStrToBuffer(buffer, bufferSize,item->getCurrentValue());
	}
	else {
		uint8_t count = item->getLengthOfEnumStr(item->getCurrentValue());
        if(count > bufferSize) count = bufferSize;
		item->copyEnumStrToBuffer(buffer + (bufferSize - count), count + 1, item->getCurrentValue());
	}
}

const char ON_STR[] PGM_TCM   = "ON";
const char OFF_STR[] PGM_TCM  = "OFF";
const char YES_STR[] PGM_TCM  = "YES";
const char NO_STR[] PGM_TCM   = " NO";
const char TRUE_STR[] PGM_TCM = " TRUE";
const char FALSE_STR[] PGM_TCM= "FALSE";

void BaseMenuRenderer::menuValueBool(BooleanMenuItem* item, MenuDrawJustification justification) {
	BooleanNaming naming = item->getBooleanNaming();
	const char* val;
	switch(naming) {
	case NAMING_ON_OFF:
		val = item->getBoolean() ? ON_STR : OFF_STR;
		break;
	case NAMING_YES_NO:
		val = item->getBoolean() ? YES_STR : NO_STR;
		break;
	default:
		val = item->getBoolean() ? TRUE_STR : FALSE_STR;
		break;
	}

	if(justification == JUSTIFY_TEXT_LEFT) {
		safeProgCpy(buffer, val, bufferSize);
	}
	else {
		uint8_t len = safeProgStrLen(val);
        if(len > bufferSize) len = bufferSize;
		safeProgCpy(buffer + (bufferSize - len), val, bufferSize);
	}
}

void BaseMenuRenderer::menuValueFloat(FloatMenuItem* item, MenuDrawJustification justification) {
	char sz[20];
	sz[0]=0;
	ltoa((long)item->getFloatValue(), sz, 10);
	appendChar(sz, '.', sizeof sz);
	
	long dpDivisor = dpToDivisor(item->getDecimalPlaces());
	long whole = item->getFloatValue();
	long fract = abs((item->getFloatValue() - whole) * dpDivisor);
	fastltoa_mv(sz, fract, dpDivisor, '0', sizeof sz);

	if(justification == JUSTIFY_TEXT_LEFT) {
		strcpy(buffer, sz);
	}
	else {
		uint8_t count = strlen(sz);
		int cpy = bufferSize - count;
		strcpy(buffer + cpy, sz);
	}
}

void BaseMenuRenderer::menuValueRuntime(RuntimeMenuItem* item, MenuDrawJustification justification) {
	if(justification == JUSTIFY_TEXT_LEFT) {
		item->copyValue(buffer, bufferSize);
	}
	else {
		char sz[20];
		item->copyValue(sz, sizeof(sz));
		uint8_t count = strlen(sz);
		int cpy = bufferSize - count;
		strcpy(buffer + cpy, sz);
	}
}

void BaseMenuRenderer::takeOverDisplay(RendererCallbackFn displayFn) {
	// when we set this, we are stopping tcMenu rendering and letting this take over
	renderCallback = displayFn;
}

void BaseMenuRenderer::giveBackDisplay() {
	// clear off the rendering callback.
	renderCallback = NULL;
	prepareNewSubmenu(currentRoot);
}

MenuItem* BaseMenuRenderer::getParentAndReset() {
    return getParentRootAndVisit(currentRoot, [](MenuItem* curr) {
		curr->setActive(false);
		curr->setEditing(false);
    });
}

void BaseMenuRenderer::prepareNewSubmenu(MenuItem* newItems) {
	menuAltered();
	currentRoot = newItems;
	currentRoot->setActive(true);

	if (newItems->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		ListRuntimeMenuItem* listMenu = reinterpret_cast<ListRuntimeMenuItem*>(newItems);
		listMenu->setActiveIndex(0);
		menuMgr.setItemsInCurrentMenu(listMenu->getNumberOfParts());
	}
	else {
		menuMgr.setItemsInCurrentMenu(itemCount(newItems) - 1);
	}
	redrawRequirement(MENUDRAW_COMPLETE_REDRAW);
}

MenuItem* BaseMenuRenderer::getItemAtPosition(uint8_t pos) {
	uint8_t i = 0;
	MenuItem* itm = currentRoot;

	while (itm != NULL) {
		if (i == pos) {
			return itm;
		}
		i++;
		itm = itm->getNext();
	}

	return currentRoot;
}

int BaseMenuRenderer::offsetOfCurrentActive() {
	uint8_t i = 0;
	MenuItem* itm = currentRoot;
	while (itm != NULL) {
		if (itm->isActive() || itm->isEditing()) {
			return i;
		}
		i++;
		itm = itm->getNext();
	}

	return 0;
}

void BaseMenuRenderer::onHold() {
	if (currentEditor != NULL && isMenuRuntimeMultiEdit(currentEditor)) {
		prepareNewSubmenu(currentRoot);
	}
	else {
		prepareNewSubmenu(getParentAndReset());
	}
}

void BaseMenuRenderer::onSelectPressed(MenuItem* toEdit) {
    if(dialog != NULL && dialog->isInUse()) {
		// we dont handle click events when a dialog is being drawn.
        // instead we give events to it, as it has the display
        RotaryEncoder* encoder = switches.getEncoder();
		dialog->dialogRendering((encoder != NULL) ? encoder->getCurrentReading() : 0, true);
        return;
    }
	
    if(renderCallback) {
		// we dont handle click events when the display is taken over
		// instead we tell the custom renderer that we've had a click
        RotaryEncoder* encoder = switches.getEncoder();
		renderCallback((encoder != NULL) ? encoder->getCurrentReading() : 0, true);
		return;
	}

    // if we are already editing an item then we need to stop editing
    // that item once it has been selected with a click again.
	if (currentEditor != NULL) {
		if (isMenuRuntimeMultiEdit(currentEditor)) {
			EditableMultiPartMenuItem<void*>* editableItem = reinterpret_cast<EditableMultiPartMenuItem<void*>*>(currentEditor);

			// unless we've run out of parts to edit, stay in edit mode, moving to next part.
			int editorRange = editableItem->nextPart();
			if (editorRange != 0) {
				switches.changeEncoderPrecision(editorRange, editableItem->getPartValueAsInt());
				return;
			}
		}

		currentEditor->setEditing(false);
		currentEditor->setActive(true);
		serdebugF2("onSel curr!=null", currentEditor->getId());
		currentEditor = NULL;
		menuMgr.setItemsInCurrentMenu(itemCount(currentRoot) - 1, offsetOfCurrentActive());
		redrawRequirement(MENUDRAW_EDITOR_CHANGE);
	}

    // if there's a new item specified in toEdit, it means we need to change
    // the current editor (if it's possible to edit that value)
	if(toEdit != NULL) {
		if (toEdit->getMenuType() == MENUTYPE_SUB_VALUE) {
			SubMenuItem* sub = reinterpret_cast<SubMenuItem*>(toEdit);
			sub->setActive(false);
           	getParentAndReset();
			prepareNewSubmenu(sub->getChild());
		}
		if (toEdit->getMenuType() == MENUTYPE_RUNTIME_LIST) {
			if (currentRoot == toEdit) {
				ListRuntimeMenuItem* listItem = reinterpret_cast<ListRuntimeMenuItem*>(toEdit);
				serdebugF2("List press: ", listItem->getActiveIndex());
				if (listItem->getActiveIndex() == 0) {
					prepareNewSubmenu(getParentAndReset());
				}
				else {
					listItem->getChildItem(listItem->getActiveIndex() - 1)->triggerCallback();
					// reset to parent after doing the callback
					listItem->asParent();
				}
			}
			else prepareNewSubmenu(toEdit);
		}
		else if (toEdit->getMenuType() == MENUTYPE_BACK_VALUE) {
			toEdit->setActive(false);
			prepareNewSubmenu(getParentAndReset());
		}
		else if(toEdit->getMenuType() == MENUTYPE_ACTION_VALUE) {
			toEdit->triggerCallback();
		}
		else {
			setupForEditing(toEdit);
			redrawRequirement(MENUDRAW_EDITOR_CHANGE);
		}
	}
	menuAltered();
}

void BaseMenuRenderer::setupForEditing(MenuItem* item) {
	// if the item is NULL, or it's read only, then it can't be edited.
	if (item == NULL || item->isReadOnly()) return;

	MenuType ty = item->getMenuType();
	if ((ty == MENUTYPE_ENUM_VALUE || ty == MENUTYPE_INT_VALUE)) {
		// these are the only types we can edit with a rotary encoder & LCD.
		currentEditor = item;
		currentEditor->setEditing(true);
		switches.changeEncoderPrecision(item->getMaximumValue(), reinterpret_cast<ValueMenuItem*>(currentEditor)->getCurrentValue());
	}
	else if (ty == MENUTYPE_BOOLEAN_VALUE) {
		// we don't actually edit boolean items, just toggle them instead
		BooleanMenuItem* boolItem = (BooleanMenuItem*)item;
		boolItem->setBoolean(!boolItem->getBoolean());
	}
	else if (isMenuRuntimeMultiEdit(item)) {
		currentEditor = item;
		EditableMultiPartMenuItem<void*>* editableItem = reinterpret_cast<EditableMultiPartMenuItem<void*>*>(item);
		editableItem->beginMultiEdit();
		int range = editableItem->nextPart();
		switches.changeEncoderPrecision(range, editableItem->getPartValueAsInt());
	}
}

void BaseMenuRenderer::setFirstWidget(TitleWidget* widget) {
	this->firstWidget = widget;
	this->redrawMode = MENUDRAW_COMPLETE_REDRAW;
}

TitleWidget::TitleWidget(const uint8_t * const* icons, uint8_t maxStateIcons, uint8_t width, uint8_t height, TitleWidget* next) {
	this->iconData = icons;
	this->maxStateIcons = maxStateIcons;
	this->width = width;
	this->height = height;
	this->currentState = 0;
	this->next = next;
	this->changed = true;
}

class NoRenderDialog : public BaseDialog {
public:
    NoRenderDialog() { 
        bitWrite(flags, DLG_FLAG_SMALLDISPLAY, false);
    }
protected:
    void internalRender(int currentValue) override { /* does nothing */ }
};

BaseDialog* NoRenderer::getDialog() {
    if(dialog == NULL) {
        dialog = new NoRenderDialog();
    }
    return dialog;
}

bool isItemActionable(MenuItem* item) {
	if (item->getMenuType() == MENUTYPE_SUB_VALUE || item->getMenuType() == MENUTYPE_ACTION_VALUE) return true;
	if (item->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		return reinterpret_cast<ListRuntimeMenuItem*>(item)->isActingAsParent();
	}
	return false;
}
