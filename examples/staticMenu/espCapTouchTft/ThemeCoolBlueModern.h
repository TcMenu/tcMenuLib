#ifndef TCMENU_THEME_BLOCK
#define TCMENU_THEME_BLOCK

#include <graphics/TcThemeBuilder.h>

#include <UnicodeFontDefs.h>
extern const UnicodeFont OpenSansRegular16pt[];
extern const UnicodeFont RobotoMedium24[];



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
            .withItemPadding(MenuPadding(8))
            .withRenderingSettings(BaseGraphicalRenderer::TITLE_ALWAYS, false)
            .withPalette(defaultItemPalette)
            .withTcUnicodeFont(OpenSansRegular16pt)
            .withSpacing(1)
            .withStandardMedResCursorIcons()
            .enableTcUnicode();

    themeBuilder.defaultTitleProperties()
            .withTcUnicodeFont(RobotoMedium24)
            .withPalette(defaultTitlePalette)
            .withPadding(MenuPadding(4))
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE)
            .withSpacing(2)
            .withBorder(MenuBorder(4, BORD_FILL_ROUNDED))
            .apply();

    themeBuilder.defaultActionProperties()
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE)
            .withPadding(MenuPadding(8))
            .withPalette(defaultActionPalette)
            .apply();

    themeBuilder.defaultItemProperties()
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT)
            .apply();

    themeBuilder.apply();
}

#endif //TCMENU_THEME_BLOCK

