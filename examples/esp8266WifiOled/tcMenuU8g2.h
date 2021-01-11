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

#ifndef _TCMENU_U8G2_H_
#define _TCMENU_U8G2_H_

#include <tcMenu.h>
#include <tcUtil.h>
#include <U8g2lib.h>
#include <BaseGraphicalRenderer.h>
#include <BaseDialog.h>
#include <tcUtil.h>

extern const ConnectorLocalInfo applicationInfo;

/**
 * A standard menu render configuration that describes how to renderer each item and the title.
 * Specialised for u8g2 fonts.
 */ 
typedef struct ColorGfxMenuConfig<const uint8_t*> U8g2GfxMenuConfig;

// some colour displays don't create this value
#ifndef BLACK
#define BLACK 0
#endif

// some colour displays don't create this value
#ifndef WHITE
#define WHITE 1
#endif

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
class U8g2MenuRenderer : public BaseGraphicalRenderer {
private:
	U8G2* u8g2;
	ConfigurableItemDisplayPropertiesFactory propertiesFactory;
    bool redrawNeeded;
public:
	U8g2MenuRenderer(uint8_t bufferSize = 20) : BaseGraphicalRenderer(bufferSize, 1, 1, false, applicationInfo.name) {
		this->u8g2 = nullptr;
	}
    ~U8g2MenuRenderer() override = default;

	void setGraphicsDevice(U8G2* u8g2, U8g2GfxMenuConfig *gfxConfig);
	void setGraphicsDevice(U8G2* u8g2);

    void drawWidget(Coord where, TitleWidget *widget, color_t fg, color_t bg) override;
    void drawMenuItem(GridPositionRowCacheEntry* entry, Coord where, Coord areaSize) override;
    void drawingCommand(RenderDrawingCommand command) override;

    ItemDisplayPropertiesFactory &getDisplayPropertiesFactory() override {
        return propertiesFactory;
    }

    U8G2* getGraphics() { return u8g2; }
private:
    void drawBitmap(int x, int y, int w, int h, const unsigned char *bmp);
    void drawTextualItem(GridPositionRowCacheEntry* entry, Coord where, Coord size);
    void drawIconItem(GridPositionRowCacheEntry* pEntry, Coord where, Coord size);
    void drawSlider(GridPositionRowCacheEntry* pEntry, Coord where, Coord size);

    void internalDrawText(GridPositionRowCacheEntry* pEntry, Coord where, Coord size, int leftOffset, bool xorMode);
    int drawCoreLineItem(GridPositionRowCacheEntry* pEntry, const Coord &where, const Coord &size);
};


/**
 * Provides a basic graphics configuration suitable for low / medium resolution displays
 * @param config usually a global variable holding the graphics configuration.
 */
void prepareBasicU8x8Config(U8g2GfxMenuConfig& config);

#endif // _TCMENU_U8G2_H_
