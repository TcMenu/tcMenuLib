/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef __MENU_MANAGER_H
#define __MENU_MANAGER_H

#include <IoAbstraction.h>
#include "tcUtil.h"
#include "MenuItems.h"
#include "RuntimeMenuItem.h"
#include "BaseRenderers.h"
#include "EepromAbstraction.h"

// forward reference
class MenuRenderer;

/**
 * @file tcMenu.h
 * 
 * The menu manager is responsible for managing a set of menu items, and is configured with a renderer and input
 * capability in order to present the menu. Remotes generally also the the menu manager to find out about the
 * overall structure
 */
class MenuManager {
private:
	MenuItem* rootMenu;
	MenuRenderer* renderer;
public:
	/**
	 * Initialise the menu manager to use a hardware rotary encoder
	 * @param renderer the renderer used for drawing
	 * @param root the first menu item
	 * @param encoderPinA encoder A pin
	 * @param encorerPinB encoder B pin
	 * @param encoderButton the OK button for the menu select / edit action
	 */
	void initForEncoder(MenuRenderer* renderer, MenuItem* root, uint8_t encoderPinA, uint8_t encoderPinB, uint8_t encoderButton);
	
	/**
	 * Initialise for up down and OK button, instead of using hardware changeEncoderPrecision
	 * @param renderer the renderer used for drawing
	 * @param root the first menu item
	 * @param upPin the button on up
	 * @param downPin the button for down
	 * @param okPin the OK button for the menu select / edit action
	 */
	void initForUpDownOk(MenuRenderer* renderer, MenuItem* root, uint8_t upPin, uint8_t downPin, uint8_t okPin);

	/**
	 * Initialise in situations where local input is not needed or where a custom type of input is needed
     * that is not one of the common types.
     * 
     * In the case of custom input make sure that:
     * 
     * 1. something will call `menuMgr.onMenuSelect(bool held)` when the select button is pressed
     * 2. something will call `menuMgr.valueChanged(int value)` when the current value goes up / down.
     * 
	 * @param renderer the renderer used for drawing
	 * @param root the first menu item
	 */
	void initWithoutInput(MenuRenderer* renderer, MenuItem* root);

    /**
     * Sometimes you need to use the menu structure before everything is initialised, in this case
     * you can call this function early on to set up the root menu item.
     */
    void setRootMenu(MenuItem* menuItem) {
        rootMenu = menuItem;
    }

	/** 
	 * called when the rotary encoder has moved to a new position to update the menu
	 * @param value the new changed value
	 */
	void valueChanged(int value);

	/**
	 * Called when the OK button has been pressed
	 * @param held if the button is held down
	 */
	void onMenuSelect(bool held);

	/**
	 * Sets the number of items and offset of the items in the current menu
	 * @param size the number of items
	 * @param offs the offset within the items
	 */
	void setItemsInCurrentMenu(int size, int offs = 0) { switches.changeEncoderPrecision(size, offs); }

	/**
	 * Used during initialisation to load the previously stored state. Only if the magic key matches at location 0.
	 */
	void load(EepromAbstraction& eeprom, uint16_t magicKey = 0xfade);

	/**
	 * Call to save all item values into eeprom. The magic key is saved at location 0 if not already set. This is a
	 * lazy save that reads the eeprom values first, and only saves to eeprom when there are changes.
	 */
	void save(EepromAbstraction& eeprom, uint16_t magicKey = 0xfade);

	/**
	 * Find the menu item that is currently active.
	 */
	MenuItem* findCurrentActive();

	/**
	 * Get the root of all menus, the first menu item basically
	 */
	MenuItem* getRoot() { return rootMenu; }

    /**
     * Get the renderer that this menu is using
     */
    MenuRenderer* getRenderer() { return renderer; }
};

/**
 * The global instance of MenuManager, always use this instance.
 */
extern MenuManager menuMgr;

#endif // defined header file
