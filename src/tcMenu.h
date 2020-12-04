/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_MANAGER_H
#define TCMENU_MANAGER_H

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

/**
 * Used with the change notification method on MenuManager to be notified when there is an important event on the
 * menu.. Implement this class in order to receive edit and structure change notifications.
 */
class MenuManagerObserver {
public:
    /**
     * Indicates that the menu structure has changed in a way that requires a new bootstrap and complete redraw. For
     * example when a new menu item is added, or when static values such as the name or info block data change.
     */
    virtual void structureHasChanged()=0;

    /**
     * This method is called when editing is started with the menu manager, you can prevent editing by returning false.
     * @param item the item that is about to start editing
     * @return true to allow editing, otherwise false
     */
    virtual bool menuEditStarting(MenuItem* item)=0;

    /**
     * This method indicates that editing has completed editing, it is different to the menu item callback in that
     * it is only called when the menu is edited. You cannot prevent completion, but you could present a dialog
     * if the value was incorrectly adjusted.
     * @param item the item that has finished editing.
     */
    virtual void menuEditEnded(MenuItem* item)=0;
};

/**
 * This is used to simulate the old commit hook callback.
 */
class CommitCallbackObserver : public MenuManagerObserver {
private:
    MenuCallbackFn commitCb;
public:
    CommitCallbackObserver(MenuCallbackFn callbackFn) {
        commitCb = callbackFn;
    }

    void structureHasChanged() override {}
    bool menuEditStarting(MenuItem*) override { return true; }
    void menuEditEnded(MenuItem* item) override {
        commitCb(item->getId());
    }
};

#ifndef MAX_MENU_NOTIFIERS
#define MAX_MENU_NOTIFIERS 4
#endif

/**
 * MenuManager ties together all the parts of the menu app, it looks after the menu structure that's being presented,
 * the renderer, security, and optionally an eeprom.
 */
class MenuManager {
private:
	MenuItem* rootMenu;
	MenuRenderer* renderer;
	MenuItem* currentRoot;
	MenuItem* currentEditor;
	SecuredMenuPopup* securedMenuPopup;
	AuthenticationManager *authenticationManager;
    EepromAbstraction* eepromRef;
    MenuManagerObserver* structureNotifier[MAX_MENU_NOTIFIERS];
public:
	MenuManager();

	/**
	 * Initialise the menu manager to use a hardware rotary encoder
	 * @param renderer the renderer used for drawing
	 * @param root the first menu item
	 * @param encoderPinA encoder A pin
	 * @param encorerPinB encoder B pin
	 * @param encoderButton the OK button for the menu select / edit action
	 */
	void initForEncoder(MenuRenderer* renderer, MenuItem* root, pinid_t encoderPinA, pinid_t encoderPinB, pinid_t encoderButton);
	
	/**
	 * Initialise for up down and OK button, instead of using hardware changeEncoderPrecision
	 * @param renderer the renderer used for drawing
	 * @param root the first menu item
	 * @param downPin the button for down
	 * @param upPin the button on up
	 * @param okPin the OK button for the menu select / edit action
	 */
	void initForUpDownOk(MenuRenderer* renderer, MenuItem* root, pinid_t downPin, pinid_t upPin, pinid_t okPin);

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
    void setBackButton(pinid_t backButtonPin);

    /**
     * YOu can add a next button that generally performs the next or right function
     * @param nextButtonPin the pin to which the next button is assigned
     */
    void setNextButton(pinid_t nextButtonPin);

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
     * This is a special callback, one per menu that indicates when a commit has taken place rather
     * than when there has been a change. It uses the same callback signature as the standard change
     * callback.
     * @param commitCallback the callback to be notified when there is a commit event. 
     */
    void setItemCommittedHook(MenuCallbackFn commitCallback) {
        addChangeNotification(new CommitCallbackObserver(commitCallback));
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
	void setItemsInCurrentMenu(int size, int offs = 0);

	EepromAbstraction* getEepromAbstraction() { return eepromRef; }

	/**
	 * Sets the global eeprom reference that tcMenu and other apps can use throughout the program.
	 * @param globalRom
	 */
	void setEepromRef(EepromAbstraction* globalRom) {
	    eepromRef = globalRom;
	}

	/**
	 * Used during initialisation to load the previously stored state. Only if the magic key matches at location 0.
	 * It will also set the global eeprom variable like calling setEepromRef().
	 */
	void load(EepromAbstraction& eeprom, uint16_t magicKey = 0xfade, TimerFn onEepromEmpty = nullptr);


    /**
     * Used during initialisation to load the previously stored state. Only if the magic key matches at location 0.
     * This version requires that you have first set the eeprom abstraction using setEepromRef().
     */
    void load(uint16_t magicKey = 0xfade, TimerFn onEepromEmpty = nullptr);


    /**
	 * Saves the menu using the EEPROM ref that was set up either during load, or by calling setEepromRef(). To use
	 * this version you must ensure the eeprom was previously set.
	 * The magic key is saved first, followed by each item that has an eeprom location set. Only changes are saved because
	 * before each write we check if the value has actually changed.
	 * @param magicKey the key that indicates the values are valid.
	 */
	void save(uint16_t magicKey = 0xfade) { if(eepromRef) saveMenuStructure(eepromRef, magicKey); }

	/**
	 * Call to save all item values into eeprom. The magic key is saved at location 0 if not already set. This is a
	 * lazy save that reads the eeprom values first, and only saves to eeprom when there are changes. Use this version
	 * when you want to override the eeprom used
	 */
	void save(EepromAbstraction& eeprom, uint16_t magicKey = 0xfade) { saveMenuStructure(&eeprom, magicKey); }

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
		return switches.getEncoder() != nullptr ? switches.getEncoder()->getCurrentReading() : 0;
	}

	/**
	 * gets the secure menu popup instance that can be used to ask for
	 * a password on the device.
	 * @return the single instance of this popup.
	 */
	SecuredMenuPopup* secureMenuInstance();

	void stopEditingCurrentItem(bool checkMultiPart);

	/**
	 * Adds a menu item into the tree directly after the existing item provided. Never add an item that's already in
	 * the tree. Don't forget that if you use silent, to call menuStructureChanged() after you're done adding items.
	 * @param existing where in the tree the new item is to be added.
	 * @param toAdd the item that should be added, must not be in the tree already.
	 * @param silent do not run the structure changed callback.
	 */
	void addMenuAfter(MenuItem* existing, MenuItem* toAdd, bool silent = false);

	/**
	 * Call this method after making any structural change to the menu tree. For example adding a new menu item,
	 * changing the item name or static data such as size parameters. For example: modifying an items name, etc.
	 */
    void notifyStructureChanged();

    /**
     * Adds an observer that will be notified of structure changes in the menu
     * @param observer to be notified of structure changes.
     */
    void addChangeNotification(MenuManagerObserver* observer);
protected:
	void setupForEditing(MenuItem* item);
	void actionOnCurrentItem(MenuItem * toEdit);

    void notifyEditEnd(MenuItem *pItem);
    bool notifyEditStarting(MenuItem *pItem);
};

/**
 * The global instance of MenuManager, always use this instance.
 */
extern MenuManager menuMgr;

#endif // defined header file
