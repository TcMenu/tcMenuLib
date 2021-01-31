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
#include <Wire.h>
#include "tcMenuU8g2.h"

const uint8_t* safeGetFont(const void* fnt) {
    if(fnt) return static_cast<const uint8_t *>(fnt);
    return u8g2_font_6x10_tf;
}

static uint8_t bytesSent = 0;

#if WANT_TASK_MANAGER_FRIENDLY_YIELD == 1
TwoWire* U8g2Drawable::pWire = nullptr;
uint8_t u8g2_byte_with_yield(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    if(!U8g2Drawable::pWire) return 0;
    switch(msg)
    {
        case U8X8_MSG_BYTE_SEND:
            bytesSent+= arg_int;
            U8g2Drawable::pWire->write((uint8_t *)arg_ptr, (int)arg_int);
            if(bytesSent > 16) {
                taskManager.yieldForMicros(0);
                bytesSent = 0;
            }
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            U8g2Drawable::pWire->beginTransmission(u8x8_GetI2CAddress(u8x8)>>1);
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            U8g2Drawable::pWire->endTransmission();
            break;
        case U8X8_MSG_BYTE_INIT:
        case U8X8_MSG_BYTE_SET_DC:
            break;
        default:
            return 0;
    }
    return 1;
}
#endif // WANT_TASK_MANAGER_FRIENDLY_YIELD

U8g2Drawable::U8g2Drawable(U8G2 *u8g2, TwoWire* wireImpl) : u8g2(u8g2) {
    pWire = wireImpl;
#if WANT_TASK_MANAGER_FRIENDLY_YIELD == 1
    if(wireImpl) {
        u8g2->getU8x8()->byte_cb = u8g2_byte_with_yield;
    }
#endif
}

void U8g2Drawable::drawText(const Coord &where, const void *font, int mag, color_t textColor, const char *text) {
    u8g2->setFont(safeGetFont(font));
    u8g2->setFontMode(textColor == 2);
    auto extraHeight = u8g2->getMaxCharHeight();
    u8g2->setDrawColor(textColor);
    u8g2->setCursor(where.x, where.y + extraHeight);
    u8g2->print(text);
}

void U8g2Drawable::drawBitmap(const Coord &where, const DrawableIcon *icon, color_t defColor, bool selected) {
    u8g2->setDrawColor(defColor);
    if(icon->getIconType() == DrawableIcon::ICON_XBITMAP) {
#if defined(__AVR__) || defined(ESP8266)
        u8g2->drawXBMP(where.x, where.y, icon->getDimensions().x, icon->getDimensions().y, icon->getIcon(selected));
#else
        u8g2->drawXBM(where.x, where.y, icon->getDimensions().x, icon->getDimensions().y, icon->getIcon(selected));
#endif
    }
}

void U8g2Drawable::drawXBitmap(const Coord &where, const Coord &size, const uint8_t *data, color_t fgColor, color_t bgColor) {
    u8g2->setDrawColor(bgColor);
    u8g2->drawBox(where.x, where.y, size.x, size.y);
    u8g2->setDrawColor(fgColor);
#if defined(__AVR__) || defined(ESP8266)
    u8g2->drawXBMP(where.x, where.y, size.x, size.y, data);
#else
    u8g2->drawXBM(where.x, where.y, size.x, size.y, data);
#endif
}

void U8g2Drawable::drawBox(const Coord &where, const Coord &size, color_t color, bool filled) {
    u8g2->setDrawColor(color);
    if(filled) {
        u8g2->drawBox(where.x, where.y, size.x, size.y);
    }
    else {
        u8g2->drawFrame(where.x, where.y, size.x, size.y);
    }
}

void U8g2Drawable::transaction(bool isStarting, bool redrawNeeded) {
    if(isStarting) {
        u8g2->setFontPosBottom();
        u8g2->setFontRefHeightExtendedText();
    }
    else if(redrawNeeded) {
        u8g2->sendBuffer();
    }
}

Coord U8g2Drawable::textExtents(const void *font, int mag, const char *text, int *baseline) {
    u8g2->setFont(safeGetFont(font));
    if(baseline) *baseline = (int)u8g2->getFontDescent();
    return Coord(u8g2->getStrWidth(text), u8g2->getMaxCharHeight());
}

//
// Default graphics configuration
//

/**
 * Provides a basic graphics configuration suitable for low / medium resolution displays. Do not use in new code, this
 * will be removed in a future release.
 *
 * @param config usually a global variable holding the graphics configuration.
 * @deprecated use display factory based configuration instead, see documentation
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
