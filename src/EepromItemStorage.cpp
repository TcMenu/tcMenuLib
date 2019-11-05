/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "EepromItemStorage.h"
#include "tcMenu.h"
#include "EditableLargeNumberMenuItem.h"

void saveRecursively(EepromAbstraction& eeprom, MenuItem* nextMenuItem) {
	while (nextMenuItem) {
		if (nextMenuItem->getMenuType() == MENUTYPE_SUB_VALUE) {
			saveRecursively(eeprom, ((SubMenuItem*)nextMenuItem)->getChild());
		}
		else if (nextMenuItem->getEepromPosition() == 0xffff) {
			// ignore this one, not got an eeprom entry..
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_TEXT_VALUE) {
			TextMenuItem* textItem = (TextMenuItem*)nextMenuItem;
			eeprom.writeArrayToRom(textItem->getEepromPosition(), (const uint8_t*)(textItem->getTextValue()), textItem->textLength());
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_TIME) {
			TimeFormattedMenuItem* timeItem = reinterpret_cast<TimeFormattedMenuItem*>(nextMenuItem);
			eeprom.writeArrayToRom(timeItem->getEepromPosition(), (const uint8_t*)(timeItem->getUnderlyingData()), 4);
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_IPADDRESS) {
			IpAddressMenuItem* ipItem = reinterpret_cast<IpAddressMenuItem*>(nextMenuItem);
			eeprom.writeArrayToRom(ipItem->getEepromPosition(), ipItem->getIpAddress(), 4);
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_LARGENUM_VALUE) {
			EditableLargeNumberMenuItem* numItem = reinterpret_cast<EditableLargeNumberMenuItem*>(nextMenuItem);
			eeprom.write8(numItem->getEepromPosition(), numItem->getLargeNumber()->isNegative());
			eeprom.writeArrayToRom(numItem->getEepromPosition() + 1, numItem->getLargeNumber()->getNumberBuffer(), 6);
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_INT_VALUE) {
			AnalogMenuItem* intItem = (AnalogMenuItem*)nextMenuItem;
			eeprom.write16(intItem->getEepromPosition(), intItem->getCurrentValue());
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_ENUM_VALUE) {
			EnumMenuItem* valItem = (EnumMenuItem*)nextMenuItem;
			eeprom.write16(valItem->getEepromPosition(), valItem->getCurrentValue());
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
			BooleanMenuItem* valItem = (BooleanMenuItem*)nextMenuItem;
			eeprom.write8(valItem->getEepromPosition(), valItem->getCurrentValue());
		}
		nextMenuItem = nextMenuItem->getNext();
	}
}

void saveMenuStructure(EepromAbstraction & eeprom, uint16_t magicKey) {
	serdebugF2("Save to EEPROM with key ", magicKey);
	eeprom.write16(0, magicKey);
	saveRecursively(eeprom, menuMgr.getRoot());
}

void loadRecursively(EepromAbstraction& eeprom, MenuItem* nextMenuItem) {
	while (nextMenuItem) {
		if (nextMenuItem->getMenuType() == MENUTYPE_SUB_VALUE) {
			loadRecursively(eeprom, ((SubMenuItem*)nextMenuItem)->getChild());
		}
		else if (nextMenuItem->getEepromPosition() == 0xffff) {
			// ignore this one, not got an eeprom entry..
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_TEXT_VALUE) {
			TextMenuItem* textItem = reinterpret_cast<TextMenuItem*>(nextMenuItem);
			eeprom.readIntoMemArray((uint8_t*)textItem->getTextValue(), textItem->getEepromPosition(), textItem->textLength());
			textItem->cleanUpArray();
			textItem->setSendRemoteNeededAll();
			textItem->setChanged(true);
			textItem->triggerCallback();
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_TIME) {
			TimeFormattedMenuItem* timeItem = reinterpret_cast<TimeFormattedMenuItem*>(nextMenuItem);
			eeprom.readIntoMemArray((uint8_t*)timeItem->getUnderlyingData(), timeItem->getEepromPosition(), 4);
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
		else if (nextMenuItem->getMenuType() == MENUTYPE_LARGENUM_VALUE) {
			EditableLargeNumberMenuItem* numItem = reinterpret_cast<EditableLargeNumberMenuItem*>(nextMenuItem);
			numItem->getLargeNumber()->setNegative(eeprom.read8(numItem->getEepromPosition()));
			eeprom.readIntoMemArray(numItem->getLargeNumber()->getNumberBuffer(), numItem->getEepromPosition() + 1, 6);
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_INT_VALUE) {
			AnalogMenuItem* intItem = (AnalogMenuItem*)nextMenuItem;
			intItem->setCurrentValue(eeprom.read16(intItem->getEepromPosition()));
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_ENUM_VALUE) {
			EnumMenuItem* valItem = (EnumMenuItem*)nextMenuItem;
			valItem->setCurrentValue(eeprom.read16(valItem->getEepromPosition()));
		}
		else if (nextMenuItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
			BooleanMenuItem* valItem = (BooleanMenuItem*)nextMenuItem;
			valItem->setCurrentValue(eeprom.read8(valItem->getEepromPosition()));
		}
		nextMenuItem = nextMenuItem->getNext();
	}
}

void loadMenuStructure(EepromAbstraction & eeprom, uint16_t magicKey) {
	if (eeprom.read16(0) == magicKey) {
		serdebugF2("Load from EEPROM key found ", magicKey);
		MenuItem* nextMenuItem = menuMgr.getRoot();
		loadRecursively(eeprom, nextMenuItem);
	}
	else {
		serdebugF2("EEPROM Key NOT found ", magicKey);
	}
}
