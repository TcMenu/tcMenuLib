/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "EepromItemStorage.h"
#include "tcMenu.h"
#include "EditableLargeNumberMenuItem.h"
#include "ScrollChoiceMenuItem.h"
#include "MenuIterator.h"

bool tcMenuUseSizedEeprom = false;

uint16_t saveRecursively(EepromAbstraction* eeprom, MenuItem* nextMenuItem) {
    uint16_t lastItemSaved = 0;
    while (nextMenuItem) {
        if (nextMenuItem->getMenuType() == MENUTYPE_SUB_VALUE) {
            lastItemSaved = internal_max(lastItemSaved, saveRecursively(eeprom, ((SubMenuItem *) nextMenuItem)->getChild()));
        } else {
            saveMenuItem(eeprom, nextMenuItem);
            if(nextMenuItem->getEepromPosition() != 0xFFFF) {
                lastItemSaved = internal_max(lastItemSaved, nextMenuItem->getEepromPosition());
            }
        }
        nextMenuItem = nextMenuItem->getNext();
    }
    return lastItemSaved;
}

void saveMenuItem(EepromAbstraction* eeprom, MenuItem* nextMenuItem) {
    if (nextMenuItem->getEepromPosition() == 0xffff) {
        // ignore this one, not got an eeprom entry..
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_TEXT_VALUE) {
        auto textItem = (TextMenuItem*)nextMenuItem;
        eeprom->writeArrayToRom(textItem->getEepromPosition(), (const uint8_t*)(textItem->getTextValue()), textItem->textLength());
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_TIME) {
        auto timeItem = reinterpret_cast<TimeFormattedMenuItem*>(nextMenuItem);
        eeprom->writeArrayToRom(timeItem->getEepromPosition(), (const uint8_t*)(timeItem->getUnderlyingData()), 4);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_DATE) {
        auto* dateItem = reinterpret_cast<DateFormattedMenuItem*>(nextMenuItem);
        eeprom->writeArrayToRom(dateItem->getEepromPosition(), (const uint8_t*)(dateItem->getUnderlyingData()), 4);

    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_IPADDRESS) {
        auto ipItem = reinterpret_cast<IpAddressMenuItem*>(nextMenuItem);
        eeprom->writeArrayToRom(ipItem->getEepromPosition(), ipItem->getIpAddress(), 4);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_LARGENUM_VALUE) {
        auto numItem = reinterpret_cast<EditableLargeNumberMenuItem*>(nextMenuItem);
        eeprom->write8(numItem->getEepromPosition(), numItem->getLargeNumber()->isNegative());
        eeprom->writeArrayToRom(numItem->getEepromPosition() + 1, numItem->getLargeNumber()->getNumberBuffer(), 6);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_SCROLLER_VALUE) {
        auto scroller = reinterpret_cast<ScrollChoiceMenuItem*>(nextMenuItem);
        eeprom->write16(scroller->getEepromPosition(), scroller->getCurrentValue());
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_COLOR_VALUE) {
        auto rgb = reinterpret_cast<Rgb32MenuItem*>(nextMenuItem);
        auto data = rgb->getUnderlying();
        eeprom->write8(rgb->getEepromPosition(), data->red);
        eeprom->write8(rgb->getEepromPosition() + 1, data->green);
        eeprom->write8(rgb->getEepromPosition() + 2, data->blue);
        eeprom->write8(rgb->getEepromPosition() + 3, data->alpha);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_INT_VALUE) {
        auto intItem = (AnalogMenuItem*)nextMenuItem;
        eeprom->write16(intItem->getEepromPosition(), intItem->getCurrentValue());
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_ENUM_VALUE) {
        auto valItem = (EnumMenuItem*)nextMenuItem;
        eeprom->write16(valItem->getEepromPosition(), valItem->getCurrentValue());
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
        auto valItem = (BooleanMenuItem*)nextMenuItem;
        eeprom->write8(valItem->getEepromPosition(), valItem->getCurrentValue());
    }
}

void saveMenuStructure(EepromAbstraction* eeprom, uint16_t magicKey) {
	serlogF2(SER_TCMENU_INFO, "Save to EEPROM with key ", magicKey);
	eeprom->write16(0, magicKey);
	uint16_t maxPos = saveRecursively(eeprom, menuMgr.getRoot());
    if(tcMenuUseSizedEeprom) {
        eeprom->write16(2, maxPos);
    }
}

void loadSingleItem(EepromAbstraction* eeprom, MenuItem* nextMenuItem) {
    if (nextMenuItem->getEepromPosition() == 0xffff) {
        // ignore this one, not got an eeprom entry..
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_TEXT_VALUE) {
        auto textItem = reinterpret_cast<TextMenuItem*>(nextMenuItem);
        eeprom->readIntoMemArray((uint8_t*)textItem->getTextValue(), textItem->getEepromPosition(), textItem->textLength());
        textItem->cleanUpArray();
        textItem->setChanged(true);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_TIME) {
        auto timeItem = reinterpret_cast<TimeFormattedMenuItem*>(nextMenuItem);
        eeprom->readIntoMemArray((uint8_t*)timeItem->getUnderlyingData(), timeItem->getEepromPosition(), 4);
        timeItem->setChanged(true);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_DATE) {
        auto dateItem = reinterpret_cast<DateFormattedMenuItem*>(nextMenuItem);
        eeprom->readIntoMemArray((uint8_t*)dateItem->getUnderlyingData(), dateItem->getEepromPosition(), 4);
        dateItem->setChanged(true);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_IPADDRESS) {
        auto ipItem = reinterpret_cast<IpAddressMenuItem*>(nextMenuItem);
        eeprom->readIntoMemArray(ipItem->getIpAddress(), ipItem->getEepromPosition(), 4);
        ipItem->setChanged(true);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_SCROLLER_VALUE) {
        auto scroller = reinterpret_cast<ScrollChoiceMenuItem*>(nextMenuItem);
        scroller->setCurrentValue(eeprom->read16(scroller->getEepromPosition()), true);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_COLOR_VALUE) {
        auto rgb = reinterpret_cast<Rgb32MenuItem*>(nextMenuItem);
        auto data = rgb->getUnderlying();
        data->red = eeprom->read8(rgb->getEepromPosition());
        data->green = eeprom->read8(rgb->getEepromPosition() + 1);
        data->blue = eeprom->read8(rgb->getEepromPosition() + 2);
        data->alpha = eeprom->read8(rgb->getEepromPosition() + 3);
        rgb->setChanged(true);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_LARGENUM_VALUE) {
        auto numItem = reinterpret_cast<EditableLargeNumberMenuItem*>(nextMenuItem);
        numItem->getLargeNumber()->setNegative(eeprom->read8(numItem->getEepromPosition()));
        eeprom->readIntoMemArray(numItem->getLargeNumber()->getNumberBuffer(), numItem->getEepromPosition() + 1, 6);
        numItem->setChanged(true);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_INT_VALUE) {
        auto intItem = (AnalogMenuItem*)nextMenuItem;
        intItem->setCurrentValue(eeprom->read16(intItem->getEepromPosition()), true);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_ENUM_VALUE) {
        auto valItem = (EnumMenuItem*)nextMenuItem;
        valItem->setCurrentValue(eeprom->read16(valItem->getEepromPosition()), true);
    }
    else if (nextMenuItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
        auto valItem = (BooleanMenuItem*)nextMenuItem;
        valItem->setCurrentValue(eeprom->read8(valItem->getEepromPosition()), true);
    }
}

void loadRecursively(EepromAbstraction* eeprom, MenuItem* nextMenuItem, uint16_t maxLoadPosition) {
	while (nextMenuItem) {
		if (nextMenuItem->getMenuType() == MENUTYPE_SUB_VALUE) {
			loadRecursively(eeprom, ((SubMenuItem*)nextMenuItem)->getChild(), maxLoadPosition);
		}
		else {
            uint16_t romLoc = nextMenuItem->getEepromPosition();
            if(romLoc <= maxLoadPosition) {
                loadSingleItem(eeprom, nextMenuItem);
            } else if(romLoc != 0xFFFF) {
                serlogF4(SER_TCMENU_DEBUG, "MenuItem EEPROM load skipped ", romLoc, maxLoadPosition, nextMenuItem->getId());
            }
		}
		nextMenuItem = nextMenuItem->getNext();
	}
}

bool loadMenuStructure(EepromAbstraction* eeprom, uint16_t magicKey) {
	if (eeprom->read16(0) == magicKey) {
        uint16_t maxEntry = (tcMenuUseSizedEeprom) ? eeprom->read16(2) : 0xFFFE;
		serlogFHex(SER_TCMENU_INFO, "Load from EEPROM key found ", magicKey);
		MenuItem* nextMenuItem = menuMgr.getRoot();
		loadRecursively(eeprom, nextMenuItem, maxEntry);
		return true;
	}
	else {
		serlogFHex(SER_WARNING, "EEPROM Key NOT found ", magicKey);
		return false;
	}
}

bool loadMenuItem(EepromAbstraction* eeprom, MenuItem* theItem, uint16_t magicKey) {
    if (eeprom->read16(0) == magicKey && (!tcMenuUseSizedEeprom || eeprom->read16(2) <= theItem->getEepromPosition())) {
        loadSingleItem(eeprom, theItem);
        return true;
    }
    else {
        return false;
    }
}

void triggerAllChangedCallbacks() {
    getParentRootAndVisit(menuMgr.getRoot(), [](MenuItem* item) {
        if(item->isChanged(0) && item->getEepromPosition() != 0xffff) {
            item->triggerCallback();
        }
    });
}

void setSizeBasedEEPROMStorageEnabled(bool ena) {
    tcMenuUseSizedEeprom = ena;
}
