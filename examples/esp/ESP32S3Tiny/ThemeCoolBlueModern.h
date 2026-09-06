#ifndef TCMENU_THEME_BLOCK
#define TCMENU_THEME_BLOCK

#include <graphics/TcThemeBuilder.h>

#include <UnicodeFontDefs.h>
extern const UnicodeFont OpenSansRegular12pt[];
extern const UnicodeFont RobotoRegular14pt[];



color_t defaultItemPalette[] = { RGB(255, 255, 255), RGB(0, 64, 135), RGB(20, 133, 255), RGB(31, 100, 178) };
color_t defaultActionPalette[] = { RGB(255, 255, 255), RGB(0, 45, 120), RGB(20, 133, 255), RGB(31, 100, 178) };
color_t defaultTitlePalette[] = { RGB(0, 0, 0), RGB(20, 132, 255), RGB(192, 192, 192), RGB(64, 64, 64) };

/**
 * This is one of the stock themes, you can modify it to meet your requirements, and it will not be updated by tcMenu
 * Designer unless you delete it. This sets up the fonts, spacing and padding for all items.
 * @param gr the graphical renderer
 */
void applyTheme(GraphicsDeviceRenderer& gr) {

    // See https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/themes/rendering-with-themes-icons-grids/
    TcThemeBuilder themeBuilder(gr);
    themeBuilder.withSelectedColors(RGB(31, 88, 100), RGB(255, 255, 255))
            .dimensionsFromRenderer()
            .withItemPadding(MenuPadding(2))
            .withRenderingSettings(BaseGraphicalRenderer::NO_TITLE, false)
            .withPalette(defaultItemPalette)
            .withTcUnicodeFont(OpenSansRegular12pt)
            .withSpacing(1)
            .withStandardLowResCursorIcons()
            .enableTcUnicode();

    themeBuilder.defaultTitleProperties()
            .withTcUnicodeFont(RobotoRegular14pt)
            .withPalette(defaultTitlePalette)
            .withPadding(MenuPadding(2))
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT)
            .withSpacing(2)
            .apply();

    themeBuilder.defaultActionProperties()
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT)
            .withPadding(MenuPadding(2))
            .withPalette(defaultActionPalette)
            .apply();

    themeBuilder.defaultItemProperties()
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT)
            .apply();

    themeBuilder.apply();
}

#endif //TCMENU_THEME_BLOCK

