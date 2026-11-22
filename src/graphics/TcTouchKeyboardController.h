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
        char column[12] = {};
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
    };

    class TouchKeyboardLayout {
        TouchKeyboardRow rows[4];
        Coord size;
        int16_t eachKeyHeight;
        int16_t screenHeight;
        uint8_t actualRows;
    public:
        explicit TouchKeyboardLayout(const Coord& size) : rows{}, size(size), eachKeyHeight(), screenHeight(size.y),actualRows(0) {}

        TouchKeyboardLayout* withRow(const char* rowKeys, uint8_t keys) {
            if (actualRows < 4 && keys > 0 && keys <= 12) {
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
    };

    class KeyboardTouchEditor {
    protected:
        EditableMultiPartMenuItem* menuItem = nullptr;
        int editIndex = 0;
    public:
        virtual void keypress(TcTouchKeyboardController* controller, char ch) = 0;
        KeyboardTouchEditor* initFor(MenuItem* m) {
            menuItem = reinterpret_cast<EditableMultiPartMenuItem*>(m);
            editIndex = 0;
            menuItem->beginMultiEdit();
            menuItem->nextPart();
            return this;
        }

        virtual ~KeyboardTouchEditor() = default;
    };

    class SingleValuePerFieldTouchEditor : public KeyboardTouchEditor {
    public:
        void initSingle(MenuItem* item) { initFor(item); }
        void keypress(TcTouchKeyboardController* controller, char ch) override;
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

    class TcTouchKeyboardController : public CustomDrawing, public TouchObserver {
        MenuTouchScreenManager *touchScreenManager = nullptr;
        TcDrawableButton upButton;
        TcDrawableButton downButton;
        TouchKeyboardLayout* keyboardLayout;
        KeyboardTouchEditor* itemEditor{};
        EditableMultiPartMenuItem* itemBeingEdited{};
        GraphicsDeviceRenderer& touchRenderer;
        ItemDisplayProperties& properties;

        // these are the default keyboard and editor layouts that are built in
        TouchKeyboardLayout* defKeyboard;
        SingleValuePerFieldTouchEditor perCharEditor;
        SummingValueKeyboardTouchEditor summingEditor;
        bool dirty = false;
    public:
        TcTouchKeyboardController(TouchKeyboardLayout* defKeyboard, GraphicsDeviceRenderer& renderer, ItemDisplayProperties& properties);
        void setTouchScreenManager(MenuTouchScreenManager* touchScreenManager) { this->touchScreenManager = touchScreenManager; }
        void started(BaseMenuRenderer* currentRenderer) override;
        void reset() override;
        void renderLoop(unsigned int currentValue, RenderPressMode userClick) override;
        void startEditing(MenuItem* menuItem, KeyboardTouchEditor* actEditor);
        void drawScrollbar(DeviceDrawable* drawable, const Coord& displayDims, int titleHeight);
        void reportTouchUpDown(const Coord& touch) const;
        void stopEditing();
        [[nodiscard]] bool shouldPassThroughTouch() const { return keyboardLayout != nullptr; }

        void touched(const TouchNotification& notification) override;
        void needsRedraw(bool moreToGo);

    protected:
        void renderKeyboard();
        void renderItem();
    };

    TcTouchKeyboardController* defaultTouchKeyboardController(const Coord& size, GraphicsDeviceRenderer& renderer, ItemDisplayProperties& properties);

}
#endif
