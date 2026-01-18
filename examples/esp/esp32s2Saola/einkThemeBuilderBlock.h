#ifndef TCMENU_THEME_BLOCK
#define TCMENU_THEME_BLOCK

#include <graphics/TcThemeBuilder.h>
#include <UnicodeFontDefs.h>
#include <Fonts/OpenSansRegular12pt.h>
#include <Fonts/OpenSansRegular14pt.h>


color_t defaultItemPalette[] = { GxEPD_BLACK, GxEPD_WHITE, GxEPD_BLACK, GxEPD_BLACK };
color_t defaultTitlePalette[] = { GxEPD_WHITE, GxEPD_BLACK, GxEPD_WHITE, GxEPD_WHITE };

/**
 * This is one of the stock themes, you can modify it to meet your requirements, and it will not be updated by tcMenu
 * Designer unless you delete it. This sets up the fonts, spacing and padding for all items.
 * @param gr the graphical renderer
 */
void applyTheme(GraphicsDeviceRenderer& gr) {

    // See https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/themes/rendering-with-themes-icons-grids/
    TcThemeBuilder themeBuilder(gr);
    themeBuilder.withSelectedColors(GxEPD_WHITE, GxEPD_BLACK)
            .dimensionsFromRenderer()
            .withItemPadding(MenuPadding(2))
            .withRenderingSettings(BaseGraphicalRenderer::TITLE_FIRST_ROW, false)
            .withPalette(defaultItemPalette)
            .withTcUnicodeFont(OpenSansRegular12pt)
            .withSpacing(1)
            .withStandardLowResCursorIcons()
            .enableTcUnicode();

    themeBuilder.defaultTitleProperties()
            .withTcUnicodeFont(OpenSansRegular14pt)
            .withPalette(defaultTitlePalette)
            .withPadding(MenuPadding(2))
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE)
            .withSpacing(2)
            .apply();

    themeBuilder.defaultActionProperties()
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE)
            .apply();

    themeBuilder.defaultItemProperties()
            .withJustification(tcgfx::GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT)
            .apply();

    themeBuilder.apply();
}

#endif //TCMENU_THEME_BLOCK

