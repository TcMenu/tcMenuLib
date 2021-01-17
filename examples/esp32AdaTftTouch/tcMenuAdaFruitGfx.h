/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file tcMenuAdaFruitGfx.h
 * 
 * AdaFruit_GFX renderer that renders menus onto this type of display. This file is a plugin file and should not
 * be directly edited, it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 * 
 * LIBRARY REQUIREMENT
 * This library requires the AdaGfx library along with a suitable driver.
 */


#ifndef _TCMENU_TCMENUADAFRUITGFX_H_
#define _TCMENU_TCMENUADAFRUITGFX_H_

#include <tcMenu.h>
#include <tcUtil.h>
#include <BaseRenderers.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <gfxfont.h>
#include <GfxMenuConfig.h>
#include <BaseDialog.h>
#include <BaseGraphicalRenderer.h>
#include <TouchScreen.h>
#include <AnalogDeviceAbstraction.h>

#define DISPLAY_HAS_MEMBUFFER false

// some colour displays don't create this value
#ifndef BLACK
#define BLACK 0
#endif

// some colour displays don't create this value
#ifndef WHITE
#define WHITE 0xffff
#endif

extern const unsigned char PROGMEM loResEditingIcon[];
extern const unsigned char PROGMEM loResActiveIcon[];

extern const ConnectorLocalInfo applicationInfo;

/**
 * A standard menu render configuration that describes how to renderer each item and the title.
 * Specialised for Adafruit_GFX fonts.
 */
typedef struct ColorGfxMenuConfig<const GFXfont*> AdaColorGfxMenuConfig;

void drawCookieCutBitmap(Adafruit_GFX* gfx, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w,
                         int16_t h, int16_t totalWidth, int16_t xStart, int16_t yStart,
                         uint16_t fgColor, uint16_t bgColor);

/**
 * A basic renderer that can use the AdaFruit_GFX library to render information onto a suitable
 * display. It is your responsibility to fully initialise and prepare the display before passing
 * it to this renderer. The usual procedure is to create a display variable globally in your
 * sketch and then provide that as the parameter to setGraphicsDevice. If you are using the
 * designer you provide the display variable name in the code generation parameters.
 *
 * You can also override many elements of the display using AdaColorGfxMenuConfig, to use the defaults
 * just call prepareAdaColorDefaultGfxConfig(..) passing it a pointer to your config object. Again the
 * designer UI takes care of this.
 */
class AdaFruitGfxMenuRenderer : public BaseGraphicalRenderer {
private:
    ConfigurableItemDisplayPropertiesFactory propertiesFactory;
    Adafruit_GFX* graphics;
    bool redrawNeeded = true;
    uint8_t marginBetweenAreas = 5; // between the title and items, items and button bars etc. default 5
public:
    AdaFruitGfxMenuRenderer(uint8_t bufferSize = 20) : BaseGraphicalRenderer(bufferSize, 1, 1, false, applicationInfo.name) {
        this->graphics = nullptr;
    }
    ~AdaFruitGfxMenuRenderer() override = default;
    void setGraphicsDevice(Adafruit_GFX* graphics, AdaColorGfxMenuConfig *gfxConfig);
    void setGraphicsDevice(Adafruit_GFX *graphics);
    void setGraphicsDevice(Adafruit_GFX *graphics, const GFXfont* itemFont, const GFXfont* titleFont, bool needEditingIcons, int rotation);

    void drawWidget(Coord where, TitleWidget *widget, color_t colorFg, color_t colorBg) override;
    void drawMenuItem(GridPositionRowCacheEntry *entry, Coord where, Coord areaSize) override;
    void drawingCommand(RenderDrawingCommand command) override;
    ItemDisplayPropertiesFactory &getDisplayPropertiesFactory() override { return propertiesFactory; }

    Adafruit_GFX* getGraphics() { return graphics; }

    int heightForFontPadding(const GFXfont *font, int mag, MenuPadding &padding);
private:
    int drawCoreLineItem(GridPositionRowCacheEntry* entry, DrawableIcon* icon, const Coord &where, const Coord &size);
    void drawTextualItem(GridPositionRowCacheEntry* entry, Coord where, Coord size);
    void drawSlider(GridPositionRowCacheEntry* entry, AnalogMenuItem* pItem, Coord where, Coord size);
    void drawUpDownItem(GridPositionRowCacheEntry* entry, Coord where, Coord size);
    void internalDrawText(GridPositionRowCacheEntry *entry, Coord where, Coord size);

    void drawIconItem(GridPositionRowCacheEntry *pEntry, Coord where, Coord size);

    void drawButton(char button, Coord where, int buttonSize);
};

/**
 * Provides the extents of the text as a Coord which is easier to work with.
 * @param graphics the graphics object as a pointer
 * @param text the text to measure
 * @param x starting location X
 * @param y starting location Y
 * @return the coord object containing width and height
 */
Coord textExtents(Adafruit_GFX* graphics, const char* text, int16_t x, int16_t y);

/**
 * Returns the total height and baseline for the font based on a few of the largest characters.
 * @param gfx the graphics device to use
 * @param font the font to find the largest possible size for.
 * @return the font size in the y element, and the baseline to add to padding in the x dimension.
 */
Coord getHeightAndBaselineForFont(Adafruit_GFX* gfx, const GFXfont* font);


/**
 * The default graphics configuration for Ada GFX that needs no fonts and uses reasonable spacing options
 * for 100 - 150 dpi displays.
 */
void prepareAdaColorDefaultGfxConfig(AdaColorGfxMenuConfig* config);

/**
 * A graphics configuration suitable for lower resolution displays such as the 5110, these settings may be appropriate
 */
void prepareAdaMonoGfxConfigLoRes(AdaColorGfxMenuConfig* config);

/**
 * A graphics configuration suitable for oled screens using the adafruit driver.
 */
void prepareAdaMonoGfxConfigOled(AdaColorGfxMenuConfig* config);

#endif /* _TCMENU_TCMENUADAFRUITGFX_H_ */
