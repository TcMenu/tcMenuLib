/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "GraphicsDeviceRenderer.h"
#include <tcUnicodeHelper.h>

namespace tcgfx {

    const Coord rendererXbmArrowSize(8, 11);
    static unsigned char rendererUpArrowXbm[] = { 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03 };
    static unsigned char rendererDownArrowXbm[] = { 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0 };

    inline bool isActiveOrEditing(MenuItem* pItem) {
        auto mt = pItem->getMenuType();
        return (pItem->isEditing() || pItem->isActive()) && mt != MENUTYPE_TITLE_ITEM && mt != MENUTYPE_BACK_VALUE;
    }

    GraphicsDeviceRenderer::GraphicsDeviceRenderer(int bufferSize, const char *appTitle, DeviceDrawable *drawable)
            : BaseGraphicalRenderer(bufferSize, 1, 1, false, appTitle), rootDrawable(drawable), drawable(drawable) {
        const Coord &coord = rootDrawable->getDisplayDimensions();
        width = coord.x;
        height = coord.y;
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
        entry->getMenuItem()->setChanged(false);

        // if it's in a multi grid layout, put a small gap at the start of each one.
        if(entry->getPosition().getGridSize() > 1) {
            auto space = entry->getDisplayProperties()->getSpaceAfter();
            drawable->setDrawColor(entry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND));
            drawable->drawBox(where, Coord(space, areaSize.y), true);
            areaSize.x -= space;
            where.x += space;
        }

        // if we are drawing everything, then we need to clear out the areas in between items.
        if(drawAll && entry->getDisplayProperties()->getSpaceAfter() > 0) {
            auto* bgConfig = propertiesFactory.configFor(menuMgr.getCurrentSubMenu(), ItemDisplayProperties::COMPTYPE_ITEM);
            drawable->setDrawColor(bgConfig->getColor(ItemDisplayProperties::BACKGROUND));
            drawable->drawBox(Coord(where.x, where.y + areaSize.y), Coord(areaSize.x, entry->getDisplayProperties()->getSpaceAfter()), true);
        }

        // icons never use double buffer drawing because they may use a lot of BPP and don't change often in the main
        auto drawingMode = entry->getPosition().getDrawingMode();
        if(drawingMode == GridPosition::DRAW_AS_ICON_ONLY || drawingMode == GridPosition::DRAW_AS_ICON_TEXT) {
            drawIconItem(entry, where, areaSize);
            return;
        }

        DeviceDrawable* subDevice = nullptr;
        if(rootDrawable->getSubDeviceType() == DeviceDrawable::SUB_DEVICE_4BPP) {
            subDevice = rootDrawable->getSubDeviceFor(where, areaSize, entry->getDisplayProperties()->getPalette(), 6);
        } else if(rootDrawable->getSubDeviceType() == DeviceDrawable::SUB_DEVICE_2BPP) {
            color_t palette[4];
            bool selected = isActiveOrEditing(entry->getMenuItem());
            palette[ItemDisplayProperties::TEXT] = (selected) ? propertiesFactory.getSelectedColor(ItemDisplayProperties::TEXT) : entry->getDisplayProperties()->getPalette()[ItemDisplayProperties::TEXT];
            palette[ItemDisplayProperties::BACKGROUND] = (selected) ? propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND) : entry->getDisplayProperties()->getPalette()[ItemDisplayProperties::BACKGROUND];
            palette[ItemDisplayProperties::HIGHLIGHT1] = entry->getDisplayProperties()->getPalette()[ItemDisplayProperties::HIGHLIGHT1];
            palette[ItemDisplayProperties::HIGHLIGHT2] = entry->getDisplayProperties()->getPalette()[ItemDisplayProperties::HIGHLIGHT2];
            subDevice = rootDrawable->getSubDeviceFor(where, areaSize, palette, 4);
        }

        if(subDevice) {
            subDevice->startDraw();
        }
        drawable = subDevice ? subDevice : rootDrawable;
        Coord wh = subDevice ? Coord(0,0) : where;

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

    int GraphicsDeviceRenderer::calculateSpaceBetween(const void* font, uint8_t mag, const char* buffer, int start, int end) {
        int bufferLen = (int)strlen(buffer);
        int pos = start;
        size_t idx = 0;
        char sz[20];
        while(pos < bufferLen && pos < end && idx < (sizeof(sz) - 1)) {
            sz[idx] = buffer[pos];
            pos++;
            idx++;
        }
        sz[idx] = 0;
        auto extents = drawable->textExtents(font, mag, sz);
        return int(extents.x);
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

        copyMenuItemValue(pEntry->getMenuItem(), buffer, bufferSize);
        bool weAreEditingWithCursor = pEntry->getMenuItem()->isEditing() && menuMgr.getCurrentEditor() != nullptr
                                      && editorHintNeedsCursor(menuMgr.getEditorHints().getEditorRenderingType());

        bool valueNeeded = true;
        if(pEntry->getMenuItem()->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
            if (reinterpret_cast<BooleanMenuItem *>(pEntry->getMenuItem())->getBooleanNaming() == NAMING_CHECKBOX) {
                valueNeeded = false;
            }
        }

        if(just == GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT || weAreEditingWithCursor) {
            // special case, title left, value right.
            Coord wh = Coord(where.x + padding.left, where.y + padding.top);
            pEntry->getMenuItem()->copyNameToBuffer(buffer, bufferSize);
            serlogF4(SER_TCMENU_DEBUG, "item: ", buffer, size.y, where.y);
            drawable->setDrawColor(fg);
            drawable->drawText(wh, props->getFont(), props->getFontMagnification(), buffer);

            if(valueNeeded) {
                copyMenuItemValue(pEntry->getMenuItem(), buffer, bufferSize);
            } else buffer[0] = 0;
            int16_t right = where.x + size.x - (drawable->textExtents(props->getFont(), props->getFontMagnification(), buffer).x + padding.right);
            wh.x = right;
            if(weAreEditingWithCursor) {
                drawable->setDrawColor(propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND, true));
                auto& hints = menuMgr.getEditorHints();
                int startX = calculateSpaceBetween(props->getFont(), props->getFontMagnification(), buffer, 0, hints.getStartIndex() );
                int lenX = max(MINIMUM_CURSOR_SIZE, calculateSpaceBetween(props->getFont(), props->getFontMagnification(), buffer, hints.getStartIndex(), hints.getEndIndex()));
                int whereX = min(int(width) - MINIMUM_CURSOR_SIZE, int(wh.x + startX));
                drawable->drawBox(Coord(whereX, where.y + size.y - 1), Coord(lenX, 1), true);
                drawable->setDrawColor(fg);
                if(hints.getEndIndex() > strlen(buffer)) wh.x = wh.x - (unsigned int)MINIMUM_CURSOR_SIZE;
                drawable->drawText(wh, props->getFont(), props->getFontMagnification(), buffer);
            }
            else {
                drawable->drawText(wh, props->getFont(), props->getFontMagnification(), buffer);
            }
        }
        else {
            char sz[32];
            bool nameNeeded = itemNeedsName(just);
            if(valueNeeded && nameNeeded) {
                copyMenuItemNameAndValue(pEntry->getMenuItem(), sz, sizeof sz, 0);
            } else if(valueNeeded) {
                copyMenuItemValue(pEntry->getMenuItem(), sz, sizeof sz);
            } else {
                pEntry->getMenuItem()->copyNameToBuffer(sz, sizeof sz);
            }

            int startPosition = padding.left;
            if(coreJustification(just) == GridPosition::CORE_JUSTIFY_RIGHT) {
                startPosition = size.x - (drawable->textExtents(props->getFont(), props->getFontMagnification(), sz).x + padding.right);
            }
            else if(coreJustification(just) == GridPosition::CORE_JUSTIFY_CENTER) {
                startPosition = ((size.x - drawable->textExtents(props->getFont(), props->getFontMagnification(), sz).x) / 2) + padding.right;
            }
            drawable->setDrawColor(fg);
            drawable->drawText(Coord(startPosition + where.x, where.y + padding.top), props->getFont(), props->getFontMagnification(), sz);
            serlogF4(SER_TCMENU_DEBUG, "intTx ", sz, startPosition + where.x, (where.y + size.y) - padding.bottom);
        }
    }

    void GraphicsDeviceRenderer::drawCoreLineItem(GridPositionRowCacheEntry* entry, DrawableIcon* icon, Coord &where, Coord &size, bool drawBg) {
        auto pad = entry->getDisplayProperties()->getPadding();
        serlogF4(SER_TCMENU_DEBUG, "Drawing at: ", where.y, size.x, size.y);

        // work out what background and drawing colors to use
        color_t bgColor, textColor;
        bool forceBorder = false;

        if(isActiveOrEditing(entry->getMenuItem())) {
            bgColor = propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND);
            textColor = propertiesFactory.getSelectedColor(ItemDisplayProperties::TEXT);
            forceBorder = (icon == nullptr) && (propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND) == entry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND));
        }
        else {
            bgColor = entry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND);
            textColor = entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT2);
        }

        auto xoffset = (icon) ? icon->getDimensions().x : 0;

        // draw any active arrow or blank space that's needed first
        if((entry->getMenuItem()->isEditing() || entry->getMenuItem()->isActive())) {
            drawable->setDrawColor(bgColor);
            // drawing too much as a real impact on some displays, when we don't need to render the background, we do not
            Coord adjustedSize = drawBg ? size : Coord(xoffset + pad.left, size.y);
            drawable->drawBox(where, adjustedSize, true);
            drawable->setColors(textColor, bgColor);
            if(icon) {
                int imgMiddleY = where.y + ((size.y - icon->getDimensions().y) / 2);
                drawable->drawXBitmap(Coord(where.x + pad.left, imgMiddleY), icon->getDimensions(),
                                      icon->getIcon(false));
            }
        }
        else {
            Coord adjustedSize = drawBg ? size : Coord(xoffset + pad.left, size.y);
            drawable->setDrawColor(bgColor);
            drawable->drawBox(where, adjustedSize, true);
        }

        if(icon) {
            auto adjust = icon->getDimensions().x + pad.left;
            where.x += adjust;
            size.x -= adjust;
        }

        // draw the border around the item (excluding the arrow)
        MenuBorder border = entry->getDisplayProperties()->getBorder();
        if(!border.isBorderOff() || forceBorder) {
            if(forceBorder) border = MenuBorder(1);
            drawable->setDrawColor(textColor);
            drawBorderAndAdjustSize(where, size, border);
        }

    }

    void GraphicsDeviceRenderer::drawBorderAndAdjustSize(Coord &where, Coord &size, MenuBorder &border) {
        if(border.areAllBordersEqual()) {
            for(int i=0; i<border.left;++i) {
                drawable->drawBox(where, size, false);
                where.x++;
                where.y++;
                size.x -= 2;
                size.y -= 2;
            }
        }
        else {
            if(border.left) {
                drawable->drawBox(Coord(where.x, where.y), Coord(border.left, size.y), true);
                where.x -= border.left;
                size.x -= border.left;
            }
            if(border.right) {
                drawable->drawBox(Coord(where.x + size.x - border.right, where.y), Coord(border.right, size.y), true);
                where.x -= border.right;
                size.x -= border.right;
            }
            if(border.bottom) {
                drawable->drawBox(Coord(where.x, where.y + size.y - border.bottom), Coord(size.x, border.bottom), true);
                where.y -= border.bottom;
                size.y -= border.bottom;
            }
            if(border.top) {
                drawable->drawBox(Coord(where.x, where.y), Coord(size.x, border.top), true);
                where.y -= border.bottom;
                size.y -= border.bottom;
            }
        }
    }

    void GraphicsDeviceRenderer::drawCheckbox(GridPositionRowCacheEntry *entry, Coord& where, Coord& size) {
        auto padding = entry->getDisplayProperties()->getPadding();
        auto* icon = propertiesFactory.iconForMenuItem(entry->getMenuItem()->isEditing() ? SPECIAL_ID_EDIT_ICON :SPECIAL_ID_ACTIVE_ICON);

        drawCoreLineItem(entry, icon, where, size, true);
        auto hei = size.y - (padding.top + padding.top);
        auto startingX = where.x + size.x - (padding.left + padding.right + hei);
        auto boolItem = reinterpret_cast<BooleanMenuItem*>(entry->getMenuItem());
        auto hl = entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT1);
        auto txtCol = entry->getDisplayProperties()->getColor(ItemDisplayProperties::TEXT);

        drawable->setDrawColor(txtCol);
        drawable->drawBox(Coord(startingX, where.y + padding.top), Coord(hei, hei), false);
        if(hl != txtCol) {
            drawable->drawBox(Coord(startingX + 1, where.y + padding.top + 1), Coord(hei - 2, hei - 2), false);
        }
        if(boolItem->getBoolean()) {
            drawable->setDrawColor(hl);
            drawable->drawBox(Coord(startingX + 2, where.y + padding.top + 2), Coord(hei - 4, hei - 4), true);
        }
        internalDrawText(entry, where, Coord(size.x - (hei + padding.left), size.y));
    }

    void GraphicsDeviceRenderer::drawUpDownItem(GridPositionRowCacheEntry *entry, Coord& where, Coord& size) {
        auto padding = entry->getDisplayProperties()->getPadding();
        auto* icon = propertiesFactory.iconForMenuItem(entry->getMenuItem()->isEditing() ? SPECIAL_ID_EDIT_ICON :SPECIAL_ID_ACTIVE_ICON);

        drawCoreLineItem(entry, icon, where, size, true);

        if(hasTouchScreen && (entry->getMenuItem()->isActive() || entry->getMenuItem()->isEditing())) {
            int buttonSize = size.y - 1;
            int offset = (buttonSize - rendererXbmArrowSize.y) / 2;
            int downButtonLocation = where.x;
            int upButtonLocation = (size.x + where.x) - (padding.right + buttonSize);
            int textStartX = size.y + padding.left;
            auto hl = entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT1);
            auto txtCol = entry->getDisplayProperties()->getColor(ItemDisplayProperties::TEXT);
            drawable->setDrawColor(hl);
            drawable->drawBox(Coord(downButtonLocation, where.y), Coord(size.y, size.y), true);
            drawable->drawBox(Coord(upButtonLocation, where.y), Coord(size.y, size.y), true);
            drawable->setColors(txtCol, hl);
            drawable->drawXBitmap(Coord(downButtonLocation + offset, where.y + offset), rendererXbmArrowSize, rendererDownArrowXbm);
            drawable->drawXBitmap(Coord(upButtonLocation + offset, where.y + offset), rendererXbmArrowSize, rendererUpArrowXbm);

            internalDrawText(entry, Coord(where.x + textStartX, where.y),
                             Coord(size.x - (((buttonSize + padding.right) * 2)), size.y));
        }
        else {
            internalDrawText(entry, Coord(where.x, where.y), Coord(size.x, size.y));
        }
    }

    void GraphicsDeviceRenderer::drawTextualItem(GridPositionRowCacheEntry* pEntry, Coord& where, Coord& size) {
        if(pEntry->getMenuItem()->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
            auto boolItem = reinterpret_cast<BooleanMenuItem*>(pEntry->getMenuItem());
            if(boolItem->getBooleanNaming() == NAMING_CHECKBOX) {
                drawCheckbox(pEntry, where, size);
                return;
            }
        }
        auto* icon = propertiesFactory.iconForMenuItem(pEntry->getMenuItem()->isEditing() ? SPECIAL_ID_EDIT_ICON :SPECIAL_ID_ACTIVE_ICON);
        drawCoreLineItem(pEntry, icon, where, size, true);
        internalDrawText(pEntry, Coord(where.x, where.y), Coord(size.x, size.y));
    }

    void GraphicsDeviceRenderer::drawSlider(GridPositionRowCacheEntry* entry, AnalogMenuItem* pItem, Coord& where, Coord& size) {
        auto* icon = propertiesFactory.iconForMenuItem(entry->getMenuItem()->isEditing() ? SPECIAL_ID_EDIT_ICON :SPECIAL_ID_ACTIVE_ICON);
        drawCoreLineItem(entry, icon, where, size, false);
        ItemDisplayProperties *props = entry->getDisplayProperties();
        MenuPadding pad = props->getPadding();
        int maximumSliderArea = size.x - pad.right;
        int filledAreaX = analogRangeToScreen(pItem, maximumSliderArea);
        int outsideAreaX = maximumSliderArea - filledAreaX;
        drawable->setDrawColor(props->getColor(ItemDisplayProperties::HIGHLIGHT1));
        drawable->drawBox(Coord(where.x, where.y), Coord(filledAreaX, size.y), true);
        auto mainBg = (pItem->isActive() || pItem->isEditing()) ? propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND) : props->getColor(ItemDisplayProperties::BACKGROUND);
        drawable->setDrawColor(mainBg);
        drawable->drawBox(Coord(where.x + filledAreaX, where.y), Coord(outsideAreaX, size.y), true);
        internalDrawText(entry, Coord(where.x, where.y), Coord(size.x, size.y));
    }

    void GraphicsDeviceRenderer::drawIconItem(GridPositionRowCacheEntry* pEntry, Coord& where, Coord& size) {
        auto* pItem = pEntry->getMenuItem();

        drawCoreLineItem(pEntry, nullptr, where, size, true);

        auto* pIcon = propertiesFactory.iconForMenuItem(pItem->getId());
        if(pIcon == nullptr) return;

        auto effectiveTop = where.y + pEntry->getDisplayProperties()->getPadding().top;
        Coord iconWhere(where.x + ((size.x - pIcon->getDimensions().x) / 2), effectiveTop);

        bool sel = false;
        if(pItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
            auto* boolItem = reinterpret_cast<BooleanMenuItem*>(pItem);
            sel = boolItem->getBoolean();
        }

        if(isActiveOrEditing(pEntry->getMenuItem())) {
            drawable->setColors(propertiesFactory.getSelectedColor(ItemDisplayProperties::TEXT), propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND));
        }
        else {
            drawable->setColors(pEntry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT2), pEntry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND));
        }
        drawable->drawBitmap(iconWhere, pIcon, sel);

        if(pEntry->getPosition().getDrawingMode() == GridPosition::DRAW_AS_ICON_TEXT) {
            effectiveTop += pIcon->getDimensions().y;
            internalDrawText(pEntry, Coord(where.x, effectiveTop), Coord(size.x, size.y - effectiveTop));
        }
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

    void GraphicsDeviceRenderer::fillWithBackgroundTo(int endPoint) {
        if(endPoint >= height) return; // nothing to do when the display is already full.
        auto* bgConfig = propertiesFactory.configFor(menuMgr.getCurrentMenu(), ItemDisplayProperties::COMPTYPE_ITEM);
        drawable->setDrawColor(bgConfig->getColor(ItemDisplayProperties::BACKGROUND));
        drawable->drawBox(Coord(0, endPoint), Coord(width, height-endPoint), true);
    }

    void DeviceDrawable::drawText(const Coord& where, const void* font, int mag, const char* text) {
        auto handler = getUnicodeHandler(false);
        if(handler) {
            handler->setDrawColor(drawColor);
            setTcFontAccordingToMag(handler, font, mag);
            handler->setCursor((int)where.x, (int)where.y + (handler->getYAdvance() - handler->getBaseline()));
            handler->print(text);
        } else {
            internalDrawText(where, font, mag, text);
        }
    }

    UnicodeFontHandler *DeviceDrawable::getUnicodeHandler(bool enableIfNeeded) {
        if(fontHandler == nullptr && enableIfNeeded) {
            fontHandler = createFontHandler();
        }
        return fontHandler; // if null, there is no font handler.
    }

    UnicodeFontHandler *DeviceDrawable::createFontHandler() {
        return fontHandler = new UnicodeFontHandler(this, ENCMODE_UTF8);
    }

    Coord DeviceDrawable::textExtents(const void *font, int mag, const char *text, int *baseline) {
        auto handler = getUnicodeHandler(false);
        if(handler) {
            setTcFontAccordingToMag(handler, font, mag);
            return handler->textExtents(text, baseline, false);
        } else {
            return internalTextExtents(font, mag, text, baseline);
        }
    }

    void setTcFontAccordingToMag(UnicodeFontHandler* handler, const void* font, int mag) {
        if(mag == 0) {
            handler->setFont((UnicodeFont*) font);
        } else {
            handler->setFont((GFXfont*) font);
        }
    }
} // namespace tcgfx
