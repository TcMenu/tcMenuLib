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

/**
 * Holds the graphical configuration of how to render a menu onto a both mono and colour displays. If you don't intend
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
 * mehtod shipped with the plugin. This is for higher resolution colour displays.
 */
void prepareDefaultGfxConfig(ColorGfxMenuConfig<void*>* config);

/** A structure that holds both X and Y direction in a single 32 bit integer. Both x and y are public */
struct Coord {
    /** 
     * Create a coord based on an X and Y location 
     * @param x the x location
     * @param y the y location
     */
    Coord(int x, int y) {
        this->x = x;
        this->y = y;
    }
    int32_t x:15;
    int32_t y:15;
};

/**
 * The default editing icon for approx 100-150 dpi resolution displays 
 */
extern const unsigned char PROGMEM loResEditingIcon[];

/**
 * The default active icon for approx 100-150 dpi resolution displays 
 */
extern const unsigned char PROGMEM loResActiveIcon[];

/**
 * The low resolution icon for indicating active status
 */
extern const unsigned char PROGMEM defActiveIcon[];

/**
 * The low resolution icon for editing status
 */
extern const unsigned char PROGMEM defEditingIcon[];

#endif // _GFX_MENU_CONFIG_H_
