/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>
#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include <IoAbstraction.h>

MenuManager menuMgr;

void MenuManager::initForUpDownOk(MenuRenderer* renderer, MenuItem* root, uint8_t pinUp, uint8_t pinDown, uint8_t pinOk) {
	this->renderer = renderer;
	this->rootMenu = root;

	switches.addSwitch(pinOk, NULL);
    switches.onRelease(pinOk, [](uint8_t /*key*/, bool held) { menuMgr.onMenuSelect(held); }); 
	setupUpDownButtonEncoder(pinUp, pinDown, [](int value) {menuMgr.valueChanged(value); });

	renderer->initialise();
}

void MenuManager::initForEncoder(MenuRenderer* renderer,  MenuItem* root, uint8_t encoderPinA, uint8_t encoderPinB, uint8_t encoderButton) {
	this->renderer = renderer;
	this->rootMenu = root;

	switches.addSwitch(encoderButton, NULL);
    switches.onRelease(encoderButton, [](uint8_t /*key*/, bool held) { menuMgr.onMenuSelect(held); }); 
	setupRotaryEncoderWithInterrupt(encoderPinA, encoderPinB, [](int value) {menuMgr.valueChanged(value); });

	renderer->initialise();
}

void MenuManager::initWithoutInput(MenuRenderer* renderer, MenuItem* root) {
	this->renderer = renderer;
	this->rootMenu = root;

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
	MenuItem* currentEditor = renderer->getCurrentEditor();
	if (currentEditor && isMenuBasedOnValueItem(currentEditor)) {
		((ValueMenuItem*)currentEditor)->setCurrentValue(value);
	}
	else if (currentEditor && isMenuRuntimeMultiEdit(currentEditor)) {
		reinterpret_cast<EditableMultiPartMenuItem<void*>*>(currentEditor)->valueChanged(value);
	}
	else {
		renderer->activeIndexChanged(value);
	}

	// lastly if this is a type of renderer that's interested in resetting, let it know we changed
	if (renderer->getRendererType() == RENDERER_TYPE_BASE) {
		reinterpret_cast<BaseMenuRenderer*>(renderer)->menuAltered();
	}
}

/**
 * Called when the button on the encoder (OK button) is pressed. Most of this is left to the renderer to decide.
 */
void MenuManager::onMenuSelect(bool held) {
    if(held) {
        renderer->onHold();
    }
	else if (renderer->getCurrentEditor() != NULL) {
		renderer->onSelectPressed(NULL);
	}
	else {
		renderer->onSelectPressed(findCurrentActive());
	}
}

/**
 * Finds teh currently active menu item with the selected SubMenuItem
 */
MenuItem* MenuManager::findCurrentActive() {
	MenuItem* itm = renderer->getCurrentSubMenu();
	while (itm != NULL) {
		if (itm->isActive()) {
			return itm;
		}
		itm = itm->getNext();
	}

	return renderer->getCurrentSubMenu();
}

void loadRecursively(EepromAbstraction& eeprom, MenuItem* nextMenuItem) {
	while(nextMenuItem) {
		if(nextMenuItem->getMenuType() == MENUTYPE_SUB_VALUE) {
			loadRecursively(eeprom, ((SubMenuItem*)nextMenuItem)->getChild());
		}
		else if(nextMenuItem->getEepromPosition() == 0xffff) {
			// ignore this one, not got an eeprom entry..
		}
		else if(nextMenuItem->getMenuType() == MENUTYPE_TEXT_VALUE) {
			TextMenuItem* textItem = reinterpret_cast<TextMenuItem*>(nextMenuItem);
			eeprom.readIntoMemArray((uint8_t*) textItem->getTextValue(), textItem->getEepromPosition(), textItem->textLength());
			textItem->cleanUpArray();
			textItem->setSendRemoteNeededAll();
			textItem->setChanged(true);
			textItem->triggerCallback();
		}
        else if (nextMenuItem->getMenuType() == MENUTYPE_TIME) {
            TimeFormattedMenuItem* timeItem = reinterpret_cast<TimeFormattedMenuItem*>(nextMenuItem);
            eeprom.readIntoMemArray((uint8_t*) timeItem->getUnderlyingData(), timeItem->getEepromPosition(), 4);
			timeItem->setSendRemoteNeededAll();
			timeItem->setChanged(true);
			timeItem->triggerCallback();
        }
		else if (nextMenuItem->getMenuType() == MENUTYPE_IPADDRESS) {
			IpAddressMenuItem* ipItem = reinterpret_cast<IpAddressMenuItem*>(nextMenuItem);
			eeprom.readIntoMemArray(ipItem->getIpAddress(), ipItem->getEepromPosition(), 4);
			ipItem->setSendRemoteNeededAll();
			ipItem->setChanged(true);
			ipItem->triggerCallback();
		}
		else if(nextMenuItem->getMenuType() == MENUTYPE_INT_VALUE) {
			AnalogMenuItem* intItem = (AnalogMenuItem*)nextMenuItem;
			intItem->setCurrentValue(eeprom.read16(intItem->getEepromPosition()));
		}
		else if(nextMenuItem->getMenuType() == MENUTYPE_ENUM_VALUE) {
			EnumMenuItem* valItem = (EnumMenuItem*)nextMenuItem;
			valItem->setCurrentValue(eeprom.read16(valItem->getEepromPosition()));
		}
		else if(nextMenuItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
			BooleanMenuItem* valItem = (BooleanMenuItem*)nextMenuItem;
			valItem->setCurrentValue(eeprom.read8(valItem->getEepromPosition()));
		}
		nextMenuItem = nextMenuItem->getNext();
	}
}

void MenuManager::load(EepromAbstraction& eeprom, uint16_t magicKey) {
	if(eeprom.read16(0) == magicKey) {
		MenuItem* nextMenuItem = rootMenu;
		loadRecursively(eeprom, nextMenuItem);
	}
}

void saveRecursively(EepromAbstraction& eeprom, MenuItem* nextMenuItem) {
	while(nextMenuItem) {
		if(nextMenuItem->getMenuType() == MENUTYPE_SUB_VALUE) {
			saveRecursively(eeprom, ((SubMenuItem*)nextMenuItem)->getChild());
		}
		else if(nextMenuItem->getEepromPosition() == 0xffff) {
			// ignore this one, not got an eeprom entry..
		}
		else if(nextMenuItem->getMenuType() == MENUTYPE_TEXT_VALUE) {
			TextMenuItem* textItem = (TextMenuItem*) nextMenuItem;
			eeprom.writeArrayToRom(textItem->getEepromPosition(), (const uint8_t*) (textItem->getTextValue()), textItem->textLength());
		}
		else if(nextMenuItem->getMenuType() == MENUTYPE_TIME) {
			TimeFormattedMenuItem* timeItem = reinterpret_cast<TimeFormattedMenuItem*>(nextMenuItem);
			eeprom.writeArrayToRom(timeItem->getEepromPosition(), (const uint8_t*) (timeItem->getUnderlyingData()), 4);
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_IPADDRESS) {
			IpAddressMenuItem* ipItem = reinterpret_cast<IpAddressMenuItem*>(nextMenuItem);
			eeprom.writeArrayToRom(ipItem->getEepromPosition(), ipItem->getIpAddress(), 4);
		}
		else if(nextMenuItem->getMenuType() == MENUTYPE_INT_VALUE) {
			AnalogMenuItem* intItem = (AnalogMenuItem*)nextMenuItem;
			eeprom.write16(intItem->getEepromPosition(), intItem->getCurrentValue());
		}
		else if(nextMenuItem->getMenuType() == MENUTYPE_ENUM_VALUE) {
			EnumMenuItem* valItem = (EnumMenuItem*)nextMenuItem;
			eeprom.write16(valItem->getEepromPosition(), valItem->getCurrentValue());
		}
		else if(nextMenuItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
			BooleanMenuItem* valItem = (BooleanMenuItem*)nextMenuItem;
			eeprom.write8(valItem->getEepromPosition(), valItem->getCurrentValue());
		}
		nextMenuItem = nextMenuItem->getNext();
	}
}

void MenuManager::save(EepromAbstraction& eeprom, uint16_t magicKey) {
	eeprom.write16(0, magicKey);
	saveRecursively(eeprom, rootMenu);
}