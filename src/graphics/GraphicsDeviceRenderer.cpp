/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "GraphicsDeviceRenderer.h"

namespace tcgfx {

const Coord zeroPointCoord(0,0);

const Coord rendererXbmArrowSize(8, 11);
static unsigned char rendererUpArrowXbm[] = { 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03 };
static unsigned char rendererDownArrowXbm[] = { 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0 };

color_t defaultItemPaletteMono[] = {2, 0, 1, 0};
color_t defaultTitlePaletteMono[] = {0, 1, 0, 0};
color_t defaultTitlePalette[] = {RGB(0,0,0), RGB(20,132,255), RGB(192,192,192), RGB(0,133,255)};
color_t defaultItemPalette[] = {RGB(255, 255, 255), RGB(0,64,135), RGB(20,133,255), RGB(31,100,178)};

GraphicsDeviceRenderer::GraphicsDeviceRenderer(int bufferSize, const char *appTitle, DeviceDrawable *drawable)
        : BaseGraphicalRenderer(bufferSize, 1, 1, false, appTitle), rootDrawable(drawable), drawable(drawable) {
}

void GraphicsDeviceRenderer::drawingCommand(BaseGraphicalRenderer::RenderDrawingCommand command) {
    switch(command) {
        case DRAW_COMMAND_CLEAR: {
            auto* cfg = propertiesFactory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM);
            drawable->setDrawColor(cfg->getColor(ItemDisplayProperties::BACKGROUND));
            drawable->drawBox(Coord(0, 0), Coord(width, height), true);
            break;
        }
        case DRAW_COMMAND_START:
        case DRAW_COMMAND_ENDED:
            drawable->transaction(command == DRAW_COMMAND_START, redrawNeeded);
            redrawNeeded = false;
            break;
    }
}

void GraphicsDeviceRenderer::drawWidget(Coord where, TitleWidget *widget, color_t colorFg, color_t colorBg) {
    redrawNeeded = true;
    drawable->setColors(colorFg, colorBg);
    drawable->drawXBitmap(where, Coord(widget->getWidth(), widget->getHeight()), widget->getCurrentIcon());
}

void GraphicsDeviceRenderer::drawMenuItem(GridPositionRowCacheEntry *entry, Coord where, Coord areaSize, bool drawAll) {
    redrawNeeded = true;

    // icons never use double buffer drawing because they use a lot of BPP
    auto drawingMode = entry->getPosition().getDrawingMode();
    if(drawingMode == GridPosition::DRAW_AS_ICON_ONLY || drawingMode == GridPosition::DRAW_AS_ICON_TEXT) {
        drawIconItem(entry, where, areaSize);
        return;
    }

    // if we are drawing everything, then we need to clear out the areas in between items.
    if(drawAll && entry->getDisplayProperties()->getSpaceAfter() > 0) {
        auto* bgConfig = propertiesFactory.configFor(menuMgr.getCurrentSubMenu(), ItemDisplayProperties::COMPTYPE_ITEM);
        drawable->setDrawColor(bgConfig->getColor(ItemDisplayProperties::BACKGROUND));
        drawable->drawBox(Coord(where.x, where.y + areaSize.y), Coord(areaSize.x, entry->getDisplayProperties()->getSpaceAfter()), true);
    }

    auto* subDevice = rootDrawable->getSubDeviceFor(where, areaSize, entry->getDisplayProperties()->getPalette(), 6);
    if(subDevice) {
        subDevice->startDraw();
    }

    drawable = subDevice ? subDevice : rootDrawable;
    const Coord& wh = subDevice ? zeroPointCoord : where;
    switch(drawingMode) {
        case GridPosition::DRAW_TEXTUAL_ITEM:
        case GridPosition::DRAW_TITLE_ITEM:
        default:
            drawTextualItem(entry, wh, areaSize);
            break;
        case GridPosition::DRAW_INTEGER_AS_UP_DOWN:
            drawUpDownItem(entry, wh, areaSize);
            break;
        case GridPosition::DRAW_INTEGER_AS_SCROLL:
            if(entry->getMenuItem()->getMenuType() != MENUTYPE_INT_VALUE) return; // disallowed
            drawSlider(entry, reinterpret_cast<AnalogMenuItem*>(entry->getMenuItem()), wh, areaSize);
            break;
    }

    if(subDevice) {
        drawable = rootDrawable;
        subDevice->endDraw();
    }
}

inline bool isActiveOrEditing(MenuItem* pItem) {
    auto mt = pItem->getMenuType();
    return (pItem->isEditing() || pItem->isActive()) && mt != MENUTYPE_TITLE_ITEM && mt != MENUTYPE_BACK_VALUE;
}

void GraphicsDeviceRenderer::internalDrawText(GridPositionRowCacheEntry* pEntry, const Coord& where, const Coord& size) {
    GridPosition::GridJustification just = pEntry->getPosition().getJustification();
    ItemDisplayProperties *props = pEntry->getDisplayProperties();
    auto padding = props->getPadding();

    color_t fg;
    if(isActiveOrEditing(pEntry->getMenuItem())) {
        fg = propertiesFactory.getSelectedColor(ItemDisplayProperties::TEXT);
    }
    else {
        fg = props->getColor(ItemDisplayProperties::TEXT);
    }
    
    if(just == GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT) {
        // special case, title left, value right.
        Coord wh = Coord(where.x + padding.left, where.y + padding.top);
        pEntry->getMenuItem()->copyNameToBuffer(buffer, bufferSize);
        serdebugF4("item: ", buffer, size.y, where.y);
        drawable->setDrawColor(fg);
        drawable->drawText(wh, props->getFont(), props->getFontMagnification(), buffer);

        copyMenuItemValue(pEntry->getMenuItem(), buffer, bufferSize);
        int16_t right = where.x + size.x - (drawable->textExtents(props->getFont(), props->getFontMagnification(), buffer).x + padding.right);
        wh.x = right;
        drawable->setDrawColor(fg);
        drawable->drawText(wh, props->getFont(), props->getFontMagnification(), buffer);
    }
    else {
        char sz[32];
        if(itemNeedsValue(just))
            copyMenuItemNameAndValue(pEntry->getMenuItem(), sz, sizeof sz, 0);
        else
            pEntry->getMenuItem()->copyNameToBuffer(sz, sizeof sz);

        int startPosition = padding.left;
        if(just == GridPosition::JUSTIFY_RIGHT_WITH_VALUE || just == GridPosition::JUSTIFY_RIGHT_NO_VALUE) {
            startPosition = size.x - (drawable->textExtents(props->getFont(), props->getFontMagnification(), sz).x + padding.right);
        }
        else if(just == GridPosition::JUSTIFY_CENTER_WITH_VALUE || just == GridPosition::JUSTIFY_CENTER_NO_VALUE) {
            startPosition = ((size.x - drawable->textExtents(props->getFont(), props->getFontMagnification(), sz).x) / 2) + padding.right;
        }
        drawable->setDrawColor(fg);
        drawable->drawText(Coord(startPosition + where.x, where.y + padding.top), props->getFont(), props->getFontMagnification(), sz);
        serdebugF4("intTx ", sz, startPosition + where.x, (where.y + size.y) - padding.bottom);
    }
}

int GraphicsDeviceRenderer::drawCoreLineItem(GridPositionRowCacheEntry* entry, DrawableIcon* icon, const Coord &where, const Coord &size) {
    auto pad = entry->getDisplayProperties()->getPadding();
    serdebugF4("Drawing at: ", where.y, size.x, size.y);

    entry->getMenuItem()->setChanged(false); // we are drawing the item so it's no longer changed.

    color_t bgColor, textColor;
    if(isActiveOrEditing(entry->getMenuItem())) {
        bgColor = propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND);
        textColor = propertiesFactory.getSelectedColor(ItemDisplayProperties::TEXT);
    }
    else {
        bgColor = entry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND);
        textColor = entry->getDisplayProperties()->getColor(ItemDisplayProperties::TEXT);
    }

    int imgMiddleY = where.y + ((size.y - icon->getDimensions().y) / 2);
    if(entry->getMenuItem()->isEditing() || entry->getMenuItem()->isActive()) {
        drawable->setDrawColor(bgColor);
        drawable->drawBox(where, size, true);
        drawable->setColors(textColor, bgColor);
        drawable->drawXBitmap(Coord(pad.left, imgMiddleY), icon->getDimensions(),  icon->getIcon(false));
    }
    else {
        drawable->setDrawColor(bgColor);
        drawable->drawBox(where, size, true);
    }
    return imgMiddleY;
}

void GraphicsDeviceRenderer::drawUpDownItem(GridPositionRowCacheEntry *entry, const Coord& where, const Coord& size) {
    auto padding = entry->getDisplayProperties()->getPadding();
    auto* icon = propertiesFactory.iconForMenuItem(entry->getMenuItem()->isEditing() ? SPECIAL_ID_EDIT_ICON :SPECIAL_ID_ACTIVE_ICON);
    int iconOffset = (icon) ? icon->getDimensions().x + padding.left : 0;

    drawCoreLineItem(entry, icon, where, size);

    if(hasTouchScreen && (entry->getMenuItem()->isActive() || entry->getMenuItem()->isEditing())) {
        int buttonSize = size.y - 1;
        int offset = (buttonSize - rendererXbmArrowSize.y) / 2;
        int downButtonLocation = where.x + iconOffset;
        int upButtonLocation = (size.x + where.x) - (padding.right + buttonSize);
        int textStartX = iconOffset + size.y + padding.left;
        auto hl = entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT1);
        auto txtCol = entry->getDisplayProperties()->getColor(ItemDisplayProperties::TEXT);
        drawable->setDrawColor(hl);
        drawable->drawBox(Coord(downButtonLocation, where.y), Coord(size.y, size.y), true);
        drawable->drawBox(Coord(upButtonLocation, where.y), Coord(size.y, size.y), true);
        drawable->setColors(txtCol, hl);
        drawable->drawXBitmap(Coord(downButtonLocation + offset, where.y + offset), rendererXbmArrowSize, rendererDownArrowXbm);
        drawable->drawXBitmap(Coord(upButtonLocation + offset, where.y + offset), rendererXbmArrowSize, rendererUpArrowXbm);

        internalDrawText(entry, Coord(where.x + textStartX, where.y),
                         Coord(size.x - (((buttonSize + padding.right) * 2) + iconOffset), size.y));
    }
    else {
        internalDrawText(entry, Coord(where.x + iconOffset, where.y), Coord(size.x - iconOffset, size.y));
    }
}

void GraphicsDeviceRenderer::drawTextualItem(GridPositionRowCacheEntry* pEntry, const Coord& where, const Coord& size) {
    auto padding = pEntry->getDisplayProperties()->getPadding();
    auto* icon = propertiesFactory.iconForMenuItem(pEntry->getMenuItem()->isEditing() ? SPECIAL_ID_EDIT_ICON :SPECIAL_ID_ACTIVE_ICON);
    int iconOffset = (icon) ? icon->getDimensions().x + padding.left : 0;
    drawCoreLineItem(pEntry, icon, where, size);

    internalDrawText(pEntry, Coord(where.x + iconOffset, where.y), Coord(size.x - iconOffset, size.y));
}


void GraphicsDeviceRenderer::drawSlider(GridPositionRowCacheEntry* entry, AnalogMenuItem* pItem, const Coord& where, const Coord& size) {
    auto* icon = propertiesFactory.iconForMenuItem(entry->getMenuItem()->isEditing() ? SPECIAL_ID_EDIT_ICON :SPECIAL_ID_ACTIVE_ICON);
    drawCoreLineItem(entry, icon, where, size);
    MenuPadding pad = entry->getDisplayProperties()->getPadding();
    int iconOffset = (icon) ? pad.left + icon->getDimensions().x : 0;
    int maximumSliderArea = size.x - (iconOffset + where.x + pad.right);
    int filledAreaX = analogRangeToScreen(pItem, maximumSliderArea);
    int outsideAreaX = maximumSliderArea - filledAreaX;
    drawable->setDrawColor(entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT1));
    drawable->drawBox(Coord(where.x + iconOffset, where.y), Coord(filledAreaX, size.y), true);
    drawable->setDrawColor(entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT2));
    drawable->drawBox(Coord(where.x + iconOffset + filledAreaX, where.y), Coord(outsideAreaX, size.y), true);
    internalDrawText(entry, Coord(where.x + iconOffset, where.y), Coord(size.x - iconOffset, size.y));
}

void GraphicsDeviceRenderer::drawIconItem(GridPositionRowCacheEntry* pEntry, const Coord& where, const Coord& size) {
    auto* props = pEntry->getDisplayProperties();
    auto* pItem = pEntry->getMenuItem();

    drawable->setDrawColor(props->getColor(ItemDisplayProperties::BACKGROUND));
    drawable->drawBox(where, size, true);

    if(pItem->isActive()) {
        drawable->setDrawColor(props->getColor(ItemDisplayProperties::TEXT));
        drawable->drawBox(where, size, false);
    }

    auto* pIcon = propertiesFactory.iconForMenuItem(pItem->getId());
    if(pIcon == nullptr) return;

    Coord iconWhere(where.x + ((size.x - pIcon->getDimensions().x) / 2), where.y + pEntry->getDisplayProperties()->getPadding().top);

    bool sel = false;
    if(pItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
        auto* boolItem = reinterpret_cast<BooleanMenuItem*>(pItem);
        sel = boolItem->getBoolean();
    }
    drawable->setColors(props->getColor(ItemDisplayProperties::HIGHLIGHT1), props->getColor(ItemDisplayProperties::BACKGROUND));
    drawable->drawBitmap(iconWhere, pIcon, sel);
}

int GraphicsDeviceRenderer::heightForFontPadding(const void *font, int mag, MenuPadding &padding) {
    int baseline=0;
    Coord sizeInfo = drawable->textExtents(font, mag, "();yg1", &baseline);
    int hei = sizeInfo.y + padding.top + padding.bottom;
    padding.bottom += baseline; // add the baseline to padding.
    return hei;
}

void GraphicsDeviceRenderer::setGraphicsConfiguration(void* cfg) {
    auto* gfxConfig = reinterpret_cast<ColorGfxMenuConfig<const void*>*>(cfg);
    if (gfxConfig->editIcon == nullptr || gfxConfig->activeIcon == nullptr) {
        gfxConfig->editIcon = defEditingIcon;
        gfxConfig->activeIcon = defActiveIcon;
        gfxConfig->editIconWidth = 16;
        gfxConfig->editIconHeight = 12;
    }

    setUseSliderForAnalog(false);// the colour choices probably won't work well with this.

    int titleHeight = heightForFontPadding(gfxConfig->titleFont, gfxConfig->titleFontMagnification, gfxConfig->titlePadding);
    int itemHeight = heightForFontPadding(gfxConfig->itemFont, gfxConfig->itemFontMagnification, gfxConfig->itemPadding);

    preparePropertiesFromConfig(propertiesFactory, reinterpret_cast<const ColorGfxMenuConfig<const void *>*>(gfxConfig), titleHeight, itemHeight);

    setDisplayDimensions(rootDrawable->getDisplayDimensions().x, rootDrawable->getDisplayDimensions().y);
}

void GraphicsDeviceRenderer::prepareDisplay(bool mono, const void* itemFont, int itemMag, const void* titleFont, int titleMag, bool needEditingIcons) {
    setDisplayDimensions(rootDrawable->getDisplayDimensions().x, rootDrawable->getDisplayDimensions().y);

    auto& factory = propertiesFactory;

    if(mono) {
        factory.setSelectedColors(0, 2);
    }
    else {
        factory.setSelectedColors(RGB(31, 88, 100), RGB(255, 255, 255));
    }

    bool medResOrBetter = width > 160;
    MenuPadding titlePadding(medResOrBetter ? 4 : 2);
    MenuPadding itemPadding(medResOrBetter ? 2 : 1);
    int titleHeight = heightForFontPadding(titleFont, titleMag, titlePadding);
    int itemHeight = heightForFontPadding(itemFont, itemMag, itemPadding);
    auto* titlePalette = mono ? defaultTitlePaletteMono : defaultTitlePalette;
    auto* itemPalette = mono ? defaultItemPaletteMono : defaultItemPalette;

    if(needEditingIcons && itemHeight > 12) {
        factory.addImageToCache(DrawableIcon(SPECIAL_ID_EDIT_ICON, Coord(16, 12),DrawableIcon::ICON_XBITMAP, defEditingIcon));
        factory.addImageToCache(DrawableIcon(SPECIAL_ID_ACTIVE_ICON, Coord(16, 12),DrawableIcon::ICON_XBITMAP, defActiveIcon));
    }
    else {
        factory.addImageToCache(DrawableIcon(SPECIAL_ID_EDIT_ICON, Coord(8, 6),DrawableIcon::ICON_XBITMAP, loResEditingIcon));
        factory.addImageToCache(DrawableIcon(SPECIAL_ID_ACTIVE_ICON, Coord(8, 6),DrawableIcon::ICON_XBITMAP, loResActiveIcon));
    }


    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, titlePalette, titlePadding, titleFont, titleMag, medResOrBetter ? 3 : 1, titleHeight, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE );
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, itemPalette, itemPadding, itemFont, itemMag, 1, itemHeight, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT );
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, itemPalette, itemPadding, itemFont, itemMag, 1, itemHeight, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE );
}

void GraphicsDeviceRenderer::fillWithBackgroundTo(int endPoint) {
    auto* bgConfig = propertiesFactory.configFor(menuMgr.getCurrentSubMenu(), ItemDisplayProperties::COMPTYPE_ITEM);
    drawable->setDrawColor(bgConfig->getColor(ItemDisplayProperties::BACKGROUND));
    drawable->drawBox(Coord(0, endPoint), Coord(width, height-endPoint), true);
}

} // namespace tcgfx