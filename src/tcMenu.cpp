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
#include <IoAbstraction.h>

MenuManager menuMgr;

void MenuManager::initForUpDownOk(MenuRenderer* renderer, MenuItem* root, pinid_t pinDown, pinid_t pinUp, pinid_t pinOk) {
	this->renderer = renderer;
	this->currentRoot = this->rootMenu = root;

	switches.addSwitch(pinOk, nullptr);
    switches.onRelease(pinOk, [](pinid_t /*key*/, bool held) { menuMgr.onMenuSelect(held); });
	setupUpDownButtonEncoder(pinUp, pinDown, [](int value) {menuMgr.valueChanged(value); });
	renderer->initialise();
}

void MenuManager::initForEncoder(MenuRenderer* renderer,  MenuItem* root, pinid_t encoderPinA, pinid_t encoderPinB, pinid_t encoderButton) {
	this->renderer = renderer;
	this->currentRoot = this->rootMenu = root;

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
        auto editableItem = reinterpret_cast<EditableMultiPartMenuItem<void*>*>(currentEditor);
		
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
        setCurrentMenu(getParentAndReset());
    }
    else if(currentEditor == nullptr && !dirIsBack) {
        MenuItem* currentActive = menuMgr.findCurrentActive();
        if(currentActive != nullptr && currentActive->getMenuType() == MENUTYPE_SUB_VALUE) {
            setCurrentMenu(currentActive);
        }
    }
}

void MenuManager::initWithoutInput(MenuRenderer* renderer, MenuItem* root) {
	this->renderer = renderer;
	this->currentRoot = this->rootMenu = root;

	renderer->initialise();
}

bool isMenuBoolean(MenuType ty) {
	return ty == MENUTYPE_BOOLEAN_VALUE;
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
		reinterpret_cast<EditableMultiPartMenuItem<void*>*>(currentEditor)->valueChanged(value);
	}
	else if(currentEditor && currentEditor->getMenuType() == MENUTYPE_SCROLLER_VALUE) {
	    reinterpret_cast<ScrollChoiceMenuItem*>(currentEditor)->setCurrentValue(value);
	}
	else {
		serdebugF2("valueChanged V=", value);
		if (menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST) {
			reinterpret_cast<ListRuntimeMenuItem*>(menuMgr.getCurrentMenu())->setActiveIndex(value);
		}
		else {
			MenuItem* currentActive = menuMgr.findCurrentActive();
			currentActive->setActive(false);
			currentActive = getItemAtPosition(currentRoot, value);
			currentActive->setActive(true);
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
            setCurrentMenu(currentRoot);
        }
        else {
            setCurrentMenu(getParentAndReset());
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

void MenuManager::actionOnCurrentItem(MenuItem* toEdit) {
	auto* baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);

	// if there's a new item specified in toEdit, it means we need to change
	// the current editor (if it's possible to edit that value)
	if (toEdit->getMenuType() == MENUTYPE_SUB_VALUE) {
	    SubMenuItem* subMenu = reinterpret_cast<SubMenuItem*>(toEdit);
		if (subMenu->isSecured() && authenticationManager != nullptr) {
			serdebugF2("Submenu is secured: ", toEdit->getId());
			SecuredMenuPopup* popup = secureMenuInstance();
			popup->start(subMenu);
			currentRoot = popup->getRootItem();
			baseRenderer->prepareNewSubmenu();
		}
		else {
			menuMgr.setCurrentMenu(subMenu->getChild());
		}
	}
	else if (toEdit->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		if (menuMgr.getCurrentMenu() == toEdit) {
			auto* listItem = reinterpret_cast<ListRuntimeMenuItem*>(toEdit);
			serdebugF2("List select: ", listItem->getActiveIndex());
			if (listItem->getActiveIndex() == 0) {
				menuMgr.setCurrentMenu(menuMgr.getParentAndReset());
			}
			else {
				listItem->getChildItem(listItem->getActiveIndex() - 1)->triggerCallback();
				// reset to parent after doing the callback
				listItem->asParent();
			}
		}
		else menuMgr.setCurrentMenu(toEdit);
	}
	else if (toEdit->getMenuType() == MENUTYPE_BACK_VALUE) {
		toEdit->setActive(false);
		menuMgr.setCurrentMenu(menuMgr.getParentAndReset());
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
		auto* editableItem = reinterpret_cast<EditableMultiPartMenuItem<void*>*>(menuMgr.getCurrentEditor());

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


	if (renderer->getRendererType() == RENDERER_TYPE_BASE) {
		auto* baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);
		baseRenderer->redrawRequirement(MENUDRAW_EDITOR_CHANGE);
	}
}

MenuItem* MenuManager::getParentAndReset() {
	return getParentRootAndVisit(menuMgr.getCurrentMenu(), [](MenuItem* curr) {
		curr->setActive(false);
		curr->setEditing(false);
	});
}

/**
 * Finds teh currently active menu item with the selected SubMenuItem
 */
MenuItem* MenuManager::findCurrentActive() {
	MenuItem* itm = getCurrentMenu();
	while (itm != nullptr) {
		if (itm->isActive()) {
			return itm;
		}
		itm = itm->getNext();
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
		switches.getEncoder()->setUserIntention(CHANGE_VALUE);
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
        auto* editableItem = reinterpret_cast<EditableMultiPartMenuItem<void*>*>(item);
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

void MenuManager::setCurrentMenu(MenuItem * theItem) {
	serdebugF2("setCurrentMenu: ", theItem->getId());

	if (renderer->getRendererType() != RENDERER_TYPE_BASE) return;
	auto* baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);

	menuMgr.setCurrentEditor(nullptr);

	getParentAndReset();

	MenuItem* root = theItem;
	currentRoot = root;
	root->setActive(true);
	baseRenderer->prepareNewSubmenu();
}

SecuredMenuPopup* MenuManager::secureMenuInstance() {
	if (securedMenuPopup == nullptr) securedMenuPopup = new SecuredMenuPopup(authenticationManager);
	return securedMenuPopup;
}

MenuManager::MenuManager() : structureNotifier() {
    this->currentEditor = nullptr;
    this->currentRoot = nullptr;
    this->renderer = nullptr;
    this->rootMenu = nullptr;
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
