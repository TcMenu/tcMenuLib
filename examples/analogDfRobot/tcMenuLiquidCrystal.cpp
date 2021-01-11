/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * LiquidCrystalIO renderer that renders menus onto this type of display. This file is a plugin file and should not
 * be directly edited, it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 * 
 * LIBRARY REQUIREMENT
 * This renderer is designed for use with this library: https://github.com/davetcc/LiquidCrystalIO
 */

#include "tcMenuLiquidCrystal.h"
#include "tcUtil.h"

extern const ConnectorLocalInfo applicationInfo;

LiquidCrystalRenderer::LiquidCrystalRenderer(LiquidCrystal& lcd, int dimX, int dimY) : BaseGraphicalRenderer(dimX, dimX, dimY, true, applicationInfo.name) {
    this->lcd = &lcd;
    this->backChar = '<';
    this->forwardChar = '>';
    this->editChar = '=';
}

void LiquidCrystalRenderer::initialise() {
    // first we create the custom characters for any title widget.
    // we iterate over each widget then over each each icon.
    TitleWidget* wid = firstWidget;
    int charNo = 0;
    while(wid != NULL) {
        serdebugF2("Title widget present max=", wid->getMaxValue());
        for(int i = 0; i < wid->getMaxValue(); i++) {
            serdebugF2("Creating char ", charNo);
            lcd->createCharPgm((uint8_t)charNo, wid->getIcon(i));
            charNo++;
        }
        wid = wid->getNext();
    }
    lcd->clear();

    BaseGraphicalRenderer::initialise();
}

LiquidCrystalRenderer::~LiquidCrystalRenderer() {
    delete this->buffer;
    delete dialog;
}

void LiquidCrystalRenderer::setEditorChars(char back, char forward, char edit) {
    backChar = back;
    forwardChar = forward;
    editChar = edit;
}

void LiquidCrystalRenderer::drawWidget(Coord where, TitleWidget *widget, color_t, color_t) {
    char ch = char(widget->getHeight() + widget->getCurrentState());
    serdebugF4("draw widget", where.x, where.y, (int)ch);
    lcd->setCursor(where.x, where.y);
    widget->setChanged(false);
    lcd->write(ch);
}

bool itemNeedsValue(GridPosition::GridJustification justification) {
    return (justification == GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE ||
            justification == GridPosition::JUSTIFY_CENTER_WITH_VALUE ||
            justification == GridPosition::JUSTIFY_RIGHT_WITH_VALUE);
}

int calculateOffset(GridPosition::GridJustification just, int totalLen, const char* sz) {
    int len = strlen(sz);
    if(len > totalLen || just == GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE || just == GridPosition::JUSTIFY_LEFT_NO_VALUE) return 0;

    if(just == GridPosition::JUSTIFY_RIGHT_WITH_VALUE || just == GridPosition::JUSTIFY_RIGHT_NO_VALUE) {
        return (totalLen - len) - 1;
    }
    else {
        // must be centered in this case.
        return (totalLen - len) / 2;
    }
}

void copyIntoBuffer(char* buffer, const char* source, int offset, int bufferLen) {
    int len = strlen(source);
    for(int i=0; i<len; i++) {
        auto pos = offset+i;
        if(pos >= bufferLen) return;
        buffer[pos] = source[i];
    }
}

void LiquidCrystalRenderer::drawMenuItem(GridPositionRowCacheEntry* entry, Coord where, Coord areaSize) {
    auto* theItem = entry->getMenuItem();
    theItem->setChanged(false);

    buffer[0] = theItem->isEditing() ? editChar : (theItem->isActive() ? forwardChar : ' ');

    if(entry->getPosition().getJustification() == GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT) {
        lcd->setCursor(where.x, where.y);
        int offs = 1;
        uint8_t finalPos = theItem->copyNameToBuffer(buffer, offs, bufferSize);
        for(uint8_t i = finalPos; i < areaSize.x; ++i)  buffer[i] = 32;
        buffer[bufferSize] = 0;
        menuValueToText(theItem, JUSTIFY_TEXT_RIGHT);
    }
    else {
        char sz[20];
        for(uint8_t i = 1; i < (uint8_t)areaSize.x; ++i)  buffer[i] = 32;
        buffer[areaSize.x] = 0;
        if(itemNeedsValue(entry->getPosition().getJustification())) {
            copyMenuItemNameAndValue(theItem, sz, sizeof sz, 0);
        }
        else {
            theItem->copyNameToBuffer(sz, sizeof sz);
        }
        int position = calculateOffset(entry->getPosition().getJustification(), areaSize.x, sz);
        copyIntoBuffer(&buffer[1], sz, position, bufferSize - 1);
        lcd->setCursor(where.x, where.y);
    }
    serdebugF3("Buffer: ", where.y, buffer);
    lcd->print(buffer);
}

void LiquidCrystalRenderer::drawingCommand(RenderDrawingCommand command) {
    switch (command) {
        case DRAW_COMMAND_CLEAR:
            lcd->clear();
            break;
        default:
            break;
    }
}

BaseDialog* LiquidCrystalRenderer::getDialog() {
    if(dialog == NULL) {
        dialog = new LiquidCrystalDialog(this);
    }
    return dialog;
}

// dialog

void LiquidCrystalDialog::internalRender(int currentValue) {
    LiquidCrystalRenderer* lcdRender = ((LiquidCrystalRenderer*)MenuRenderer::getInstance());
    LiquidCrystal* lcd = lcdRender->getLCD();
    if(needsDrawing == MENUDRAW_COMPLETE_REDRAW) {
        lcd->clear();
    }

    char data[20];
    strncpy_P(data, headerPgm, sizeof(data));
    data[sizeof(data)-1]=0;
    lcd->setCursor(0,0);
    lcd->print(data);

    // we can only print the buffer on a newline when there's enough rows.
    // so on 16x2 we have to show the buffer over the header. It's all we
    // can do.
    int nextY = 3;
    if(isCompressedMode()) {
        int len = strlen(lcdRender->getBuffer());
        int startX = lcdRender->getBufferSize() - len;
        lcd->setCursor(startX,0);
        lcd->print(lcdRender->getBuffer());
        nextY = 1;
    }
    else {
        lcd->setCursor(0,1);
        lcd->print(lcdRender->getBuffer());
    }

    if(button1 != BTNTYPE_NONE) {
        copyButtonText(data, 0, currentValue);
        lcd->setCursor(0, nextY);
        lcd->print(data);
    }
    if(button2 != BTNTYPE_NONE) {
        copyButtonText(data, 1, currentValue);
        int startX = lcdRender->getBufferSize() - strlen(data);
        lcd->setCursor(startX, nextY);
        lcd->print(data);
    }
}
