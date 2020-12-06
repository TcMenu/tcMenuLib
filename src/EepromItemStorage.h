/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _EEPROM_ITEM_STORAGE_H_
#define _EEPROM_ITEM_STORAGE_H_

/**
 * @file EepromItemStorage.h
 * this file contains a series of helper methods for loading and saving menu item to eeprom.
 */

#include "EepromAbstraction.h"

class MenuItem;

/**
 * Save a menu structure to the EEPROM storage device passed in, such that all editable menu items with valid EEPROM
 * address (IE not -1). The eeprom structure will be saved after the first two bytes which are the magic key. This key
 * is so that we know the structure is valid when loading back
 * @param eeprom  the EEPROM device to save to
 * @param magicKey the magic key to store, will be validated on loading back.
 */
void saveMenuStructure(EepromAbstraction* eeprom, uint16_t magicKey = 0xfade);

/**
 * Loads a menu structure back from EEPROM storage into the menu items, but only if the magic key in the first two
 * bytes matches exactly.
 * @param eeprom  the EEPROM storage to load from
 * @param magicKey the key to check against, only loaded if the key matches.
 */
bool loadMenuStructure(EepromAbstraction* eeprom, uint16_t magicKey = 0xfade);

/**
 * Loads a single menu item back from storage, this can be used to selectively load critical menu items before
 * the setupMenu() call is made, but care must be taken to ensure that the callbacks for these items do not rely
 * on any menu infrastructure whatsoever.
 * @param eeprom  the EEPROM storage to load from
 * @param theItem the menu item to try and load if the magic key matches
 * @param magicKey the key to check against, only loaded if the key matches.
 */
bool loadMenuItem(EepromAbstraction* eeprom, MenuItem* theItem, uint16_t magicKey = 0xfade);

#endif //_EEPROM_ITEM_STORAGE_H_