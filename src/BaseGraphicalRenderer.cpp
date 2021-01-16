/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "BaseGraphicalRenderer.h"
#include "BaseDialog.h"

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

GridPositionRowCacheEntry* BaseGraphicalRenderer::findRowCol(int idx, int row, int col) {
    while(idx < itemOrderByRow.count()) {
        auto* ent = itemOrderByRow.itemAtIndex(idx);
        if(ent->getPosition().getRow() == row && ent->getPosition().getGridPosition() == col) {
            return ent;
        }
        idx++;
    }
    return nullptr;
}

GridPositionRowCacheEntry* BaseGraphicalRenderer::findMenuEntryAndDimensions(const Coord& screenPos, Coord& localStart, Coord& localSize) {
    int rowStartY = 0;
    for(int i=lastOffset; i<itemOrderByRow.count(); i++) {
        int rowHeight = heightOfRow(i, 1);
        int rowEndY = rowStartY + rowHeight;

        if(screenPos.y > rowStartY && screenPos.y < rowEndY) {
            auto* pEntry = itemOrderByRow.itemAtIndex(i);
            localStart.y = rowStartY;
            localSize.y = rowHeight;
            if(pEntry->getPosition().getGridSize() == 1) {
                localStart.x = 0;
                localSize.x = width;
            }
            else {
                int colWidth = width / pEntry->getPosition().getGridSize();
                int column = (screenPos.x / colWidth);
                localStart.x = column * colWidth;
                localSize.x = colWidth;
                return findRowCol(i, pEntry->getPosition().getRow(), column);
            }
        }
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
                lastRow = itemCfg->getPosition().getRow();
            }
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

GridPosition::GridDrawingMode modeFromItem(MenuItem* item) {
    switch(item->getMenuType()) {
        case MENUTYPE_INT_VALUE:
            return GridPosition::DRAW_INTEGER_AS_SCROLL;
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

    uint8_t row = 0;

    if(root == menuMgr.getRoot() && titleMode != NO_TITLE) {
        serdebugF("Add title");
        auto* myProps = getDisplayPropertiesFactory().configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);
        appTitleMenuItem.setTitleHeaderPgm(pgmTitle);
        itemOrderByRow.add(GridPositionRowCacheEntry(&appTitleMenuItem,
                                                     GridPosition(GridPosition::DRAW_TITLE_ITEM, myProps->getDefaultJustification(), 0), myProps));
        row++;
    }

    auto* item = root;
    if(root->getMenuType() == MENUTYPE_BACK_VALUE) {
        serdebugF2("Handling back item", root->getId());
        auto* myProps = getDisplayPropertiesFactory().configFor(root, ItemDisplayProperties::COMPTYPE_TITLE);
        itemOrderByRow.add(GridPositionRowCacheEntry(root, GridPosition(GridPosition::DRAW_TITLE_ITEM, myProps->getDefaultJustification(), 0), myProps));
        item = root->getNext();
        row++;
    }

    while(item != nullptr) {
        if(item->isVisible()) {
            auto* conf = getDisplayPropertiesFactory().gridPositionForItem(item);
            if (conf && !safeMode) {
                serdebugF3("Add config id at row", item->getId(), conf->getPosition().getRow());
                auto compType = toComponentType(conf->getPosition().getDrawingMode(), item);
                itemOrderByRow.add(GridPositionRowCacheEntry(item, conf->getPosition(), getDisplayPropertiesFactory().configFor(item, compType)));
            } else {
                serdebugF3("Add manual id at row", item->getId(), row);
                auto mode = modeFromItem(item);
                auto* itemProps = getDisplayPropertiesFactory().configFor(item, toComponentType(mode, item));
                itemOrderByRow.add(GridPositionRowCacheEntry(item, GridPosition(mode, itemProps->getDefaultJustification(), 1, 1, row, 0), itemProps));

                // we cannot use a row that's already been used by an override, so we make sure we select an empty row here.
                while(++row < 100 && itemOrderByRow.getByKey(row) != nullptr);
            }
        }
        item = item->getNext();
    }

    if(areRowsOutOfOrder() && !safeMode) recalculateDisplayOrder(item, true);

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

    color_t widFg, widBg;
    widFg = itemOrderByRow.itemAtIndex(0)->getDisplayProperties()->getColor(ItemDisplayProperties::HIGHLIGHT1);
    widBg = itemOrderByRow.itemAtIndex(0)->getDisplayProperties()->getColor(ItemDisplayProperties::BACKGROUND);
    if(itemOrderByRow.itemAtIndex(0)->getMenuItem()->isActive()) {
        widFg = getDisplayPropertiesFactory().getSelectedColor(ItemDisplayProperties::TEXT);
        widBg = getDisplayPropertiesFactory().getSelectedColor(ItemDisplayProperties::BACKGROUND);
    }

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
            bool isAction = (pMenuItem->getMenuType() == MENUTYPE_ACTION_VALUE || pMenuItem->getMenuType() == MENUTYPE_SUB_VALUE);
            return isAction ? ItemDisplayProperties::COMPTYPE_ACTION : ItemDisplayProperties::COMPTYPE_ITEM;
    }
}

BaseDialog* BaseGraphicalRenderer::getDialog() {
    if(dialog == nullptr) {
        dialog = new MenuBasedDialog();
    }
    return dialog;
}
