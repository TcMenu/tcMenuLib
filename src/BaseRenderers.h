/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _BASE_RENDERERS_H_
#define _BASE_RENDERERS_H_

#include "tcMenu.h"
#include "RuntimeMenuItem.h"
#include "MenuIterator.h"
#include <TaskManager.h>

/** Checks if a given menu item can have an action performed on it.
 */
bool isItemActionable(MenuItem* item);

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
 * an enumeration of possible values that are given to either custom render functions or used internally
 * by renderers to describe the state of the select button. Values should be self explanatory. High
 * compatability with bool for determining if the button is pressed.
 */
enum RenderPressMode: byte { RPRESS_NONE = 0, RPRESS_PRESSED = 1, RPRESS_HELD = 2 };

/**
 * Used to take over rendering for a period of time. Normally one calls renderer.takeOverDisplay(..) 
 * with a reference to a function meeting this spec. Whenever the callback occurs the current value
 * of the rotary encoder is provided along with the state of the menu select switch.
 * 
 * @param currentValue is the current value of the rotary encoder
 * @param userClicked if the user clicked the select button
 */
typedef void (*RendererCallbackFn)(unsigned int currentValue, RenderPressMode userClicked);

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
#ifdef __AVR__
	const uint8_t* getCurrentIcon() {
        changed = false;
        return (const uint8_t *)pgm_read_ptr(&iconData[currentState]);
    }
    const uint8_t* getIcon(int num) {
        if(num >= maxStateIcons) num = 0;
        return (const uint8_t *)pgm_read_ptr(&iconData[num]);
    }
#else
	const uint8_t* getCurrentIcon() {
        changed = false;
        return iconData[currentState];
    }
    const uint8_t* getIcon(int num) {
        if(num >= maxStateIcons) num = 0;
        return iconData[num];
    }
#endif

	/** sets the current state of the widget, there must be an icon for this value */
	void setCurrentState(uint8_t state) {
        // if outside of allowable icons or value hasn't changed just return.
		if (state >= maxStateIcons || currentState == state) return; 
        
		this->currentState = state;
		this->changed = true;
	}

	/** checks if the widget has changed since last drawn. */
	bool isChanged() {return this->changed;}

    /** Sets the changed flag on this widget */
    void setChanged(bool ch) { changed = ch; }

	/** gets the width of all the images */
	uint8_t getWidth() {return width;}

	/** gets the height of all the images */
	uint8_t getHeight() {return height;}

    /** the maximum state value - ie number of icons */
    uint8_t getMaxValue() { return maxStateIcons; }

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
	 * Allows the select key to be overriden for situations such as dialogs and other
	 * special cases.
	 * @param held true when the select was held down.
	 * @return true to indicate we consumed the event, otherwise false.
	 */
	virtual bool tryTakeSelectIfNeeded(int currentReading, RenderPressMode press) = 0;

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
	bool tryTakeSelectIfNeeded(int, RenderPressMode) override { return false; }
	void initialise() override { }
    BaseDialog* getDialog() override;
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
    ResetCallbackFn resetCallback;
    BaseDialog* dialog;

	RenderPressMode renderFnPressType;
	RendererCallbackFn renderCallback;
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
	 * If this renderer has been taken over, displaying a dialog or needs to do
	 * something special with a button press, then this function can take action
	 * BEFORE anything else
	 * @param editor the current editor
	 * @return Ah it
	 */
	virtual bool tryTakeSelectIfNeeded(int currentReading, RenderPressMode pressMode);

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

	/**
	 * Used to set up a new submenu for display on the renderer
	 * @param newItems the new submenu
	 */
	void prepareNewSubmenu();

	/**
	 * Sets the type of redraw that is needed
	 * @param state the required redraw
	 */
	void redrawRequirement(MenuRedrawState state) { if (state > redrawMode) redrawMode = state; }

protected:
	/**
	 * Convert a menu item into a textual representation in the buffer
	 * @param item the menu item
	 * @param justification use either JUSTIFY_TEXT_LEFT or JUSTIFY_TEXT_RIGHT
	 */
	void menuValueToText(MenuItem* item, MenuDrawJustification justification);
	
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


#endif
