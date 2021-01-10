/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file tcMenuU8g2.h
 * 
 * U8g2 renderer that renders menus onto this type of display. This file is a plugin file and should not
 * be directly edited, it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 * 
 * LIBRARY REQUIREMENT
 * This library requires the u8g2 library available for download from your IDE library manager.
 */

#include <U8g2lib.h>
#include "tcMenuU8g2.h"

void U8g2MenuRenderer::setGraphicsDevice(U8G2* u8g2, U8g2GfxMenuConfig *gfxConfig) {

	this->u8g2 = u8g2;

	if (gfxConfig->editIcon == nullptr || gfxConfig->activeIcon == nullptr) {
		gfxConfig->editIcon = defEditingIcon;
		gfxConfig->activeIcon = defActiveIcon;
		gfxConfig->editIconWidth = 16;
		gfxConfig->editIconHeight = 12;
	}

    // font cannot be NULL on this board, we default if it is.
    if(gfxConfig->itemFont == nullptr) gfxConfig->itemFont = u8g2_font_6x10_tf;
    if(gfxConfig->titleFont == nullptr) gfxConfig->titleFont = u8g2_font_6x10_tf;

    setDisplayDimensions(u8g2->getWidth(), u8g2->getHeight());
    recalculateTitleAndRowHeights();
}

void U8g2MenuRenderer::setGraphicsDevice(U8G2* u8g2, ItemDisplayPropertiesFactory *factory) {

    this->u8g2 = u8g2;
    this->displayPropsFactory = factory;

    setDisplayDimensions(u8g2->getWidth(), u8g2->getHeight());
    recalculateTitleAndRowHeights();
}

void U8g2MenuRenderer::recalculateTitleAndRowHeights() {
    u8g2->setFont(displayPropsFactory->configFor(MENUID_NOTSET, ItemDisplayPropertiesFactory::TEXT_ITEM));
    int titleY = u8g2->getMaxCharHeight() + gfxConfig->titlePadding.top + gfxConfig->titlePadding.bottom;
    u8g2->setFont(gfxConfig->itemFont);
    int rowY = u8g2->getMaxCharHeight() + gfxConfig->itemPadding.top + gfxConfig->itemPadding.bottom;

    setStandardRowSize(rowY, titleY, gfxConfig->widgetPadding);
}

void U8g2MenuRenderer::drawWidget(Coord where, TitleWidget *widget) {
    serdebugF("widget redraw");
    u8g2->setColorIndex(gfxConfig->widgetColor);
    drawBitmap(where.x, where.y, widget->getWidth(), widget->getHeight(), widget->getCurrentIcon());
    redrawNeeded = true;
}

void U8g2MenuRenderer::drawMenuItem(MenuItem *theItem, GridPosition::GridDrawingMode mode, Coord where, Coord areaSize) {
    redrawNeeded = true;
    switch(mode) {
        case GridPosition::DRAW_TEXTUAL_ITEM:
        case GridPosition::DRAW_INTEGER_AS_UP_DOWN:
            drawTextualItem(theItem, where, areaSize);
            break;
        case GridPosition::DRAW_INTEGER_AS_SCROLL:
            if(theItem->getMenuType() != MENUTYPE_INT_VALUE) return; // disallowed
            drawSlider(reinterpret_cast<AnalogMenuItem*>(theItem), where, areaSize);
            break;
        case GridPosition::DRAW_AS_ICON_ONLY:
        case GridPosition::DRAW_AS_ICON_TEXT:
            drawIconItem(theItem, where, areaSize, mode == GridPosition::DRAW_AS_ICON_TEXT);
            break;
        case GridPosition::DRAW_TITLE_ITEM:
            drawTitleArea(theItem, where, areaSize);
            break;
    }
}

void U8g2MenuRenderer::drawingCommand(BaseGraphicalRenderer::RenderDrawingCommand command) {
    switch(command) {
        case DRAW_COMMAND_CLEAR:
            u8g2->setFont(gfxConfig->itemFont);
            u8g2->setFontPosBottom();
            u8g2->setFontRefHeightExtendedText();
            u8g2->setColorIndex(gfxConfig->bgItemColor);
            u8g2->drawBox(0,0,u8g2->getDisplayWidth(), u8g2->getDisplayHeight());
            redrawNeeded = true;
            serdebugF("cls");
            break;
        case DRAW_COMMAND_START:
            break;
        case DRAW_COMMAND_ENDED:
            if(redrawNeeded) {
                serdebugF("send buffer");
                redrawNeeded = false;
                u8g2->sendBuffer();
            }
            break;
    }
}

void U8g2MenuRenderer::drawTitleArea(MenuItem* pItem, Coord where, Coord size) {
    u8g2->setFont(gfxConfig->titleFont);
    u8g2->setFontDirection(0);

    int fontYStart = (where.y + size.y) - (gfxConfig->titlePadding.bottom);

    u8g2->setColorIndex(gfxConfig->bgTitleColor);
    u8g2->drawBox(where.x, where.y, size.x, size.y);
    u8g2->setColorIndex(gfxConfig->fgTitleColor);
    u8g2->setCursor(where.x + gfxConfig->titlePadding.left, fontYStart);
    pItem->copyNameToBuffer(buffer, bufferSize);
    serdebugF4("title: ", buffer, where.x, fontYStart);

    if(pItem->isActive()) u8g2->print('>');
    u8g2->print(buffer);
    if(pItem->getMenuType() == MENUTYPE_BACK_VALUE && pItem->isActive()) u8g2->print(" [..]");
}

void U8g2MenuRenderer::drawIconItem(MenuItem *pItem, Coord where, Coord size, bool withText) {
    u8g2->setColorIndex(gfxConfig->bgItemColor);
    u8g2->drawBox(where.x, where.y, size.x, size.y);

    u8g2->setColorIndex(gfxConfig->fgItemColor);
    if(pItem->isActive()) {
        u8g2->drawFrame(where.x, where.y, size.x, size.y);
    }

    auto* pIcon = displayPropsFactory->iconForMenuItem(pItem);
    if(pIcon == nullptr || pIcon->getIconType() != DrawableIcon::ICON_XBITMAP) return;

    int xStart = where.x + ((size.x - pIcon->getDimensions().x) / 2);
    int yStart = where.y + ((size.y - pIcon->getDimensions().y) / 2);

    bool sel = false;
    if(pItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
        auto* boolItem = reinterpret_cast<BooleanMenuItem*>(pItem);
        sel = boolItem->getBoolean();
    }
    drawBitmap(xStart, yStart, pIcon->getDimensions().x, pIcon->getDimensions().y, pIcon->getIcon(sel));
}

void U8g2MenuRenderer::drawSlider(AnalogMenuItem* pItem, Coord where, Coord size) {
    u8g2->setFont(gfxConfig->itemFont);
    drawCoreLineItem(pItem, where, size);

    int sliderStartX = where.x + gfxConfig->itemPadding.left + gfxConfig->editIconWidth + gfxConfig->itemPadding.left;
    int maximumSliderArea = size.x - (sliderStartX + gfxConfig->itemPadding.right);
    int filledAreaX = analogRangeToScreen(pItem, maximumSliderArea);
    int filledHeight = size.y - (gfxConfig->itemPadding.bottom + gfxConfig->itemPadding.top);
    u8g2->drawBox(sliderStartX, where.y + gfxConfig->itemPadding.top, filledAreaX, filledHeight);

    int prevCol = u8g2->getDrawColor();
    u8g2->setFont(gfxConfig->itemFont);
    u8g2->setDrawColor(2);
    u8g2->setFontMode(true);
    int drawingPositionY = where.y + (size.y - (gfxConfig->itemPadding.bottom));
    copyMenuItemNameAndValue(pItem, buffer, bufferSize);
    int textStart = sliderStartX + (maximumSliderArea - u8g2->getStrWidth(buffer)) / 2;
    u8g2->setCursor(textStart, drawingPositionY);
    u8g2->print(buffer);
    u8g2->setDrawColor(prevCol);
    u8g2->setFontMode(false);
}

void U8g2MenuRenderer::drawTextualItem(MenuItem *pItem, Coord where, Coord size) {
    u8g2->setFont(gfxConfig->itemFont);
    int imgMiddleY = drawCoreLineItem(pItem, where, size);

    int textPos = where.x + gfxConfig->itemPadding.left + gfxConfig->editIconWidth + gfxConfig->itemPadding.left;

    int drawingPositionY = where.y + (size.y - (gfxConfig->itemPadding.bottom));
    u8g2->setCursor(textPos, drawingPositionY);
    pItem->copyNameToBuffer(buffer, bufferSize);
    serdebugF4("item: ", buffer, size.y, where.y);

    u8g2->print(buffer);

    if(isItemActionable(pItem)) {
        int rightOffset = u8g2->getDisplayWidth() - (gfxConfig->itemPadding.right + gfxConfig->editIconWidth);
        u8g2->setColorIndex(gfxConfig->fgSelectColor);
        drawBitmap(rightOffset, imgMiddleY, gfxConfig->editIconWidth, gfxConfig->editIconHeight, gfxConfig->activeIcon);
        buffer[0] = 0;
    }

    copyMenuItemValue(pItem, buffer, bufferSize);

    int16_t right = size.x - (u8g2->getStrWidth(buffer) + gfxConfig->itemPadding.right);
    u8g2->setCursor(right, drawingPositionY);
    u8g2->print(buffer);
}

int U8g2MenuRenderer::drawCoreLineItem(MenuItem *pItem, const Coord &where, const Coord &size) {
    int icoWid = gfxConfig->editIconWidth;
    int icoHei = gfxConfig->editIconHeight;

    int imgMiddleY = where.y + ((size.y - icoHei) / 2);

    if(pItem->isEditing()) {
        u8g2->setColorIndex(gfxConfig->bgSelectColor);
        u8g2->drawBox(where.x, where.y, size.x, size.y);
        u8g2->setColorIndex(gfxConfig->fgSelectColor);
        drawBitmap(where.x + gfxConfig->itemPadding.left, imgMiddleY, icoWid, icoHei, gfxConfig->editIcon);
    }
    else if(pItem->isActive()) {
        u8g2->setColorIndex(gfxConfig->bgSelectColor);
        u8g2->drawBox(where.x, where.y, size.x, size.y);
        u8g2->setColorIndex(gfxConfig->fgSelectColor);
        drawBitmap(where.x + gfxConfig->itemPadding.left, imgMiddleY, icoWid, icoHei, gfxConfig->activeIcon);
    }
    else {
        u8g2->setColorIndex(gfxConfig->bgItemColor);
        u8g2->drawBox(where.x, where.y, size.x, size.y);
        u8g2->setColorIndex(gfxConfig->fgItemColor);
    }
    return imgMiddleY;
}

void U8g2MenuRenderer::drawBitmap(int x, int y, int w, int h, const unsigned char *bmp) {
#if defined(__AVR__) || defined(ESP8266)
    u8g2->drawXBMP(x, y, w, h, bmp);
#else
    u8g2->drawXBM(x, y, w, h, bmp);
#endif
}

//
// Default graphics configuration
//

/**
 * Provides a basic graphics configuration suitable for low / medium resolution displays
 * @param config usually a global variable holding the graphics configuration.
 */
void prepareBasicU8x8Config(U8g2GfxMenuConfig& config) {
	makePadding(config.titlePadding, 1, 1, 1, 1);
	makePadding(config.itemPadding, 1, 1, 1, 1);
	makePadding(config.widgetPadding, 2, 2, 0, 2);

	config.bgTitleColor = WHITE;
	config.fgTitleColor = BLACK;
	config.titleFont = u8g2_font_6x12_tf;
	config.titleBottomMargin = 1;
	config.widgetColor = BLACK;
	config.titleFontMagnification = 1;

	config.bgItemColor = BLACK;
	config.fgItemColor = WHITE;
	config.bgSelectColor = BLACK;
	config.fgSelectColor = WHITE;
	config.itemFont = u8g2_font_6x10_tf;
	config.itemFontMagnification = 1;

    config.editIcon = loResEditingIcon;
    config.activeIcon = loResActiveIcon;
    config.editIconHeight = 6;
    config.editIconWidth = 8;
}

//
// Dialog code
//

BaseDialog* U8g2MenuRenderer::getDialog() {
    if(dialog == NULL) {
        dialog = new U8g2Dialog();
    }
    return dialog;
}

U8g2Dialog::U8g2Dialog() {
    U8g2MenuRenderer* r = reinterpret_cast<U8g2MenuRenderer*>(MenuRenderer::getInstance());
    bitWrite(flags, DLG_FLAG_SMALLDISPLAY, (r->getGraphics()->getDisplayWidth() < 100));
}

void U8g2Dialog::internalRender(int currentValue) {
    U8g2MenuRenderer* adaRenderer = reinterpret_cast<U8g2MenuRenderer*>(MenuRenderer::getInstance());
    U8g2GfxMenuConfig* gfxConfig = adaRenderer->getGfxConfig();
    U8G2* graphics = adaRenderer->getGraphics();

    if(needsDrawing == MENUDRAW_COMPLETE_REDRAW) {
        // clear screen first in complete draw mode
        graphics->setColorIndex(gfxConfig->bgItemColor);
        graphics->drawBox(0,0,graphics->getDisplayWidth(), graphics->getDisplayHeight());
    }

    graphics->setFont(gfxConfig->itemFont);

    char data[20];
    safeProgCpy(data, headerPgm, sizeof(data));

	int fontYStart = gfxConfig->itemPadding.top;
  	int extentY = graphics->getMaxCharHeight();
	int dlgNextDraw = extentY + gfxConfig->titlePadding.top + gfxConfig->titlePadding.bottom;
	if (gfxConfig->itemFont) {
	 	fontYStart = dlgNextDraw - (gfxConfig->titlePadding.bottom);
	}

    graphics->setColorIndex(gfxConfig->bgTitleColor);
	graphics->drawBox(0, 0, graphics->getDisplayWidth(), dlgNextDraw);
	graphics->setColorIndex(gfxConfig->fgTitleColor);
	graphics->setCursor(gfxConfig->titlePadding.left, fontYStart);
	graphics->print(data);

	dlgNextDraw += gfxConfig->titleBottomMargin;

    int startingPosition = dlgNextDraw;
    fontYStart = dlgNextDraw + gfxConfig->itemPadding.top;
	dlgNextDraw = dlgNextDraw + extentY + gfxConfig->titlePadding.top + gfxConfig->titlePadding.bottom;
	if (gfxConfig->itemFont) {
	 	fontYStart = dlgNextDraw - (gfxConfig->titlePadding.bottom);
	}
    graphics->setColorIndex(gfxConfig->bgItemColor);
    graphics->drawBox(0, startingPosition, graphics->getDisplayWidth(), dlgNextDraw);
	graphics->setColorIndex(gfxConfig->fgItemColor);
	graphics->setCursor(gfxConfig->titlePadding.left, fontYStart);

	graphics->print(MenuRenderer::getInstance()->getBuffer());
    
    bool active;
    if(button1 != BTNTYPE_NONE) {
        active = copyButtonText(data, 0, currentValue);
        drawButton(graphics, gfxConfig, data, 0, active);
    }
    if(button2 != BTNTYPE_NONE) {
        active = copyButtonText(data, 1, currentValue);
        drawButton(graphics, gfxConfig, data, 1, active);
    }
    graphics->sendBuffer();
}

void U8g2Dialog::drawButton(U8G2* gfx, U8g2GfxMenuConfig* config, const char* title, uint8_t num, bool active) {
	int extentY = gfx->getMaxCharHeight();
    int itemHeight = ( extentY + config->itemPadding.top + config->itemPadding.bottom);
    int start = gfx->getDisplayHeight() - itemHeight;
    int fontYStart = start + config->itemPadding.top;
	if (config->itemFont) {
        fontYStart += extentY;
	}
    int buttonWidth = gfx->getDisplayWidth() / 2;
    int xOffset = (num == 0) ? 0 : buttonWidth;
    gfx->setColorIndex(active ? config->bgSelectColor : config->bgItemColor);
    gfx->drawBox(xOffset, start, buttonWidth, itemHeight);
	gfx->setColorIndex(active ? config->fgSelectColor : config->fgItemColor);
    gfx->setCursor(xOffset + ((buttonWidth - gfx->getStrWidth(title)) / 2), fontYStart);
    gfx->print(title);
}
