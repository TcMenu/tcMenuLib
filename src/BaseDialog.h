/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _BASE_DIALOG_H_
#define _BASE_DIALOG_H_

/**
 * @file BaseDialog.h
 * 
 * The definition of the base dialog object, that can be used with any display type. These are designed
 * to handle simple user informational pages and questions.
 * 
 * Dialogs are a core function of tcMenu, and must be supported on all available platforms.
 */

/**
 * The types of button that can be passed to setButton for button 1 and button 2
 */
enum ButtonType: byte { BTNTYPE_NONE = 0, BTNTYPE_OK, BTNTYPE_ACCEPT, BTNTYPE_CANCEL, BTNTYPE_CLOSE };

class BaseDialog;
/**
 * If you need to capture the finished state of the dialog, then you create a function to this spec
 * and it will be called back once the dialog is dismissed. Example:
 * `void completedCallback(ButtonType buttonPressed);`
 */
typedef void (*CompletedHandlerFn)(ButtonType buttonPressed, void* yourData);

#define DLG_FLAG_SMALLDISPLAY 0
#define DLG_FLAG_INUSE 1
#define DLG_FLAG_CAN_SEND_REMOTE 2 
#define DLG_FLAG_REMOTE_0 3
#define DLG_FLAG_REMOTE_1 4
#define DLG_FLAG_REMOTE_2 5
#define DLG_FLAG_REMOTE_3 6
#define DLG_FLAG_REMOTE_4 7

#define DLG_FLAG_REMOTE_MASK 0xf8

#define DLG_VISIBLE 'S'
#define DLG_HIDDEN 'H'
#define DLG_ACTION 'A'

class TagValueRemoteConnector; // forward reference

/**
 * A dialog is able to take over the display completely in order to present the user with information
 * or a question. Similar to a message box on desktop platforms. These allow for the simplest of user
 * interactions in a display independent way. This base class has most of the functions needed to
 * handle the non-rendering related activities. Never create one directly, always use the instance
 * that's available from the renderer using `renderer.getDialog()`.
 */
class BaseDialog {
protected:
    const char *headerPgm;
    CompletedHandlerFn completedHandler;
    void* userData;
    ButtonType button1;
    ButtonType button2;
    uint8_t lastBtnVal;
    uint8_t flags;
    MenuRedrawState needsDrawing;
public:
    /**
     * Create the base dialog and clear down all the fields
     */
    BaseDialog();
    virtual ~BaseDialog() {}

    /**
     * Sets the buttons that are available for pressing. One of the ButtonType definitions.
     * You can optionally define if 0 or 1 should be the default.
     * @param btn1 the first button type (can be BTNTYPE_NONE, and btn2 becomes the only button)
     * @param btn2 the second button type
     * @param defVal optionally, set which button is active (0 based).
     */
    void setButtons(ButtonType btn1, ButtonType btn2, int defVal = 0);

    /**
     * Create a dialog that takes over the display and presents the header and
     * buffer, with the buttons set up as per `setButtons`
     */
    void show(const char* headerPgm, bool allowRemote, CompletedHandlerFn completedHandler = NULL);
    
    /**
     * You can set an item of data that will be passed to the callback when executed.
     * @param data a pointer to your own data that will be given back to you in the call back.
     */
    void setUserData(void *data) { this->userData = data; }

    /**
     * Copy text into the buffer, that will be displayed along with the header.
     * @param sz the text to copy
     */
    void copyIntoBuffer(const char* sz);

    /**
     * Close the current dialog by taking off the display and clearing the inuse flag.
     */
    void hide();

    /** Indicates that this dialog is presently in use */
    bool isInUse() {return bitRead(flags, DLG_FLAG_INUSE);}

    /** Indicates that this menu is on a very limited display size */
    bool isCompressedMode() {return bitRead(flags, DLG_FLAG_SMALLDISPLAY);}

    /**
     * Called by the renderer to draw our dialog, in a game loop style as per all tcMenu
     * rendering.
     * @param currentValue the value of the encoder used to handle which button is active.
     * @param userClicked indicates when the user clicks an item.
     */
    void dialogRendering(unsigned int currentValue, bool userClicked);

    bool isRemoteUpdateNeeded(int remote) { return bitRead(flags, remote + DLG_FLAG_REMOTE_0) && bitRead(flags, DLG_FLAG_CAN_SEND_REMOTE); }
    void setRemoteUpdateNeeded(int remote, bool b) { bitWrite(flags, remote + DLG_FLAG_REMOTE_0, b); }
    void setRemoteUpdateNeededAll() { flags |= DLG_FLAG_REMOTE_MASK; }
    void clearRemoteUpdateNeededAll() { flags &= ~DLG_FLAG_REMOTE_MASK; }
    void setRemoteAllowed(bool allowed) {bitWrite(flags, DLG_FLAG_CAN_SEND_REMOTE, allowed); }

    void encodeMessage(TagValueRemoteConnector* remote);
    void remoteAction(ButtonType type);
protected:
    /**
     * Sets the inuse flag to true or false
     */
    void setInUse(bool b) {bitWrite(flags, DLG_FLAG_INUSE, b);}

    /**
     * Needs to be overriden by each leaf renderer implementation in order to draw the actual
     * dialog onto the display.
     * @param currentValue the encoder currentValue
     */
    virtual void internalRender(int currentValue)=0;

    /**
     * Set if redraw is needed
     * @param b true for redraw needed
     */
    void setNeedsDrawing(bool b) {
        if(b && needsDrawing == MENUDRAW_COMPLETE_REDRAW) return;
        needsDrawing = b ? MENUDRAW_EDITOR_CHANGE : MENUDRAW_NO_CHANGE;
    }

    /**
     * Finds the button that is currently selected
     * @param the current value of the encoder
     * @return the active button
     */
    ButtonType findActiveBtn(unsigned int currentValue);

    /**
     * Copies button text into the buffer provided by data, based on the button
     * number (0 or 1) and the current value of the encoder.
     * @param data the buffer to copy into, must fit at least the largest button copyButtonText
     * @param buttonNum the button number either 0, 1
     * @param currentValue current value from encoder.
     * @return true if this button is active, otherwise false.
     */
    bool copyButtonText(char* data, int buttonNum, int currentValue);
};

#endif //_BASE_DIALOG_H_
