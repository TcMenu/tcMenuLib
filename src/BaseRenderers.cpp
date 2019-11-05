/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include "tcUtil.h"
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
	this->lastOffset = 0;
    this->firstWidget = NULL;
    this->dialog = NULL;
    MenuRenderer::theInstance = this;
}

void BaseMenuRenderer::initialise() {
	ticksToReset = MAX_TICKS;
	renderCallback = NULL;
	redrawMode = MENUDRAW_COMPLETE_REDRAW;

	resetToDefault();

	taskManager.scheduleFixedRate(SCREEN_DRAW_INTERVAL, this);
}

bool BaseMenuRenderer::tryTakeSelectIfNeeded(int currentReading, RenderPressMode pressType) {
	// always set the menu as altered.
	menuAltered();

	BaseDialog* dialog = getDialog();
	if (renderCallback != NULL || (dialog != NULL && dialog->isInUse()) ) {
		// When there's a dialog, or render function, just record the change until exec().
		renderFnPressType = pressType;
		return true;
	}

	// standard processing of the event required.
	return false;
}

void BaseMenuRenderer::exec() {

	if(dialog!=NULL && dialog->isInUse()) {
		dialog->dialogRendering(menuMgr.getCurrentRangeValue(), renderFnPressType);
    }
    else if(getRenderingCallback()) {
		renderCallback(menuMgr.getCurrentRangeValue(), renderFnPressType);
	}
	else {
		render();
	}
}

void BaseMenuRenderer::resetToDefault() {
    serdebugF2("Display reset - timeout ticks: ", resetValInTicks);
	menuMgr.setCurrentMenu(menuMgr.getRoot());
	ticksToReset = MAX_TICKS;

    // once the menu has been reset, if the reset callback is present
    // then we call it.
    if(resetCallback) resetCallback();
}

void BaseMenuRenderer::countdownToDefaulting() {
	if (ticksToReset == 0) {
		resetToDefault();
		ticksToReset = MAX_TICKS;
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
	case MENUTYPE_LARGENUM_VALUE:
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
	renderFnPressType = RPRESS_NONE;
	renderCallback = displayFn;
}

void BaseMenuRenderer::giveBackDisplay() {
	// clear off the rendering callback.
	renderFnPressType = RPRESS_NONE;
	renderCallback = NULL;
	menuMgr.setCurrentMenu(menuMgr.getRoot());
	menuAltered();
}

void BaseMenuRenderer::prepareNewSubmenu() {
	menuMgr.getParentAndReset();

	if (menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		ListRuntimeMenuItem* listMenu = reinterpret_cast<ListRuntimeMenuItem*>(menuMgr.getCurrentMenu());
		listMenu->setActiveIndex(0);
		menuMgr.setItemsInCurrentMenu(listMenu->getNumberOfParts());
	}
	else {
		menuMgr.setItemsInCurrentMenu(itemCount(menuMgr.getCurrentMenu()) - 1);
	}
	redrawRequirement(MENUDRAW_COMPLETE_REDRAW);
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
	if (item->getMenuType() == MENUTYPE_SUB_VALUE || item->getMenuType() == MENUTYPE_ACTION_VALUE 
		|| item->getMenuType() == MENUTYPE_ACTIVATE_SUBMENU || item->getMenuType() == MENUTYPE_RUNTIME_VALUE) return true;

	if (item->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		return reinterpret_cast<ListRuntimeMenuItem*>(item)->isActingAsParent();
	}
	return false;
}
