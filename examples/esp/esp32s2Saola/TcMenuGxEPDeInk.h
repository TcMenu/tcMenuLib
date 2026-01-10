/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */
#ifndef TCLIBRARYDEV_TCMENUGXEPDEINK_H
#define TCLIBRARYDEV_TCMENUGXEPDEINK_H

#include <tcUtil.h>
#include <tcUnicodeHelper.h>
#include <gfxfont.h>
#include <graphics/GfxMenuConfig.h>
#include <graphics/GraphicsDeviceRenderer.h>

#include <GxEPD2_BW.h>

// some colour displays don't create this value
#ifndef BLACK
#define BLACK GxEPD_BLACK
#endif

// some colour displays don't create this value
#ifndef WHITE
#define WHITE GxEPD_WHITE
#endif

using namespace tcgfx;
extern const ConnectorLocalInfo applicationInfo;

#define MAX_DISPLAY_BUFFER_SIZE 15000ul // ~15k is a good compromise
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
typedef GxEPD2_BW<GxEPD2_154_GDEY0154D67, MAX_HEIGHT(GxEPD2_154_GDEY0154D67)> TcGxEPD2;

class TcMenuGxEPDeInk : public DeviceDrawable {
    TcGxEPD2& display;
    const GFXfont* computedFont = nullptr;
    int16_t computedBaseline = 0;
    int16_t computedHeight = 0;
    bool firstPageDone = false;
public:
    explicit TcMenuGxEPDeInk(TcGxEPD2& display) : display(display) {
    }
    ~TcMenuGxEPDeInk() override = default;

    DeviceDrawable* getSubDeviceFor(const Coord &where, const Coord &size, const color_t *palette, int paletteSize) override {return nullptr; }

    void internalDrawText(const Coord &where, const void *font, int mag, const char *text) override;
    void drawBitmap(const Coord &where, const DrawableIcon *icon, bool selected) override;
    void drawXBitmap(const Coord &where, const Coord &size, const uint8_t *data) override;
    void drawBox(const Coord &where, const Coord &size, bool filled) override;
    void drawCircle(const Coord &where, int radius, bool filled) override;
    void drawPolygon(const Coord *points, int numPoints, bool filled) override;

    Coord getDisplayDimensions() override {  return {display.width(), display.height()}; }
    void transaction(bool isStarting, bool redrawNeeded) override;
    Coord internalTextExtents(const void *font, int mag, const char *text, int *baseline) override;

    void drawPixel(uint16_t x, uint16_t y) override;
    UnicodeFontHandler *createFontHandler() override;
protected:
    void computeBaselineIfNeeded(const GFXfont* font);

};

class TcMenuGxEPDFontPlotter : public TextPlotPipeline {
private:
    TcGxEPD2& display;
public:
    explicit TcMenuGxEPDFontPlotter(TcGxEPD2& display) : display(display) {}
    void drawPixel(uint16_t x, uint16_t y, uint32_t color) override;
    void setCursor(const Coord &where) override;

    Coord getCursor() override;

    Coord getDimensions() override;
};


#endif //TCLIBRARYDEV_TCMENUGXEPDEINK_H
