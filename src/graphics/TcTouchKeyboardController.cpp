//
// Created by Dave Cherry on 17/10/2026.
//

#include "TcTouchKeyboardController.h"
#include "stockIcons/directionalIcons.h"
#include "tcMenu.h"
#include "MenuItems.h"
#include "EditableLargeNumberMenuItem.h"

using namespace tcgfx;

DrawableIcon drawUpArrowLgeBitmap(65000U, Coord(16, 20), DrawableIcon::ICON_XBITMAP, tcMenuUpArrowLgeBitmap, nullptr);
DrawableIcon drawDownArrowLgeBitmap(65001U, Coord(16, 20), DrawableIcon::ICON_XBITMAP, tcMenuDownArrowLgeBitmap, nullptr);

static constexpr int KEY_GAP = 2;
static constexpr int KEY_TEXT_PADDING = 4;
static constexpr int EDIT_TEXT_MARGIN = 4;
static constexpr int EDIT_TEXT_GAP = 8;

int TouchKeyboardRow::getTextHeight(DeviceDrawableHelper& helper) const {
    int height = 0;
    for (int i = 0; i < actualColumns; ++i) {
        const char text[2] = { column[i], '\0' };
        int baseline;
        int textHeight = helper.textExtents(text, &baseline).y;
        if (textHeight > height) height = textHeight;
    }
    return height;
}

uint8_t TouchKeyboardRow::touchInRow(const Coord& touch, bool touched) {
    if (touch.x < 0 || touch.x >= eachKeySize * actualColumns) {
        return 0xFF;
    }

    if (touched) {
        pressedKey = column[internal_min(actualColumns - 1, touch.x / eachKeySize)];
        return pressedKey;
    } else {
        return 0xFF;
    }
}

void TouchKeyboardLayout::calculateKeyHeight(DeviceDrawableHelper& helper) {
    int textHeight = 0;
    for (int i = 0; i < actualRows; ++i) {
        int rowHeight = rows[i].getTextHeight(helper);
        if (rowHeight > textHeight) textHeight = rowHeight;
    }
    eachKeyHeight = textHeight + 2 * (KEY_TEXT_PADDING + KEY_GAP);
    if (actualRows && eachKeyHeight > size.y / actualRows) eachKeyHeight = size.y / actualRows;
}

SingleValuePerFieldTouchEditor* SingleValuePerFieldTouchEditor::initSingle(MenuItem* item) {
    initFor(item);
    if (item->getMenuType() == MENUTYPE_LARGENUM_VALUE) {
        if (auto numItem = reinterpret_cast<EditableLargeNumberMenuItem*>(item); numItem->isNegativeAllowed()) {
            numItem->nextPart();
        }
    }

    return this;
}

void SingleValuePerFieldTouchEditor::keypress(TcTouchKeyboardController* controller, char ch) {
    bool moreToGo = true;
    if (menuItem->getMenuType() == MENUTYPE_TEXT_VALUE) {
        if (ch == 0xFE) {
            capsLock = true;
        } else {
            const auto txtItem = reinterpret_cast<TextMenuItem*>(menuItem);
            if (capsLock) {
                ch = toupper(ch);
            }
            txtItem->valueChangedFromKeyboard(ch);
            moreToGo = menuItem->nextPart() != 0;
        }
    } else if (menuItem->getMenuType() == MENUTYPE_LARGENUM_VALUE) {
        const auto lgeItem = reinterpret_cast<EditableLargeNumberMenuItem*>(menuItem);
        if (ch == '+' && lgeItem->isNegativeAllowed()) {
            const auto neg = lgeItem->getLargeNumber()->isNegative() ? 0 : 1;
            lgeItem->setNegativeFlagValue(neg);
        } else if (ch == '>') {
            moreToGo = menuItem->nextPart() != 0;
        } else if (ch >= '0' && ch <= '9') {
            menuItem->valueChanged(ch);
            moreToGo = menuItem->nextPart() != 0;
        }
    }
    controller->needsRedraw(moreToGo);
}

SummingValueKeyboardTouchEditor* SummingValueKeyboardTouchEditor::initSumming(MenuItem* item) {
    initFor(item);
    currentValueBeingEdited = 0;
    return this;
}

void SummingValueKeyboardTouchEditor::keypress(TcTouchKeyboardController* controller, char ch) {
    const auto range = menuItem->getCurrentRange();
    if (ch == '<') {
        currentValueBeingEdited = 0;
        menuItem->valueChanged(0);
        controller->needsRedraw(true);
        return;
    } else if (ch == '>' || ch == '.') {
        currentValueBeingEdited = 0;
        const bool moreToGo = menuItem->nextPart() != 0;
        controller->needsRedraw(moreToGo);
        return;
    } else if (ch < '0' || ch > '9') return;

    const auto num = ch - '0';
    currentValueBeingEdited = currentValueBeingEdited * 10 + num;
    if (range < currentValueBeingEdited) {
        currentValueBeingEdited = 0;
        controller->needsRedraw(true);
    } else {
        menuItem->valueChanged(currentValueBeingEdited);
        controller->needsRedraw(true);
    }
}

void TouchKeyboardRow::drawRow(GraphicsDeviceRenderer& renderer, ItemDisplayProperties& properties, int yLocationStart, int height) {
    auto* drawable = renderer.getDeviceDrawable();
    DeviceDrawableHelper helper(drawable);
    helper.setFontFromParameters(properties.getFont(), properties.getFontMagnification());
    for (int i = 0; i < actualColumns; ++i) {
        if (eachKeySize <= 2 * KEY_GAP || height <= 2 * KEY_GAP) continue;
        Coord where(i * eachKeySize + KEY_GAP, yLocationStart + KEY_GAP);
        Coord size(eachKeySize - 2 * KEY_GAP, height - 2 * KEY_GAP);
        drawable->setDrawColor(properties.getColor(ItemDisplayProperties::HIGHLIGHT1));
        drawable->drawBox(where, size, true);
        const char text[2] = { column[i], '\0' };
        int baseline;
        const Coord textSize = helper.textExtents(text, &baseline);
        helper.drawText(Coord(where.x + (size.x - textSize.x) / 2, where.y + (size.y - textSize.y) / 2),
                        properties.getColor(ItemDisplayProperties::HIGHLIGHT2), text);
    }
}


TcTouchKeyboardController::TcTouchKeyboardController(TouchKeyboardLayout* numKeyboard, TouchKeyboardLayout* txtKeyboard, GraphicsDeviceRenderer& renderer) :
    upButton(&drawUpArrowLgeBitmap), downButton(&drawDownArrowLgeBitmap), keyboardLayout(nullptr),
    touchRenderer(renderer), defNumKeyboard(numKeyboard), defTxtKeyboard(txtKeyboard)  {
}

void TcTouchKeyboardController::started(BaseMenuRenderer* currentRenderer) {
    if (currentRenderer != &touchRenderer || !touchRenderer.getDeviceDrawable()) return;
    auto* drawable = touchRenderer.getDeviceDrawable();
    drawable->startDraw();
    auto* background = touchRenderer.getGraphicsPropertiesFactory().configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM);
    drawable->setDrawColor(background->getColor(ItemDisplayProperties::BACKGROUND));
    drawable->drawBox(Coord(0, 0), drawable->getDisplayDimensions(), true);
    renderItem();
    renderKeyboard();
    drawable->endDraw(true);
}

void TcTouchKeyboardController::reset() {
    stopEditing();
}

void TcTouchKeyboardController::renderLoop(unsigned int currentValue, RenderPressMode userClick) {
    if (dirty) {
        dirty = false;
        const auto dims = touchRenderer.getDeviceDrawable()->getDisplayDimensions();
        const auto bg = touchRenderer.getGraphicsPropertiesFactory().configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM)->getColor(ItemDisplayProperties::BACKGROUND);
        touchRenderer.getDeviceDrawable()->setDrawColor(bg);
        touchRenderer.getDeviceDrawable()->drawBox(Coord(0, 0), Coord(dims.x, (dims.y - keyboardLayout->getRequiredHeight() - 10)), true );
        renderItem();
    }
}

void TcTouchKeyboardController::startEditing(MenuItem* menuItem) {
    if (menuItem == nullptr) {
        serlogF(SER_WARNING, "TchEd-null");
        return;
    } else if (menuItem->getMenuType() == MENUTYPE_TEXT_VALUE || menuItem->getMenuType() == MENUTYPE_LARGENUM_VALUE) {
        itemEditor = perCharEditor.initSingle(menuItem);
    } else if (menuItem->getMenuType() == MENUTYPE_DATE || menuItem->getMenuType() == MENUTYPE_IPADDRESS || menuItem->getMenuType() == MENUTYPE_COLOR_VALUE || menuItem->getMenuType() == MENUTYPE_TIME) {
        itemEditor = summingEditor.initSumming(menuItem);
    } else {
        // it has been called with an unsupported menu type, only runtime editable items can use this path
        return;
    }

    // we put the item into edit mode
    auto* multi = reinterpret_cast<EditableMultiPartMenuItem*>(menuItem);
    menuMgr.setEditorHintsLocked(false);
    menuMgr.setCurrentEditor(menuItem);
    itemBeingEdited = multi;
    keyboardLayout = (menuItem->getMenuType() == MENUTYPE_TEXT_VALUE) ? defTxtKeyboard : defNumKeyboard;

    // here we take over the display to render a keyboard
    touchRenderer.setCustomDrawingHandler(this);
    touchRenderer.takeOverDisplay();
    if (touchScreenManager) touchScreenManager->registerSecondaryObserver(this);
}

void TcTouchKeyboardController::renderItem() {
    if (!itemBeingEdited || !touchRenderer.getDeviceDrawable()) return;
    auto& factory = touchRenderer.getGraphicsPropertiesFactory();
    auto* titleProps = factory.configFor(itemBeingEdited, ItemDisplayProperties::COMPTYPE_TITLE);
    auto* itemProps = factory.configFor(itemBeingEdited, ItemDisplayProperties::COMPTYPE_ITEM);
    DeviceDrawableHelper helper(touchRenderer.getDeviceDrawable());
    char buffer[64] = "Edit ";
    itemBeingEdited->copyNameToBuffer(buffer, 5, sizeof buffer);
    if (itemEditor->isCapsDown()) {
        appendChar(buffer, '^', sizeof buffer);
    }
    helper.setFontFromParameters(titleProps->getFont(), titleProps->getFontMagnification());
    int baseline;
    int titleHeight = helper.textExtents(buffer, &baseline).y;
    helper.drawText(Coord(EDIT_TEXT_MARGIN, EDIT_TEXT_MARGIN), titleProps->getColor(ItemDisplayProperties::TEXT), buffer);

    menuMgr.setEditorHints(CurrentEditorRenderingHints::EDITOR_REGULAR);
    copyMenuItemValue(itemBeingEdited, buffer, sizeof buffer, true);
    helper.setFontFromParameters(itemProps->getFont(), itemProps->getFontMagnification());
    Coord valuePos(EDIT_TEXT_MARGIN, EDIT_TEXT_MARGIN + titleHeight + EDIT_TEXT_GAP);
    auto& hints = menuMgr.getEditorHints();
    if (hints.getEditorRenderingType() == CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT) {
        auto length = strlen(buffer);
        const int start = internal_min((int)length, internal_max(0, hints.getStartIndex()));
        const int end = internal_min((int)length, internal_max(start, hints.getEndIndex()));
        char saved = buffer[start];
        buffer[start] = 0;
        int startX = helper.textExtents(buffer, &baseline).x;
        buffer[start] = saved;
        saved = buffer[end];
        buffer[end] = 0;
        int cursorWidth = internal_max(MINIMUM_CURSOR_SIZE, int(helper.textExtents(buffer + start, &baseline).x));
        buffer[end] = saved;
        int valueHeight = helper.textExtents(buffer, &baseline).y;
        int cursorX = internal_min(int(touchRenderer.getWidth()) - MINIMUM_CURSOR_SIZE, int(valuePos.x + startX));
        auto* drawable = touchRenderer.getDeviceDrawable();
        drawable->setDrawColor(factory.getSelectedColor(ItemDisplayProperties::TEXT, true));
        drawable->drawBox(Coord(cursorX, valuePos.y + valueHeight), Coord(cursorWidth, 1), true);
    }
    helper.drawText(valuePos, itemProps->getColor(ItemDisplayProperties::TEXT), buffer);
}

void TcTouchKeyboardController::touched(const TouchNotification& notification) {
    if (keyboardLayout) {
        bool tch = notification.getTouchState() == iotouch::TOUCHED;
        uint8_t touched = keyboardLayout->touchInRow(notification.getCursorPosition(), tch);
        if (touched == 0xFF) return;
        if (touched == 'X') {
            stopEditing();
        } else {
            itemEditor->keypress(this, touched);
        }
    }
}

void TcTouchKeyboardController::needsRedraw(bool moreToGo) {
    dirty = true;
    if (!moreToGo) {
        stopEditing();
    }
}

void TcTouchKeyboardController::renderKeyboard() const {
    auto* layout = keyboardLayout ? keyboardLayout : defNumKeyboard;
    if (layout && touchRenderer.getDeviceDrawable()) {
        auto& factory = touchRenderer.getGraphicsPropertiesFactory();
        const auto location = keyboardLayout->getKeysPerRow() > 5 ? ItemDisplayProperties::COMPTYPE_ITEM : ItemDisplayProperties::COMPTYPE_TITLE;
        auto* properties = factory.configFor(itemBeingEdited, location);

        DeviceDrawableHelper helper(touchRenderer.getDeviceDrawable());
        helper.setFontFromParameters(properties->getFont(), properties->getFontMagnification());
        layout->calculateKeyHeight(helper);
        layout->drawRows(touchRenderer, *properties, touchRenderer.getHeight() - layout->getRequiredHeight());
    }
}

void TcTouchKeyboardController::drawScrollbar(DeviceDrawable* drawable, const Coord& displayDims, int titleHeight) {
    auto& factory = touchRenderer.getGraphicsPropertiesFactory();
    auto* properties = factory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);

    const auto yHei = displayDims.y - titleHeight;
    const auto xLoc = displayDims.x - TC_KEYBOARD_BUTTON_SIZE;
    upButton.setPositionAndSize(Coord(xLoc, titleHeight), Coord(TC_KEYBOARD_BUTTON_SIZE, yHei / 2));
    upButton.setColors(properties->getColor(ItemDisplayProperties::TEXT), properties->getColor(ItemDisplayProperties::BACKGROUND), properties->getColor(ItemDisplayProperties::HIGHLIGHT1));
    upButton.paintButton(drawable);
    downButton.setPositionAndSize(Coord(xLoc, titleHeight + yHei / 2), Coord(TC_KEYBOARD_BUTTON_SIZE, yHei / 2));
    downButton.setColors(properties->getColor(ItemDisplayProperties::TEXT), properties->getColor(ItemDisplayProperties::BACKGROUND), properties->getColor(ItemDisplayProperties::HIGHLIGHT1));
    downButton.paintButton(drawable);
}

void TcTouchKeyboardController::reportTouchUpDown(const Coord& touch) const {
    if (upButton.touchInBounds(touch)) {
        //perform up action
        switches.getEncoder()->increment(-1);
    } else if (downButton.touchInBounds(touch)) {
        // perform down action
        switches.getEncoder()->increment(+1);
    }
}

void TcTouchKeyboardController::stopEditing() {
    if (touchScreenManager) touchScreenManager->unregisterSecondaryObserver(this);
    touchRenderer.giveBackDisplay();
    itemBeingEdited->stopMultiEdit();
    menuMgr.setCurrentEditor(nullptr);
    touchRenderer.redrawRequirement(MENUDRAW_COMPLETE_REDRAW);
    keyboardLayout = nullptr;
    itemBeingEdited = nullptr;
}

TcTouchKeyboardController* tcgfx::defaultTouchKeyboardController(const Coord& size, GraphicsDeviceRenderer& renderer) {
    auto* layout = new TouchKeyboardLayout(size);
    auto* layoutText = new TouchKeyboardLayout(size);
    layout->withRow("789>", 4)
        ->withRow("456<", 4)
        ->withRow("123X", 4)
        ->withRow(".0+", 3);
    layoutText->withRow("1234567890", 10)
              ->withRow("qwertyuiop", 10)
              ->withRow("asdfghjkl!", 10)
              ->withRow("zxcvbnm,.?", 10)
              ->withRow(" *#@$\0xFE", 6);

    auto* touchKeyboard = new TcTouchKeyboardController(layout, layoutText, renderer);
    return touchKeyboard;
}
