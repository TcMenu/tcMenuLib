/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include "tcUtil.h"
#include "RemoteMenuItem.h"
#include "BaseRenderers.h"
#include "BaseDialog.h"
#include "graphics/BaseGraphicalRenderer.h"
#include <IoLogging.h>

MenuRenderer* MenuRenderer::theInstance = nullptr;

class RenderingMenuMgrObserver : public MenuManagerObserver {
public:
    void structureHasChanged() override {
        auto myRenderer = MenuRenderer::getInstance();
        if(myRenderer->getRendererType() == RENDER_TYPE_CONFIGURABLE) {
            auto gfxRenderer = reinterpret_cast<tcgfx::BaseGraphicalRenderer*>(myRenderer);
            gfxRenderer->displayPropertiesHaveChanged();
        } else if(myRenderer->getRendererType() != RENDER_TYPE_NOLOCAL) {
            serlogF(SER_TCMENU_DEBUG, "Completely invalidate the display");
            reinterpret_cast<BaseMenuRenderer*>(myRenderer)->invalidateAll();
        }
    }

    bool menuEditStarting(MenuItem *item) override {
        return true;
    }

    void menuEditEnded(MenuItem *item) override {}
} menuMgrListener;


BaseMenuRenderer::BaseMenuRenderer(int bufferSize, RendererType rType, uint8_t displayNum) : MenuRenderer(rType, bufferSize) {
	ticksToReset = 0;
    lastOffset = 0;
    displayNumber = displayNum;
    resetValInTicks = 30 * TC_DISPLAY_UPDATES_PER_SECOND;
    renderCallback = nullptr;
    renderFnPressType = RPRESS_NONE;
	redrawMode = MENUDRAW_COMPLETE_REDRAW;
	this->lastOffset = 0;
    this->firstWidget = nullptr;
    this->dialog = nullptr;
    displayTakenMode = NOT_TAKEN_OVER;
    updatesPerSecond = TC_DISPLAY_UPDATES_PER_SECOND;
    // if this is the default display, it becomes the static (IE global) instance for dialogs etc.
    if(displayNum == 0) {
        MenuRenderer::theInstance = this;
    }
}

void BaseMenuRenderer::initialise() {
	ticksToReset = resetValInTicks;
	redrawMode = MENUDRAW_COMPLETE_REDRAW;

    menuMgr.changeMenu();
    if(updatesPerSecond == 0) updatesPerSecond = TC_DISPLAY_UPDATES_PER_SECOND;

    taskManager.scheduleOnce(250, this);
	menuMgr.addChangeNotification(&menuMgrListener);
}

bool BaseMenuRenderer::tryTakeSelectIfNeeded(int currentReading, RenderPressMode pressType) {
	// always set the menu as altered.
	menuAltered();

	if (displayTakenMode != NOT_TAKEN_OVER || (dialog != nullptr && dialog->isRenderNeeded()) ) {
		// When there's a dialog, or render function, just record the change until exec().
		renderFnPressType = pressType;
		return true;
	}

	// standard processing of the event required.
	return false;
}

void BaseMenuRenderer::exec() {
    // If a dialog is active, they take priority, either menu based or rendering based dialog, these are highest.
    // Then we check if the display is taken over, if it is, that takes priority (either functional or OO takeover,
    // finally we render the menu if none of the above are true.
	if(dialog!=nullptr && dialog->isRenderNeeded()) {
		dialog->dialogRendering(menuMgr.getCurrentRangeValue(), renderFnPressType);
    } else if(dialog!=nullptr && dialog->isInUse()) {
        displayTakenMode = NOT_TAKEN_OVER;
        render();
    }else if(displayTakenMode == NOT_TAKEN_OVER) {
        render();
    } else if(displayTakenMode == START_CUSTOM_DRAW) {
        customDrawing->started(this);
        displayTakenMode = RUNNING_CUSTOM_DRAW;
    } else if(displayTakenMode == RUNNING_CUSTOM_DRAW) {
        customDrawing->renderLoop(menuMgr.getCurrentRangeValue(), renderFnPressType);
    } else if(displayTakenMode == TAKEN_OVER_FN) {
	    renderCallback(menuMgr.getCurrentRangeValue(), renderFnPressType);
	}

    // here we work out when we should be called again, this allows the number of updates per second to be changed
    // during the run of the application.
    if(updatesPerSecond != UPDATES_SEC_DISPLAY_OFF) {
        int refreshInterval = 1000 / updatesPerSecond;
        taskManager.scheduleOnce(refreshInterval, this);
    }
}

void BaseMenuRenderer::resetToDefault() {
    serlogF2(SER_TCMENU_INFO, "Display reset - timeout ticks: ", resetValInTicks);
	menuMgr.resetMenu(true);
	ticksToReset = MAX_TICKS;

    // once the menu has been reset, if the reset callback is present
    // then we call it.
    if(customDrawing) {
        customDrawing->reset();
    }
}

void BaseMenuRenderer::countdownToDefaulting() {
    if(dialog != nullptr && dialog->isInUse()) {
        ticksToReset = resetValInTicks;
        return;
    }
	if (ticksToReset == 0) {
		resetToDefault();
		ticksToReset = MAX_TICKS;
	}
	else if (ticksToReset != MAX_TICKS) {
		--ticksToReset;
	}
}

void BaseMenuRenderer::takeOverDisplay(RendererCallbackFn displayFn) {
    if(displayFn == nullptr && customDrawing == nullptr) return;
	// when we set this, we are stopping tcMenu rendering and letting this take over
	renderFnPressType = RPRESS_NONE;
    displayTakenMode = displayFn ? TAKEN_OVER_FN : START_CUSTOM_DRAW;
	renderCallback = displayFn;
}

void BaseMenuRenderer::giveBackDisplay() {
	// clear off the rendering callback.
	renderFnPressType = RPRESS_NONE;
	renderCallback = nullptr;
	displayTakenMode = NOT_TAKEN_OVER;
	menuMgr.changeMenu();
	menuAltered();
}

void BaseMenuRenderer::setFirstWidget(TitleWidget* widget) {
	this->firstWidget = widget;
	this->redrawMode = MENUDRAW_COMPLETE_REDRAW;
}

uint8_t BaseMenuRenderer::setActiveItem(MenuItem* item) {
    if(!item) return 0;
    if(activeItem) activeItem->setChanged(true);
    item->setChanged(true);
    activeItem = item;
    return findItemIndex(menuMgr.getCurrentMenu(), item);
}

int BaseMenuRenderer::findItemIndex(MenuItem *root, MenuItem *toFind) {
    uint8_t i = 0;
    MenuItem *itm = root;
    while (itm != nullptr) {
        if(itm->isVisible()) {
            if (itm == toFind) return i;
            i = i + 1;
        }
        itm = itm->getNext();
    }
    return 0;
}

int BaseMenuRenderer::findActiveItem(MenuItem *root) {
    return findItemIndex(menuMgr.getCurrentMenu(), activeItem);
}

uint8_t BaseMenuRenderer::itemCount(MenuItem* item, bool includeNonVisible) {
    uint8_t count = 0;
    while (item) {
        if (includeNonVisible || item->isVisible()) ++count;
        item = item->getNext();
    }
    return count;
}

MenuItem *BaseMenuRenderer::getMenuItemAtIndex(MenuItem *root, uint8_t pos) {
    uint8_t i = 0;
    MenuItem *itm = root;

    while (itm != nullptr) {
        if (itm->isVisible()) {
            if (i == pos) {
                return itm;
            }
            i++;
        }
        itm = itm->getNext();
    }
    return root;
}

void BaseMenuRenderer::setUpdatesPerSecond(int updatesSec) {
    bool needsReschedule = updatesPerSecond == UPDATES_SEC_DISPLAY_OFF;
    updatesPerSecond = updatesSec;
    if(resetValInTicks != MAX_TICKS) {
        resetValInTicks = 30 * updatesSec;
    }
    if(needsReschedule) {
        taskManager.execute(this);
    }
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
    if(dialog == nullptr) {
        dialog = new NoRenderDialog();
    }
    return dialog;
}

bool isItemActionable(MenuItem* item) {
	if (item->getMenuType() == MENUTYPE_SUB_VALUE || item->getMenuType() == MENUTYPE_ACTION_VALUE 
		|| item->getMenuType() == MENUTYPE_ACTIVATE_SUBMENU || item->getMenuType() == MENUTYPE_RUNTIME_VALUE
        || item->getMenuType() == MENUTYPE_TITLE_ITEM || item->getMenuType() == MENUTYPE_DIALOG_BUTTON) return true;

	if (item->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		return reinterpret_cast<ListRuntimeMenuItem*>(item)->isActingAsParent();
	}
	return false;
}

bool isCardLayoutActive(MenuItem* rootItem) {
    if(BaseMenuRenderer::getInstance()->getRendererType() == RENDER_TYPE_CONFIGURABLE) {
        auto r = reinterpret_cast<tcgfx::BaseGraphicalRenderer*>(BaseMenuRenderer::getInstance());
        return (r->getLayoutMode(getSubMenuFor(rootItem)) == tcgfx::LAYOUT_CARD_SIDEWAYS);
    } else {
        return false;
    }
}
