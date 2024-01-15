/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "TcThemeBuilder.h"

void ThemePropertiesBuilder::apply() {
    auto autoHeight = themeBuilder->getRenderer().heightForFontPadding(fontData, fontMag, padding);
    if(currentLevel == THEME_GLOBAL) {
        themeBuilder->getItemFactory().setDrawingPropertiesDefault(
                componentType, palette, padding, fontData, fontMag, spacing, autoHeight, justification, border);
    } else if(currentLevel == THEME_SUB) {
        themeBuilder->getItemFactory().setDrawingPropertiesAllInSub(
                componentType, menuItem->getId(), palette, padding, fontData, fontMag, spacing, autoHeight, justification, border);
    }
    else {
        if (currentLevel == THEME_ITEM_NEEDS_PROPS || currentLevel == THEME_ITEM_NEEDS_BOTH) {
            themeBuilder->getItemFactory().setDrawingPropertiesForItem(
                    componentType, menuItem->getId(), palette, padding, fontData, fontMag, spacing, autoHeight, justification,
                    border);
        }

        if (gridHeight == 0xFFFF) {
            if (drawingMode == GridPosition::DRAW_AS_ICON_ONLY || drawingMode == GridPosition::DRAW_AS_ICON_TEXT) {
                auto da = themeBuilder->getItemFactory().iconForMenuItem(menuItem->getId());
                if (da != nullptr) {
                    int y = da->getDimensions().y;
                    if(drawingMode == GridPosition::DRAW_AS_ICON_ONLY) {
                        gridHeight = y + spacing + padding.top + padding.right + border.top + border.bottom;
                    } else {
                        gridHeight = y + themeBuilder->getRenderer().heightForFontPadding(fontData, fontMag, padding);
                    }
                } else {
                    gridHeight = -1;
                }
            } else {
                gridHeight = autoHeight;
            }
        }

        themeBuilder->getItemFactory().addGridPosition(menuItem, GridPosition(drawingMode, justification, colCount, colPos, row, gridHeight));
    }
}

void ThemePropertiesBuilder::initForLevel(TcThemeBuilder *b, ItemDisplayProperties::ComponentType compType,
                                          ThemePropertiesBuilder::ThemeLevel level, MenuItem* item) {
    componentType = compType;
    currentLevel = level;
    menuItem = item;
    themeBuilder = b;
    fontData = themeBuilder->getDefaultFontData();
    fontMag = themeBuilder->getDefaultFontMag();
    padding = themeBuilder->getPaddingFor(compType);
    spacing = themeBuilder->getDefaultSpacing();
    memcpy(palette, themeBuilder->getDefaultPalette(), sizeof palette);
    gridHeight = 0xFFFF;
    colPos = 1;
    colCount = 1;
    row = 1;

    if(level != THEME_GLOBAL) {
        auto props = b->getItemFactory().configFor(nullptr, compType);
        if(props->getFont() != nullptr) {
            fontData = props->getFont();
            fontMag = props->getFontMagnification();
        }
        border = props->getBorder();
        justification = props->getDefaultJustification();
    } else {
        justification = GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT;
        border = MenuBorder(0);
    }
}

ThemePropertiesBuilder& ThemePropertiesBuilder::withImageOfType(Coord size, DrawableIcon::IconType iconType, const uint8_t* regIcon, const uint8_t* selIcon) {
    themeBuilder->getItemFactory().addImageToCache(DrawableIcon(menuItem->getId(), size, iconType, regIcon, selIcon));
    if(drawingMode != GridPosition::DRAW_AS_ICON_TEXT) drawingMode = GridPosition::DRAW_AS_ICON_ONLY;
    needsGrid(false);
    return *this;
}

TcThemeBuilder &TcThemeBuilder::withCursorIconsXbmp(Coord size, const uint8_t *editIcon, const uint8_t *activeIcon) {
    factory.addImageToCache(DrawableIcon(SPECIAL_ID_EDIT_ICON, size, DrawableIcon::ICON_XBITMAP, editIcon));
    factory.addImageToCache(DrawableIcon(SPECIAL_ID_ACTIVE_ICON, size, DrawableIcon::ICON_XBITMAP, activeIcon));
    return *this;
}

void TcThemeBuilder::apply() {
    tcgfx::ConfigurableItemDisplayPropertiesFactory::refreshCache();
}

TcThemeBuilder& TcThemeBuilder::withPalette(const color_t *cols) {
    memcpy(defaultPalette, cols, sizeof(defaultPalette));
    return *this;
}

ThemePropertiesBuilder &TcThemeBuilder::menuItemOverride(MenuItem &item) {
    ItemDisplayProperties::ComponentType ty = (isItemActionable(&item)) ? ItemDisplayProperties::COMPTYPE_ACTION : ItemDisplayProperties::COMPTYPE_ITEM;
    propertiesBuilder.initForLevel(this, ty, ThemePropertiesBuilder::THEME_ITEM, &item);
    return propertiesBuilder;
}

TcThemeBuilder& TcThemeBuilder::enableCardLayoutWithXbmImages(Coord iconSize, const uint8_t *leftIcon, const uint8_t *rightIcon, bool isMono) {
    renderer.enableCardLayout(
            DrawableIcon(-1, iconSize, tcgfx::DrawableIcon::ICON_XBITMAP, leftIcon, nullptr),
            DrawableIcon(-1, iconSize, tcgfx::DrawableIcon::ICON_XBITMAP, rightIcon, nullptr),
            nullptr, isMono);
    return *this;
}

TcThemeBuilder& TcThemeBuilder::setMenuAsCard(SubMenuItem &item, bool on) {
    renderer.setCardLayoutStatusForSubMenu(&item, true);
    return *this;
}

TcThemeBuilder &TcThemeBuilder::withStandardLowResCursorIcons() {
    return withCursorIconsXbmp(Coord(8, 6), loResEditingIcon, loResActiveIcon);
}

TcThemeBuilder &TcThemeBuilder::withStandardMedResCursorIcons() {
    return withCursorIconsXbmp(Coord(16, 12), defEditingIcon, defActiveIcon);
}

TcThemeBuilder & TcThemeBuilder::enablingTcUnicode() {
    renderer.enableTcUnicode();
    return *this;
}
