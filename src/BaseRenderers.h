/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _BASE_RENDERERS_H_
#define _BASE_RENDERERS_H_

#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include <TaskManager.h>

// forward reference.
class BaseDialog;

/**
 * @file BaseRenderers.h
 * This file contains the common code for rendering onto displays, making it much easier to implement
 * a renderer.
 */

/** the frequency at which the screen is redrawn (only if needed). */
#define SCREEN_DRAW_INTERVAL 250
/** the number of ticks the menu should reset to defaults after not being used */
#define SECONDS_IN_TICKS (1000 / SCREEN_DRAW_INTERVAL)
/** The maximum number of ticks that */
#define MAX_TICKS 0xffff

bool isItemActionable(MenuItem* item);

/**
 * Used to take over rendering for a period of time. Normally one calls renderer.takeOverDisplay(..) 
 * with a reference to a function meeting this spec. Whenever the callback occurs the current value
 * of the rotary encoder is provided along with the state of the menu select switch.
 * 
 * @param currentValue is the current value of the rotary encoder
 * @param userClicked if the user clicked the select button
 */
typedef void (*RendererCallbackFn)(unsigned int currentValue, bool userClicked);

/**
 * Used to indicate when the renderer is about to be reset, you could use this to do custom rendering
 * when the menu is not active, for example taking over the display until some condition is met.
 */
typedef void (*ResetCallbackFn)();

/**
 * Title widgets allow for drawing small graphics in the title area, for example connectivity status
 * of the wifi network, if a remote connection to the menu is active. They are in a linked list so
 * as to make storage as efficient as possible. Chain them together using the constructor or setNext().
 * Image icons should be declared using PGM_TCM rather than prog mem to be compatible with all boards.
 * 
 * Thread / interrupt safety: get/setCurrentState & isChanged can be called from threads / interrupts
 * 
 * Currently, only graphical renderers can use title widgets.
 */
class TitleWidget {
private:
	const uint8_t* const* iconData;
	volatile uint8_t currentState;
	volatile bool changed;
	uint8_t width;
	uint8_t height;
	uint8_t maxStateIcons;
	TitleWidget* next;
public:
	/** Construct a widget with its icons and size */
	TitleWidget(const uint8_t* const* icons, uint8_t maxStateIcons, uint8_t width, uint8_t height, TitleWidget* next = NULL);
	/** Get the current state that the widget represents */
	uint8_t getCurrentState() {return currentState;}
	/** gets the current icon data */
	const uint8_t* getCurrentIcon() {
        changed = false;
#ifdef __AVR__
        return pgm_read_ptr(&iconData[currentState]);
#else
        return iconData[currentState];
#endif
    }
	/** sets the current state of the widget, there must be an icon for this value */
	void setCurrentState(uint8_t state) {
        // if outside of allowable icons or value hasn't changed just return.
		if (state >= maxStateIcons || currentState == state) return; 
        
		this->currentState = state;
		this->changed = true;
	}
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

enum RendererType: byte { RENDER_TYPE_NOLOCAL, RENDERER_TYPE_BASE };

/** 
 * Each display must have a renderer, even if it is the NoRenderer, the NoRenderer is for situations
 * where the control is performed exclusively by a remote device.
 */
class MenuRenderer {
protected:
    static MenuRenderer* theInstance;
    char *buffer;
	uint8_t bufferSize;
    RendererType rendererType;
public:
    MenuRenderer(RendererType rendererType, int bufferSize) { 
        buffer = new char[bufferSize+1]; 
        this->bufferSize = bufferSize; 
        this->rendererType = rendererType;
    }
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
     * Indicates to the renderer that select  has been held in
     */
    virtual void onHold() = 0;

	/**
	 * Renderers work out which submenu is current.
	 * @returns the current sub menu
	 */
	virtual MenuItem* getCurrentSubMenu() = 0;

    /**
     * Gets the dialog instance that is associated with this renderer or NULL if
     * this renderer cannot display dialogs (only NoRenderer case).
     */
    virtual BaseDialog* getDialog() = 0;

	/** virtual destructor is required by the language */
	virtual ~MenuRenderer() { }

    /* Gets the rendering instance */
    static MenuRenderer* getInstance() { return theInstance; }

    /**
     * Gets the buffer that is used internally for render buffering.
     */
    char* getBuffer() {return buffer;}
    
    /**
     * Gets the buffer size of the buffer
     */
    uint8_t getBufferSize() {return bufferSize;}

    /** Returns if this is a no display type renderer or a base renderer type. */
    RendererType getRendererType() { return rendererType; }
};

/**
 * Used by renderers to determine how significant a redraw is needed at the next redraw interval.
 * They are prioritised in ascending order, so a more complete redraw has a higher number.
 */
enum MenuRedrawState: byte {
	MENUDRAW_NO_CHANGE = 0, MENUDRAW_EDITOR_CHANGE, MENUDRAW_COMPLETE_REDRAW
};

enum MenuDrawJustification: byte { JUSTIFY_TEXT_LEFT, JUSTIFY_TEXT_RIGHT };

/**
 * A renderer that does nothing, for cases where there's no display
 */
class NoRenderer : public MenuRenderer {
private:
    BaseDialog* dialog;
public:
    NoRenderer() : MenuRenderer(RENDER_TYPE_NOLOCAL, 20) { MenuRenderer::theInstance = this; dialog = NULL;}
    ~NoRenderer() override { }
	void activeIndexChanged(uint8_t /*ignored*/) override {  }
	MenuItem* getCurrentSubMenu() override { return NULL; }
	MenuItem* getCurrentEditor() override { return NULL; }
	void onSelectPressed(MenuItem* /*ignored*/) override { }
	void initialise() override { }
    void onHold() override { }
    BaseDialog* getDialog() override;
    char* getBuffer();
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
class BaseMenuRenderer : public MenuRenderer, Executable {
protected:
	uint8_t lastOffset;
	uint16_t ticksToReset;
    uint16_t resetValInTicks;
	MenuRedrawState redrawMode;
	TitleWidget* firstWidget;
	RendererCallbackFn renderCallback;
    ResetCallbackFn resetCallback;
	MenuItem* currentRoot;
	MenuItem* currentEditor;
    BaseDialog* dialog;
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
     * Adjust the default reset interval of 30 seconds. Maximum value is 60 seconds.
     * At this point the reset callback is called and the menu is reset to root with
     * no present editor.
     * @param resetTime
     */
    void setResetIntervalTimeSeconds(uint16_t interval) { 
        unsigned int ticks = interval * SECONDS_IN_TICKS;
        resetValInTicks = ticks; 
    }

    /**
     * Sets the callback that will receive reset events when the menu has not been edited
     * for some time.
     */
    void setResetCallback(ResetCallbackFn resetFn) { resetCallback = resetFn; }

    /**
     * Called by taskManager when we are scheduled
     */
    virtual void exec();

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
     * When the select button is held in, the action is defined here
     */
    void onHold() override;

	/**
	 * For menu systems that support title widgets, this will allow the first widget.
	 * @param the first widget in a chain of widgets linked by next pointer.
	 */
	void setFirstWidget(TitleWidget* widget);

	/**
	 * Called when the menu has been altered, to reset the countdown to
	 * reset behaviour
	 */
	void menuAltered() { ticksToReset = resetValInTicks; }

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

	/**
	 * Used to reset the display to it's default state, root menu, nothing being edited
	 */
	void resetToDefault();

protected:
    /**
     * Gets the parent of the current menu.
     * @return the parent for the current root.
     */
    MenuItem* getParentAndReset();
	/**
	 * Convert a menu item into a textual representation in the buffer
	 * @param item the menu item
	 * @param justification use either JUSTIFY_TEXT_LEFT or JUSTIFY_TEXT_RIGHT
	 */
	void menuValueToText(MenuItem* item, MenuDrawJustification justification);

	/**
	 * Sets the type of redraw that is needed
	 * @param state the required redraw
	 */
	void redrawRequirement(MenuRedrawState state) { if (state > redrawMode) redrawMode = state; }
	
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
	
private:
	void menuValueAnalog(AnalogMenuItem* item, MenuDrawJustification justification);
	void menuValueEnum(EnumMenuItem* item, MenuDrawJustification justification);
	void menuValueBool(BooleanMenuItem* item, MenuDrawJustification justification);
	void menuValueRuntime(RuntimeMenuItem* item, MenuDrawJustification justification);
	void menuValueFloat(FloatMenuItem* item, MenuDrawJustification justification);
};

/** Counts the number of items from this menu item to the end of the list */
uint8_t itemCount(MenuItem* item);

#endif
