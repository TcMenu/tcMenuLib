/*
* Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file TcTouchKeyboardController.h
 * @brief Contains the implementation of a touch keyboard controller used to handle scrolling in touch mode and also
 *        to present a keyboard on demand.
 */

#ifndef TC_TOUCH_KEYBOARD_CONTROLLER_H
#define TC_TOUCH_KEYBOARD_CONTROLLER_H
#include "TcDrawableButton.h"

#ifndef TC_KEYBOARD_BUTTON_SIZE
#define TC_KEYBOARD_BUTTON_SIZE 24
#endif

namespace tcgfx {
    class TcTouchKeyboardController;

    class TouchKeyboardRow {
        char column[12];
        uint8_t actualColumns;
        uint8_t eachKeySize;
        uint8_t pressedKey;
        KeyPressState state;
    public:
        TouchKeyboardRow( ) = default;
        TouchKeyboardRow(const TouchKeyboardRow& ) = default;
        TouchKeyboardRow& operator=(const TouchKeyboardRow& ) = default;

        TouchKeyboardRow(const char* columnKeys, uint8_t keyCount, int width) {
            memcpy(column, columnKeys, keyCount);
            actualColumns = keyCount;
            eachKeySize = width / keyCount;
            pressedKey = 0xFF;
            state = NOT_PRESSED;
        }

        void drawRow(GraphicsDeviceRenderer& renderer, int yLocationStart, int height) {
            for (int i=0; i<actualColumns; ++i) {

            }
        }

        uint8_t touchInRow(const Coord& touch) {
            if (touch.x < 0 || touch.x >= eachKeySize * actualColumns) {
                return 0xFF;
            }

            if (pressedKey == 0xFF) {
                pressedKey = column[touch.x / eachKeySize];
                return pressedKey;
            } else if (touch.x >= eachKeySize * pressedKey && touch.x < eachKeySize * (pressedKey + 1)) {
                state = BUTTON_HELD;
                return 0xFF;
            } else {
                state = NOT_PRESSED;
                pressedKey = 0xFF;
                return 0xFF;
            }
        }
    };

    class TouchKeyboardLayout {
        TouchKeyboardRow rows[4];
        Coord size;
        uint8_t actualRows;
        uint8_t eachKeyHeight;
    public:
        explicit TouchKeyboardLayout(const Coord& size) : rows{}, size(size), actualRows(0), eachKeyHeight(0) {}
        TouchKeyboardLayout* withRow(const char* rowKeys, uint8_t keys) {
            if (actualRows < 4) {
                rows[actualRows] = TouchKeyboardRow(rowKeys, keys, size.x);
                actualRows++;
            }
            return this;
        }

        uint8_t touchInRow(const Coord& touch) {
            if (touch.y < 0 || touch.y >= eachKeyHeight * actualRows) {
                return 0xFF; // outside of range.
            }
            return rows[touch.y / eachKeyHeight].touchInRow(touch);
        }

        void drawRows(GraphicsDeviceRenderer& renderer, int yLocation) {
            auto yLocationStart = yLocation;
            for (int i=0; i<actualRows; ++i) {
                rows[i].drawRow(renderer, yLocationStart, eachKeyHeight);
                yLocationStart += eachKeyHeight;
            }
        }

        [[nodiscard]] int getRequiredHeight() const {
            return actualRows * eachKeyHeight;
        }
    };

    class KeyboardTouchEditor {
    protected:
        EditableMultiPartMenuItem* menuItem = nullptr;
        int editIndex = 0;
    public:
        virtual void keypress(TcTouchKeyboardController* controller, char ch) = 0;
        KeyboardTouchEditor* initFor(MenuItem* MenuItem) {
            menuItem = reinterpret_cast<EditableMultiPartMenuItem*>(MenuItem);
            editIndex = 0;
            menuItem->beginMultiEdit();
            return this;
        }

        virtual ~KeyboardTouchEditor() = default;
    };

    class SingleValuePerFieldTouchEditor : public KeyboardTouchEditor {
    public:
        void initSingle(MenuItem* item) { initFor(item); }
        void keypress(TcTouchKeyboardController* controller, char ch) override {}
    };

    class SummingValueKeyboardTouchEditor : public KeyboardTouchEditor {
        int currentValueBeingEdited = 0;
    public:
        void initSumming(MenuItem* item) {
            initFor(item);
            currentValueBeingEdited = 0;
        }
        void keypress(TcTouchKeyboardController* controller, char ch) override {}
    };

    class TcTouchKeyboardController {
        TcDrawableButton upButton;
        TcDrawableButton downButton;
        TouchKeyboardLayout* keyboardLayout;
        KeyboardTouchEditor* itemEditor{};
        GraphicsDeviceRenderer& touchRenderer;
        ItemDisplayProperties& properties;

        // these are the default keyboard and editor layouts that are built in
        TouchKeyboardLayout* defKeyboard;
        SingleValuePerFieldTouchEditor perCharEditor;
        SummingValueKeyboardTouchEditor summingEditor;
    public:
        TcTouchKeyboardController(TouchKeyboardLayout* defKeyboard, GraphicsDeviceRenderer& renderer, ItemDisplayProperties& properties);
        void initialiseScrollbar(int titleHeight);
        void startEditing(MenuItem* menuItem, KeyboardTouchEditor* actEditor);
        void drawKeyboardController(DeviceDrawable* drawable, const Coord& displayDims, int titleHeight);
        void reportTouch(const Coord& touch) const;
        void stopEditing();
    };

    TcTouchKeyboardController* defaultTouchKeyboardController(const Coord& size, GraphicsDeviceRenderer& renderer, ItemDisplayProperties& properties);

}
#endif
