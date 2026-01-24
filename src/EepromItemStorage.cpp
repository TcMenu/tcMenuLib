/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "EepromItemStorage.h"
#include "tcMenu.h"
#include "EditableLargeNumberMenuItem.h"
#include "ScrollChoiceMenuItem.h"
#include "MenuIterator.h"

TcEepromStorageMode tcStorageMode = TC_STORE_ROM_LEGACY;

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
        serlogF3(SER_TCMENU_INFO, "Save to EEPROM with key, mode ", magicKey, tcStorageMode);
    if (tcStorageMode == TC_STORE_ROM_DYNAMIC) {
        DynamicEepromStore dynamic;
        dynamic.saveMenuStructure(eeprom, magicKey);
    } else {
        eeprom->write16(0, magicKey);
        uint16_t maxPos = saveRecursively(eeprom, menuMgr.getRoot());
        if(tcStorageMode == TC_STORE_ROM_WITH_SIZE) {
            eeprom->write16(2, maxPos);
        }
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
    if (tcStorageMode == TC_STORE_ROM_DYNAMIC) {
        DynamicEepromStore dynamic;
        return dynamic.loadMenuStructure(eeprom, magicKey);
    }

	if (eeprom->read16(0) == magicKey) {
        uint16_t maxEntry = (tcStorageMode == TC_STORE_ROM_WITH_SIZE) ? eeprom->read16(2) : 0xFFFE;
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
    if (tcStorageMode == TC_STORE_ROM_DYNAMIC) return false; // cant read single item
    bool tcMenuUseSizedEeprom = tcStorageMode == TC_STORE_ROM_WITH_SIZE;
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
    setEepromStorageMode(ena ? TC_STORE_ROM_WITH_SIZE : TC_STORE_ROM_LEGACY);
}

void setEepromStorageMode(TcEepromStorageMode mode) {
    tcStorageMode = mode;
}

#define MAX_ALLOWABLE 0x7FFF

/**
 * We load items from the eeprom one at a time, each item is in memory has a two byte ID, then two bytes
 * for size, followed by the data for that size. This allows up to 64K of data for each item. An ID and size of
 * zero indicate end of stream
 *
 * | ID | Size | Data      |
 * | 1  | 2    | 0x0001    |
 * | 0  | 0    | none      |
 *
 * @param eeprom
 * @param magicKey
 * @return
 */
bool DynamicEepromStore::loadMenuStructure(EepromAbstraction *eeprom, uint16_t magicKey) {
    if (eeprom->read16(0) != magicKey || tcStorageMode != TC_STORE_ROM_DYNAMIC) {
        return false;
    }
    serlogF2(SER_TCMENU_INFO, "Load dynamic EEPROM with key ", magicKey);

    uint16_t position = 2; // Start after magic key

    while (position < MAX_ALLOWABLE) {
        uint16_t itemId = eeprom->read16(position);
        position += 2;

        uint16_t dataLength = eeprom->read16(position);
        position += 2;

        if (itemId == 0 || dataLength == 0) {
            return true; // End of stored items
        }

        MenuItem *item = getMenuItemById(itemId);
        if (item != nullptr) {
            // Temporarily set the eeprom position to where the data is stored
            loadItemFromRom(eeprom, item, position, dataLength);
        }

        position += dataLength;
    }

    return false;
}

bool DynamicEepromStore::saveMenuStructure(EepromAbstraction *eeprom, uint16_t magicKey) {
    if (eeprom->read16(0) != magicKey || tcStorageMode != TC_STORE_ROM_DYNAMIC) {
        return false;
    }

    uint16_t position = 2;
    MenuItemIterator iterator;;
    MenuItem *next;
    while ((next = iterator.nextItem()) != nullptr) {
        if (next->getEepromPosition() == 0xFFFF) continue;
        size_t written = saveItemDynamically(eeprom, next, position + 2);
        if (written > 0) {
            eeprom->write16(position, next->getId());
            position += written + 2;
        }
    }
    return true;
}

void DynamicEepromStore::loadItemFromRom(EepromAbstraction* eeprom, MenuItem* nextMenuItem, EepromPosition pos, size_t len) {
    if (nextMenuItem->getEepromPosition() == 0xFFFF) return;
    auto menuType = nextMenuItem->getMenuType();
    if (menuType == MENUTYPE_TEXT_VALUE) {
        auto textItem = asTextItem(nextMenuItem);
        eeprom->readCharArrIntoMemArray(const_cast<char *>(textItem->getTextValue()), pos, textItem->textLength());
        textItem->cleanUpArray();
        textItem->setChanged(true);
    }
    else if (menuType == MENUTYPE_TIME) {
        auto timeItem = asTimeItem(nextMenuItem);
        eeprom->readIntoMemArray(reinterpret_cast<uint8_t *>(timeItem->getUnderlyingData()), pos, 4);
        timeItem->setChanged(true);
    }
    else if (menuType == MENUTYPE_DATE) {
        auto dateItem = asDateItem(nextMenuItem);
        eeprom->readIntoMemArray(reinterpret_cast<uint8_t *>(dateItem->getUnderlyingData()), pos, 4);
        dateItem->setChanged(true);
    }
    else if (menuType == MENUTYPE_IPADDRESS) {
        auto ipItem = asIpAddressItem(nextMenuItem);
        eeprom->readIntoMemArray(ipItem->getIpAddress(), pos, 4);
        ipItem->setChanged(true);
    }
    else if (menuType == MENUTYPE_SCROLLER_VALUE) {
        auto scroller = asScrollChoiceItem(nextMenuItem);
        scroller->setCurrentValue(eeprom->read16(pos), true);
    }
    else if (menuType == MENUTYPE_COLOR_VALUE) {
        auto rgb = reinterpret_cast<Rgb32MenuItem*>(nextMenuItem);
        auto data = rgb->getUnderlying();
        data->red = eeprom->read8(pos);
        data->green = eeprom->read8(pos + 1);
        data->blue = eeprom->read8(pos + 2);
        data->alpha = eeprom->read8(pos + 3);
        rgb->setChanged(true);
    }
    else if (menuType == MENUTYPE_LARGENUM_VALUE) {
        auto numItem = asLargeNumberItem(nextMenuItem);
        numItem->getLargeNumber()->setNegative(eeprom->read8(pos));
        eeprom->readIntoMemArray(numItem->getLargeNumber()->getNumberBuffer(), pos + 1, 6);
        numItem->setChanged(true);
    }
    else if (menuType == MENUTYPE_INT_VALUE || menuType == MENUTYPE_ENUM_VALUE || menuType == MENUTYPE_BOOLEAN_VALUE) {
        auto intItem = reinterpret_cast<ValueMenuItem*>(nextMenuItem);
        intItem->setCurrentValue(eeprom->read16(pos), true);
    }
}

size_t DynamicEepromStore::saveItemDynamically(EepromAbstraction *eeprom, MenuItem *nextMenuItem, uint16_t pos) {
    auto menuType = nextMenuItem->getMenuType();
    if (menuType == MENUTYPE_TEXT_VALUE) {
        auto textItem = asTextItem(nextMenuItem);
        eeprom->write16(pos, textItem->textLength());
        eeprom->writeCharArrToRom(pos + 2, textItem->getTextValue(), textItem->textLength());
        return textItem->textLength();
    }
    else if (menuType == MENUTYPE_TIME) {
        auto timeItem = asTimeItem(nextMenuItem);
        eeprom->write16(pos, 4);
        eeprom->writeArrayToRom(pos + 2, reinterpret_cast<const uint8_t *>(timeItem->getUnderlyingData()), 4);
        return 4;
    }
    else if (menuType == MENUTYPE_DATE) {
        auto dateItem = asDateItem(nextMenuItem);
        eeprom->write16(pos, 4);
        eeprom->writeArrayToRom(pos + 2, reinterpret_cast<const uint8_t *>(dateItem->getUnderlyingData()), 4);
        return 4;
    }
    else if (menuType == MENUTYPE_IPADDRESS) {
        auto ipItem = asIpAddressItem(nextMenuItem);
        eeprom->write16(pos, 4);
        eeprom->writeArrayToRom(pos + 2, ipItem->getIpAddress(), 4);
        return 4;
    }
    else if (menuType == MENUTYPE_SCROLLER_VALUE) {
        auto scroller = asScrollChoiceItem(nextMenuItem);
        eeprom->write16(pos, 2);
        eeprom->write16(pos + 2, scroller->getCurrentValue());
        return 2;
    }
    else if (menuType == MENUTYPE_COLOR_VALUE) {
        auto rgb = reinterpret_cast<Rgb32MenuItem*>(nextMenuItem);
        auto data = rgb->getUnderlying();
        eeprom->write16(pos, 4);
        eeprom->write8(pos + 2, data->red);
        eeprom->write8(pos + 3, data->green);
        eeprom->write8(pos + 4, data->blue);
        eeprom->write8(pos + 5, data->alpha);
        rgb->setChanged(true);
        return 4;
    }
    else if (menuType == MENUTYPE_LARGENUM_VALUE) {
        auto numItem = asLargeNumberItem(nextMenuItem);
        eeprom->write16(pos, 8);
        numItem->getLargeNumber()->setNegative(eeprom->read8(pos));
        eeprom->readIntoMemArray(numItem->getLargeNumber()->getNumberBuffer(), pos + 1, 6);
        numItem->setChanged(true);
        return 8;
    }
    else if (menuType == MENUTYPE_INT_VALUE || menuType == MENUTYPE_ENUM_VALUE || menuType == MENUTYPE_BOOLEAN_VALUE) {
        auto intItem = reinterpret_cast<ValueMenuItem*>(nextMenuItem);
        eeprom->write16(pos, 2);
        eeprom->write16(pos + 2, intItem->getCurrentValue());
        return 2;
    }
    return -1;
}
