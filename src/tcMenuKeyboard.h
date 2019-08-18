/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _TCMENU_KEYBOARD_H_
#define _TCMENU_KEYBOARD_H_

#include <KeyboardManager.h>

/**
 * @file tcMenuKeyboard.h contains a keyboard listener for use with MatrixKeyboardManager that can
 * control a menu system
 */

/** internal enumeration used by the keyboard listener to hold state */
enum MenuEditingKeyMode: byte {
	KEYEDIT_NONE,
	KEYEDIT_ANALOG_EDIT_WHOLE,
	KEYEDIT_ANALOG_EDIT_FRACT,
	KEYEDIT_MULTIEDIT_INT_START = 100,
};

/**
 * An implementation of the key listener that can be used with TcMenu to edit menu items and control
 * the menu. When not in edit mode, the keyboard 0-9 keys can be used to select menu items. Pressing
 * '#' will reset the menu and pressing '*' will do the equivalent of select.
 *
 * When editing:
 * * boolean - pressing 0 = false, pressing any other digit = true
 * * analog - pressing # resets the value, pressing * is a decimal point, 0-9 increase reduce the number
 * * enum - values 0-9 represent the values in the enum
 * * text - the value of the key is added to the string
 * * ip/date - the integer values are editing one as if using the encoder.
 */
class MenuEditingKeyListener : public KeyboardListener {
private:
	WholeAndFraction currentValue;
	MenuItem* currentEditor;
	MenuEditingKeyMode mode;
public:
	MenuEditingKeyListener() {
		currentEditor = NULL;
		mode = KEYEDIT_NONE;
	}
    void keyPressed(char key, bool held) override;
    void keyReleased(char key) override;
private:
	void processSimpleValueKeyPress(ValueMenuItem* item, char key);
	void processAnalogKeyPress(AnalogMenuItem* item, char key);
	void processMultiEditKeyPress(TextMenuItem* item, char key);
	void processIntegerMultiEdit(EditableMultiPartMenuItem<byte[4]>* item, char key);
	void clearState();
};

#endif // _TCMENU_KEYBOARD_H_
