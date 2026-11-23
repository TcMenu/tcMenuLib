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

    constexpr uint8_t noKeyPressed = 0xFF;
    constexpr uint8_t capsKey = 0xFE;

    class TouchKeyboardRow {
        char column[10] = {};
        uint8_t actualColumns = 0;
        int eachKeySize = 0;
        uint8_t pressedKey = 0;
    public:
        TouchKeyboardRow( ) = default;
        TouchKeyboardRow(const TouchKeyboardRow& ) = default;
        TouchKeyboardRow& operator=(const TouchKeyboardRow& ) = default;

        TouchKeyboardRow(const char* columnKeys, uint8_t keyCount, int width) {
            memcpy(column, columnKeys, keyCount);
            actualColumns = keyCount;
            eachKeySize = width / keyCount;
            pressedKey = 0xFF;
        }

        void drawRow(GraphicsDeviceRenderer& renderer, ItemDisplayProperties& properties, int yLocationStart, int height);
        int getTextHeight(DeviceDrawableHelper& helper) const;

        uint8_t touchInRow(const Coord& touch, bool touched);
        [[nodiscard]] uint8_t getNumKeys() const { return actualColumns; }
    };

    class TouchKeyboardLayout {
        TouchKeyboardRow rows[5];
        Coord size;
        int16_t eachKeyHeight;
        int16_t screenHeight;
        uint8_t actualRows;
    public:
        explicit TouchKeyboardLayout(const Coord& size) : rows{}, size(size), eachKeyHeight(), screenHeight(size.y),actualRows(0) {}

        TouchKeyboardLayout* withRow(const char* rowKeys, uint8_t keys) {
            if (actualRows < 5 && keys > 0 && keys <= 12) {
                rows[actualRows] = TouchKeyboardRow(rowKeys, keys, size.x);
                actualRows++;
            }
            return this;
        }

        uint8_t touchInRow(const Coord& touch, bool touched) {
            if (eachKeyHeight <= 0 || touch.y < (screenHeight - (eachKeyHeight * actualRows))) {
                return 0xFF; // outside of range.
            }
            int yPos = internal_max(0, touch.y - (screenHeight - (eachKeyHeight * actualRows)));
            return rows[internal_min(yPos / eachKeyHeight, 3)].touchInRow(touch, touched);
        }

        void calculateKeyHeight(DeviceDrawableHelper& helper);

        void drawRows(GraphicsDeviceRenderer& renderer, ItemDisplayProperties& properties, int yLocation) {
            auto yLocationStart = yLocation;
            for (int i=0; i<actualRows; ++i) {
                rows[i].drawRow(renderer, properties, yLocationStart, eachKeyHeight);
                yLocationStart += eachKeyHeight;
            }
        }

        [[nodiscard]] int getRequiredHeight() const {
            return actualRows * eachKeyHeight;
        }

        [[nodiscard]] int getKeysPerRow() const {
            return rows[0].getNumKeys();
        }
    };

    class KeyboardTouchEditor {
    protected:
        EditableMultiPartMenuItem* menuItem = nullptr;
        bool capsLock = false;
    public:
        virtual void keypress(TcTouchKeyboardController* controller, char ch) = 0;
        void initFor(MenuItem* m) {
            menuItem = reinterpret_cast<EditableMultiPartMenuItem*>(m);
            menuItem->beginMultiEdit();
            menuItem->nextPart();
        }

        virtual ~KeyboardTouchEditor() = default;
        [[nodiscard]] bool isCapsDown() const { return capsLock; }
    };

    class SingleValuePerFieldTouchEditor : public KeyboardTouchEditor {
    public:
        SingleValuePerFieldTouchEditor* initSingle(MenuItem* item);
        void keypress(TcTouchKeyboardController* controller, char ch) override;
    };

    class SummingValueKeyboardTouchEditor : public KeyboardTouchEditor {
        int currentValueBeingEdited = 0;
    public:
        SummingValueKeyboardTouchEditor* initSumming(MenuItem* item);
        void keypress(TcTouchKeyboardController* controller, char ch) override;
    };

    class TcTouchKeyboardController : public CustomDrawing, public TouchObserver {
        MenuTouchScreenManager *touchScreenManager = nullptr;
        TcDrawableButton upButton;
        TcDrawableButton downButton;
        TouchKeyboardLayout* keyboardLayout;
        KeyboardTouchEditor* itemEditor{};
        EditableMultiPartMenuItem* itemBeingEdited{};
        GraphicsDeviceRenderer& touchRenderer;

        // these are the default keyboard and editor layouts that are built in
        TouchKeyboardLayout *defNumKeyboard, *defTxtKeyboard;
        SingleValuePerFieldTouchEditor perCharEditor;
        SummingValueKeyboardTouchEditor summingEditor;
        bool dirty = false;
    public:
        TcTouchKeyboardController(TouchKeyboardLayout* defKeyboard, TouchKeyboardLayout* txtKeyboard, GraphicsDeviceRenderer& renderer);
        void setTouchScreenManager(MenuTouchScreenManager* touchScreenManager) { this->touchScreenManager = touchScreenManager; }
        void started(BaseMenuRenderer* currentRenderer) override;
        void reset() override;
        void renderLoop(unsigned int currentValue, RenderPressMode userClick) override;
        void startEditing(MenuItem* menuItem);
        void drawScrollbar(DeviceDrawable* drawable, const Coord& displayDims, int titleHeight);
        void reportTouchUpDown(const Coord& touch) const;
        void stopEditing();
        [[nodiscard]] bool shouldPassThroughTouch() const { return keyboardLayout != nullptr; }

        void touched(const TouchNotification& notification) override;
        void needsRedraw(bool moreToGo);

    protected:
        void renderKeyboard() const;
        void renderItem();
    };

    TcTouchKeyboardController* defaultTouchKeyboardController(const Coord& size, GraphicsDeviceRenderer& renderer);

}
#endif
