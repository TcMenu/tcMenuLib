/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>
#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include "MenuIterator.h"
#include "SecuredMenuPopup.h"
#include <IoAbstraction.h>

MenuManager menuMgr;

void MenuManager::initForUpDownOk(MenuRenderer* renderer, MenuItem* root, uint8_t pinUp, uint8_t pinDown, uint8_t pinOk) {
	this->renderer = renderer;
	this->currentRoot = this->rootMenu = root;

	switches.addSwitch(pinOk, NULL);
    switches.onRelease(pinOk, [](uint8_t /*key*/, bool held) { menuMgr.onMenuSelect(held); }); 
	setupUpDownButtonEncoder(pinUp, pinDown, [](int value) {menuMgr.valueChanged(value); });
	renderer->initialise();
}

void MenuManager::initForEncoder(MenuRenderer* renderer,  MenuItem* root, uint8_t encoderPinA, uint8_t encoderPinB, uint8_t encoderButton) {
	this->renderer = renderer;
	this->currentRoot = this->rootMenu = root;

	switches.addSwitch(encoderButton, NULL);
    switches.onRelease(encoderButton, [](uint8_t /*key*/, bool held) { menuMgr.onMenuSelect(held); }); 
	setupRotaryEncoderWithInterrupt(encoderPinA, encoderPinB, [](int value) {menuMgr.valueChanged(value); });

	renderer->initialise();
}

void MenuManager::setBackButton(uint8_t backButtonPin) {
    switches.addSwitch(backButtonPin, [](uint8_t, bool held){
        if(!held) menuMgr.performDirectionMove(true);
    });
}

void MenuManager::setNextButton(uint8_t nextButtonPin) {
    switches.addSwitch(nextButtonPin, [](uint8_t, bool held){
        if(!held) menuMgr.performDirectionMove(false);
    });    
}

void MenuManager::performDirectionMove(bool dirIsBack) {
    if(currentEditor != NULL && isMenuRuntimeMultiEdit(currentEditor)) {
        EditableMultiPartMenuItem<void*>* editableItem = reinterpret_cast<EditableMultiPartMenuItem<void*>*>(currentEditor);
		
        int editorRange = dirIsBack ? editableItem->previousPart() : editableItem->nextPart();
		if (editorRange != 0) {
			switches.changeEncoderPrecision(editorRange, editableItem->getPartValueAsInt());
		}
        else {
            currentEditor->setEditing(false);
            currentEditor->setActive(true);
            currentEditor = NULL;
        }
    }
    else if(currentEditor != NULL) {
        currentEditor->setEditing(false);
        currentEditor->setActive(true);
        currentEditor = NULL;
    }
    else if(currentEditor == NULL && dirIsBack) {
        setCurrentMenu(getParentAndReset());
    }
    else if(currentEditor == NULL && !dirIsBack) {
        MenuItem* currentActive = menuMgr.findCurrentActive();
        if(currentActive != NULL && currentActive->getMenuType() == MENUTYPE_SUB_VALUE) {
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
        if (currentEditor != NULL && isMenuRuntimeMultiEdit(currentEditor)) {
            setCurrentMenu(currentRoot);
        }
        else {
            setCurrentMenu(getParentAndReset());
        }
    }
	else if (getCurrentEditor() != NULL) {
		stopEditingCurrentItem();
	}
	else  {
		MenuItem* toEdit = findCurrentActive();
		actionOnCurrentItem(toEdit);
	}
}

void MenuManager::actionOnCurrentItem(MenuItem* toEdit) {
	BaseMenuRenderer* baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);

	// if there's a new item specified in toEdit, it means we need to change
	// the current editor (if it's possible to edit that value)
	if (toEdit->getMenuType() == MENUTYPE_SUB_VALUE) {
	    SubMenuItem* subMenu = reinterpret_cast<SubMenuItem*>(toEdit);
		if (subMenu->isSecured() && authenticationManager != NULL) {
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
			ListRuntimeMenuItem* listItem = reinterpret_cast<ListRuntimeMenuItem*>(toEdit);
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

void MenuManager::stopEditingCurrentItem() {

	if (isMenuRuntimeMultiEdit(menuMgr.getCurrentEditor())) {
		EditableMultiPartMenuItem<void*>* editableItem = reinterpret_cast<EditableMultiPartMenuItem<void*>*>(menuMgr.getCurrentEditor());

		// unless we've run out of parts to edit, stay in edit mode, moving to next part.
		int editorRange = editableItem->nextPart();
		if (editorRange != 0) {
			switches.changeEncoderPrecision(editorRange, editableItem->getPartValueAsInt());
			return;
		}
	}

	currentEditor->setEditing(false);
	currentEditor = NULL;
	setItemsInCurrentMenu(itemCount(menuMgr.getCurrentMenu()) - 1, offsetOfCurrentActive(menuMgr.getCurrentMenu()));

	if (renderer->getRendererType() == RENDERER_TYPE_BASE) {
		BaseMenuRenderer* baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);
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
	while (itm != NULL) {
		if (itm->isActive()) {
			return itm;
		}
		itm = itm->getNext();
	}

	return getCurrentMenu();
}

void MenuManager::setupForEditing(MenuItem* item) {
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

void MenuManager::setCurrentEditor(MenuItem * editor) {
	if (currentEditor != NULL) {
		currentEditor->setEditing(false);
		currentEditor->setActive(editor == NULL);
	}
	currentEditor = editor;
}

void MenuManager::setCurrentMenu(MenuItem * theItem) {
	serdebugF2("setCurrentMenu: ", theItem->getId());

	if (renderer->getRendererType() != RENDERER_TYPE_BASE) return;
	BaseMenuRenderer* baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);

	menuMgr.setCurrentEditor(NULL);

	getParentAndReset();

	MenuItem* root = theItem;
	currentRoot = root;
	root->setActive(true);
	baseRenderer->prepareNewSubmenu();
}

SecuredMenuPopup* MenuManager::secureMenuInstance() {
	if (securedMenuPopup == NULL) securedMenuPopup = new SecuredMenuPopup(authenticationManager);
	return securedMenuPopup;
}
