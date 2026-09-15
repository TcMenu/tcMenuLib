/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "GraphicsDeviceRenderer.h"
#include "DeviceDrawableHelper.h"
#include <tcUnicodeHelper.h>
#include "CardLayoutPanel.h"

#ifdef TC_TOUCH_DEBUG
extern MenuTouchScreenManager touchScreen;
#endif

namespace tcgfx {

    const Coord rendererXbmArrowSize(8, 11);
    static unsigned char rendererUpArrowXbm[] = { 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03 };
    static unsigned char rendererDownArrowXbm[] = { 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0 };

    bool GraphicsDeviceRenderer::isActiveOrEditing(MenuItem* pItem, const DrawingFlags& drawingFlags) const {
        // in card layout we should not select.
        if(cardLayoutPane != nullptr && cardLayoutPane->isSubMenuCardLayout(menuMgr.getCurrentSubMenu())) return false;

        auto mt = pItem->getMenuType();
        return (drawingFlags.isEditing() || drawingFlags.isActive()) && mt != MENUTYPE_TITLE_ITEM && mt != MENUTYPE_BACK_VALUE;
    }

    GraphicsDeviceRenderer::GraphicsDeviceRenderer(int bufferSize, const char *appTitle, DeviceDrawable *drawable)
            : BaseGraphicalRenderer(bufferSize, 1, 1, false, appTitle), rootDrawable(drawable), helper(drawable) {
        if (rootDrawable) {
            const Coord &coord = rootDrawable->getDisplayDimensions();
            width = coord.x;
            height = coord.y;
        }
    }

    void GraphicsDeviceRenderer::drawingCommand(BaseGraphicalRenderer::RenderDrawingCommand command) {
        const auto* overallCfg = propertiesFactory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM);
        overallScreenBgColor = overallCfg->getColor(ItemDisplayProperties::BACKGROUND);

        switch(command) {
            case DRAW_COMMAND_CLEAR: {
                helper.getDrawable()->setDrawColor(overallCfg->getColor(ItemDisplayProperties::BACKGROUND));
                helper.getDrawable()->drawBox(Coord(0, 0), Coord(width, height), true);
                break;
            }
            case DRAW_COMMAND_START:
            case DRAW_COMMAND_ENDED:
                helper.getDrawable()->transaction(command == DRAW_COMMAND_START, redrawNeeded);
                redrawNeeded = false;
                break;
        }
    }

    void GraphicsDeviceRenderer::drawWidget(Coord where, TitleWidget *widget, color_t colorFg, color_t colorBg) {
        redrawNeeded = true;
        helper.getDrawable()->setColors(colorFg, colorBg);
        helper.getDrawable()->drawXBitmap(where, Coord(widget->getWidth(), widget->getHeight()), widget->getCurrentIcon());
    }

    void GraphicsDeviceRenderer::drawMenuItem(GridPositionRowCacheEntry *entry, Coord where, Coord areaSize, const DrawingFlags& drawingFlags) {
        redrawNeeded = true;
        entry->getMenuItem()->setChanged(displayNumber, false);

        // if it's in a multi grid layout, put a small gap at the start of each one.
        if(entry->getPosition().getGridSize() > 1) {
            auto space = entry->getDisplayProperties()->getSpaceAfter();
            helper.getDrawable()->setDrawColor(entry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND));
            helper.getDrawable()->drawBox(where, Coord(space, areaSize.y), true);
            areaSize.x -= space;
            where.x += space;
        }

        // if we are drawing everything, then we need to clear out the areas in between items.
        if(drawingFlags.isDrawingAll() && entry->getDisplayProperties()->getSpaceAfter() > 0) {
            auto* bgConfig = propertiesFactory.configFor(menuMgr.getCurrentSubMenu(), ItemDisplayProperties::COMPTYPE_ITEM);
            helper.getDrawable()->setDrawColor(bgConfig->getColor(ItemDisplayProperties::BACKGROUND));
            helper.getDrawable()->drawBox(Coord(where.x, where.y + areaSize.y), Coord(areaSize.x, entry->getDisplayProperties()->getSpaceAfter()), true);
        }

        // icons never use double buffer drawing because they may use a lot of BPP and don't change often in the main
        auto drawingMode = entry->getPosition().getDrawingMode();
        if(drawingMode == GridPosition::DRAW_AS_ICON_ONLY || drawingMode == GridPosition::DRAW_AS_ICON_TEXT) {
            drawIconItem(entry, where, areaSize, drawingFlags);
            return;
        }

        color_t palette[5];
        bool selected = isActiveOrEditing(entry->getMenuItem(), drawingFlags);
        palette[ItemDisplayProperties::TEXT] = (selected) ? propertiesFactory.getSelectedColor(ItemDisplayProperties::TEXT) : entry->getDisplayProperties()->getPalette()[ItemDisplayProperties::TEXT];
        palette[ItemDisplayProperties::BACKGROUND] = (selected) ? propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND) : entry->getDisplayProperties()->getPalette()[ItemDisplayProperties::BACKGROUND];
        palette[ItemDisplayProperties::HIGHLIGHT1] = entry->getDisplayProperties()->getPalette()[ItemDisplayProperties::HIGHLIGHT1];
        palette[ItemDisplayProperties::HIGHLIGHT2] = entry->getDisplayProperties()->getPalette()[ItemDisplayProperties::HIGHLIGHT2];
        palette[4] = overallScreenBgColor;
        const int paletteSize = rootDrawable->getSubDeviceType() == DeviceDrawable::SUB_DEVICE_4BPP ? 5 : 4;
        helper.reConfigure(palette, paletteSize, where, areaSize);

        Coord wh = helper.offsetLocation(where);


        switch(drawingMode) {
            case GridPosition::DRAW_TEXTUAL_ITEM:
            case GridPosition::DRAW_TITLE_ITEM:
            default:
                drawTextualItem(entry, wh, areaSize, drawingFlags);
                break;
            case GridPosition::DRAW_INTEGER_AS_UP_DOWN:
                drawUpDownItem(entry, wh, areaSize, drawingFlags);
                break;
            case GridPosition::DRAW_INTEGER_AS_SCROLL:
                if(entry->getMenuItem()->getMenuType() != MENUTYPE_INT_VALUE) return; // disallowed
                drawSlider(entry, reinterpret_cast<AnalogMenuItem*>(entry->getMenuItem()), wh, areaSize, drawingFlags);
                break;
        }

        helper.endDraw();

#ifdef TC_TOUCH_DEBUG
        // This is for debugging of touch coordinates
        Coord circleLoc = Coord(
                touchScreen.getLastX() * rootDrawable->getDisplayDimensions().x,
                touchScreen.getLastY() * rootDrawable->getDisplayDimensions().y);
        rootDrawable->setDrawColor(RGB(128, 128, 128));
        rootDrawable->drawCircle(circleLoc, 5, true);
#endif
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
        auto extents = helper.getDrawable()->textExtents(font, mag, sz);
        return int(extents.x);
    }

    void GraphicsDeviceRenderer::internalDrawText(GridPositionRowCacheEntry* pEntry, const Coord& where, const Coord& size, const DrawingFlags& drawingFlags) {
        GridPosition::GridJustification just = pEntry->getPosition().getJustification();
        ItemDisplayProperties *props = pEntry->getDisplayProperties();
        auto padding = props->getPadding();

        color_t fg;
        if(isActiveOrEditing(pEntry->getMenuItem(), drawingFlags)) {
            fg = propertiesFactory.getSelectedColor(ItemDisplayProperties::TEXT);
        }
        else {
            fg = props->getColor(ItemDisplayProperties::TEXT);
        }

        copyMenuItemValue(pEntry->getMenuItem(), buffer, bufferSize, drawingFlags.isActive());
        bool weAreEditingWithCursor = drawingFlags.isEditing() && editorHintNeedsCursor(menuMgr.getEditorHints().getEditorRenderingType());

        bool valueNeeded = itemNeedsValue(pEntry->getPosition().getJustification());
        if(pEntry->getMenuItem()->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
            if (reinterpret_cast<BooleanMenuItem *>(pEntry->getMenuItem())->getBooleanNaming() == NAMING_CHECKBOX) {
                valueNeeded = false;
            }
        }

        helper.setFontFromParameters(props->getFont(), props->getFontMagnification());

        if(just == GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT || weAreEditingWithCursor) {
            // special case, title left, value right.
            Coord wh = Coord(where.x + padding.left, where.y + padding.top);
            pEntry->getMenuItem()->copyNameToBuffer(buffer, bufferSize);
            serlogF4(SER_TCMENU_DEBUG, "item: ", buffer, size.y, where.y);
            helper.getDrawable()->setDrawColor(fg);
            helper.drawText(wh, fg, buffer);

            if(valueNeeded) {
                copyMenuItemValue(pEntry->getMenuItem(), buffer, bufferSize, drawingFlags.isActive());
            } else buffer[0] = 0;
            int bl;
            int16_t right = where.x + size.x - (helper.textExtents(buffer, &bl).x + padding.right);
            wh.x = right;
            if(weAreEditingWithCursor) {
                helper.getDrawable()->setDrawColor(propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND, true));
                auto& hints = menuMgr.getEditorHints();
                int startX = calculateSpaceBetween(props->getFont(), props->getFontMagnification(), buffer, 0, hints.getStartIndex() );
                int lenX = internal_max(MINIMUM_CURSOR_SIZE, calculateSpaceBetween(props->getFont(), props->getFontMagnification(), buffer, hints.getStartIndex(), hints.getEndIndex()));
                int whereX = internal_min(int(width) - MINIMUM_CURSOR_SIZE, int(wh.x + startX));
                helper.getDrawable()->drawBox(Coord(whereX, where.y + size.y - 1), Coord(lenX, 1), true);
                if(size_t(hints.getEndIndex()) > strlen(buffer)) wh.x = wh.x - (unsigned int)MINIMUM_CURSOR_SIZE;
                helper.drawText(wh, fg, buffer);
            }
            else {
                helper.drawText(wh, fg, buffer);
            }
        }
        else {
            char sz[32];
            bool nameNeeded = itemNeedsName(just);
            if(valueNeeded && nameNeeded) {
                copyMenuItemNameAndValue(pEntry->getMenuItem(), sz, sizeof sz, 0, drawingFlags.isActive());
            } else if(valueNeeded) {
                copyMenuItemValue(pEntry->getMenuItem(), sz, sizeof sz, drawingFlags.isActive());
            } else {
                pEntry->getMenuItem()->copyNameToBuffer(sz, sizeof sz);
            }

            int startPosition = padding.left;
            int bl;
            if(coreJustification(just) == GridPosition::CORE_JUSTIFY_RIGHT) {
                startPosition = size.x - (helper.textExtents(sz, &bl).x + padding.right);
            }
            else if(coreJustification(just) == GridPosition::CORE_JUSTIFY_CENTER) {
                startPosition = ((size.x - helper.textExtents(sz, &bl).x) / 2) + padding.right;
            }
            helper.drawText(Coord(startPosition + where.x, where.y + padding.top), fg, sz);
            serlogF4(SER_TCMENU_DEBUG, "intTx ", sz, startPosition + where.x, (where.y + size.y) - padding.bottom);
        }
    }

    static void adjustForIconPosition(DrawableIcon* icon, Coord& where, Coord& size, MenuPadding pad) {
        if(icon) {
            const auto adjust = icon->getDimensions().x + pad.left;
            where.x += adjust;
            size.x -= adjust;
        }
    }

    void GraphicsDeviceRenderer::drawCoreLineItem(GridPositionRowCacheEntry* entry, DrawableIcon* icon, Coord &where, Coord &size,
                                                  const DrawingFlags& drawingFlags, bool drawBg) {
        auto pad = entry->getDisplayProperties()->getPadding();
        serlogF4(SER_TCMENU_DEBUG, "Drawing at: ", where.y, size.x, size.y);

        // work out what background and drawing colors to use
        color_t bgColor, textColor;
        bool forceBorder = false;

        color_t entryBg = entry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND);
        color_t selBg = propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND);
        if(isActiveOrEditing(entry->getMenuItem(), drawingFlags)) {
            bgColor = selBg;
            textColor = propertiesFactory.getSelectedColor(ItemDisplayProperties::TEXT);
            forceBorder = (icon == nullptr) && (selBg == entryBg) && isEditStatusIconEnabled();
        }
        else {
            bgColor = entryBg;
            textColor = entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT2);
        }

        auto xoffset = (icon) ? icon->getDimensions().x : 0;

        // draw any active arrow or blank space that's needed first
        if((drawingFlags.isEditing() || drawingFlags.isActive())) {
            helper.getDrawable()->setDrawColor(bgColor);
            // drawing too much as a real impact on some displays, when we don't need to render the background, we do not
            Coord adjustedSize = drawBg ? size : Coord(xoffset + pad.left, size.y);
            helper.getDrawable()->drawBox(where, adjustedSize, true);
            helper.getDrawable()->setColors(textColor, bgColor);
            if(icon) {
                int imgMiddleY = where.y + ((size.y - icon->getDimensions().y) / 2);
                helper.getDrawable()->drawXBitmap(Coord(where.x + pad.left, imgMiddleY), icon->getDimensions(),
                                      icon->getIcon(false));
            }
        }
        else {
            Coord adjustedSize = drawBg ? size : Coord(xoffset + pad.left, size.y);
            helper.getDrawable()->setDrawColor(bgColor);
            helper.getDrawable()->drawBox(where, adjustedSize, true);
        }

        // draw the border (if filled mode it is removing pixels)
        MenuBorder border = entry->getDisplayProperties()->getBorder();
        if(!border.isBorderOff() || forceBorder) {
            if (border.getBorderType() == BORD_FILL_ROUNDED) {
                // remove the elements of he border to make the previous fill look rounded
                helper.getDrawable()->setDrawColor(overallScreenBgColor);
                drawBorderAndAdjustSize(where, size, border);
                adjustForIconPosition(icon, where, size, pad);
            } else {
                adjustForIconPosition(icon, where, size, pad);

                // draw the border excluding the icon
                if(forceBorder) border = MenuBorder(1);
                helper.getDrawable()->setDrawColor(textColor);
                drawBorderAndAdjustSize(where, size, border);
            }
        } else {
            adjustForIconPosition(icon, where, size, pad);
        }

    }

    void GraphicsDeviceRenderer::drawBorderAndAdjustSize(Coord &where, Coord &size, MenuBorder &border) {
        if(border.areAllBordersEqual()) {
            for(int i=0; i<border.getBoxDim(border.left);++i) {
                helper.getDrawable()->drawBox(where, size, false);
                where.x++;
                where.y++;
                size.x -= 2;
                size.y -= 2;
            }
        }
        else {
            if(border.left) {
                const auto boxDim = border.getBoxDim(border.left);
                helper.getDrawable()->drawBox(Coord(where.x, where.y), Coord(boxDim, size.y), true);
                where.x -= boxDim;
                size.x -= boxDim;
            }
            if(border.right) {
                const auto boxDim = border.getBoxDim(border.right);
                helper.getDrawable()->drawBox(Coord(where.x + size.x - boxDim, where.y), Coord(boxDim, size.y), true);
                where.x -= boxDim;
                size.x -= boxDim;
            }
            if(border.bottom) {
                const auto boxDim = border.getBoxDim(border.bottom);
                helper.getDrawable()->drawBox(Coord(where.x, where.y + size.y - boxDim), Coord(size.x, boxDim), true);
                where.y -= boxDim;
                size.y -= boxDim;
            }
            if(border.top) {
                const auto boxDim = border.getBoxDim(border.top);
                helper.getDrawable()->drawBox(Coord(where.x, where.y), Coord(size.x, boxDim), true);
                where.y -= boxDim;
                size.y -= boxDim;
            }
        }
        if (border.getBorderType() == BORD_FILL_ROUNDED) {
            roundCornerLeft(1, border.left, where);
            roundCornerLeft(-1, border.left, Coord(where.x, where.y + size.y));
            roundCornerRight(1, border.right, Coord(where.x + size.x, where.y));
            roundCornerRight(-1, border.right, Coord(where.x + size.x, where.y + size.y));
        }
    }

    const uint8_t radius2[] = {
        0b00000000,
        0b00000001
    };
    const uint8_t radius3[] = {
        0b00000000,
        0b00000011,
        0b00000011
    };

    const uint8_t radius4[] = {
        0b00000000,
        0b00000011,
        0b00000111,
        0b00000111
    };

    const uint8_t radius5[] = {
        0b00000000,
        0b00000011,
        0b00000111,
        0b00001111,
        0b00001111
    };

    const uint8_t radius6[] = {
        0b00000000,
        0b00000011,
        0b00000111,
        0b00001111,
        0b00011111,
        0b00011111
    };

    const uint8_t radius7[] = {
        0b00000000,
        0b00000011,
        0b00001111,
        0b00011111,
        0b00011111,
        0b00111111,
        0b00111111
    };

    const uint8_t* radiusArrays[] = {
        radius2,
        radius3,
        radius4,
        radius5,
        radius6,
        radius7
    };

    void GraphicsDeviceRenderer::roundCornerLeft(int16_t direction, int radius, Coord where) {
        if (!radius) return;
        if (radius == 1) {
            helper.getDrawable()->drawPixel(where.x, where.y);
        } else if (radius > 1 && radius < 8) {
            const uint8_t* circleData = radiusArrays[radius - 2];
            for (int i =0; i < radius; ++i) {
                auto thisLine = circleData[i];
                int counter = 0;
                for (int j = radius - 1; j >= 0; --j) {
                    if (!(thisLine & (1 << j))) {
                        helper.getDrawable()->drawPixel(where.x + counter, where.y);
                    }
                    counter++;
                }
                where.y = where.y + direction;
            }
        }
    }

    void GraphicsDeviceRenderer::roundCornerRight(int16_t direction, int radius, Coord where) {
        if (!radius) return;
        if (radius == 1) {
            helper.getDrawable()->drawPixel(where.x, where.y);
        } else {
            const uint8_t* circleData = radiusArrays[radius - 2];
            for (int i =0; i < radius; ++i) {
                auto thisLine = circleData[i];
                int counter = radius;
                for (int j = 0; j < radius; ++j) {
                    if (!(thisLine & (1 << j))) {
                        helper.getDrawable()->drawPixel(where.x - counter, where.y);
                    }
                    counter--;
                }
                where.y = where.y + direction;
            }
        }
    }

    void GraphicsDeviceRenderer::drawCheckbox(GridPositionRowCacheEntry *entry, Coord& where, Coord& size, const DrawingFlags& drawingFlags) {
        auto padding = entry->getDisplayProperties()->getPadding();
        auto* icon = getStateIndicatorIcon(entry);

        drawCoreLineItem(entry, icon, where, size, drawingFlags, true);
        auto hei = size.y - (padding.top + padding.top);
        auto startingX = where.x + size.x - (padding.left + padding.right + hei);
        auto boolItem = reinterpret_cast<BooleanMenuItem*>(entry->getMenuItem());
        auto hl = entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT1);
        auto txtCol = entry->getDisplayProperties()->getColor(ItemDisplayProperties::TEXT);

        helper.getDrawable()->setDrawColor(txtCol);
        helper.getDrawable()->drawBox(Coord(startingX, where.y + padding.top), Coord(hei, hei), false);
        if(hl != txtCol) {
            helper.getDrawable()->drawBox(Coord(startingX + 1, where.y + padding.top + 1), Coord(hei - 2, hei - 2), false);
        }
        if(boolItem->getBoolean()) {
            helper.getDrawable()->setDrawColor(hl);
            helper.getDrawable()->drawBox(Coord(startingX + 2, where.y + padding.top + 2), Coord(hei - 4, hei - 4), true);
        }
        internalDrawText(entry, where, Coord(size.x - (hei + padding.left), size.y), drawingFlags);
    }

    void GraphicsDeviceRenderer::drawUpDownItem(GridPositionRowCacheEntry *entry, Coord& where, Coord& size, const DrawingFlags& drawingFlags) {
        auto padding = entry->getDisplayProperties()->getPadding();
        auto* icon = getStateIndicatorIcon(entry);

        drawCoreLineItem(entry, icon, where, size, drawingFlags, true);

        // Touch specific code. We must never put editing buttons on read only items, only drawing buttons on the left
        // and right if item is active/editable.
        if(isHasTouchInterface() && !entry->getMenuItem()->isReadOnly() && (drawingFlags.isActive() || drawingFlags.isEditing())) {
            int buttonSize = size.y - 1;
            int offset = (buttonSize - rendererXbmArrowSize.y) / 2;
            int downButtonLocation = where.x;
            int upButtonLocation = (size.x + where.x) - (padding.right + buttonSize);
            int textStartX = size.y + padding.left;
            auto hl = entry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT1);
            auto txtCol = entry->getDisplayProperties()->getColor(ItemDisplayProperties::TEXT);
            helper.getDrawable()->setDrawColor(hl);
            helper.getDrawable()->drawBox(Coord(downButtonLocation, where.y), Coord(size.y, size.y), true);
            helper.getDrawable()->drawBox(Coord(upButtonLocation, where.y), Coord(size.y, size.y), true);
            helper.getDrawable()->setColors(txtCol, hl);
            helper.getDrawable()->drawXBitmap(Coord(downButtonLocation + offset, where.y + offset), rendererXbmArrowSize, rendererDownArrowXbm);
            helper.getDrawable()->drawXBitmap(Coord(upButtonLocation + offset, where.y + offset), rendererXbmArrowSize, rendererUpArrowXbm);

            internalDrawText(entry, Coord(where.x + textStartX, where.y),
                             Coord(size.x - (((buttonSize + padding.right) * 2)), size.y), drawingFlags);
        }
        else {
            internalDrawText(entry, Coord(where.x, where.y), Coord(size.x, size.y), drawingFlags);
        }
    }

    DrawableIcon *GraphicsDeviceRenderer::getStateIndicatorIcon(GridPositionRowCacheEntry *entry) {
        if(!isEditStatusIconEnabled()) return nullptr; // no edit icons when explicitly turned off
        return propertiesFactory.iconForMenuItem(entry->getMenuItem() == menuMgr.getCurrentEditor() ? SPECIAL_ID_EDIT_ICON : SPECIAL_ID_ACTIVE_ICON);
    }

    void GraphicsDeviceRenderer::drawTextualItem(GridPositionRowCacheEntry* pEntry, Coord& where, Coord& size, const DrawingFlags& drawingFlags) {
        if(pEntry->getMenuItem()->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
            auto boolItem = reinterpret_cast<BooleanMenuItem*>(pEntry->getMenuItem());
            if(boolItem->getBooleanNaming() == NAMING_CHECKBOX) {
                drawCheckbox(pEntry, where, size, drawingFlags);
                return;
            }
        }
        auto* icon = getStateIndicatorIcon(pEntry);
        drawCoreLineItem(pEntry, icon, where, size, drawingFlags, true);
        internalDrawText(pEntry, Coord(where.x, where.y), Coord(size.x, size.y), drawingFlags);
    }

    void GraphicsDeviceRenderer::drawSlider(GridPositionRowCacheEntry* entry, AnalogMenuItem* pItem, Coord& where, Coord& size,
                                            const DrawingFlags& drawingFlags) {
        auto* icon = getStateIndicatorIcon(entry);
        drawCoreLineItem(entry, icon, where, size, drawingFlags, false);
        ItemDisplayProperties *props = entry->getDisplayProperties();
        MenuPadding pad = props->getPadding();
        int maximumSliderArea = size.x - pad.right;
        int filledAreaX = analogRangeToScreen(pItem, maximumSliderArea);
        int outsideAreaX = maximumSliderArea - filledAreaX;
        helper.getDrawable()->setDrawColor(props->getColor(ItemDisplayProperties::HIGHLIGHT1));
        helper.getDrawable()->drawBox(Coord(where.x, where.y), Coord(filledAreaX, size.y), true);
        auto mainBg = isActiveOrEditing(pItem, drawingFlags) ? propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND) : props->getColor(ItemDisplayProperties::BACKGROUND);
        helper.getDrawable()->setDrawColor(mainBg);
        helper.getDrawable()->drawBox(Coord(where.x + filledAreaX, where.y), Coord(outsideAreaX, size.y), true);
        internalDrawText(entry, Coord(where.x, where.y), Coord(size.x, size.y), drawingFlags);
    }

    void GraphicsDeviceRenderer::drawIconItem(GridPositionRowCacheEntry* pEntry, Coord& where, Coord& size, const DrawingFlags& drawingFlags) {
        auto* pItem = pEntry->getMenuItem();

        drawCoreLineItem(pEntry, nullptr, where, size, drawingFlags, true);

        auto* pIcon = propertiesFactory.iconForMenuItem(pItem->getId());
        if(pIcon == nullptr) return;

        auto effectiveTop = where.y + pEntry->getDisplayProperties()->getPadding().top;
        Coord iconWhere(where.x + ((size.x - pIcon->getDimensions().x) / 2), effectiveTop);

        bool sel = false;
        if(pItem->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
            auto* boolItem = reinterpret_cast<BooleanMenuItem*>(pItem);
            sel = boolItem->getBoolean();
        }

        if(isActiveOrEditing(pEntry->getMenuItem(), drawingFlags)) {
            helper.getDrawable()->setColors(propertiesFactory.getSelectedColor(ItemDisplayProperties::TEXT), propertiesFactory.getSelectedColor(ItemDisplayProperties::BACKGROUND));
        }
        else {
            helper.getDrawable()->setColors(pEntry->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT2), pEntry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND));
        }
        helper.getDrawable()->drawBitmap(iconWhere, pIcon, sel);

        if(pEntry->getPosition().getDrawingMode() == GridPosition::DRAW_AS_ICON_TEXT) {
            effectiveTop += pIcon->getDimensions().y;
            internalDrawText(pEntry, Coord(where.x, effectiveTop), Coord(size.x, size.y - effectiveTop), drawingFlags);
        }
    }

    int GraphicsDeviceRenderer::heightForFontPadding(const void *font, int mag, MenuPadding &padding, const MenuBorder& border) {
        int baseline=0;
        helper.setFontFromParameters(font, mag);
        Coord sizeInfo = helper.textExtents("();yg1", &baseline);
        int borderHeight = border.top + border.bottom;
        if (border.getBorderType() == BORD_FILL_ROUNDED) {
            borderHeight = borderHeight / 2; // filled borders pad with half the border height.
        }
        int hei = sizeInfo.y + padding.top + padding.bottom + borderHeight;
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

        int titleHeight = heightForFontPadding(gfxConfig->titleFont, gfxConfig->titleFontMagnification, gfxConfig->titlePadding, MenuBorder(0));
        int itemHeight = heightForFontPadding(gfxConfig->itemFont, gfxConfig->itemFontMagnification, gfxConfig->itemPadding, MenuBorder(0));

        preparePropertiesFromConfig(propertiesFactory, reinterpret_cast<const ColorGfxMenuConfig<const void *>*>(gfxConfig), titleHeight, itemHeight);

        setDisplayDimensions(rootDrawable->getDisplayDimensions().x, rootDrawable->getDisplayDimensions().y);
    }

    void GraphicsDeviceRenderer::fillWithBackgroundTo(int endPoint) {
        if(endPoint >= height) return; // nothing to do when the display is already full.
        helper.getDrawable()->setDrawColor(overallScreenBgColor);
        helper.getDrawable()->drawBox(Coord(0, endPoint), Coord(width, height-endPoint), true);
    }

    void GraphicsDeviceRenderer::subMenuRender(MenuItem* rootItem, uint8_t& locRedrawMode, bool& forceDrawWidgets) {
        if(cardLayoutPane != nullptr && cardLayoutPane->isSubMenuCardLayout(menuMgr.getCurrentSubMenu())) {
            GridPositionRowCacheEntry *titleEntry = itemOrderByRow.itemAtIndex(0);
            int activeIndex = offsetOfCurrentActive(rootItem);
            if(activeIndex == 0 && titleMode != NO_TITLE) activeIndex = 1; // do not allow 0 in this mode
            GridPositionRowCacheEntry *entry = itemOrderByRow.itemAtIndex(activeIndex);
            bool titleNeeded = titleMode == TITLE_ALWAYS || titleMode == TITLE_FIRST_ROW;
            if (locRedrawMode == MENUDRAW_COMPLETE_REDRAW) {
                cardLayoutPane->forMenu(titleEntry->getDisplayProperties(), entry->getDisplayProperties(), this, titleNeeded);
                forceDrawWidgets = true;
            }
            if (titleNeeded && (locRedrawMode == MENUDRAW_COMPLETE_REDRAW || titleEntry->getMenuItem()->isChanged(displayNumber))) {
                bool active = activeItem == titleEntry->getMenuItem();
                drawMenuItem(titleEntry, Coord(0, 0), cardLayoutPane->getTitleSize(), DrawingFlags(true, active, menuMgr.getCurrentEditor() == titleEntry->getMenuItem()));
                forceDrawWidgets = true;
            } else {
                forceDrawWidgets = true; // we always need to draw the titleWidgets if there is no title item
            }
            if (entry->getMenuItem()->isChanged(displayNumber) || locRedrawMode == MENUDRAW_COMPLETE_REDRAW) {
                getDeviceDrawable()->setDrawColor(entry->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND));
                getDeviceDrawable()->drawBox(cardLayoutPane->getMenuLocation(), cardLayoutPane->getMenuSize(), true);
                int offsetY = (cardLayoutPane->getMenuSize().y - int(entry->getHeight())) / 2;
                Coord menuStart(cardLayoutPane->getMenuLocation().x, cardLayoutPane->getMenuLocation().y + offsetY);
                Coord menuSize(cardLayoutPane->getMenuSize().x, int(entry->getHeight()));
                bool active = activeItem == entry->getMenuItem();
                drawMenuItem(entry, menuStart, menuSize, DrawingFlags(false, active, menuMgr.getCurrentEditor() == entry->getMenuItem()));
            }
            cardLayoutPane->prepareAndPaintButtons(this, activeIndex, itemOrderByRow.count(), titleMode != NO_TITLE);
            setTitleOnDisplay(true);
        } else {
            if(locRedrawMode == MENUDRAW_COMPLETE_REDRAW && cardLayoutPane != nullptr) {
                cardLayoutPane->notInUse();
            }
            BaseGraphicalRenderer::subMenuRender(rootItem, locRedrawMode, forceDrawWidgets);
        }
    }
    
    void GraphicsDeviceRenderer::enableCardLayout(const DrawableIcon& left, const DrawableIcon& right, MenuTouchScreenManager* touchScreenManager, bool monoDisplay) {
        if(cardLayoutPane == nullptr) {
            cardLayoutPane = new CardLayoutPane(&left, &right, touchScreenManager, monoDisplay);
        } else {
            serlogF(SER_TCMENU_INFO, "Card already init");
        }
    }

    void GraphicsDeviceRenderer::setCardLayoutStatusForSubMenu(MenuItem* root, bool onOrOff) {
        if(cardLayoutPane != nullptr) {
            cardLayoutPane->setEnablementForSub(root, onOrOff);
        } else {
            serlogF(SER_ERROR, "Card null");
        }
    }

    LayoutMode GraphicsDeviceRenderer::getLayoutMode(MenuItem* rootItem) {
        if(cardLayoutPane == nullptr) return LAYOUT_VERTICAL_DEFAULT;

        return (cardLayoutPane->isSubMenuCardLayout(rootItem)) ? LAYOUT_CARD_SIDEWAYS : LAYOUT_VERTICAL_DEFAULT;
    }

    void GraphicsDeviceRenderer::setDrawable(DeviceDrawable *drawable) {
        this->rootDrawable = drawable;
        this->helper.setDrawable(drawable);
        const Coord &coord = rootDrawable->getDisplayDimensions();
        width = coord.x;
        height = coord.y;
    }
} // namespace tcgfx
