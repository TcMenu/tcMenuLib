#ifndef TCMENU_THEME_BLOCK
#define TCMENU_THEME_BLOCK

#include <graphics/TcThemeBuilder.h>

#include <UnicodeFontDefs.h>
extern const UnicodeFont OpenSansRegular7pt[];
extern const UnicodeFont OpenSansRegular8pt[];



color_t defaultItemPalette[] = { WHITE, BLACK, WHITE, WHITE };
color_t defaultActionPalette[] = { WHITE, BLACK, WHITE, WHITE };
color_t defaultTitlePalette[] = { WHITE, BLACK, WHITE, WHITE };

/**
 * This is one of the stock themes, you can modify it to meet your requirements, and it will not be updated by tcMenu
 * Designer unless you delete it. This sets up the fonts, spacing and padding for all items.
 * @param gr the graphical renderer
 */
void applyTheme(GraphicsDeviceRenderer& gr) {

    // See https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/themes/rendering-with-themes-icons-grids/
    TcThemeBuilder themeBuilder(gr);
    themeBuilder.withSelectedColors(BLACK, WHITE)
            .dimensionsFromRenderer()
            .withItemPadding(MenuPadding(2))
            .withRenderingSettings(BaseGraphicalRenderer::TITLE_FIRST_ROW, false)
            .withPalette(defaultItemPalette)
            .withTcUnicodeFont(OpenSansRegular7pt)
            .withSpacing(1)
            .withStandardLowResCursorIcons()
            .enableTcUnicode();

    themeBuilder.defaultTitleProperties()
            .withTcUnicodeFont(OpenSansRegular8pt)
            .withPalette(defaultTitlePalette)
            .withPadding(MenuPadding(2))
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE)
            .withSpacing(2)
            .withBorder(MenuBorder(0, 0, 1, 0))
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

