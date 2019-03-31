#ifndef _GFX_MENU_CONFIG_H_
#define _GFX_MENU_CONFIG_H_

#include <tcUtil.h>

/**
 * @file GfxMenuConfig.h
 * 
 * This file contains the base drawing configuration structures and helper methods for
 * drawing onto graphical screens, be it mono or colour. Also there's some additional
 * structures for describing colours, coordinates and padding.
 */

#define RGB(r, g, b) (uint16_t)( ((r>>3)<<11) | ((g>>2)<<5) | (b>>3) )


/**
 * Defines padding for menu rendering when using the standard AdaGfx renderer. Each
 * position can hold the value 0..15
 */
struct MenuPadding {
	uint16_t top: 4;
	uint16_t right : 4;
	uint16_t bottom: 4;
	uint16_t left: 4;
};

/**
 * Populate a padding structure with values using the same form as HTML, top, right, bottom, left.
 * @param padding reference type of padding
 * @param top the top value
 * @param right the right value
 * @param bottom the bottom value
 * @param left the left value
 */
inline void makePadding(MenuPadding& padding, int top, int right, int bottom, int left) {
	padding.top = top; 
	padding.right = right; 
	padding.bottom = bottom; 
	padding.left = left;
}

enum MonoDrawMode: byte { DRAWM_NORMAL, DRAWM_INVERSE, DRAWM_HORIZLINE };

/**
 * Provides graphical configuration on how to render to a monochrome bit mapped display. If you don't
 * intend to customise this initially call the factory method provided with your renderer.
 */
template<typename FONTPTR> struct MonoGfxMenuConfig {
	MenuPadding titlePadding;
	FONTPTR titleFont;
	MenuPadding itemPadding;
	FONTPTR itemFont;

    MonoDrawMode titleDrawMode;
    MonoDrawMode selectDrawMode;

    // icons that represent the selected and editing states.
	const uint8_t* activeIcon;
	const uint8_t* editIcon;
	uint8_t editIconWidth;
	uint8_t editIconHeight;
		
	uint8_t titleBottomMargin;
	uint8_t titleFontMagnification;
	uint8_t itemFontMagnification;
};

/**
 * Holds the graphical configuration of how to render a menu onto a colour display. If you don't intend
 * to override this initially just call the factory method provided with your renderer.
 */
template<typename FONTPTR> struct ColorGfxMenuConfig {
	uint32_t bgTitleColor;
	uint32_t fgTitleColor;
	MenuPadding titlePadding;
	FONTPTR titleFont;

	uint32_t bgItemColor;
	uint32_t fgItemColor;
	MenuPadding itemPadding;
	FONTPTR itemFont;

	uint32_t bgSelectColor;
	uint32_t fgSelectColor;

	uint32_t widgetColor;
	MenuPadding widgetPadding;

	const uint8_t* activeIcon;
	const uint8_t* editIcon;
	uint8_t editIconWidth;
	uint8_t editIconHeight;
		
	uint8_t titleBottomMargin;
	uint8_t titleFontMagnification;
	uint8_t itemFontMagnification;

};

/**
 * This is an internal method, used by display specific plugins. Prefer to use the
 * mehtod shipped with the plugin.
 */
void prepareDefaultGfxConfig(ColorGfxMenuConfig<void*>* config);

typedef uint32_t Coord;
#define MakeCoord(x, y) ((((long)x)<<16)|y)
#define CoordX(c) (c>>16)
#define CoordY(c) (c&0xffff)

/**
 * The default editing icon for approx 100-150 dpi resolution displays 
 */
const uint8_t defEditingIcon[] PGM_TCM = {
		0b11111111,0b11111111,
		0b01111111,0b11111111,
		0b00011100,0b00000000,
		0b00000111,0b00000000,
		0b00000001,0b1100000,
		0b00000000,0b00111000,
		0b00000000,0b00111000,
		0b00000001,0b11000000,
		0b00000111,0b00000000,
		0b00011100,0b00000000,
		0b01111111,0b11111111,
		0b11111111,0b11111111
};

/**
 * The default active icon for approx 100-150 dpi resolution displays 
 */
const uint8_t defActiveIcon[] PGM_TCM = {
		0b00000000,0b11100000,
		0b00000000,0b11110000,
		0b00000000,0b11111000,
		0b00000000,0b11111100,
		0b00000000,0b11111110,
		0b11111111,0b11111111,
		0b11111111,0b11111111,
		0b00000000,0b11111110,
		0b00000000,0b11111100,
		0b00000000,0b11111000,
		0b00000000,0b11110000,
		0b00000000,0b11100000
};

#endif // _GFX_MENU_CONFIG_H_
