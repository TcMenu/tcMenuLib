/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * Adafruit_GFX renderer that renders menus onto this type of display. This file is a plugin file and should not
 * be directly edited, it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 * 
 * LIBRARY REQUIREMENT
 * This library requires the AdaGfx library along with a suitable driver.
 */

#include "tcMenuAdaFruitGfx.h"

extern const ConnectorLocalInfo applicationInfo;

int drawingCount = 0;

#if DISPLAY_HAS_MEMBUFFER == true
    #define refreshDisplayIfNeeded(gr, needUpd) {if(needUpd) reinterpret_cast<Adafruit_ILI9341*>(gr)->display();}
#else
    #define refreshDisplayIfNeeded(g, n)
#endif

const char MENU_BACK_TEXT[] PROGMEM = "[..]";

Coord textExtents(Adafruit_GFX* graphics, const char* text, int16_t x, int16_t y) {
	int16_t x1, y1;
	uint16_t w, h;
	graphics->getTextBounds((char*)text, x, y, &x1, &y1, &w, &h);

    serdebugF4("Textbounds (y1, w, h): ", y1, w, h);
	return Coord(w, h);
}

void AdaFruitGfxMenuRenderer::setGraphicsDevice(Adafruit_GFX* graphics, AdaColorGfxMenuConfig *gfxConfig) {

	if (gfxConfig->editIcon == NULL || gfxConfig->activeIcon == NULL) {
		gfxConfig->editIcon = defEditingIcon;
		gfxConfig->activeIcon = defActiveIcon;
		gfxConfig->editIconWidth = 16;
		gfxConfig->editIconHeight = 12;
	}

	this->graphics = graphics;

	auto factory = displayPropertiesFactory;
	// TEXT, BACKGROUND, HIGHLIGHT1, HIGHLIGHT2, SELECTED_FG, SELECTED_BG
	color_t paletteItems[] { gfxConfig->fgItemColor, gfxConfig->bgItemColor, gfxConfig->bgSelectColor, gfxConfig->fgSelectColor};
	color_t titleItems[] { gfxConfig->fgTitleColor, gfxConfig->bgTitleColor, gfxConfig->fgTitleColor, gfxConfig->fgSelectColor};

	factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, paletteItems, gfxConfig->itemPadding, gfxConfig->itemFont, gfxConfig->itemFontMagnification);
	factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, paletteItems, gfxConfig->itemPadding, gfxConfig->itemFont, gfxConfig->itemFontMagnification);
	factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, titleItems, gfxConfig->titlePadding, gfxConfig->itemFont, gfxConfig->itemFontMagnification, 4);
    factory.setSelectedColors(gfxConfig->bgSelectColor, gfxConfig->fgSelectColor);

	factory.addImageToCache(DrawableIcon(SPECIAL_ID_EDIT_ICON, Coord(gfxConfig->editIconWidth, gfxConfig->editIconHeight), DrawableIcon::ICON_XBITMAP, gfxConfig->editIcon));
	factory.addImageToCache(DrawableIcon(SPECIAL_ID_ACTIVE_ICON, Coord(gfxConfig->editIconWidth, gfxConfig->editIconHeight), DrawableIcon::ICON_XBITMAP, gfxConfig->activeIcon));

    setDisplayDimensions(graphics->width(), graphics->height());
    recalculateTitleAndRowHeights();
}

void AdaFruitGfxMenuRenderer::recalculateTitleAndRowHeights() {
    auto* pCfg = displayPropertiesFactory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);
    graphics->setFont((const GFXfont*)pCfg->getFont());
    graphics->setTextSize(pCfg->getFontMagnification());
    int titleY = textExtents(graphics, "Ag", 30,30).y + pCfg->getPadding().top + pCfg->getPadding().bottom;
    MenuPadding widgetPadding = pCfg->getPadding();

    pCfg = displayPropertiesFactory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM);
    graphics->setFont((const GFXfont*)pCfg->getFont());
    graphics->setTextSize(pCfg->getFontMagnification());
    int rowY = textExtents(graphics, "Ag", 30, 30).y + pCfg->getPadding().top + pCfg->getPadding().bottom;

    setStandardRowSize(rowY, titleY, widgetPadding);
}

void AdaFruitGfxMenuRenderer::drawWidget(Coord where, TitleWidget *widget, color_t colFg, color_t colBg) {
    serdebugF3("Drawing widget pos,icon: ", where.x, widget->getCurrentState());

    graphics->fillRect(where.x, where.y, widget->getWidth(), widget->getHeight(), colFg);

    graphics->drawXBitmap(where.x, where.y, widget->getCurrentIcon(), widget->getWidth(), widget->getHeight(), colBg);
}

void AdaFruitGfxMenuRenderer::drawMenuItem(GridPositionRowCacheEntry* entry, Coord where, Coord areaSize) {
    redrawNeeded = true;
    switch(entry->getPosition().getDrawingMode()) {
        case GridPosition::DRAW_TEXTUAL_ITEM:
        case GridPosition::DRAW_INTEGER_AS_UP_DOWN:
            drawTextualItem(entry, where, areaSize);
            break;
        case GridPosition::DRAW_INTEGER_AS_SCROLL:
            if(entry->getMenuItem()->getMenuType() != MENUTYPE_INT_VALUE) return; // disallowed
            drawSlider(entry, reinterpret_cast<AnalogMenuItem*>(entry->getMenuItem()), where, areaSize);
            break;
        case GridPosition::DRAW_AS_ICON_ONLY:
        case GridPosition::DRAW_AS_ICON_TEXT:
            //drawIconItem(theItem, where, areaSize, mode == GridPosition::DRAW_AS_ICON_TEXT);
            break;
        case GridPosition::DRAW_TITLE_ITEM:
            drawTitleArea(entry, where, areaSize);
            break;
    }
}

void AdaFruitGfxMenuRenderer::drawingCommand(BaseGraphicalRenderer::RenderDrawingCommand command) {
    switch(command) {
        case DRAW_COMMAND_CLEAR: {
            auto* pCfg = displayPropertiesFactory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM);
            graphics->fillScreen(pCfg->getColor(ItemDisplayProperties::BACKGROUND));
            serdebugF("cls");
            break;
        }
        case DRAW_COMMAND_START:
            break;
        case DRAW_COMMAND_ENDED:
            if(redrawNeeded) {
                refreshDisplayIfNeeded(graphics, requiresUpdate);
                redrawNeeded = false;
            }
            break;
    }
}

int AdaFruitGfxMenuRenderer::drawCoreLineItem(GridPositionRowCacheEntry* entry, DrawableIcon* icon, const Coord &where, const Coord &size) {
    auto pad = entry->getDisplayProperties()->getPadding();
    serdebugF4("Drawing at: ", where.y, size.x, size.y);

    entry->getMenuItem()->setChanged(false); // we are drawing the item so it's no longer changed.

    auto bgColor = entry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND);
    auto textColor = entry->getDisplayProperties()->getColor(ItemDisplayProperties::TEXT);

    int imgMiddleY = where.y + ((size.y - icon->getDimensions().y) / 2);
    if(entry->getMenuItem()->isEditing() || entry->getMenuItem()->isActive()) {
        graphics->fillRect(where.x, where.y, size.x, size.y, bgColor);
        graphics->drawXBitmap(pad.left, imgMiddleY, icon->getIcon(false), icon->getDimensions().x, icon->getDimensions().y, textColor);
        graphics->setTextColor(textColor);
    }
    else {
        graphics->fillRect(where.x, where.y, size.x, size.y, bgColor);
        graphics->setTextColor(textColor);
    }
    return imgMiddleY;
}

void AdaFruitGfxMenuRenderer::drawTextualItem(GridPositionRowCacheEntry* entry, Coord where, Coord size) {
    graphics->setFont(static_cast<const GFXfont *>(entry->getDisplayProperties()->getFont()));
    graphics->setTextSize(entry->getDisplayProperties()->getFontMagnification());

    auto* icon = displayPropertiesFactory.iconForMenuItem(entry->getMenuItem()->isEditing() ? SPECIAL_ID_EDIT_ICON :SPECIAL_ID_ACTIVE_ICON);
    int imgMiddleY = drawCoreLineItem(entry, icon, where, size);

    MenuPadding pad = entry->getDisplayProperties()->getPadding();
    int textPos = where.x + pad.left + icon->getDimensions().x + pad.left;

    int drawingPositionY = where.y;
    if(entry->getDisplayProperties()->getFont()) {
      drawingPositionY += (size.y - (pad.bottom));
    }
    graphics->setCursor(textPos, drawingPositionY);
    entry->getMenuItem()->copyNameToBuffer(buffer, bufferSize);
    serdebugF4("item: ", buffer, size.y, where.y);

    graphics->print(buffer);

    if(isItemActionable(entry->getMenuItem())) {
        int rightOffset = size.x - (pad.right + icon->getDimensions().x);
        graphics->setTextColor(entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT1));
        graphics->drawXBitmap(rightOffset, imgMiddleY, icon->getIcon(false),
                              icon->getDimensions().x, icon->getDimensions().y,
                              entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT1));
    }

    copyMenuItemValue(entry->getMenuItem(), buffer, bufferSize);

    int16_t right = size.x - (textExtents(graphics, buffer, 0, 20).x + pad.right);
    graphics->setCursor(right, drawingPositionY);
    graphics->print(buffer);
}

void AdaFruitGfxMenuRenderer::drawSlider(GridPositionRowCacheEntry* entry, AnalogMenuItem* pItem, Coord where, Coord size) {
    auto* icon = displayPropertiesFactory.iconForMenuItem(entry->getMenuItem()->isEditing() ? SPECIAL_ID_EDIT_ICON :SPECIAL_ID_ACTIVE_ICON);
    graphics->setFont(static_cast<const GFXfont *>(entry->getDisplayProperties()->getFont()));
    graphics->setTextSize(entry->getDisplayProperties()->getFontMagnification());
    drawCoreLineItem(entry, icon, where, size);
    MenuPadding pad = entry->getDisplayProperties()->getPadding();

    int sliderStartX = where.x + pad.left + icon->getDimensions().x + pad.left;
    int maximumSliderArea = size.x - (sliderStartX + pad.right);
    int filledAreaX = analogRangeToScreen(pItem, maximumSliderArea);
    int filledHeight = size.y - (pad.bottom + pad.top);
    graphics->fillRect(sliderStartX, where.y + pad.top, filledAreaX, filledHeight, entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT1));
    int drawingPositionY = where.y;
    if(entry->getDisplayProperties()->getFont()) drawingPositionY += (size.y - (pad.bottom));
    copyMenuItemNameAndValue(pItem, buffer, bufferSize);
    int textStart = sliderStartX + (maximumSliderArea - textExtents(graphics, buffer, 10, 30).x) / 2;
    graphics->setCursor(textStart, drawingPositionY);
    graphics->print(buffer);
}

void AdaFruitGfxMenuRenderer::drawTitleArea(GridPositionRowCacheEntry* entry, Coord where, Coord size) {
    graphics->setFont(static_cast<const GFXfont *>(entry->getDisplayProperties()->getFont()));
    graphics->setTextSize(entry->getDisplayProperties()->getFontMagnification());
    MenuPadding pad = entry->getDisplayProperties()->getPadding();

    int fontYStart = where.y + pad.top;
    if (entry->getDisplayProperties()->getFont()) {
        fontYStart = where.y - (pad.bottom);
    }

    graphics->fillRect(where.x, where.y, size.x, size.y, entry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND));
    graphics->setTextColor(entry->getDisplayProperties()->getColor(ItemDisplayProperties::TEXT));
    graphics->setCursor(where.x, fontYStart);

    auto* pItem = entry->getMenuItem();
    pItem->copyNameToBuffer(buffer, bufferSize);
    if(pItem->isActive()) graphics->print('>');
    graphics->print(buffer);
    if(pItem->getMenuType() == MENUTYPE_BACK_VALUE && pItem->isActive()) graphics->print(" [..]");
}

void prepareAdaColorDefaultGfxConfig(AdaColorGfxMenuConfig* config) { 
    prepareDefaultGfxConfig((ColorGfxMenuConfig<void*>*)config);
}

void prepareAdaMonoGfxConfigLoRes(AdaColorGfxMenuConfig* config) {
	makePadding(config->titlePadding, 2, 1, 1, 1);
	makePadding(config->itemPadding, 1, 1, 1, 1);
	makePadding(config->widgetPadding, 2, 2, 0, 2);

	config->bgTitleColor = BLACK;
	config->fgTitleColor = WHITE;
	config->titleFont = NULL;
	config->titleBottomMargin = 2;
	config->widgetColor = WHITE;
	config->titleFontMagnification = 1;

	config->bgItemColor = WHITE;
	config->fgItemColor = BLACK;
	config->bgSelectColor = BLACK;
	config->fgSelectColor = WHITE;
	config->itemFont = NULL;
	config->itemFontMagnification = 1;

    config->editIcon = loResEditingIcon;
    config->activeIcon = loResActiveIcon;
    config->editIconHeight = 6;
    config->editIconWidth = 8;
}

void prepareAdaMonoGfxConfigOled(AdaColorGfxMenuConfig* config) {
	makePadding(config->titlePadding, 2, 1, 1, 1);
	makePadding(config->itemPadding, 1, 1, 1, 1);
	makePadding(config->widgetPadding, 2, 2, 0, 2);

	config->bgTitleColor = WHITE;
	config->fgTitleColor = BLACK;
	config->titleFont = NULL;
	config->titleBottomMargin = 2;
	config->widgetColor = BLACK;
	config->titleFontMagnification = 1;

	config->bgItemColor = BLACK;
	config->fgItemColor = WHITE;
	config->bgSelectColor = WHITE;
	config->fgSelectColor = BLACK;
	config->itemFont = NULL;
	config->itemFontMagnification = 1;

    config->editIcon = loResEditingIcon;
    config->activeIcon = loResActiveIcon;
    config->editIconHeight = 6;
    config->editIconWidth = 8;
}

BaseDialog* AdaFruitGfxMenuRenderer::getDialog() {
    if(dialog == NULL) {
        dialog = new AdaGfxDialog();
    }
    return dialog;
}

void AdaGfxDialog::internalRender(int currentValue) {
    AdaFruitGfxMenuRenderer* adaRenderer = reinterpret_cast<AdaFruitGfxMenuRenderer*>(MenuRenderer::getInstance());
    Adafruit_GFX* graphics = adaRenderer->getGraphics();

    auto& fact = adaRenderer->getDisplayPropertiesFactory();
    auto* pItemCfg = fact.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM);
    auto* pTitleCfg = fact.configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);
    if(needsDrawing == MENUDRAW_COMPLETE_REDRAW) {
        graphics->fillScreen(pItemCfg->getColor(ItemDisplayProperties::BACKGROUND));
    }

    graphics->setFont(static_cast<const GFXfont *>(pItemCfg->getFont()));
	graphics->setTextSize(pItemCfg->getFontMagnification());

    char data[20];
    safeProgCpy(data, headerPgm, sizeof(data));

	int fontYStart = pItemCfg->getPadding().top;
	Coord extents = textExtents(graphics, data, 0, pItemCfg->getFont() ? graphics->height() : 0);
	int dlgNextDraw = extents.y + pTitleCfg->getPadding().top + pTitleCfg->getPadding().bottom;
	if (pItemCfg->getFont()) {
	 	fontYStart = dlgNextDraw - (pTitleCfg->getPadding().bottom);
	}

	graphics->fillRect(0, 0, graphics->width(), dlgNextDraw, pTitleCfg->getColor(ItemDisplayProperties::BACKGROUND));
	graphics->setTextColor(pTitleCfg->getColor(ItemDisplayProperties::TEXT));
	graphics->setCursor(pTitleCfg->getPadding().left, fontYStart);
	graphics->print(data);

	dlgNextDraw += pTitleCfg->getSpaceAfter();

    int startingPosition = dlgNextDraw;
    fontYStart = dlgNextDraw + pItemCfg->getPadding().top;
	dlgNextDraw = dlgNextDraw + extents.y + pTitleCfg->getPadding().top + pTitleCfg->getPadding().bottom;
	if (pItemCfg->getFont()) {
	 	fontYStart = dlgNextDraw - (pTitleCfg->getPadding().bottom);
	}
    graphics->fillRect(0, startingPosition, graphics->width(), dlgNextDraw, pItemCfg->getColor(ItemDisplayProperties::BACKGROUND));
	graphics->setTextColor(pItemCfg->getColor(ItemDisplayProperties::TEXT));
	graphics->setCursor(pTitleCfg->getPadding().left, fontYStart);

	graphics->print(MenuRenderer::getInstance()->getBuffer());
    
    bool active;
    if(button1 != BTNTYPE_NONE) {
        active = copyButtonText(data, 0, currentValue);
        drawButton(graphics, pTitleCfg, data, 0, active);
    }
    if(button2 != BTNTYPE_NONE) {
        active = copyButtonText(data, 1, currentValue);
        drawButton(graphics, pTitleCfg, data, 1, active);
    }

    refreshDisplayIfNeeded(graphics, true);
}
void AdaGfxDialog::drawButton(Adafruit_GFX* gfx, ItemDisplayProperties* props, const char* title, uint8_t num, bool active) {
	Coord extents = textExtents(gfx, title, 0, props->getFont() ? gfx->height() : 0);
    int itemHeight = ( extents.y + props->getPadding().top + props->getPadding().bottom);
    int start = gfx->height() - itemHeight;
    int fontYStart = start + props->getPadding().top;
	if (props->getFont()) {
        fontYStart += extents.y;
	}
    int buttonWidth = gfx->width() / 2;
    int xOffset = (num == 0) ? 0 : buttonWidth;
    gfx->fillRect(xOffset, start, buttonWidth, itemHeight, props->getColor(active ? ItemDisplayProperties::HIGHLIGHT2 : ItemDisplayProperties::BACKGROUND));
	gfx->setTextColor(props->getColor(ItemDisplayProperties::TEXT));
    gfx->setCursor(xOffset + ((buttonWidth - extents.x) / 2), fontYStart);
    gfx->print(title);
}


/** A couple of helper functions that we'll submit for inclusion once a bit more testing is done */

/**************************************************************************/
/*!
   @brief      Draw a RAM-resident 1-bit image at the specified (x,y) position,
   from image data that may be wider or taller than the desired width and height.
   Imagine a cookie dough rolled out, where you can cut a rectangle out of it.
   It uses the specified foreground (for set bits) and background (unset bits) colors.
   This is particularly useful for GFXCanvas1 operations, where you can allocate the
   largest canvas needed and then use it for all drawing operations.

    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with monochrome bitmap
    @param    w   width of the portion you want to draw
    @param    h   Height of the portion you want to draw
    @param    totalWidth actual width of the bitmap
    @param    xStart X position of the image in the data
    @param    yStart Y position of the image in the data
    @param    color 16-bit 5-6-5 Color to draw pixels with
    @param    bg 16-bit 5-6-5 Color to draw background with
*/
/**************************************************************************/
void drawCookieCutBitmap(Adafruit_GFX* gfx, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w,
                         int16_t h, int16_t totalWidth, int16_t xStart, int16_t yStart,
                         uint16_t fgColor, uint16_t bgColor) {

    // total width here is different to the width we are drawing, imagine rolling out a long
    // line of dough and cutting cookies from it. The cookie is the part of the image we want
    uint16_t byteWidth = (totalWidth + 7) / 8; // Bitmap scanline pad = whole byte
    uint16_t yEnd = h + yStart;
    uint16_t xEnd = w + xStart;
    uint8_t byte;

    gfx->startWrite();

    for (uint16_t j = yStart; j < yEnd; j++, y++) {
        byte = bitmap[size_t(((j * byteWidth) + xStart) / 8)];
        for (uint16_t i = xStart; i < xEnd; i++) {
            if (i & 7U)
                byte <<= 1U;
            else
                byte = bitmap[size_t((j * byteWidth) + i / 8)];
            gfx->writePixel(x + (i - xStart), y, (byte & 0x80U) ? fgColor : bgColor);
        }
    }

    gfx->endWrite();
}

/**************************************************************************/
/*!
   @brief      Draw a RAM-resident 2-bit image at the specified (x,y) position,
   from image data that may be wider or taller than the desired width and height.
   Imagine a cookie dough rolled out, where you can cut a rectangle out of it.
   It uses the specified foreground (for set bits) and background (unset bits) colors.
   This is particularly useful for GFXCanvas2 operations, where you can allocate the
   largest canvas needed and then use it for all drawing operations.

    @param    x   Top left corner x coordinate
    @param    y   Top left corner y coordinate
    @param    bitmap  byte array with monochrome bitmap
    @param    w   width of the portion you want to draw
    @param    h   Height of the portion you want to draw
    @param    totalWidth actual width of the bitmap
    @param    xStart X position of the image in the data
    @param    yStart Y position of the image in the data
    @param    palette an array of 4 colours
*/
/**************************************************************************/
void drawCookieCutBitmap2(Adafruit_GFX* gfx, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w,
                          int16_t h, int16_t totalWidth, int16_t xStart, int16_t yStart,
                          uint16_t* palette) {

    // total width here is different to the width we are drawing, imagine rolling out a long
    // line of dough and cutting cookies from it. The cookie is the part of the image we want
    uint16_t byteWidth = (totalWidth + 3) / 4; // Bitmap scanline pad = whole byte
    uint16_t yEnd = h + yStart;
    uint16_t xEnd = w + xStart;
    uint8_t byte;

    gfx->startWrite();

    for (uint16_t j = yStart; j < yEnd; j++, y++) {
        byte = bitmap[size_t(((j * byteWidth) + xStart) / 8)];
        for (uint16_t i = xStart; i < xEnd; i++) {
            if (i & 3U)
                byte <<= 2U;
            else
                byte = bitmap[size_t((j * byteWidth) + i / 8)];
            auto col = (byte & 0xC0U) >> 6U;
            gfx->writePixel(x + (i - xStart), y,  palette[col]);
        }
    }

    gfx->endWrite();
}
