/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _BASE_RENDERERS_H_
#define _BASE_RENDERERS_H_

#include "tcMenu.h"

/**
 * @file BaseRenderers.h
 * This file contains the common code for rendering onto displays, making it much easier to implement
 * a renderer.
 */

/** the frequency at which the screen is redrawn (only if needed). */
#define SCREEN_DRAW_INTERVAL 250
/** the number of ticks the menu should reset to defaults after not being used */
#define TICKS_BEFORE_DEFAULTING 120

/**
 *  Used to take over rendering for a period of time. Normally one calls renderer.takeOverDisplay(..) 
 * with a reference to a function meeting this spec. 
 */
typedef void (*RendererCallbackFn)(bool userClicked);

/**
 * Title widgets allow for drawing small graphics in the title area, for example connectivity status
 * of the wifi network, if a remote connection to the menu is active. They are in a linked list so
 * as to make storage as efficient as possible. Chain them together using the constructor or setNext().
 * Image icons should be declared in PROGMEM.
 * 
 * Thread / interrupt safety: get/setCurrentState & isChanged can be called from threads / interrupts
 * 
 * Currently, only graphical renderers can use title widgets.
 */
class TitleWidget {
private:
	const uint8_t **iconData;
	volatile uint8_t currentState;
	volatile bool changed;
	uint8_t width;
	uint8_t height;
	TitleWidget* next;
public:
	/** Construct a widget with its icons and size */
	TitleWidget(const uint8_t** icons, uint8_t width, uint8_t height, TitleWidget* next = NULL);
	/** Get the current state that the widget represents */
	uint8_t getCurrentState() {return currentState;}
	/** gets the current icon data */
	const uint8_t* getCurrentIcon() {return iconData[currentState]; changed = false;}
	/** sets the current state of the widget, there must be an icon for this value */
	void setCurrentState(uint8_t state) {this->currentState = state;this->changed = true;}
	/** checks if the widget has changed since last drawn. */
	bool isChanged() {return this->changed;}
	/** gets the width of all the images */
	uint8_t getWidth() {return width;}
	/** gets the height of all the images */
	uint8_t getHeight() {return height;}
	/** gets the next widget in the chain or null */
	TitleWidget* getNext() {return next;}
	/** sets the next widget in the chain */
	void setNext(TitleWidget* next) {this->next = next;}
};

/** 
 * Each display must have a renderer, even if it is the NoRenderer, the NoRenderer is for situations
 * where the control is performed exclusively by a remote device.
 */
class MenuRenderer {
public:
	/**
	 * This is called when the menu manager is created, to let the display perform one off tasks
	 * to prepare the display for use
	 */
	virtual void initialise() = 0;

	/**
	 * Tell the renderer that a new item has become active
	 * @index the new active index
	 */
	virtual void activeIndexChanged(uint8_t index) = 0;

	/**
	 * Renders keep track of which item is currently being edited, only one item can be edited at
	 * once
	 * @returns the current item being edited
	 */
	virtual MenuItem* getCurrentEditor() = 0;

	/**
	 * Sets the current editor, only one item can be edited at once.
	 */
	virtual void onSelectPressed(MenuItem* editItem) = 0;

	/**
	 * Renderers work out which submenu is current.
	 * @returns the current sub menu
	 */
	virtual MenuItem* getCurrentSubMenu() = 0;

	/** virtual destructor is required by the language */
	virtual ~MenuRenderer() { }
};

/**
 * Used by renderers to determine how significant a redraw is needed at the next redraw interval.
 * They are prioritised in ascending order, so a more complete redraw has a higher number.
 */
enum MenuRedrawState : byte {
	MENUDRAW_NO_CHANGE = 0, MENUDRAW_EDITOR_CHANGE, MENUDRAW_COMPLETE_REDRAW
};

/**
 * Used by the base renderer, to indicate if you want text formatted left or right justified in
 * the buffer.
 */
enum MenuDrawJustification: byte {
	JUSTIFY_TEXT_LEFT, JUSTIFY_TEXT_RIGHT
};

/**
 * A renderer that does nothing, for cases where there's no display
 */
class NoRenderer : public MenuRenderer {
public:
	virtual void activeIndexChanged(__attribute__((unused)) uint8_t ignored) {  }
	virtual MenuItem* getCurrentSubMenu() { return NULL; }
	virtual MenuItem* getCurrentEditor() { return NULL; }
	virtual void onSelectPressed(__attribute__((unused)) MenuItem* ignored) { }
	virtual void initialise() { }
};

class RemoteMenuItem; // forward reference.

/**
 * This class provides the base functionality that will be required by most implementations
 * of renderer, you can extend this class to add new renderers, it proivides much of the
 * core functionality that's needed by a renderer.
 * 
 * Renderers work in a similar way to a game loop, the render method is repeatedly called
 * and in this loop any rendering or event callbacks should be handled. 
 */
class BaseMenuRenderer : public MenuRenderer {
protected:
	char* buffer;
	uint8_t bufferSize;
	TitleWidget* firstWidget;
	uint8_t ticksToReset;
	MenuRedrawState redrawMode;
	uint8_t lastOffset;
	RendererCallbackFn renderCallback;
	MenuItem* currentRoot;
	MenuItem* currentEditor;

public:
	/**
	 * constructs the renderer with a given buffer size 
	 * @param bufferSize size of text buffer to create
	 */
	BaseMenuRenderer(int bufferSize);
	
	/** 
	 * Destructs the class removing the allocated buffer
	 */
	virtual ~BaseMenuRenderer() {delete buffer;}
	
	/**
	 * Initialise the render setting up tasks
	 */
	virtual void initialise();

	/**
	 * This is the rendering call that must be implemented by subclasses. Generally
	 * it is called a few times a second, and should render the menu, if changes are
	 * detected
	 */
	virtual void render() = 0;

	/**
	 * Get the current MenuItem that is being edited (or null)
	 */
	virtual MenuItem* getCurrentEditor() { return currentEditor; }

	/** 
	 * Get the current sub menu that is being rendered
	 */
	virtual MenuItem* getCurrentSubMenu() { return currentRoot; }

	/**
	 * Used to indicate that the active index has changed, this is used to move
	 * the offset of the menu.
	 * @param index the index offset needed to show the active menu item
	 */
	virtual void activeIndexChanged(uint8_t index);

	/**
	 * When select is pressed this method works out what action to take
	 * @param editor the current editor
	 */
	virtual void onSelectPressed(MenuItem* editor);

	/**
	 * For menu systems that support title widgets, this will allow the first widget.
	 * @param the first widget in a chain of widgets linked by next pointer.
	 */
	void setFirstWidget(TitleWidget* widget);

	/**
	 * Called when the menu has been altered, to reset the countdown to
	 * reset behaviour
	 */
	void menuAltered() { ticksToReset = TICKS_BEFORE_DEFAULTING; }

	/**
	 * In order to take over the display, provide a callback function that will receive
	 * the regular render call backs instead of this renderer.
	 * @param displayFn the callback to render the display
	 */
	void takeOverDisplay(RendererCallbackFn displayFn);

	/**
	 * Call this method to clear custom display rendering after a call to takeOverDisplay.
	 * It will cause a complete repaint of the display.
	 */
	void giveBackDisplay();

	/**
	 * Returns a pointer to the rendering callback
	 */
	RendererCallbackFn getRenderingCallback() { return renderCallback; }

	static BaseMenuRenderer* INSTANCE;

protected:
	/**
	 * Convert a menu item into a textual representation in the buffer
	 * @param item the menu item
	 * @param justification use either JUSTIFY_TEXT_LEFT or JUSTIFY_TEXT_RIGHT
	 */
	void menuValueToText(MenuItem* item, MenuDrawJustification justification);

	/**
	 * Internal method that handles each ticks
	 */
	void handleTicks();

	/**
	 * Sets the type of redraw that is needed
	 * @param state the required redraw
	 */
	void redrawRequirement(MenuRedrawState state) { if (state > redrawMode) redrawMode = state; }
	
	/**
	 * Used to reset the display to it's default state, root menu, nothing being edited
	 */
	void resetToDefault();

	/**
	 * Used to set up a new submenu for display on the renderer
	 * @param newItems the new submenu
	 */
	void prepareNewSubmenu(MenuItem* newItems);

	/**
	 * Find the item at a given position in the current submenu
	 * @param pos the integer offset
	 */
	MenuItem* getItemAtPosition(uint8_t pos);

	/**
	 * set up an item to be edited
	 * @param toEdit the item to edited
	 */
	void setupForEditing(MenuItem* toEdit);

	/**
	 * Find the offset of the currently active item
	 */
	int offsetOfCurrentActive();

	/**
	 * set up a countdown to default back to the submenu
	 */
	void countdownToDefaulting();
	
	void findFirstVisible();

private:
	void menuValueAnalog(AnalogMenuItem* item, MenuDrawJustification justification);
	void menuValueEnum(EnumMenuItem* item, MenuDrawJustification justification);
	void menuValueBool(BooleanMenuItem* item, MenuDrawJustification justification);
	void menuValueExec(MenuItem* item, MenuDrawJustification justification);
	void menuValueBack(BackMenuItem* item, MenuDrawJustification justification);
	void menuValueText(TextMenuItem* item, MenuDrawJustification justification);
	void menuValueRemote(RemoteMenuItem* item, MenuDrawJustification justification);
	void menuValueFloat(FloatMenuItem* item, MenuDrawJustification justification);
};

/** Counts the number of items from this menu item to the end of the list */
uint8_t itemCount(MenuItem* item);

#endif
