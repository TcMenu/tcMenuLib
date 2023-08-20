/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <PlatformDetermination.h>
#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include "ScrollChoiceMenuItem.h"
#include "MenuIterator.h"
#include "SecuredMenuPopup.h"
#include "extras/TwoButtonSwitchEncoder.h"
#include <IoAbstraction.h>
#include <BaseDialog.h>

MenuManager menuMgr;

const SubMenuInfo TCGLB_ROOT_SUB_INFO PROGMEM = { "ROOT", 0, (uint16_t)-1, 1, nullptr };
SubMenuItem MenuManager::ROOT(&TCGLB_ROOT_SUB_INFO, nullptr);

class Digital4WayPassThruListener : public SwitchListener {
private:
    pinid_t backPin = -1, nextPin = -1;
    bool rightIsSel = false;
public:
    void init(pinid_t back, pinid_t next) {
        backPin = back;
        nextPin = next;
    }

    void onPressed(pinid_t pin, bool held) override {
        if(pin == backPin) {
            if(!held) menuMgr.performDirectionMove(true);
        } else if(pin == nextPin && !rightIsSel) {
            if(!held) menuMgr.performDirectionMove(false);
        }
    }

    void onReleased(pinid_t pin, bool held) override {
        if (rightIsSel && pin == nextPin) {
            menuMgr.onMenuSelect(held);
        }
    }

    void setRightIsSelect(bool rightSel) {
        rightIsSel = rightSel;
    }
} fourWayPassThru;

void MenuManager::initFor4WayJoystick(MenuRenderer* renderer, MenuItem* root, pinid_t downPin, pinid_t upPin, pinid_t leftPin,
                         pinid_t rightPin, pinid_t okPin, int speed) {
    this->renderer = renderer;
    setRootItem(root);
    fourWayPassThru.init(leftPin, rightPin);

    if(okPin == 0xffU) {
        fourWayPassThru.setRightIsSelect(true);
    } else {
        switches.addSwitch(okPin, nullptr);
        switches.onRelease(okPin, [](pinid_t /*key*/, bool held) { menuMgr.onMenuSelect(held); });
    }
    setupUpDownButtonEncoder(upPin, downPin, leftPin, rightPin, &fourWayPassThru, [](int val) {menuMgr.valueChanged(val);}, speed);
    renderer->initialise();
}

void MenuManager::initForTwoButton(MenuRenderer *r, MenuItem *root, pinid_t upPin, pinid_t downPin) {
    this->renderer = r;
    setRootItem(root);
    switches.setEncoder(new TwoButtonSwitchEncoder(upPin, downPin, [](int v) { menuMgr.valueChanged(v); }));
    renderer->initialise();
}

void MenuManager::initForUpDownOk(MenuRenderer* renderer, MenuItem* root, pinid_t pinDown, pinid_t pinUp, pinid_t pinOk, int speed) {
    this->renderer = renderer;
    setRootItem(root);

    switches.addSwitch(pinOk, nullptr);
    switches.onRelease(pinOk, [](pinid_t /*key*/, bool held) { menuMgr.onMenuSelect(held); });
    setupUpDownButtonEncoder(pinUp, pinDown, [](int value) {menuMgr.valueChanged(value); }, speed);
    renderer->initialise();
}

void MenuManager::initForEncoder(MenuRenderer* renderer,  MenuItem* root, pinid_t encoderPinA, pinid_t encoderPinB, pinid_t encoderButton, EncoderType type) {
	this->renderer = renderer;
    setRootItem(root);

	switches.addSwitch(encoderButton, nullptr);
    switches.onRelease(encoderButton, [](pinid_t /*key*/, bool held) { menuMgr.onMenuSelect(held); });
	setupRotaryEncoderWithInterrupt(encoderPinA, encoderPinB, [](int value) {menuMgr.valueChanged(value); }, HWACCEL_REGULAR, type);

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
			switches.changeEncoderPrecision(0, editorRange, editableItem->getPartValueAsInt(),
                                            isWrapAroundEncoder(editableItem), 1);
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
    setRootItem(root);
	renderer->initialise();
}

/**
 * Called when the rotary encoder value has changed, if we are editing this changes the value in the current editor, if we are
 * showing menu items, it changes the index of the active item (renderer will move into display if needed).
 */
void MenuManager::valueChanged(int value) {
	if (renderer->tryTakeSelectIfNeeded(value, RPRESS_NONE)) return;

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
        if(renderer->getRendererType() != RENDER_TYPE_NOLOCAL) {
            serlogF2(SER_TCMENU_DEBUG, "activate item ", value);
            auto currentActive = reinterpret_cast<BaseMenuRenderer*>(renderer)->getMenuItemAtIndex(getCurrentMenu(), value);
            if(currentActive) {
                setItemActive(currentActive);
                serlogF3(SER_TCMENU_DEBUG, "Change active (V, ID) ", value, currentActive->getId());
            }
        }
	}
}

void MenuManager::setItemActive(MenuItem* item) {
    if(item) {
        // change the encoder value if there is an encoder present
        if(renderer->getRendererType() == RENDER_TYPE_NOLOCAL || switches.getEncoder() == nullptr) return;
        auto baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);

        auto activeIdx = baseRenderer->setActiveItem(item);
        if(activeIdx <= switches.getEncoder()->getMaximumValue()) {
            switches.getEncoder()->setCurrentReading(activeIdx);
        }

        // and notify that we have just changed the value.
        for(auto n : structureNotifier) {
            if(n) n->activeItemHasChanged(item);
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
		serlogF2(SER_TCMENU_INFO, "Submenu is secured: ", nextSub->getId());
		SecuredMenuPopup* popup = secureMenuInstance();
		popup->start(subMenu);
		navigateToMenu(popup->getRootItem(), popup->getItemToActivate(), true);
	}
	else {
		navigateToMenu(subMenu->getChild(), findCurrentActive());
	}
}

void MenuManager::actionOnCurrentItem(MenuItem* toEdit) {
	auto* baseRenderer = reinterpret_cast<BaseMenuRenderer*>(renderer);

    if(!notifyEditStarting(toEdit)) return;

	// if there's a new item specified in toEdit, it means we need to change
	// the current editor (if it's possible to edit that value)
	if (toEdit->getMenuType() == MENUTYPE_SUB_VALUE) {
        if(toEdit != getCurrentMenu()) notifyEditEnd(getCurrentMenu());
		actionOnSubMenu(toEdit);
        return;
	}

	if (toEdit->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		if (menuMgr.getCurrentMenu() == toEdit) {
			auto* listItem = reinterpret_cast<ListRuntimeMenuItem*>(toEdit);
            serlogF2(SER_TCMENU_INFO, "List select: ", listItem->getActiveIndex());
			if (listItem->getActiveIndex() == 0) {
				resetMenu(false);
			}
			else {
				listItem->getChildItem(listItem->getActiveIndex() - 1)->triggerCallback();
				// reset to parent after doing the callback
				listItem->asParent();
			}
		}
		else {
            notifyEditEnd(getCurrentMenu());
            navigateToMenu(toEdit);
        }
	}
	else if (toEdit->getMenuType() == MENUTYPE_BACK_VALUE) {
	    toEdit->triggerCallback();
		resetMenu(false);
	}
	else if (isItemActionable(toEdit)) {
        serlogF2(SER_TCMENU_INFO, "Callback trigger ", toEdit->getId());
		toEdit->triggerCallback();
	}
	else {
        serlogF2(SER_TCMENU_INFO, "Edit start ", toEdit->getId());
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
			switches.changeEncoderPrecision(0, editorRange, editableItem->getPartValueAsInt(),
                                            isWrapAroundEncoder(editableItem), 1);
			return;
		}
	}

    notifyEditEnd(currentEditor);
	
    currentEditor = nullptr;
    renderingHints.changeEditingParams(CurrentEditorRenderingHints::EDITOR_REGULAR, 0, 0);
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

	auto* pItem = getParentRoot(menuMgr.getCurrentMenu());
	if(pItem == nullptr) pItem = menuMgr.getRoot();
	return pItem;
}

void MenuManager::setupForEditing(MenuItem* item) {
	// if the item is NULL, or it's read only, then it can't be edited.
	if (item == nullptr || item->isReadOnly()) return;

	MenuType ty = item->getMenuType();
	if ((ty == MENUTYPE_ENUM_VALUE || ty == MENUTYPE_INT_VALUE)) {
		// these are the only types we can edit with a rotary encoder & LCD.
		setCurrentEditor(item);
        int step = (ty == MENUTYPE_INT_VALUE) ? reinterpret_cast<AnalogMenuItem*>(item)->getStep() : 1;
		switches.changeEncoderPrecision(0, item->getMaximumValue(), reinterpret_cast<ValueMenuItem*>(currentEditor)->getCurrentValue(),
                                        isWrapAroundEncoder(currentEditor), step);
		if(switches.getEncoder()) switches.getEncoder()->setUserIntention(CHANGE_VALUE);
	}
	else if (ty == MENUTYPE_BOOLEAN_VALUE) {
		// we don't actually edit boolean items, just toggle them instead
        auto* boolItem = (BooleanMenuItem*)item;
		boolItem->setBoolean(!boolItem->getBoolean());
		notifyEditEnd(item);
	}
	else if (ty == MENUTYPE_SCROLLER_VALUE) {
        setCurrentEditor(item);
	    switches.changeEncoderPrecision(0, item->getMaximumValue(), reinterpret_cast<ScrollChoiceMenuItem*>(item)->getCurrentValue(),
                                        isWrapAroundEncoder(currentEditor), 1);
	}
	else if (isMenuRuntimeMultiEdit(item)) {
        setCurrentEditor(item);
        auto* editableItem = reinterpret_cast<EditableMultiPartMenuItem*>(item);
        editableItem->beginMultiEdit();
        int range = editableItem->nextPart();
        switches.changeEncoderPrecision(0, range, editableItem->getPartValueAsInt(),
                                        isWrapAroundEncoder(editableItem), 1);
        switches.getEncoder()->setUserIntention(CHANGE_VALUE);
    }
}

void MenuManager::setCurrentEditor(MenuItem * editor) {
	if (currentEditor != nullptr) {
		currentEditor->setChanged(true);
	}
	currentEditor = editor;

    if(currentEditor != nullptr) {
        currentEditor->setChanged(true);
    }

    renderingHints.changeEditingParams(CurrentEditorRenderingHints::EDITOR_REGULAR, 0, 0);
}

void MenuManager::changeMenu(MenuItem* possibleActive) {
    if (renderer->getRendererType() == RENDER_TYPE_NOLOCAL) return;

    serlogF2(SER_TCMENU_DEBUG, "changeMenu: ", navigator.getCurrentRoot()->getId());

    // clear the current editor and ensure all active / editing flags removed.
	menuMgr.setCurrentEditor(nullptr);
    getParentAndReset();

	// now we set up the encoder to represent the right value and mark an item as active.
    if (menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST) {
        auto* listMenu = reinterpret_cast<ListRuntimeMenuItem*>(menuMgr.getCurrentMenu());
        listMenu->setActiveIndex(0);
        setItemsInCurrentMenu(listMenu->getNumberOfRows());
    } else {
        auto* toActivate = (possibleActive) ? possibleActive : navigator.getCurrentRoot();
        auto itemIdx = offsetOfItem(toActivate);
        setItemsInCurrentMenu(itemCount(navigator.getCurrentRoot(), false) - 1, itemIdx);
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
    MenuItem* endOfAddedList = toAdd;
    while(endOfAddedList->getNext() != nullptr) {
        endOfAddedList = endOfAddedList->getNext();
    }
    endOfAddedList->setNext(existing->getNext());
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

void MenuManager::resetObservers() {
    for(auto& i : structureNotifier) {
        i= nullptr;
    }
}

void MenuManager::load(uint16_t magicKey, TimerFn onEepromEmpty) {
    if(!loadMenuStructure(eepromRef, magicKey) && onEepromEmpty != nullptr) {
        serlogF(SER_TCMENU_INFO, "Run EEPROM empty cb");
        onEepromEmpty();
    }
}

void MenuManager::load(EepromAbstraction &eeprom, uint16_t magicKey, TimerFn onEepromEmpty) {
    eepromRef = &eeprom;
    if(!loadMenuStructure(&eeprom, magicKey) && onEepromEmpty != nullptr) {
        serlogF(SER_TCMENU_INFO, "Run EEPROM empty cb");
        onEepromEmpty();
    }
}

void MenuManager::notifyEditEnd(MenuItem *item) {
    if(item == nullptr) return; // don't notify a null pointer
    for(auto & obs : structureNotifier) {
        if(obs != nullptr) {
            obs->menuEditEnded(item);
        }
    }
    serlogF2(SER_TCMENU_INFO, "Menu edit end ", item->getId());
}

bool MenuManager::notifyEditStarting(MenuItem *item) {
    if(item == nullptr) return true; // don't notify a null pointer and allow menu to proceed.

    bool goAhead = true;
    for(auto & obs : structureNotifier) {
        if(obs != nullptr) {
            goAhead = goAhead && obs->menuEditStarting(item);
        }
    }
    if(!goAhead) {
        serlogF2(SER_TCMENU_INFO, "Edit start cancelled ", item->getId());
    }
    return goAhead;
}

void MenuManager::notifyStructureChanged() {
    serlogF(SER_TCMENU_INFO, "Menu structure change");
    for(auto & i : structureNotifier) {
        if(i != nullptr) {
            i->structureHasChanged();
        }
    }
}

void MenuManager::setItemsInCurrentMenu(int size, int offs) {
    auto enc = switches.getEncoder();
    if(!enc) return;
    serlogF3(SER_TCMENU_INFO, "Set items in menu (size, offs) ", size, offs);
    enc->changePrecision(size, offs, useWrapAroundByDefault);
    enc->setUserIntention(isCardLayoutActive(getCurrentMenu()) ? SCROLL_THROUGH_SIDEWAYS : SCROLL_THROUGH_ITEMS);
}

void MenuManager::resetMenu(bool completeReset) {
    // we cannot reset the menu while a dialog is currently shown.
    if(renderer->getDialog() && renderer->getDialog()->isInUse()) return;

    MenuItem* currentActive;
    if(completeReset) {
        setRootItem(navigator.getRoot());
        currentActive = nullptr;
    } else {
        currentActive = navigator.popNavigationGetActive();
    }
    changeMenu(currentActive);
}

void MenuManager::navigateToMenu(MenuItem* theNewItem, MenuItem* possibleActive, bool customMenu) {
    navigator.navigateTo(possibleActive, theNewItem, customMenu);
    changeMenu(possibleActive);
}

void MenuManager::addEncoderWrapOverride(MenuItem &item, bool override) {
    encoderWrapOverrides.add(EncoderWrapOverride(item.getId(), override));
}

bool MenuManager::isWrapAroundEncoder(MenuItem* menuItem) {
    for(EncoderWrapOverride& item : encoderWrapOverrides) {
        if(item.getMenuId() == menuItem->getId())  return item.getOverrideValue();
    }
    return useWrapAroundByDefault;
}

void MenuManager::recalculateListIfOnDisplay(RuntimeMenuItem* runtimeItem) {
    auto enc = switches.getEncoder();
    // if there is an encoder, and the current menu is the list..
    if(enc && navigator.getCurrentRoot() == runtimeItem) {
        auto newRows = runtimeItem->getNumberOfRows();
        auto encVal = enc->getCurrentReading();
        uint8_t newPos = encVal < newRows ? encVal : (newRows - 1);
        setItemsInCurrentMenu(newRows, newPos);
    }
}

void MenuManager::setEditorHints(CurrentEditorRenderingHints::EditorRenderingType hint, size_t start, size_t end) {
    renderingHints.changeEditingParams(hint, start, end);
    serlogF4(SER_TCMENU_DEBUG, "SetEditorHints ", hint, start, end);
}

void MenuManager::setEditorHintsLocked(bool locked) {
    renderingHints.lockEditor(locked);
    serlogF2(SER_TCMENU_DEBUG, "EditorHints Locked = ", locked);
}

MenuItem *MenuManager::findCurrentActive() {
    if(renderer->getRendererType() == RENDER_TYPE_NOLOCAL) return getRoot();
    auto bmr = reinterpret_cast<BaseMenuRenderer*>(renderer);
    auto idx = bmr->findActiveItem(getCurrentMenu());
    return bmr->getMenuItemAtIndex(getCurrentMenu(), idx);
}

void MenuManager::setRootItem(MenuItem *pItem) {
    ROOT.setChild(pItem);
    navigator.setRootItem(pItem);
}

void CurrentEditorRenderingHints::changeEditingParams(CurrentEditorRenderingHints::EditorRenderingType ty, int startOffset, int endOffset) {
    if(renderingType == EDITOR_OVERRIDE_LOCK) return;
    renderingType = ty;
    editStart = startOffset;
    editEnd = endOffset;
}
