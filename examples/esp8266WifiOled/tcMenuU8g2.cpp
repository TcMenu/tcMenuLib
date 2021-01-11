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
    setDisplayDimensions(u8g2->getWidth(), u8g2->getHeight());

	if (gfxConfig->editIcon == nullptr || gfxConfig->activeIcon == nullptr) {
		gfxConfig->editIcon = defEditingIcon;
		gfxConfig->activeIcon = defActiveIcon;
		gfxConfig->editIconWidth = 16;
		gfxConfig->editIconHeight = 12;
	}

    // font cannot be NULL on this board, we default if it is.
    if(gfxConfig->itemFont == nullptr) gfxConfig->itemFont = u8g2_font_6x10_tf;
    if(gfxConfig->titleFont == nullptr) gfxConfig->titleFont = u8g2_font_6x10_tf;


    u8g2->setFont(gfxConfig->titleFont);
    int titleY = u8g2->getMaxCharHeight() + gfxConfig->titlePadding.top + gfxConfig->titlePadding.bottom;
    u8g2->setFont(gfxConfig->itemFont);
    int rowY = u8g2->getMaxCharHeight() + gfxConfig->itemPadding.top + gfxConfig->itemPadding.bottom;

    color_t itemPalette[] = {gfxConfig->fgItemColor, gfxConfig->bgItemColor, gfxConfig->fgItemColor, 2};
    color_t titlePalette[] = {gfxConfig->fgTitleColor, gfxConfig->bgTitleColor, gfxConfig->fgTitleColor, 2};
    propertiesFactory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, titlePalette, gfxConfig->titlePadding, gfxConfig->titleFont, gfxConfig->titleFontMagnification, gfxConfig->titleBottomMargin, titleY, GridPosition::JUSTIFY_CENTER_NO_VALUE);
    propertiesFactory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, itemPalette, gfxConfig->itemPadding, gfxConfig->itemFont, gfxConfig->itemFontMagnification, 0, rowY, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT);
    propertiesFactory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, itemPalette, gfxConfig->itemPadding, gfxConfig->itemFont, gfxConfig->itemFontMagnification, 0, rowY, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE);
    propertiesFactory.setSelectedColors(gfxConfig->bgSelectColor, gfxConfig->fgSelectColor);

    propertiesFactory.addImageToCache(DrawableIcon(SPECIAL_ID_ACTIVE_ICON, Coord(gfxConfig->editIconWidth, gfxConfig->editIconHeight), DrawableIcon::ICON_XBITMAP, gfxConfig->activeIcon));
    propertiesFactory.addImageToCache(DrawableIcon(SPECIAL_ID_EDIT_ICON, Coord(gfxConfig->editIconWidth, gfxConfig->editIconHeight), DrawableIcon::ICON_XBITMAP, gfxConfig->editIcon));
}

void U8g2MenuRenderer::setGraphicsDevice(U8G2* u8g2) {
    this->u8g2 = u8g2;
    setDisplayDimensions(u8g2->getWidth(), u8g2->getHeight());

    u8g2->setFont(u8g2_font_6x10_tf);
    int titleY = u8g2->getMaxCharHeight() + 8;
    int rowY = u8g2->getMaxCharHeight() + 2;

    color_t itemPalette[] = {1, 0, 1, 2};
    color_t titlePalette[] = {0, 1, 0, 2};
    propertiesFactory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, titlePalette, MenuPadding(4), u8g2_font_6x10_tf, 1, 2, titleY, GridPosition::JUSTIFY_CENTER_NO_VALUE);
    propertiesFactory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, itemPalette, MenuPadding(2), u8g2_font_6x10_tf, 1, 0, rowY, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT);
    propertiesFactory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, itemPalette, MenuPadding(2), u8g2_font_6x10_tf, 1, 0, rowY, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE);

    propertiesFactory.addImageToCache(DrawableIcon(SPECIAL_ID_ACTIVE_ICON, Coord(16, 12), DrawableIcon::ICON_XBITMAP, defActiveIcon));
    propertiesFactory.addImageToCache(DrawableIcon(SPECIAL_ID_EDIT_ICON, Coord(16, 12), DrawableIcon::ICON_XBITMAP, defEditingIcon));

}

void U8g2MenuRenderer::drawWidget(Coord where, TitleWidget *widget, color_t fg, color_t bg) {
    serdebugF3("widget redraw fg, bg ", fg, bg);
    u8g2->setDrawColor(bg);
    u8g2->drawBox(where.x, where.y, widget->getWidth(), widget->getHeight());
    u8g2->setDrawColor(fg);
    drawBitmap(where.x, where.y, widget->getWidth(), widget->getHeight(), widget->getCurrentIcon());
    redrawNeeded = true;
}

void U8g2MenuRenderer::drawMenuItem(GridPositionRowCacheEntry* pEntry, Coord where, Coord areaSize) {
    redrawNeeded = true;
    switch(pEntry->getPosition().getDrawingMode()) {
        case GridPosition::DRAW_TEXTUAL_ITEM:
        case GridPosition::DRAW_INTEGER_AS_UP_DOWN:
        case GridPosition::DRAW_TITLE_ITEM:
            drawTextualItem(pEntry, where, areaSize);
            break;
        case GridPosition::DRAW_INTEGER_AS_SCROLL:
            if(pEntry->getMenuItem()->getMenuType() != MENUTYPE_INT_VALUE) return; // disallowed
            drawSlider(pEntry, where, areaSize);
            break;
        case GridPosition::DRAW_AS_ICON_ONLY:
        case GridPosition::DRAW_AS_ICON_TEXT:
            drawIconItem(pEntry, where, areaSize);
            if(pEntry->getPosition().getDrawingMode() == GridPosition::DRAW_AS_ICON_TEXT) {
                internalDrawText(pEntry, where, areaSize, 0, false);
            }
            break;
    }
}

const uint8_t* safeGetFont(const void* fnt) {
    if(fnt) return static_cast<const uint8_t *>(fnt);
    return u8g2_font_6x10_tf;
}

void U8g2MenuRenderer::drawingCommand(BaseGraphicalRenderer::RenderDrawingCommand command) {
    switch(command) {
        case DRAW_COMMAND_CLEAR: {
            auto *props = propertiesFactory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM);
            u8g2->setFont(safeGetFont(props->getFont()));
            u8g2->setFontPosBottom();
            u8g2->setFontRefHeightExtendedText();
            u8g2->setColorIndex(props->getColor(ItemDisplayProperties::BACKGROUND));
            u8g2->drawBox(0, 0, u8g2->getDisplayWidth(), u8g2->getDisplayHeight());
            redrawNeeded = true;
            serdebugF("cls");
            break;
        }
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

void U8g2MenuRenderer::drawIconItem(GridPositionRowCacheEntry* pEntry, Coord where, Coord size) {
    auto* props = pEntry->getDisplayProperties();
    auto* pItem = pEntry->getMenuItem();

    u8g2->setColorIndex(props->getColor(ItemDisplayProperties::BACKGROUND));
    u8g2->drawBox(where.x, where.y, size.x, size.y);

    u8g2->setColorIndex(props->getColor(ItemDisplayProperties::TEXT));
    if(pItem->isActive()) {
        u8g2->drawFrame(where.x, where.y, size.x, size.y);
    }

    auto* pIcon = propertiesFactory.iconForMenuItem(pItem->getId());
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

void U8g2MenuRenderer::drawSlider(GridPositionRowCacheEntry* pEntry, Coord where, Coord size) {
    MenuPadding padding = pEntry->getDisplayProperties()->getPadding();
    drawCoreLineItem(pEntry, where, size);

    auto* editIcon = propertiesFactory.iconForMenuItem(SPECIAL_ID_ACTIVE_ICON);
    int iconOffset = (editIcon) ? editIcon->getDimensions().x + padding.left : 0;

    int sliderStartX = where.x + iconOffset;
    int maximumSliderArea = size.x - (sliderStartX + padding.right);
    auto* analogItem = reinterpret_cast<AnalogMenuItem*>(pEntry->getMenuItem());
    int filledAreaX = analogRangeToScreen(analogItem, maximumSliderArea);
    u8g2->drawBox(sliderStartX, where.y, filledAreaX, size.y);

    internalDrawText(pEntry, where, size, iconOffset, true);
}

void U8g2MenuRenderer::drawTextualItem(GridPositionRowCacheEntry* pEntry, Coord where, Coord size) {
    u8g2->setFont(static_cast<const uint8_t *>(pEntry->getDisplayProperties()->getFont()));
    drawCoreLineItem(pEntry, where, size);
    auto padding = pEntry->getDisplayProperties()->getPadding();

    auto* editIcon = propertiesFactory.iconForMenuItem(SPECIAL_ID_ACTIVE_ICON);
    int iconOffset = (editIcon) ? editIcon->getDimensions().x + padding.left : 0;

    internalDrawText(pEntry, where, size, iconOffset, false);
}

bool itemNeedsValue(GridPosition::GridJustification justification) {
    return (justification == GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE ||
            justification == GridPosition::JUSTIFY_CENTER_WITH_VALUE ||
            justification == GridPosition::JUSTIFY_RIGHT_WITH_VALUE);
}

void U8g2MenuRenderer::internalDrawText(GridPositionRowCacheEntry* pEntry, Coord where, Coord size, int leftOffset, bool modeXor) {
    GridPosition::GridJustification just = pEntry->getPosition().getJustification();
    auto padding = pEntry->getDisplayProperties()->getPadding();

    int prevCol = u8g2->getDrawColor();
    u8g2->setFont(safeGetFont(pEntry->getDisplayProperties()->getFont()));
    u8g2->setFontMode(modeXor);
    if(modeXor) {
        u8g2->setDrawColor(2);
    }
    else if(pEntry->getMenuItem()->isActive() || pEntry->getMenuItem()->isEditing()) {
        u8g2->setDrawColor(propertiesFactory.getSelectedColor(ItemDisplayProperties::TEXT));
    }
    else {
        u8g2->setDrawColor(pEntry->getDisplayProperties()->getColor(ItemDisplayProperties::TEXT));
    }

    if(just == GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT) {
        // special case, title left, value right.
        int drawingPositionY = where.y + (size.y - padding.bottom);
        u8g2->setCursor(leftOffset + where.x + padding.left, drawingPositionY);
        pEntry->getMenuItem()->copyNameToBuffer(buffer, bufferSize);
        serdebugF4("item: ", buffer, size.y, where.y);
        u8g2->print(buffer);

        copyMenuItemValue(pEntry->getMenuItem(), buffer, bufferSize);
        int16_t right = size.x - (u8g2->getStrWidth(buffer) + padding.right);
        u8g2->setCursor(right, drawingPositionY);
        u8g2->print(buffer);
    }
    else {
        char sz[32];
        if(itemNeedsValue(just))
            copyMenuItemNameAndValue(pEntry->getMenuItem(), sz, sizeof sz, ':');
        else
            pEntry->getMenuItem()->copyNameToBuffer(sz, sizeof sz);

        int startPosition = 0;
        if(just == GridPosition::JUSTIFY_RIGHT_WITH_VALUE || just == GridPosition::JUSTIFY_RIGHT_NO_VALUE) {
            startPosition = size.x - (u8g2->getStrWidth(sz) + padding.right);
        }
        else if(just == GridPosition::JUSTIFY_CENTER_WITH_VALUE || just == GridPosition::JUSTIFY_CENTER_NO_VALUE) {
            startPosition = ((size.x - u8g2->getStrWidth(sz)) / 2) + padding.right;
        }
        u8g2->setCursor(startPosition + where.x + leftOffset, (where.y + size.y) - padding.bottom);
        u8g2->print(sz);
        serdebugF4("intTx ", sz, startPosition + where.x, (where.y + size.y) - padding.bottom);
    }

    if(modeXor) {
        u8g2->setDrawColor(prevCol);
        u8g2->setFontMode(false);
    }
}

int U8g2MenuRenderer::drawCoreLineItem(GridPositionRowCacheEntry* pEntry, const Coord &where, const Coord &size) {
    auto* pItem = pEntry->getMenuItem();
    auto padding = pEntry->getDisplayProperties()->getPadding();
    auto* editIcon = propertiesFactory.iconForMenuItem(pItem->isEditing() ? SPECIAL_ID_EDIT_ICON : SPECIAL_ID_ACTIVE_ICON);

    int imgMiddleY = 0;
    if(pItem->isEditing() || pItem->isActive()) {
        u8g2->setColorIndex(propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND));
        u8g2->drawBox(where.x, where.y, size.x, size.y);
        u8g2->setColorIndex(propertiesFactory.getSelectedColor(ItemDisplayProperties::TEXT));
        if(editIcon) {
            imgMiddleY = where.y + ((size.y - editIcon->getDimensions().y) / 2);
            drawBitmap(where.x + padding.left, imgMiddleY, editIcon->getDimensions().x, editIcon->getDimensions().y, editIcon->getIcon(false));
        }
    }
    else {
        u8g2->setColorIndex(pEntry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND));
        u8g2->drawBox(where.x, where.y, size.x, size.y);
        u8g2->setColorIndex(pEntry->getDisplayProperties()->getColor(ItemDisplayProperties::TEXT));
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
