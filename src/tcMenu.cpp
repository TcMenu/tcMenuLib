/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <PlatformDetermination.h>
#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include "ScrollChoiceMenuItem.h"
#include "MenuIterator.h"
#include "SecuredMenuPopup.h"
#include "graphics/BaseGraphicalRenderer.h"
#include <IoAbstraction.h>

using namespace tcgfx;

MenuManager menuMgr;

void MenuManager::initForUpDownOk(MenuRenderer* renderer, MenuItem* root, pinid_t pinDown, pinid_t pinUp, pinid_t pinOk) {
	this->renderer = renderer;
	navigator.setRootItem(root);

	switches.addSwitch(pinOk, nullptr);
    switches.onRelease(pinOk, [](pinid_t /*key*/, bool held) { menuMgr.onMenuSelect(held); });
	setupUpDownButtonEncoder(pinUp, pinDown, [](int value) {menuMgr.valueChanged(value); });
	renderer->initialise();
}

void MenuManager::initForEncoder(MenuRenderer* renderer,  MenuItem* root, pinid_t encoderPinA, pinid_t encoderPinB, pinid_t encoderButton) {
	this->renderer = renderer;
    navigator.setRootItem(root);

	switches.addSwitch(encoderButton, nullptr);
    switches.onRelease(encoderButton, [](pinid_t /*key*/, bool held) { menuMgr.onMenuSelect(held); });
	setupRotaryEncoderWithInterrupt(encoderPinA, encoderPinB, [](int value) {menuMgr.valueChanged(value); });

	renderer->initialise();
}

void MenuManager::setBackButton(pinid_t backButtonPin) {
    switches.addSwitch(backButtonPin, [](pinid_t, bool held){
        if(!held) menuMgr.performDirectionMove(true);
    });
}

void MenuManager::setNextButton(pinid_t nextButtonPin) {
    switches.addSwitch(nextButtonPin, [](pinid_t, bool held){
        if(!held) menuMgr.performDirectionMove(false);
    });    
}

void MenuManager::performDirectionMove(bool dirIsBack) {
    if(currentEditor != nullptr && isMenuRuntimeMultiEdit(currentEditor)) {
        auto editableItem = reinterpret_cast<EditableMultiPartMenuItem*>(currentEditor);
		
        int editorRange = dirIsBack ? editableItem->previousPart() : editableItem->nextPart();
		if (editorRange != 0) {
			switches.changeEncoderPrecision(editorRange, editableItem->getPartValueAsInt());
		}
        else {
            stopEditingCurrentItem(false);
        }
    }
    else if(currentEditor != nullptr) {
        stopEditingCurrentItem(false);
    }
    else if(currentEditor == nullptr && dirIsBack) {
        getParentAndReset();
        resetMenu(false);
    }
    else if(currentEditor == nullptr && !dirIsBack) {
        MenuItem* currentActive = findCurrentActive();
        if(currentActive != nullptr && currentActive->getMenuType() == MENUTYPE_SUB_VALUE) {
            actionOnSubMenu(currentActive);
        }
    }
}

void MenuManager::initWithoutInput(MenuRenderer* renderer, MenuItem* root) {
	this->renderer = renderer;
    navigator.setRootItem(root);
	renderer->initialise();
}

/**
 * Called when the rotary encoder value has changed, if we are editing this changes the value in the current editor, if we are
 * showing menu items, it changes the index of the active item (renderer will move into display if needed).
 */
void MenuManager::valueChanged(int value) {
	if (renderer->tryTakeSelectIfNeeded(value, RPRESS_NONE)) return;
	MenuItem* currentEditor = getCurrentEditor();

	if (currentEditor && isMenuBasedOnValueItem(currentEditor)) {
		((ValueMenuItem*)currentEditor)->setCurrentValue(value);
	}
	else if (currentEditor && isMenuRuntimeMultiEdit(currentEditor)) {
		reinterpret_cast<EditableMultiPartMenuItem*>(currentEditor)->valueChanged(value);
	}
	else if(currentEditor && currentEditor->getMenuType() == MENUTYPE_SCROLLER_VALUE) {
	    reinterpret_cast<ScrollChoiceMenuItem*>(currentEditor)->setCurrentValue(value);
	}
    else if (menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST) {
        reinterpret_cast<ListRuntimeMenuItem*>(menuMgr.getCurrentMenu())->setActiveIndex(value);
    }
	else {
        MenuItem* currentActive = menuMgr.findCurrentActive();
        currentActive->setActive(false);
        if(renderer->getRendererType() == RENDER_TYPE_CONFIGURABLE) {
            currentActive = reinterpret_cast<BaseGraphicalRenderer*>(renderer)->getMenuItemAtIndex(value);
            if(currentActive) {
                currentActive->setActive(true);
                serdebugF3("Change active (V, ID) ", value, currentActive->getId());
            }
        }
        else {
            currentActive = getItemAtPosition(navigator.getCurrentRoot(), value);
            currentActive->setActive(true);
            serdebugF3("Legacy set active (V, ID) ", value, currentActive->getId());
        }
	}
}

/**
 * Called when the button on the encoder (OK button) is pressed. Most of this is left to the renderer to decide.
 */
void MenuManager::onMenuSelect(bool held) {
	if (renderer->tryTakeSelectIfNeeded(0, held ? RPRESS_HELD : RPRESS_PRESSED)) return;

	if (held) {
        if (currentEditor != nullptr && isMenuRuntimeMultiEdit(currentEditor)) {
            changeMenu();
        }
        else {
            resetMenu(true);
        }
    }
	else if (getCurrentEditor() != nullptr) {
		stopEditingCurrentItem(true);
	}
	else  {
		MenuItem* toEdit = findCurrentActive();
		actionOnCurrentItem(toEdit);
	}
}

void MenuManager::actionOnSubMenu(MenuItem* nextSub) {
	SubMenuItem* subMenu = reinterpret_cast<SubMenuItem*>(nextSub);
	if (subMenu->isSecured() && authenticationManager != nullptr) {
		serdebugF2("Submenu is secured: ", nextSub->getId());
		SecuredMenuPopup* popup = secureMenuInstance();
		popup->start(subMenu);
		navigateToMenu(popup->getRootItem());
	}
	else {
		navigateToMenu(subMenu->getChild());
	}
}

void MenuManager::actionOnCurrentItem(MenuItem* toEdit) {
	auto* baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);

	// if there's a new item specified in toEdit, it means we need to change
	// the current editor (if it's possible to edit that value)
	if (toEdit->getMenuType() == MENUTYPE_SUB_VALUE) {
		actionOnSubMenu(toEdit);
	}
	else if (toEdit->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		if (menuMgr.getCurrentMenu() == toEdit) {
			auto* listItem = reinterpret_cast<ListRuntimeMenuItem*>(toEdit);
			serdebugF2("List select: ", listItem->getActiveIndex());
			if (listItem->getActiveIndex() == 0) {
				resetMenu(false);
			}
			else {
				listItem->getChildItem(listItem->getActiveIndex() - 1)->triggerCallback();
				// reset to parent after doing the callback
				listItem->asParent();
			}
		}
		else navigateToMenu(toEdit);
	}
	else if (toEdit->getMenuType() == MENUTYPE_BACK_VALUE) {
	    toEdit->triggerCallback();
		toEdit->setActive(false);
		resetMenu(false);
	}
	else if (isItemActionable(toEdit)) {
		toEdit->triggerCallback();
	}
	else {
		menuMgr.setupForEditing(toEdit);
		baseRenderer->redrawRequirement(MENUDRAW_EDITOR_CHANGE);
	}
}

void MenuManager::stopEditingCurrentItem(bool doMultiPartNext) {

	if (doMultiPartNext && isMenuRuntimeMultiEdit(menuMgr.getCurrentEditor())) {
		auto* editableItem = reinterpret_cast<EditableMultiPartMenuItem*>(menuMgr.getCurrentEditor());

		// unless we've run out of parts to edit, stay in edit mode, moving to next part.
		int editorRange = editableItem->nextPart();
		if (editorRange != 0) {
			switches.changeEncoderPrecision(editorRange, editableItem->getPartValueAsInt());
			return;
		}
	}

	currentEditor->setEditing(false);

    notifyEditEnd(currentEditor);
	
    currentEditor = nullptr;
	setItemsInCurrentMenu(itemCount(menuMgr.getCurrentMenu()) - 1, offsetOfCurrentActive(menuMgr.getCurrentMenu()));

	if (renderer->getRendererType() != RENDER_TYPE_NOLOCAL) {
		auto* baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);
		baseRenderer->redrawRequirement(MENUDRAW_EDITOR_CHANGE);
	}
}

MenuItem* MenuManager::getParentAndReset() {
    if(menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST) {
        auto* sub = getSubMenuFor(menuMgr.getCurrentMenu());
        if(sub) return reinterpret_cast<SubMenuItem*>(sub)->getChild();
    }

	auto* pItem = getParentRootAndVisit(menuMgr.getCurrentMenu(), [](MenuItem* curr) {
		curr->setActive(false);
		curr->setEditing(false);
	});
	if(pItem == nullptr) pItem = menuMgr.getRoot();
	return pItem;
}

bool MenuManager::activateMenuItem(MenuItem *item) {
    if(renderer->getRendererType() == RENDER_TYPE_CONFIGURABLE) {
        auto* r = reinterpret_cast<BaseGraphicalRenderer*>(renderer);
        for(int i=0; i<r->getTotalItemsInMenu(); i++) {
            auto* pItem = r->getMenuItemAtIndex(i);
            if(pItem != nullptr && pItem->getId() == item->getId()) {
                valueChanged(i);
                return true;
            }
        }
    }
    return false;
}

/**
 * Finds teh currently active menu item with the selected SubMenuItem
 */
MenuItem* MenuManager::findCurrentActive() {
	MenuItem* itm = navigator.getCurrentRoot();
	while (itm != nullptr) {
		if (itm->isActive()) {
			return itm;
		}
		itm = itm->getNext();
	}

	// there's a special case for the title menu on the main page that needs to be checked against.
	if(renderer->getRendererType() == RENDER_TYPE_CONFIGURABLE) {
	    auto* pItem = reinterpret_cast<BaseGraphicalRenderer*>(renderer)->getMenuItemAtIndex(0);
	    if(pItem && pItem->isActive()) return pItem;
	}

	return getCurrentMenu();
}

void MenuManager::setupForEditing(MenuItem* item) {
	// if the item is NULL, or it's read only, then it can't be edited.
	if (item == nullptr || item->isReadOnly()) return;

	MenuType ty = item->getMenuType();
	if ((ty == MENUTYPE_ENUM_VALUE || ty == MENUTYPE_INT_VALUE)) {
		// these are the only types we can edit with a rotary encoder & LCD.
		if(!notifyEditStarting(item)) return;
		currentEditor = item;
		currentEditor->setEditing(true);
		switches.changeEncoderPrecision(item->getMaximumValue(), reinterpret_cast<ValueMenuItem*>(currentEditor)->getCurrentValue());
		if(switches.getEncoder()) switches.getEncoder()->setUserIntention(CHANGE_VALUE);
	}
	else if (ty == MENUTYPE_BOOLEAN_VALUE) {
		// we don't actually edit boolean items, just toggle them instead
        if(!notifyEditStarting(item)) return;
        auto* boolItem = (BooleanMenuItem*)item;
		boolItem->setBoolean(!boolItem->getBoolean());
		notifyEditEnd(item);
	}
	else if (ty == MENUTYPE_SCROLLER_VALUE) {
        if(!notifyEditStarting(item)) return;
        currentEditor = item;
        currentEditor->setEditing(true);
	    switches.changeEncoderPrecision(item->getMaximumValue(), reinterpret_cast<ScrollChoiceMenuItem*>(item)->getCurrentValue());
	}
	else if (isMenuRuntimeMultiEdit(item)) {
        if(!notifyEditStarting(item)) return;
        switches.getEncoder()->setUserIntention(CHANGE_VALUE);
        currentEditor = item;
        auto* editableItem = reinterpret_cast<EditableMultiPartMenuItem*>(item);
		editableItem->beginMultiEdit();
		int range = editableItem->nextPart();
		switches.changeEncoderPrecision(range, editableItem->getPartValueAsInt());
	}
}

void MenuManager::setCurrentEditor(MenuItem * editor) {
	if (currentEditor != nullptr) {
		currentEditor->setEditing(false);
		currentEditor->setActive(editor == nullptr);
	}
	currentEditor = editor;
}

void MenuManager::changeMenu(MenuItem* possibleActive) {
    if (renderer->getRendererType() == RENDER_TYPE_NOLOCAL) return;

    serdebugF2("changeMenu: ", navigator.getCurrentRoot()->getId());

    // clear the current editor and ensure all active / editing flags removed.
	menuMgr.setCurrentEditor(nullptr);

	// now we set up the encoder to represent the right value and mark an item as active.
    if (menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST) {
        getParentAndReset();
        auto* listMenu = reinterpret_cast<ListRuntimeMenuItem*>(menuMgr.getCurrentMenu());
        listMenu->setActiveIndex(0);
        menuMgr.setItemsInCurrentMenu(listMenu->getNumberOfParts());
    } else {
        auto* toActivate = (possibleActive) ? possibleActive : navigator.getCurrentRoot();
        toActivate->setActive(true);
        setItemsInCurrentMenu(itemCount(navigator.getCurrentRoot(), false) - 1, offsetOfCurrentActive(navigator.getCurrentRoot()));
    }

    // lastly force a redraw.
    reinterpret_cast<BaseMenuRenderer*>(renderer)->redrawRequirement(MENUDRAW_COMPLETE_REDRAW);
}

SecuredMenuPopup* MenuManager::secureMenuInstance() {
	if (securedMenuPopup == nullptr) securedMenuPopup = new SecuredMenuPopup(authenticationManager);
	return securedMenuPopup;
}

MenuManager::MenuManager() : navigator(), structureNotifier() {
    this->currentEditor = nullptr;
    this->renderer = nullptr;
    this->securedMenuPopup = nullptr;
    this->authenticationManager = nullptr;
    this->eepromRef = nullptr;
}

void MenuManager::addMenuAfter(MenuItem *existing, MenuItem* toAdd, bool silent) {
    toAdd->setNext(existing->getNext());
    existing->setNext(toAdd);
    if(!silent) notifyStructureChanged();
}

void MenuManager::addChangeNotification(MenuManagerObserver *observer) {
    for(auto & i : structureNotifier) {
        if(i == nullptr) {
            i = observer;
            return;
        }
    }
}

void MenuManager::load(uint16_t magicKey, TimerFn onEepromEmpty) {
    if(!loadMenuStructure(eepromRef, magicKey) && onEepromEmpty != nullptr) {
        onEepromEmpty();
    }
}

void MenuManager::load(EepromAbstraction &eeprom, uint16_t magicKey, TimerFn onEepromEmpty) {
    eepromRef = &eeprom;
    if(!loadMenuStructure(&eeprom, magicKey) && onEepromEmpty != nullptr) {
        onEepromEmpty();
    }
}

void MenuManager::notifyEditEnd(MenuItem *item) {
    for(auto & obs : structureNotifier) {
        if(obs != nullptr) {
            obs->menuEditEnded(item);
        }
    }
}

bool MenuManager::notifyEditStarting(MenuItem *item) {
    bool goAhead = true;
    for(auto & obs : structureNotifier) {
        if(obs != nullptr) {
            goAhead = goAhead && obs->menuEditStarting(item);
        }
    }
    return goAhead;
}

void MenuManager::notifyStructureChanged() {
    for(auto & i : structureNotifier) {
        if(i != nullptr) {
            i->structureHasChanged();
        }
    }
}

void MenuManager::setItemsInCurrentMenu(int size, int offs) {
    auto enc = switches.getEncoder();
    if(!enc) return;
    enc->changePrecision(size, offs);
    enc->setUserIntention(SCROLL_THROUGH_ITEMS);
}

void MenuManager::resetMenu(bool completeReset) {
    MenuItem* currentActive;
    if(completeReset) {
        navigator.setRootItem(navigator.getRoot());
        currentActive = nullptr;
    } else {
        currentActive = navigator.popNavigationGetActive();
    }
    changeMenu(currentActive);
}

void MenuManager::navigateToMenu(MenuItem* theNewItem) {
    navigator.navigateTo(findCurrentActive(), theNewItem);
    changeMenu();
}
