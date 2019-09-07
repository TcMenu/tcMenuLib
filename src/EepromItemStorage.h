#ifndef _EEPROM_ITEM_STORAGE_H_
#define _EEPROM_ITEM_STORAGE_H_

/**
 * @file EepromItemStorage.h
 * this file contains a series of helper methods for loading and saving menu item to eeprom.
 */

#include "EepromAbstraction.h"

void saveMenuStructure(EepromAbstraction& eeprom, uint16_t magicKey = 0xfade);
void loadMenuStructure(EepromAbstraction& eeprom, uint16_t magicKey = 0xfade);

#endif //_EEPROM_ITEM_STORAGE_H_