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
#include "RemoteAuthentication.h"
#include "EepromItemStorage.h"

// forward reference
class MenuRenderer;

/**
 * @file tcMenu.h
 * 
 * The menu manager is responsible for managing a set of menu items, and is configured with a renderer and input
 * capability in order to present the menu. Remotes generally also the the menu manager to find out about the
 * overall structure
 */

class SecuredMenuPopup;

class MenuManager {
private:
	MenuItem* rootMenu;
	MenuRenderer* renderer;
	MenuItem* currentRoot;
	MenuItem* currentEditor;
	SecuredMenuPopup* securedMenuPopup;
	AuthenticationManager *authenticationManager;

public:
	MenuManager() {
		this->currentEditor = NULL;
		this->currentRoot = NULL;
		this->renderer = NULL;
		this->rootMenu = NULL;
		this->securedMenuPopup = NULL;
		this->authenticationManager = NULL;
	}

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
     * You can add a back button that generally performs the back or left function
     * @param backButtonPin the pin on which the back button is assigned.
     */
    void setBackButton(uint8_t backButtonPin);

    /**
     * YOu can add a next button that generally performs the next or right function
     * @param nextButtonPin the pin to which the next button is assigned
     */
    void setNextButton(uint8_t nextButtonPin);

    /**
     * Sometimes you need to use the menu structure before everything is initialised, in this case
     * you can call this function early on to set up the root menu item.
     */
    void setRootMenu(MenuItem* menuItem) {
        rootMenu = menuItem;
    }

	/** 
	 * If you want to be able to secure sub menus you have to provide an authentication manger.
	 * Normally create a global authentication manager and pass it into menuMgr and your remote
	 * server if you have one.
	 * @param manager the authentication manager to use for the submenu pin lock.
	 */
	void setAuthenticator(AuthenticationManager* manager) { authenticationManager = manager; }

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
     * This provides support for next and back (left, right) functionality by making the menu
     * structure respond to such functions in a reasonable way.
     * @param dirIsBack true for back (left), false for next (right)
     */
    void performDirectionMove(bool dirIsBack);

	/**
	 * Sets the number of items and offset of the items in the current menu
	 * @param size the number of items
	 * @param offs the offset within the items
	 */
	void setItemsInCurrentMenu(int size, int offs = 0) { switches.changeEncoderPrecision(size, offs); }

	/**
	 * Used during initialisation to load the previously stored state. Only if the magic key matches at location 0.
	 */
	void load(EepromAbstraction& eeprom, uint16_t magicKey = 0xfade) { loadMenuStructure(eeprom, magicKey); }

	/**
	 * Call to save all item values into eeprom. The magic key is saved at location 0 if not already set. This is a
	 * lazy save that reads the eeprom values first, and only saves to eeprom when there are changes.
	 */
	void save(EepromAbstraction& eeprom, uint16_t magicKey = 0xfade) { saveMenuStructure(eeprom, magicKey); }

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

	/**
	 * Get the current MenuItem that is being edited (or null)
	 */
	MenuItem* getCurrentEditor() { return currentEditor; }

	/**
	 * Change the editor control that is receiving changes to apply to the menu item.
	 * @param editor the new editor or NULL
	 */
	void setCurrentEditor(MenuItem* editor);

	/**
	 * Set the root item of either the first menu or any sub menu
	 * @param theItem the item to become the current root of the menu.
	 */
	void setCurrentMenu(MenuItem* theItem);

	/**
	 * Get the current sub menu that is being rendered
	 */
	MenuItem* getCurrentMenu() { return currentRoot; }

	/**
	 * Get the parent of the current menu clearing all active flags too
	 */
	MenuItem* getParentAndReset();

	/**
	 * returns the range of the encoder or other device that is providing the
	 * equivalent function as the encoder.
	 */
	int getCurrentRangeValue() {
		return switches.getEncoder() != NULL ? switches.getEncoder()->getCurrentReading() : 0;
	}

	/**
	 * gets the secure menu popup instance that can be used to ask for
	 * a password on the device.
	 * @return the single instance of this popup.
	 */
	SecuredMenuPopup* secureMenuInstance();

	void stopEditingCurrentItem();
protected:
	void setupForEditing(MenuItem* item);
	void actionOnCurrentItem(MenuItem * toEdit);
};

/**
 * The global instance of MenuManager, always use this instance.
 */
extern MenuManager menuMgr;

#endif // defined header file
