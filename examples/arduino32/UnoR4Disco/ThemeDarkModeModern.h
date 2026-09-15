#ifndef TCMENU_THEME_BLOCK
#define TCMENU_THEME_BLOCK

#include <graphics/TcThemeBuilder.h>

#include <UnicodeFontDefs.h>
extern const UnicodeFont RobotoRegular12pt[];
extern const UnicodeFont RobotoRegular14pt[];



color_t defaultItemPalette[] = { RGB(255, 255, 255), RGB(0,0,0), RGB(43,43,43), RGB(65,65,65) };
color_t defaultActionPalette[] = { RGB(255, 255, 255), RGB(35,35,35), RGB(20,45,110), RGB(192,192,192) };
color_t defaultTitlePalette[] = { RGB(255,255,255), RGB(55,55,55), RGB(192,192,192), RGB(0,133,255) };

/**
 * This is one of the stock themes, you can modify it to meet your requirements, and it will not be updated by tcMenu
 * Designer unless you delete it. This sets up the fonts, spacing and padding for all items.
 * @param gr the graphical renderer
 */
void applyTheme(GraphicsDeviceRenderer& gr) {

    // See https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/themes/rendering-with-themes-icons-grids/
    TcThemeBuilder themeBuilder(gr);
    themeBuilder.withSelectedColors(RGB(46, 66, 161), RGB(255, 255, 255))
            .dimensionsFromRenderer()
            .withItemPadding(MenuPadding(2))
            .withRenderingSettings(BaseGraphicalRenderer::TITLE_FIRST_ROW, false)
            .withPalette(defaultItemPalette)
            .withTcUnicodeFont(RobotoRegular12pt)
            .withSpacing(1)
            .withStandardLowResCursorIcons()
            .enableTcUnicode();

    themeBuilder.defaultTitleProperties()
            .withTcUnicodeFont(RobotoRegular14pt)
            .withPalette(defaultTitlePalette)
            .withPadding(MenuPadding(2))
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE)
            .withSpacing(2)
            .withBorder(MenuBorder(4, BORD_FILL_ROUNDED))
            .apply();

    themeBuilder.defaultActionProperties()
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE)
            .withPadding(MenuPadding(2))
            .withPalette(defaultActionPalette)
            .apply();

    themeBuilder.defaultItemProperties()
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT)
            .apply();

    themeBuilder.apply();
}

#endif //TCMENU_THEME_BLOCK

