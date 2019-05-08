/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "tcUtil.h"
#include "MenuIterator.h"
#include "RemoteMenuItem.h"
#include "BaseRenderers.h"

BaseMenuRenderer::BaseMenuRenderer(int bufferSize) {
	buffer = new char[bufferSize + 1]; // add one to allow for the trailing 0.
	this->bufferSize = bufferSize;
	ticksToReset = 0;
	renderCallback = NULL;
	redrawMode = MENUDRAW_COMPLETE_REDRAW;
	this->currentEditor = NULL;
	this->currentRoot = menuMgr.getRoot();
	this->lastOffset = 0;
    this->firstWidget = NULL;
}

void BaseMenuRenderer::initialise() {
	ticksToReset = 0;
	renderCallback = NULL;
	redrawMode = MENUDRAW_COMPLETE_REDRAW;

	resetToDefault();

	taskManager.scheduleFixedRate(SCREEN_DRAW_INTERVAL, this);
}

void BaseMenuRenderer::exec() {
	if(getRenderingCallback()) {
        RotaryEncoder* encoder = switches.getEncoder();
		renderCallback((encoder != NULL) ? encoder->getCurrentReading() : 0, false);
	}
	else {
		render();
	}
}

void BaseMenuRenderer::activeIndexChanged(uint8_t index) {
    // we only change the active index / edit state if we own the display
    if(!getRenderingCallback()) {
        MenuItem* currentActive = menuMgr.findCurrentActive();
        currentActive->setActive(false);
        currentActive = getItemAtPosition(index);
        currentActive->setActive(true);
        menuAltered();
    }
}

void BaseMenuRenderer::resetToDefault() {
	currentEditor = NULL;
	prepareNewSubmenu(menuMgr.getRoot());
	ticksToReset = 255;
}

void BaseMenuRenderer::countdownToDefaulting() {
	if (ticksToReset == 0) {
		resetToDefault();
		ticksToReset = 255;
	}
	else if (ticksToReset != 255) {
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
		menuValueExec((SubMenuItem*)item, justification);
		break;
	case MENUTYPE_BACK_VALUE:
		menuValueBack((BackMenuItem*)item, justification);
		break;
	case MENUTYPE_TEXT_VALUE:
		menuValueText((TextMenuItem*)item, justification);
		break;
	case MENUTYPE_REMOTE_VALUE:
		menuValueRemote((RemoteMenuItem*)item, justification);
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

	int16_t calcVal = ((int16_t)item->getCurrentValue()) + ((int16_t)item->getOffset());
	int divisor = item->getDivisor();

	if (divisor < 2) {
		// in this case divisor was 0 or 1, this means treat as integer.
		itoa(calcVal, itoaBuf, 10);
	}
	else if (divisor > 10) {
		// so we can display as decimal, work out the nearest highest unit for 2dp, 3dp and 4dp.
		int fractMax = (divisor > 1000) ? divisor = 10000 : (divisor > 100) ? 1000 : 100;

		// when divisor is greater than 10 we need to deal with both parts using itoa
		int whole = calcVal / divisor;
		uint16_t fraction = abs((calcVal % divisor)) * (fractMax / divisor);

		itoa(whole, itoaBuf, 10);
		appendChar(itoaBuf, '.', sizeof itoaBuf);
		fastltoa_mv(itoaBuf, fraction, fractMax, '0', sizeof itoaBuf);
	}
	else {
		// an efficient optimisation for fractions < 10.
		int whole = calcVal / divisor;
		uint8_t fraction = abs((calcVal % divisor)) * (10 / divisor);

		itoa(whole, itoaBuf, 10);
		uint8_t decPart = strlen(itoaBuf);
		itoaBuf[decPart] = '.';
		itoaBuf[decPart + 1] = fraction + '0';
		itoaBuf[decPart + 2] = 0;
	}
	uint8_t numLen = strlen(itoaBuf);

	if(justification == JUSTIFY_TEXT_LEFT) {
		strcpy(buffer, itoaBuf);
		item->copyUnitToBuffer(buffer + numLen);
	}
	else {
		uint8_t unitLen = item->unitNameLength();
		uint8_t startPlace = bufferSize - (numLen + unitLen);
		strcpy(buffer + startPlace, itoaBuf);
		item->copyUnitToBuffer(buffer + (bufferSize - unitLen));
	}
}

void BaseMenuRenderer::menuValueEnum(EnumMenuItem* item, MenuDrawJustification justification) {
	if(justification == JUSTIFY_TEXT_LEFT) {
		item->copyEnumStrToBuffer(buffer, bufferSize,item->getCurrentValue());
	}
	else {
		uint8_t count = item->getLengthOfEnumStr(item->getCurrentValue());
        if(count > bufferSize) count = bufferSize;
		item->copyEnumStrToBuffer(buffer + (bufferSize - count), count, item->getCurrentValue());
	}
}

const char ON_STR[] PGM_TCM   = "ON";
const char OFF_STR[] PGM_TCM  = "OFF";
const char YES_STR[] PGM_TCM  = "YES";
const char NO_STR[] PGM_TCM   = " NO";
const char TRUE_STR[] PGM_TCM = " TRUE";
const char FALSE_STR[] PGM_TCM= "FALSE";
const char SUB_STR[] PGM_TCM  = "->>>";
const char BACK_MENU_NAME[] PGM_TCM  = "[Back]";

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

void BaseMenuRenderer::menuValueExec(__attribute((unused)) MenuItem* item, MenuDrawJustification justification) {
	if(justification == JUSTIFY_TEXT_LEFT) {
		safeProgCpy(buffer, SUB_STR, bufferSize);
	}
	else {
		safeProgCpy(buffer + (bufferSize - 4), SUB_STR, bufferSize);
	}
}

void BaseMenuRenderer::menuValueBack(__attribute((unused)) BackMenuItem* item, MenuDrawJustification justification) {
	if(justification == JUSTIFY_TEXT_LEFT) {
		safeProgCpy(buffer, BACK_MENU_NAME, bufferSize);
	}
	else {
		safeProgCpy(buffer + (bufferSize - 6), BACK_MENU_NAME, bufferSize);
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

void BaseMenuRenderer::menuValueText(TextMenuItem* item, MenuDrawJustification justification) {
	if(justification == JUSTIFY_TEXT_LEFT) {
		strcpy(buffer, item->getTextValue());
	}
	else {
		uint8_t count = strlen(item->getTextValue());
		int cpy = bufferSize - count;
		strcpy(buffer + cpy, item->getTextValue());
	}
}

void BaseMenuRenderer::menuValueRemote(RemoteMenuItem* item, MenuDrawJustification justification) {	
	item->getCurrentState((justification == JUSTIFY_TEXT_LEFT) ? buffer : &buffer[1], bufferSize - 1);
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
    MenuItem* par = getParentRootAndVisit(currentRoot, [](MenuItem* curr) {
		curr->setActive(false);
		curr->setEditing(false);
    });

    if(par == NULL) par = menuMgr.getRoot();
    return par;
}

void BaseMenuRenderer::prepareNewSubmenu(MenuItem* newItems) {
	menuAltered();
	currentRoot = newItems;
	currentRoot->setActive(true);

	menuMgr.setItemsInCurrentMenu(itemCount(newItems) - 1);
	redrawRequirement(MENUDRAW_COMPLETE_REDRAW);
}

void BaseMenuRenderer::setupForEditing(MenuItem* item) {
	if(currentEditor != NULL) {
		currentEditor->setEditing(false);
		currentEditor->setActive(false);
	}

	// basically clear down editor state
	if(item == NULL) {
		return;
	}

	MenuType ty = item->getMenuType();
	
	// short circuit read only, cannot be edited.
	if(item->isReadOnly()) return;

	if ((ty == MENUTYPE_ENUM_VALUE || ty == MENUTYPE_INT_VALUE)) {
		// these are the only types we can edit with a rotary encoder & LCD.
		currentEditor = item;
		currentEditor->setEditing(true);
		menuMgr.changePrecisionForType(currentEditor);
	}
	else if(ty == MENUTYPE_BOOLEAN_VALUE) {
		// we don't actually edit boolean items, just toggle them instead
		BooleanMenuItem* boolItem = (BooleanMenuItem*)item;
		boolItem->setBoolean(!boolItem->getBoolean());
	}
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
	prepareNewSubmenu(getParentAndReset());
}

void BaseMenuRenderer::onSelectPressed(MenuItem* toEdit) {
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
		currentEditor->setEditing(false);
		currentEditor->setActive(true);
		currentEditor = NULL;
		menuMgr.setItemsInCurrentMenu(itemCount(currentRoot) - 1, offsetOfCurrentActive());
		redrawRequirement(MENUDRAW_EDITOR_CHANGE);
	}

    // if there's a new item specified in toEdit, it means we need to change
    // the current editor (if it's possible to edit that value)
	if(toEdit != NULL) {
		if (toEdit->getMenuType() == MENUTYPE_SUB_VALUE) {
			toEdit->setActive(false);
           	getParentAndReset();
			prepareNewSubmenu(((SubMenuItem*)toEdit)->getChild());
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
