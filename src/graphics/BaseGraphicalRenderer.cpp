/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "BaseGraphicalRenderer.h"
#include "BaseDialog.h"

namespace tcgfx {

int appTitleRenderingFn(RuntimeMenuItem *item, uint8_t, RenderFnMode mode, char *buffer, int bufferSize);

class RuntimeTitleMenuItem : public RuntimeMenuItem {
private:
    const char* titleHeaderPgm;
    MenuCallbackFn callback;
public:
    RuntimeTitleMenuItem(uint16_t id, MenuItem *next) : RuntimeMenuItem(MENUTYPE_TITLE_ITEM, id, appTitleRenderingFn, 0, 1, next) {
        titleHeaderPgm = nullptr;
        callback = nullptr;
    }

    void setTitleHeaderPgm(const char* header) {
        titleHeaderPgm = header;
    }

    const char* getTitleHeaderPgm() {
        return titleHeaderPgm;
    }

    void setCallback(MenuCallbackFn titleCb) {
        callback = titleCb;
    }

    MenuCallbackFn getCallback() {
        return callback;
    }
};

int appTitleRenderingFn(RuntimeMenuItem *item, uint8_t, RenderFnMode mode, char *buffer, int bufferSize) {
    if (item->getMenuType() != MENUTYPE_TITLE_ITEM) return 0;
    auto* pTitleItem = reinterpret_cast<RuntimeTitleMenuItem*>(item);

    switch (mode) {
        case RENDERFN_INVOKE:
            if(pTitleItem->getCallback()) {
                auto cb = pTitleItem->getCallback();
                cb(item->getId());
            }
        case RENDERFN_VALUE: {
            buffer[0] = '^'; buffer[1] = 0;
            return true;
        }
        case RENDERFN_NAME: {
            safeProgCpy(buffer, pTitleItem->getTitleHeaderPgm(), bufferSize);
            return true;
        }
        default: return false;
    }
}

RuntimeTitleMenuItem appTitleMenuItem(0, nullptr);

void setTitlePressedCallback(MenuCallbackFn cb) {
    appTitleMenuItem.setCallback(cb);
}

void BaseGraphicalRenderer::render() {
    uint8_t locRedrawMode = redrawMode;
    redrawMode = MENUDRAW_NO_CHANGE;

    // if the root menu rootItem has changed then it's a complete re-draw and we need to calculate the orders again.
    MenuItem* rootItem = menuMgr.getCurrentMenu();
    if(currentRootMenu != rootItem)
    {
        serdebugF("root has changed");
        currentRootMenu = rootItem;
        locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
        recalculateDisplayOrder(rootItem, false);
    }

    drawingCommand(DRAW_COMMAND_START);

    if (locRedrawMode == MENUDRAW_COMPLETE_REDRAW) {
        serdebugF("Complete redraw")
        drawingCommand(DRAW_COMMAND_CLEAR);
        taskManager.yieldForMicros(0);
    }

    countdownToDefaulting();

    bool forceDrawWidgets = false;

    if (menuMgr.getCurrentMenu()->getMenuType() == MENUTYPE_RUNTIME_LIST ) {
        if (rootItem->isChanged() || locRedrawMode != MENUDRAW_NO_CHANGE) {
            renderList();
            forceDrawWidgets = true;
        }
        titleOnDisplay = (titleMode != NO_TITLE);
    }
    else {
        // first we find the first currently active rootItem in our single linked list
        int activeIndex = findActiveItem();
        int totalHeight = calculateHeightTo(activeIndex, rootItem);
        int startRow = 0;
        int adjust = lastRowExactFit ? 0 : 1;
        while (totalHeight > (height + adjust)) {
            totalHeight -= heightOfRow(startRow, 1);
            startRow++;
        }

        // the screen has moved, we must completely redraw the area, and we need a clear first.
        if(locRedrawMode != MENUDRAW_COMPLETE_REDRAW && lastOffset != startRow) {
            locRedrawMode = MENUDRAW_COMPLETE_REDRAW;
            serdebugF3("Screen Row moved ", lastOffset, startRow);
            drawingCommand(DRAW_COMMAND_CLEAR);
        }
        lastOffset = startRow;

        // and then we start drawing items until we run out of screen or items
        forceDrawWidgets = drawTheMenuItems(locRedrawMode, startRow);
        titleOnDisplay = (titleMode != NO_TITLE) && startRow == 0;
    }

    redrawAllWidgets(locRedrawMode != MENUDRAW_NO_CHANGE || forceDrawWidgets);
    drawingCommand(DRAW_COMMAND_ENDED);
}

GridPositionRowCacheEntry* BaseGraphicalRenderer::findMenuEntryAndDimensions(const Coord& screenPos, Coord& localStart, Coord& localSize) {
    if((dialog!=nullptr && dialog->isRenderNeeded()) || displayTakenMode != NOT_TAKEN_OVER) {
        return nullptr;
    }

    int rowStartY = 0;
    auto* icon = getDisplayPropertiesFactory().iconForMenuItem(SPECIAL_ID_ACTIVE_ICON);
    int iconWidth = icon ? icon->getDimensions().x : 0;

    for(int i=lastOffset; i<itemOrderByRow.count(); i++) {
        auto* pEntry = itemOrderByRow.itemAtIndex(i);
        int rowHeight = heightOfRow(pEntry->getPosition().getRow(), 1);
        int rowEndY = rowStartY + rowHeight;

        // this seems odd but we are only doing this loop to find the heights, so we
        // only check all the first entries in the row. There is code further down to
        // deal with columns.
        if(pEntry->getPosition().getGridPosition() != 1) {
            continue;
        }

        // if we are within the y bounds of of item
        if(screenPos.y > rowStartY && screenPos.y < rowEndY) {
            localStart.y = rowStartY;
            localSize.y = rowHeight;
            if(pEntry->getPosition().getGridSize() == 1) {
                // single column row, so we must be within the item
                auto iconAdjust = iconWidth + pEntry->getDisplayProperties()->getPadding().left;
                localStart.x = iconAdjust;
                localSize.x = width - iconAdjust;
                return pEntry;
            }
            else {
                // multi column row, so we must work out which column we are in.
                int colWidth = width / pEntry->getPosition().getGridSize();
                int column = (screenPos.x / colWidth);
                localStart.x = column * colWidth;
                localSize.x = colWidth;
                return itemOrderByRow.getByKey(rowCol(pEntry->getPosition().getRow(), column + 1));
            }
        }
        rowStartY += rowHeight + pEntry->getDisplayProperties()->getSpaceAfter();
    }

    // we did not find anything at that point.
    return nullptr;
}

bool BaseGraphicalRenderer::drawTheMenuItems(uint8_t locRedrawMode, int startRow) {
    int16_t ypos = 0;
    int lastRow = 0;
    int addAmount = 0;
    bool didDrawTitle = false;

    for(bsize_t i=startRow; i < itemOrderByRow.count(); i++) {
        auto* itemCfg = itemOrderByRow.itemAtIndex(i);
        auto* item = itemCfg->getMenuItem();
        if(item->isVisible())
        {
            if(lastRow != itemCfg->getPosition().getRow()) ypos += addAmount;
            int itemHeight = (int)itemCfg->getPosition().getGridHeight();
            if(itemHeight == 0) itemHeight = itemCfg->getDisplayProperties()->getRequiredHeight();
            auto extentsY = (lastRowExactFit) ? ypos + itemHeight : ypos;
            if(extentsY > height) break;

            if (locRedrawMode != MENUDRAW_NO_CHANGE || item->isChanged()) {
                serdebugF4("draw item (pos,id,chg)", i, item->getId(), item->isChanged());
                item->setChanged(false);
                taskManager.yieldForMicros(0);
                if(itemCfg->getPosition().getGridSize() > 1) {
                    int colWidth = width / itemCfg->getPosition().getGridSize();
                    int colOffset = colWidth * (itemCfg->getPosition().getGridPosition() - 1);
                    drawMenuItem(itemCfg, Coord(colOffset, ypos), Coord(colWidth, itemHeight));
                }
                else {
                    drawMenuItem(itemCfg, Coord(0, ypos), Coord(width, itemHeight));
                }
                if(itemCfg->getPosition().getDrawingMode() == GridPosition::DRAW_TITLE_ITEM && itemCfg->getPosition().getRow() == 0) {
                    didDrawTitle = true;
                }
            }
            lastRow = itemCfg->getPosition().getRow();
            addAmount = itemHeight + itemCfg->getDisplayProperties()->getSpaceAfter();
        }
    }

    return didDrawTitle;
}

void BaseGraphicalRenderer::renderList() {
    auto* runList = reinterpret_cast<ListRuntimeMenuItem*>(menuMgr.getCurrentMenu());
    auto* itemProps = getDisplayPropertiesFactory().configFor(runList, ItemDisplayProperties::COMPTYPE_ITEM);
    auto* titleProps = getDisplayPropertiesFactory().configFor(runList, ItemDisplayProperties::COMPTYPE_TITLE);
    int titleHeight = titleProps->getRequiredHeight();
    int rowHeight = itemProps->getRequiredHeight();
    int totalTitleHeight = titleHeight + titleProps->getSpaceAfter();
    int totalRowHeight = rowHeight + itemProps->getSpaceAfter();

    uint8_t maxY = min(uint8_t(((height + (rowHeight - 1)) - totalTitleHeight) / totalRowHeight), runList->getNumberOfParts());
    uint8_t currentActive = runList->getActiveIndex();

    uint8_t offset = 0;
    if (currentActive >= maxY) {
        offset = (currentActive+1) - maxY;
    }

    // these braces save memory by constraining the grid position cache entry, do not remove.
    {
        GridPositionRowCacheEntry titleEntry(runList->asBackMenu(), GridPosition(GridPosition::DRAW_TITLE_ITEM,
                                                                                 titleProps->getDefaultJustification(),
                                                                                 0, titleHeight), titleProps);
        drawMenuItem(&titleEntry, Coord(0, 0), Coord(width, titleHeight));
    }

    for (int i = 0; i < maxY; i++) {
        uint8_t current = offset + i;
        RuntimeMenuItem* toDraw = runList->getChildItem(current);
        GridPositionRowCacheEntry itemEntry(toDraw, GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, current + 1, rowHeight), itemProps);
        drawMenuItem(&itemEntry, Coord(0, totalTitleHeight + (i * totalRowHeight)), Coord(width, rowHeight));
        taskManager.yieldForMicros(0);
    }

    // reset the list item to a normal list again.
    runList->asParent();
    runList->setChanged(false);
    titleOnDisplay = true;
}

GridPosition::GridDrawingMode modeFromItem(MenuItem* item, bool useSlider) {
    switch(item->getMenuType()) {
        case MENUTYPE_INT_VALUE:
            return useSlider ? GridPosition::DRAW_INTEGER_AS_SCROLL : GridPosition::DRAW_INTEGER_AS_UP_DOWN;
        case MENUTYPE_ENUM_VALUE:
        case MENUTYPE_SCROLLER_VALUE:
            return GridPosition::DRAW_INTEGER_AS_UP_DOWN;
        default:
            return GridPosition::DRAW_TEXTUAL_ITEM;
    }
}

void BaseGraphicalRenderer::recalculateDisplayOrder(MenuItem *root, bool safeMode) {
    serdebugF2("Recalculate display order, safe=", safeMode);

    itemOrderByRow.clear();
    if(root == nullptr || root->getMenuType() == MENUTYPE_RUNTIME_LIST) return;

    if(root == menuMgr.getRoot() && titleMode != NO_TITLE) {
        serdebugF("Add title");
        auto* myProps = getDisplayPropertiesFactory().configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);
        appTitleMenuItem.setTitleHeaderPgm(pgmTitle);
        itemOrderByRow.add(GridPositionRowCacheEntry(&appTitleMenuItem,
                                                     GridPosition(GridPosition::DRAW_TITLE_ITEM, myProps->getDefaultJustification(), 0), myProps));
    }

    auto* item = root;
    if(root->getMenuType() == MENUTYPE_BACK_VALUE) {
        serdebugF2("Handling back item", root->getId());
        auto* myProps = getDisplayPropertiesFactory().configFor(root, ItemDisplayProperties::COMPTYPE_TITLE);
        itemOrderByRow.add(GridPositionRowCacheEntry(root, GridPosition(GridPosition::DRAW_TITLE_ITEM, myProps->getDefaultJustification(), 0), myProps));
        item = root->getNext();
    }

    while(item != nullptr) {
        if(item->isVisible()) {
            auto* conf = getDisplayPropertiesFactory().gridPositionForItem(item);
            if (conf && !safeMode) {
                serdebugF3("Add config id at row", item->getId(), conf->getPosition().getRow());
                auto compType = toComponentType(conf->getPosition().getDrawingMode(), item);
                itemOrderByRow.add(GridPositionRowCacheEntry(item, conf->getPosition(), getDisplayPropertiesFactory().configFor(item, compType)));
            } else {
                // We just find the first unused row and put the next item there.
                int row = 0;
                while(itemOrderByRow.getByKey(rowCol(row, 1)) != nullptr) row++;
                serdebugF3("Add manual id at row", item->getId(), row);
                auto mode = modeFromItem(item, useSliderForAnalog);
                auto* itemProps = getDisplayPropertiesFactory().configFor(item, toComponentType(mode, item));
                itemOrderByRow.add(GridPositionRowCacheEntry(item, GridPosition(mode, itemProps->getDefaultJustification(), 1, 1, row, 0), itemProps));
            }
        }
        item = item->getNext();
    }

    // here we try again, but this time we turn off any custom grids.
    if(areRowsOutOfOrder() && !safeMode) {
        recalculateDisplayOrder(root, true);
    }

    activateFirstAppropriateItem();
}

void BaseGraphicalRenderer::activateFirstAppropriateItem() {
    if(itemOrderByRow.count() == 0) return;
    bool selectedYet = false;
    for(bsize_t i=0; i<itemOrderByRow.count(); i++) {
        auto* entry = itemOrderByRow.itemAtIndex(i);
        bool skip = i < 3 && (entry->getMenuItem()->isReadOnly() || entry->getPosition().getDrawingMode() == GridPosition::DRAW_TITLE_ITEM);
        if(!skip && !selectedYet) {
            entry->getMenuItem()->setActive(true);
            selectedYet = true;
        }
        else entry->getMenuItem()->setActive(false);
    }
}

bool BaseGraphicalRenderer::areRowsOutOfOrder() {
    int lastSequence = 0;
    for(bsize_t i=0; i<itemOrderByRow.count();i++) {
        if(itemOrderByRow.itemAtIndex(i)->getPosition().getRow() > (lastSequence + 1)) {
            char sz[20];
            itemOrderByRow.itemAtIndex(i)->getMenuItem()->copyNameToBuffer(sz, sizeof sz);
            serdebugF2("Row out of order at ", sz);
            return true;
        }
        lastSequence = itemOrderByRow.itemAtIndex(i)->getPosition().getRow();
    }
    return false;
}

void BaseGraphicalRenderer::redrawAllWidgets(bool forceRedraw) {
    if(!titleOnDisplay || itemOrderByRow.count() == 0) return;
    if(itemOrderByRow.itemAtIndex(0)->getPosition().getDrawingMode() != GridPosition::DRAW_TITLE_ITEM) return;

    auto widFg = itemOrderByRow.itemAtIndex(0)->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT1);
    auto widBg = itemOrderByRow.itemAtIndex(0)->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND);

    auto* widget = this->firstWidget;
    int widgetRight = width;
    while(widget) {
        auto* myProps = getDisplayPropertiesFactory().configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);
        widgetRight = widgetRight - (myProps->getPadding().right + widget->getWidth());
        if(widget->isChanged() || forceRedraw) {
            widget->setChanged(false);
            drawWidget(Coord(widgetRight, myProps->getPadding().top), widget, widFg, widBg);
        }
        widget = widget->getNext();
    }
}

int BaseGraphicalRenderer::heightOfRow(int row, int col) {
    auto* rowData = itemOrderByRow.getByKey(rowCol(row, col));

    // if there's no configuration at this position, crazy situation, return 1..
    if(rowData == nullptr) return 1;

    // if there's a configuration, then return it, unless it's 0 then use the default
    auto itemHeight = rowData->getPosition().getGridHeight();
    return itemHeight != 0 ? itemHeight : rowData->getDisplayProperties()->getRequiredHeight();
}

int BaseGraphicalRenderer::calculateHeightTo(int index, MenuItem *pItem) {
    int totalY = 0;
    index += 1;
    for(int i=0; i<index; i++) {
        totalY += heightOfRow(i, 1);
    }
    return totalY;
}

int BaseGraphicalRenderer::findActiveItem() {
    for(bsize_t i=0;i<itemOrderByRow.count();i++) {
        auto* possibleActive = itemOrderByRow.itemAtIndex(i);
        if(possibleActive->getMenuItem()->isActive()) return possibleActive->getPosition().getRow();
    }
    return 0; // default to the title (back menu item)
}

MenuItem *BaseGraphicalRenderer::getMenuItemAtIndex(uint16_t idx) {
    if(idx >= itemOrderByRow.count()) return menuMgr.getRoot();
    return itemOrderByRow.itemAtIndex(idx)->getMenuItem();
}

ItemDisplayProperties::ComponentType BaseGraphicalRenderer::toComponentType(GridPosition::GridDrawingMode mode, MenuItem* pMenuItem) {
    if(!pMenuItem) return ItemDisplayProperties::COMPTYPE_ITEM;
    switch(mode) {
        case GridPosition::DRAW_TITLE_ITEM:
            return ItemDisplayProperties::COMPTYPE_TITLE;
        case GridPosition::DRAW_TEXTUAL_ITEM:
        case GridPosition::DRAW_INTEGER_AS_UP_DOWN:
        case GridPosition::DRAW_INTEGER_AS_SCROLL:
        case GridPosition::DRAW_AS_ICON_ONLY:
        case GridPosition::DRAW_AS_ICON_TEXT:
        default:
            return isItemActionable(pMenuItem) ? ItemDisplayProperties::COMPTYPE_ACTION : ItemDisplayProperties::COMPTYPE_ITEM;
    }
}

BaseDialog* BaseGraphicalRenderer::getDialog() {
    if(dialog == nullptr) {
        dialog = new MenuBasedDialog();
    }
    return dialog;
}

void preparePropertiesFromConfig(ConfigurableItemDisplayPropertiesFactory& factory, const ColorGfxMenuConfig<const void*>* gfxConfig, int titleHeight, int itemHeight) {
    // TEXT, BACKGROUND, HIGHLIGHT1, HIGHLIGHT2, SELECTED_FG, SELECTED_BG
    color_t paletteItems[] { gfxConfig->fgItemColor, gfxConfig->bgItemColor, gfxConfig->bgSelectColor, gfxConfig->fgSelectColor};
    color_t titleItems[] { gfxConfig->fgTitleColor, gfxConfig->bgTitleColor, gfxConfig->fgTitleColor, gfxConfig->fgSelectColor};

    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, paletteItems, gfxConfig->itemPadding, gfxConfig->itemFont, gfxConfig->itemFontMagnification, 0, itemHeight, GridPosition::JUSTIFY_LEFT_NO_VALUE);
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, paletteItems, gfxConfig->itemPadding, gfxConfig->itemFont, gfxConfig->itemFontMagnification, 0, itemHeight, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT);
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, titleItems, gfxConfig->titlePadding, gfxConfig->titleFont, gfxConfig->titleFontMagnification, gfxConfig->titleBottomMargin, titleHeight, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE);
    factory.setSelectedColors(gfxConfig->bgSelectColor, gfxConfig->fgSelectColor);

    factory.addImageToCache(DrawableIcon(SPECIAL_ID_EDIT_ICON, Coord(gfxConfig->editIconWidth, gfxConfig->editIconHeight), DrawableIcon::ICON_XBITMAP, gfxConfig->editIcon));
    factory.addImageToCache(DrawableIcon(SPECIAL_ID_ACTIVE_ICON, Coord(gfxConfig->editIconWidth, gfxConfig->editIconHeight), DrawableIcon::ICON_XBITMAP, gfxConfig->activeIcon));

}

} // namespace tcgfx
