/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_MANAGER_H
#define TCMENU_MANAGER_H

#include <IoAbstraction.h>
#include "tcUtil.h"
#include "MenuItems.h"
#include "MenuHistoryNavigator.h"
#include "RuntimeMenuItem.h"
#include "BaseRenderers.h"
#include "RemoteAuthentication.h"
#include "EepromItemStorage.h"

// forward reference
class MenuRenderer;

/**
 * @file tcMenu.h
 * @brief The menu manager is responsible for managing a set of menu items, and is configured with a renderer and input
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

    /**
     * Optionally,this method can be overridden to be told when a new item has been activated. This callback will
     * be called after the active item is changed.
     * @param newActive
     */
    virtual void activeItemHasChanged(MenuItem* newActive) {}
};

/**
 * This is used to simulate the old commit hook callback.
 */
class CommitCallbackObserver : public MenuManagerObserver {
private:
    MenuCallbackFn commitCb;
public:
    explicit CommitCallbackObserver(MenuCallbackFn callbackFn) {
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
 * Defines an encoder wrapping override, mainly used internally by menu item when encoder wrapping overrides are added
 */
class EncoderWrapOverride {
private:
    menuid_t itemId;
    bool overrideValue;
public:
    EncoderWrapOverride() = default;
    EncoderWrapOverride(const EncoderWrapOverride &other) = default;
    EncoderWrapOverride &operator=(const EncoderWrapOverride &other) = default;
    EncoderWrapOverride(menuid_t itemId, bool overrideValue) : itemId(itemId), overrideValue(overrideValue) {}

    menuid_t getKey() const { return itemId; }

    menuid_t getMenuId() const { return itemId; }
    bool getOverrideValue() const { return overrideValue; }
};

/**
 * This class is somewhat internal to menuMgr although party exposed, it allows the menu manager to store hints to
 * help rendering classes to present information about how an item is being edited, and if it is multi part which
 * index in the array is being edited.
 */
class CurrentEditorRenderingHints {
public:
    /**
     * Espresses the intent of the current editor operation, so that a suitable cursor can be displayed.
     */
    enum EditorRenderingType {
        EDITOR_REGULAR = 0,
        EDITOR_WHOLE_ONLY = 0x0001, EDITOR_FRACTION_ONLY = 0x0002,
        EDITOR_RUNTIME_TEXT = 0x0004,
        EDITOR_OVERRIDE_LOCK = 0x8000
    };
private:
    EditorRenderingType renderingType = EDITOR_REGULAR;
    int editStart = 0;
    int editEnd = 0;
public:
    CurrentEditorRenderingHints() = default;
    void changeEditingParams(EditorRenderingType ty, int startOffset, int endOffset);
    void lockEditor(bool lock) { renderingType = lock ? EDITOR_OVERRIDE_LOCK : EDITOR_REGULAR; }
    EditorRenderingType getEditorRenderingType() const { return renderingType; }
    int getStartIndex() const { return editStart; }
    int getEndIndex() const { return editEnd; }
};

/**
 * MenuManager ties together all the parts of the menu app, it looks after the menu structure that's being presented,
 * the renderer, security, and optionally an eeprom.
 */
class MenuManager {
private:
    tcnav::MenuNavigationStore navigator{};
	MenuItem* currentEditor;
	MenuRenderer* renderer;
	SecuredMenuPopup* securedMenuPopup;
	AuthenticationManager *authenticationManager;
    EepromAbstraction* eepromRef;
    MenuManagerObserver* structureNotifier[MAX_MENU_NOTIFIERS];
    bool useWrapAroundByDefault = false;
    BtreeList<menuid_t, EncoderWrapOverride> encoderWrapOverrides;
    CurrentEditorRenderingHints renderingHints;
public:
    static SubMenuItem ROOT;
	MenuManager();

    /**
     * This method tells tcMenu to always attempt to use wrap around with the encoder, IE when value reaches max
     * it goes back to min, and vica-versa. The default is NOT to wrap.
     * @param wrapAround should values wrap
     */
    void setUseWrapAroundEncoder(bool wrapAround) {
        useWrapAroundByDefault = wrapAround;
    }

    /**
     * In many cases some items would suit wrap around encoder mode and some would not. For example nearly all enum
     * and scroll items suit wrap around. Items that require many turns of the encoder also suit it, but volume for
     * example really wouldn't suit it. Imagine hitting 0 volume and wrapping back to maximum in an amplifier.
     *
     * To set the default wrap around mode call `setUseWrapAroundEncoder` first, the default is no wrapping.
     *
     * @param item the item for which the override is to be applied.
     * @param override true if wrapping is needed, otherwise false.
     */
    void addEncoderWrapOverride(MenuItem& item, bool override);

    /**
     * Check if the encoder should use wrapping for a given menu item or not. We use a pointer here because it can
     * legally be nullptr too.
     *
     * @param menuId the menu id obtained using getId() on the menu item
     * @return true if wrapping is to be used, otherwise false
     */
    bool isWrapAroundEncoder(MenuItem* item);

	/**
	 * Initialise the menu manager to use a hardware rotary encoder
	 * @param renderer the renderer used for drawing
	 * @param root the first menu item
	 * @param encoderPinA encoder A pin
	 * @param encorerPinB encoder B pin
	 * @param encoderButton the OK button for the menu select / edit action
	 * @param type optionally, you can provide the encoder type, only really needed for quarter cycle encoders.
	 */
	void initForEncoder(MenuRenderer* renderer, MenuItem* root, pinid_t encoderPinA, pinid_t encoderPinB, pinid_t encoderButton, EncoderType type = FULL_CYCLE);
	
	/**
	 * Initialise for up down and OK button, instead of using hardware changeEncoderPrecision
	 * @param renderer the renderer used for drawing
	 * @param root the first menu item
	 * @param downPin the button for down
	 * @param upPin the button on up
	 * @param okPin the OK button for the menu select / edit action
	 * @param speed the repeat key interval in ticks (lower is faster), default 20
	 */
	void initForUpDownOk(MenuRenderer* renderer, MenuItem* root, pinid_t downPin, pinid_t upPin, pinid_t okPin, int speed=20);

    /**
     * Initialise for up a 4 way digital joystick that has up, down, left and right. With this it is possible for the
     * joystick to properly handle scrolling, sideways scrolling, and regular change.
     * @param renderer the renderer used for drawing
     * @param root the first menu item
     * @param downPin the button for down
     * @param upPin the button on up
     * @param leftPin the button on left
     * @param rightPin the button on right
     * @param okPin the OK button for the menu select / edit action
     * @param speed the repeat key interval in ticks (lower is faster), default 20
     */
    void initFor4WayJoystick(MenuRenderer* renderer, MenuItem* root, pinid_t downPin, pinid_t upPin, pinid_t leftPin,
                             pinid_t rightPin, pinid_t okPin, int speed=20);

    /**
     * Initialise for up a 2 button joystick where the up doubles as back, and the down doubles as OK when held.
     * @param renderer the renderer used for drawing
     * @param root the first menu item
     * @param upPin the button on up
     * @param downPin the button for down
     */
    void initForTwoButton(MenuRenderer* renderer, MenuItem* root, pinid_t upPin, pinid_t downPin);

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
        navigator.setRootItem(menuItem);
    }

	/** 
	 * If you want to be able to secure sub menus you have to provide an authentication manger.
	 * Normally create a global authentication manager and pass it into menuMgr and your remote
	 * server if you have one.
	 * @param manager the authentication manager to use for the submenu pin lock.
	 */
	void setAuthenticator(AuthenticationManager* manager) { authenticationManager = manager; }

	/**
	 * @return the global authentication manager that has been set for this menu application, or nullptr otherwise
	 */
	AuthenticationManager* getAuthenticator() {
	    if(authenticationManager == nullptr) authenticationManager = new NoAuthenticationManager();
	    return authenticationManager;
	}

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
	MenuItem* getRoot() { return ROOT.getChild(); }

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
     * Set item to be the active item on the renderer. This also ensure any notifications are run too. It does
     * clear any currently active item. This DOES NOT update the encoder, it is only to update the active status.
     * @param item the item to active.
     */
    void setItemActive(MenuItem* item);

	/**
	 * Set the root item of either the first menu or any sub menu
	 * @param theItem the item to become the current root of the menu.
	 */
	void changeMenu(MenuItem* possibleActive=nullptr);

	/**
	 * Make the menu item `theNewItem` the ROOT of the current menu on display, this item should be either the item referred
	 * to by `rootMenuItem()` or the first child item of a submenu, normally obtained by calling `menuSub.getChild()` on the
	 * submenu. When we navigate to a new item, by default menu manager keeps breadcrumbs to be able to go back nicely
	 * afterwards. If you are navigiating to a dynamic menu that should not go into this history, set skipHistory to true.
	 *
	 * There is more information about this online see:
	 * https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menumanager-and-iteration/#navigation-around-menus
	 *
	 * @param theNewItem the first child menu item of the submenu (or root item)
	 * @param possibleActive the item to activate or null for default
	 * @param skipHistory set to true if this menu is custom and should not be stored in the history
	 */
	void navigateToMenu(MenuItem* theNewItem, MenuItem* possibleActive = nullptr, bool skipHistory = false);

	/**
	 * Force a complete reset of the menu
	 */
	void resetMenu(bool completeReset);

	/**
	 * @return the first menu item in the linked list that is being rendered
	 */
	MenuItem* getCurrentMenu() { return navigator.getCurrentRoot(); }

    /**
     * @return the submenu for the currently on display menu. If root is on display it will be nullptr.
     */
	MenuItem* getCurrentSubMenu() { return navigator.getCurrentSubMenu(); }

    /**
     * @return the underlying navigation store that manages navigation activity for this menu manager.
     */
    tcnav::MenuNavigationStore& getNavigationStore() { return navigator; }

	/**
	 * @return the parent of the current menu clearing all active flags too
	 */
	MenuItem* getParentAndReset();

	/**
	 * @return the range of the encoder or other device that is providing the
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

    /**
     * Stop editing the current item, if doMultiPartNext is true and we are editing a multi part field , then this will
     * instead move editing to the next part. Otherwise, editing will end.
     * @param checkMultiPart true to move to the next on multipart instead of ending completely
     */
	void stopEditingCurrentItem(bool doMultiPartNext);

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

    /**
     * This provides support for when lists are on the display, to allow the encoder to be updated so it can present
     * the new items. If the list is not on display, nothing is done.
     * @param runtimeItem
     */
    void recalculateListIfOnDisplay(RuntimeMenuItem* runtimeItem);

    /**
     * This sets rendering hints, so that the renderer knows that something is being edited. When the rendering hints
     * are set, they refer to the item that is currently being edited.
     * @param hint the type of edit taking place
     * @param start the starting point within the text
     * @param end the ending point within the text
     */
    void setEditorHints(CurrentEditorRenderingHints::EditorRenderingType hint, size_t start=0, size_t end=0);
    const CurrentEditorRenderingHints& getEditorHints() { return renderingHints; }

    /**
     * Lock the editor hints such that nobody can change them, useful for complex editing where more than one field is
     * being edited at once.
     * @param locked true to lock, otherwise false.
     */
    void setEditorHintsLocked(bool locked);

    /**
     * Use this with caution. Resets the entire list of observers that have previously been registered.
     */
    void resetObservers();

	/**
	 * Allows you to enable an item for editing directly without going through the UI
	 * @param item the item to become the active edit, the right submenu should have already been enabled first
	 */
	void setupForEditing(MenuItem* item);
protected:
	void actionOnCurrentItem(MenuItem * toEdit);
    void actionOnSubMenu(MenuItem* nextSub);

    void notifyEditEnd(MenuItem *pItem);
    bool notifyEditStarting(MenuItem *pItem);

    void setRootItem(MenuItem *pItem);
};

inline bool editorHintNeedsCursor(CurrentEditorRenderingHints::EditorRenderingType ty) {
    return ty != CurrentEditorRenderingHints::EDITOR_REGULAR && ty != CurrentEditorRenderingHints::EDITOR_OVERRIDE_LOCK;
}

/**
 * The global instance of MenuManager, always use this instance.
 */
extern MenuManager menuMgr;

#endif // TCMENU_MANAGER_H
