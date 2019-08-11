/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * KeyboadManager.h contains the classes needed to deal with editing using matrix keyboards
 */

#ifndef _KEYBOARD_MANGER_H_
#define _KEYBOARD_MANGER_H_

#include <IoAbstraction.h>

/**
 * You create an implementation of this class, or use the standard one in order
 * to get keyboard events when keys are pressed or released.
 */
class KeyboardListener {
public:
    /**
     * A key has been pressed or held down.
     * @param key the character code for the key
     * @param held if held down
     */
    virtual void keyPressed(char key, bool held)=0;
    /**
     * A key has been released
     * @param key the character code of the key
     */
    virtual void keyReleased(char key)=0;
};

/**
 * Keyboard layouts tell the matrix manager the layout of your keyboard matrix. There are two
 * standard ones defined in this file. They are LAYOUT_3X4, LAYOUT_4X4.
 * When creating a class of this type, be sure that your string of keyCode mappings is defined as
 * PROGMEM and at rows * cols in size.
 */
class KeyboardLayout {
private:
    uint8_t rows;
    uint8_t cols;
    const char *pgmLayout;
    uint8_t *pins;
public:
    KeyboardLayout(uint8_t rows, uint8_t cols, const char* pgmLayout) {
        this->rows = rows;
        this->cols = cols;
        this->pgmLayout = pgmLayout;
        this->pins = new uint8_t[rows + cols];
    }

    ~KeyboardLayout() {
        delete pins;
    }

    int numColumns() { return cols; }

    int numRows() { return rows; }

    void setColPin(int col, int pin) {
        if(col < cols) pins[rows + col] = pin;
    }

    void setRowPin(int row, int pin) {
        if(row < rows) pins[row] = pin;
    }

    int getRowPin(int row) {
        return pins[row];
    }

    int getColPin(int col) {
        return pins[rows + col];
    }

    char keyFor(uint8_t row, uint8_t col) {
        if(row < rows && col < cols) return pgm_read_byte_near(&pgmLayout[row * col]);
        else return 0;
    }
};

enum KeyMode: byte {
    KEYMODE_NOT_PRESSED,
    KEYMODE_DEBOUNCE,
    KEYMODE_PRESSED,
    KEYMODE_REPEATING
};

/**
 * Actually handles the activities involved in managing the matrix keyboard. This class handles
 * checking the keys for presses and reporting any keypresses to the registered listener.
 */
class MatrixKeyboardManager : public Executable {
private:
    KeyboardListener* listener;
    KeyboardLayout* layout;
    IoAbstractionRef ioRef;
    char currentKey;
    KeyMode keyMode;
    uint8_t counter;
public:
    MatrixKeyboardManager();
    void initialise(IoAbstractionRef ref, KeyboardLayout* layout, KeyboardListener* listener);
    void exec();
private:
    void setToOuput(int i);
};

/**
 * An implementation of the key listener that attempts to edit menu items.
 */
class MenuEditingKeyListener : KeyboardListener {
    void keyPressed(char key, bool held) override;
    void keyReleased(char key) override;
};

#define LAYOUT_3X4 const char KEYBOARD_STD_3X4_KEYS[] PROGMEM = "987654321#0*"; KeyboardLayout KEYBOARD_STD_3X4(3, 4, KEYBOARD_STD_3X4_KEYS)
#define LAYOUT_4X4 const char KEYBOARD_STD_3X4_KEYS[] PROGMEM = "987A654B321C*0#D"; KeyboardLayout KEYBOARD_STD_3X4(4, 4, KEYBOARD_STD_3X4_KEYS)

#endif // _KEYBOARD_MANGER_H_
