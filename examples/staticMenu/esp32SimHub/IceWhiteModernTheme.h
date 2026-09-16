#ifndef TCMENU_THEME_BLOCK
#define TCMENU_THEME_BLOCK

#include <graphics/TcThemeBuilder.h>

#include <UnicodeFontDefs.h>
extern const UnicodeFont RobotoRegular12pt[];
extern const UnicodeFont RobotoRegular18pt[];



color_t defaultItemPalette[] = { RGB(46, 58, 69), RGB(247, 249, 251), RGB(90, 160, 243), RGB(208, 232, 255) };
color_t defaultActionPalette[] = { RGB(46, 58, 69), RGB(74, 144, 226), RGB(208, 232, 255), RGB(107, 122, 136) };
color_t defaultTitlePalette[] = { RGB(46, 58, 69), RGB(74, 144, 226), RGB(107, 122, 136), RGB(208, 232, 255) };

/**
 * This is one of the stock themes, you can modify it to meet your requirements, and it will not be updated by tcMenu
 * Designer unless you delete it. This sets up the fonts, spacing and padding for all items.
 * @param gr the graphical renderer
 */
void applyTheme(GraphicsDeviceRenderer& gr) {

    // See https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/themes/rendering-with-themes-icons-grids/
    TcThemeBuilder themeBuilder(gr);
    themeBuilder.withSelectedColors(RGB(208, 232, 255), RGB(107, 122, 136))
            .dimensionsFromRenderer()
            .withItemPadding(MenuPadding(2))
            .withRenderingSettings(BaseGraphicalRenderer::TITLE_ALWAYS, false)
            .withPalette(defaultItemPalette)
            .withTcUnicodeFont(RobotoRegular12pt)
            .withSpacing(1)
            .withStandardMedResCursorIcons()
            .enableTcUnicode();

    themeBuilder.defaultTitleProperties()
            .withTcUnicodeFont(RobotoRegular18pt)
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

