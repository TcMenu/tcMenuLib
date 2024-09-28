// To use palette and size constants we need to use tcgfx types
#include <graphics/DrawingPrimitives.h>
#include <graphics/TcThemeBuilder.h>
#include "esp32SimHub_menu.h"

using namespace tcgfx;

// PALETTE_4BPP width=64, height=64, size=2048
// auto size = Coord(64, 64);
const color_t engineBitmap_palette0[] PROGMEM { RGB(255,255,255), RGB(199,207,206), RGB(89,106,118), RGB(159,168,173), RGB(130,144,151), RGB(89,106,118), RGB(227,232,235), RGB(89,106,118), RGB(130,144,151), RGB(227,232,235), RGB(130,144,151), RGB(130,144,151), RGB(217,134,105), RGB(249,113,86), RGB(44,170,204), RGB(0,0,0) };
const uint8_t engineBitmap0[] PROGMEM = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x63,0x22,0x26,0x00,0x00,0x62,0x22,0x30,0x00,0x06,0x42,0x24,0x00,0x00,0x01,0x22,0x21,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x2f,0xff,0xff,0x10,0x03,0xff,0xff,0xf2,
        0x00,0x62,0xff,0xff,0x21,0x00,0x4f,0xff,0xff,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x04,0xff,0x43,0xff,0xf6,0x6f,0xf2,0x32,0xff,0x30,0x2f,0xf4,0x3f,0xf2,0x01,0xff,0x23,
        0x2f,0xf3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6f,0xf3,0x00,0xcf,0xf4,
        0x4f,0x20,0x00,0x2f,0xf3,0xff,0x10,0x01,0xff,0x32,0xf2,0x00,0x02,0xf2,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0x20,0x00,0x02,0xff,0xff,0x40,0x00,0x1f,0xff,0xf2,0x00,0x00,
        0x2f,0xff,0xf3,0x00,0x01,0xff,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,
        0xf4,0x44,0x42,0xff,0xff,0x24,0x44,0x2f,0xff,0xf2,0x33,0x33,0x2f,0xff,0xf2,0x44,0x42,0xff,0x60,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x62,0x2f,0xff,0xff,0x22,0x22,0xff,0xff,0xf2,0x22,0x22,0xff,0xff,0xf2,0x22,0x2f,0xff,
        0xff,0x23,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x32,0x22,0x22,0x2f,0xff,0xff,0x22,
        0x22,0xff,0xff,0xf2,0x22,0x2f,0xff,0xff,0xf2,0x22,0x2f,0xff,0xff,0x22,0x10,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x12,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,0xff,0xff,0xf3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x2f,0xf2,0x33,0x33,
        0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x4f,0xff,0x30,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0xff,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2f,0x26,0x66,0x66,0x00,0x00,0x00,0x00,0x00,0x02,
        0xf2,0x00,0x00,0x00,0x00,0x13,0x44,0x60,0x14,0x33,0x10,0x03,0x44,0x36,0x00,0x00,0x00,0x00,0x00,0x64,
        0x23,0xcc,0xcf,0xff,0xff,0xff,0x40,0x00,0x00,0x00,0x00,0x02,0xf4,0x00,0x00,0x00,0x04,0xff,0xff,0x40,
        0xff,0xff,0xf6,0x1f,0xff,0xf2,0x00,0x00,0x00,0x00,0x06,0x2f,0xff,0xcd,0xcf,0xff,0xff,0xff,0xf3,0x00,
        0x00,0x00,0x00,0x02,0xf4,0x00,0x00,0x00,0x6f,0xfe,0xee,0x16,0x3e,0xee,0x36,0x6e,0xee,0xff,0x10,0x00,
        0x00,0x00,0x01,0xf2,0xef,0x2d,0xcf,0xf1,0x11,0x12,0xf2,0x00,0x00,0x00,0x00,0x02,0xf4,0x00,0x0c,0xcc,
        0xcf,0xf2,0x2e,0xee,0xe2,0x22,0xee,0xe2,0x22,0xff,0xcc,0xcc,0xcc,0xcc,0xcc,0xff,0x2f,0x2d,0xcf,0xf6,
        0x00,0x04,0xf2,0x00,0x00,0x00,0x00,0x02,0xf3,0x00,0x0c,0xcc,0xc2,0xff,0xff,0xee,0xff,0xff,0xfe,0xef,
        0xff,0xf2,0xcc,0xcc,0xcc,0xcc,0xcc,0x2f,0xff,0xcd,0xcf,0xf6,0x00,0x04,0xf2,0x00,0x00,0x00,0x00,0x02,
        0xf4,0x00,0x0c,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,
        0xcc,0xdd,0x2f,0xf6,0x00,0x04,0xf2,0x00,0x00,0x00,0x00,0x04,0xff,0x10,0x0c,0xcc,0xcc,0xcc,0xcc,0xcc,
        0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xdc,0xff,0xff,0xff,0x2f,0xf2,0x00,
        0x00,0x00,0x00,0x06,0xff,0xf2,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,
        0x22,0x22,0x22,0x22,0x22,0xff,0xff,0xff,0xff,0xff,0xf2,0x00,0x00,0x00,0x00,0x00,0x3f,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf4,
        0x44,0x42,0xf2,0x00,0x00,0x00,0x00,0x00,0x01,0x44,0x4f,0xf2,0x42,0x22,0xff,0xf4,0x42,0x22,0x44,0x22,
        0x24,0x42,0x2f,0xf2,0x22,0x44,0x2f,0xff,0x22,0x24,0xff,0x40,0x00,0x03,0xf2,0x00,0x00,0x00,0x01,0x10,
        0x00,0x00,0x0f,0xf3,0x01,0x44,0xff,0x26,0x01,0x31,0x00,0x14,0x30,0x06,0x4f,0xf4,0x00,0x00,0x12,0xff,
        0x30,0x01,0xff,0x30,0x00,0x04,0xf2,0x00,0x00,0x00,0x1f,0xf2,0x60,0x00,0x0f,0xf2,0x13,0x44,0x22,0x24,
        0x34,0x44,0x36,0x34,0x44,0x34,0x42,0x24,0x33,0x33,0x42,0x22,0x44,0x34,0xff,0x30,0x00,0x04,0xf2,0x00,
        0x00,0x00,0x03,0xff,0xf6,0x00,0x0f,0xf2,0x4f,0xff,0xff,0xff,0xff,0xff,0x44,0x2f,0xff,0x42,0xff,0xff,
        0xff,0x44,0x2f,0xff,0xff,0x24,0xff,0x30,0x00,0x04,0xf2,0x00,0x00,0x00,0x00,0x1f,0xf1,0x00,0x0f,0xf2,
        0x42,0x2f,0xff,0xf2,0x44,0x22,0x36,0x42,0x22,0x34,0xff,0xff,0x24,0x16,0x2f,0xff,0xf2,0x44,0xff,0x30,
        0x00,0x04,0xf2,0x00,0x00,0x00,0x06,0x3f,0xf1,0x00,0x02,0xff,0x24,0x42,0xff,0xf2,0x00,0x64,0x43,0x44,
        0x41,0x03,0xff,0xf2,0x60,0x00,0x4f,0xff,0x46,0x3f,0xff,0x10,0x00,0x04,0xf2,0x00,0x00,0x00,0x6f,0xff,
        0xf1,0x00,0x06,0xff,0xf4,0x33,0x4f,0xff,0x10,0x03,0x31,0x31,0x60,0x1f,0xff,0x46,0x00,0x01,0xff,0xf3,
        0x60,0x4f,0xf4,0x00,0x00,0x04,0xf2,0x00,0x00,0x00,0x01,0x3f,0xf3,0x11,0x16,0x3f,0xf3,0x11,0x01,0xff,
        0xf6,0x00,0x00,0x00,0x00,0x4f,0xf3,0x00,0x00,0x02,0xff,0x10,0x00,0x2f,0x20,0x00,0x00,0x04,0xf2,0x00,
        0x00,0x00,0x00,0x6f,0xff,0xff,0xff,0xff,0xf3,0x3f,0x36,0x2f,0xf6,0x01,0x31,0x60,0x00,0x2f,0x20,0x00,
        0x11,0x12,0xf2,0x00,0x01,0x2f,0xf1,0x00,0x00,0x04,0xf2,0x00,0x00,0x00,0x03,0xff,0xff,0xff,0xff,0xff,
        0xf4,0x2f,0xf2,0xff,0xf0,0x3f,0xff,0x26,0x00,0x2f,0x20,0x04,0xff,0xff,0xf2,0x00,0x3f,0xff,0xff,0x30,
        0x00,0x04,0xf2,0x00,0x00,0x00,0x1f,0xff,0x31,0x12,0xf2,0xc2,0xff,0xff,0xff,0xff,0xf2,0xff,0xff,0xf2,
        0x00,0x2f,0x20,0x3f,0xff,0xff,0xf2,0x06,0xff,0xff,0xff,0xf6,0x00,0x04,0xf2,0x00,0x00,0x00,0x03,0x36,
        0x00,0x02,0xf2,0xc2,0xff,0xf2,0x22,0x2f,0xff,0xff,0xcc,0xff,0x60,0x2f,0x20,0x4f,0x21,0x62,0xf2,0x03,
        0xff,0xcc,0xcf,0xf4,0x00,0x04,0xf2,0x00,0x00,0x00,0x60,0x66,0x00,0x04,0xff,0xff,0xf3,0x60,0x00,0x01,
        0x4f,0xff,0xcc,0xff,0x30,0x2f,0x21,0x2f,0x20,0x02,0xf2,0x12,0xff,0x00,0x04,0xf2,0x66,0x03,0xf2,0x00,
        0x00,0x32,0x22,0x2f,0x23,0x06,0x2f,0xf2,0x60,0x06,0x66,0x00,0x01,0xff,0xff,0xff,0xf2,0xff,0xff,0xff,
        0xf2,0x2f,0xff,0xff,0xff,0x21,0x01,0xff,0xf2,0x30,0x60,0x00,0x06,0xff,0xff,0xff,0xff,0x66,0xff,0x26,
        0x06,0x2f,0xff,0x24,0x00,0xcf,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x30,0x2f,
        0xff,0xf1,0x00,0x00,0x01,0xff,0x33,0x3f,0xff,0xff,0xff,0x10,0x1f,0xff,0xff,0xff,0x2c,0xcc,0xff,0xff,
        0x43,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0xff,0xf1,0x63,0x3f,0xf2,0x00,0x00,0x01,0xff,0x00,0x62,
        0xff,0xff,0xf2,0x06,0xff,0x23,0x61,0x2f,0xf2,0xcc,0x2f,0x21,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x62,0xf2,0x00,0xe2,0xf2,0x00,0x00,0x01,0xff,0x60,0x62,0xf4,0x4f,0xf1,0x03,0xff,0x60,0xee,0xee,
        0xff,0xcc,0x2f,0x21,0x31,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xc2,0xff,0xee,0xe2,0xf2,0x00,0x00,
        0x01,0xff,0x60,0x62,0xf1,0x1f,0xf6,0x02,0xf4,0x0e,0xee,0xee,0xff,0x2c,0xcf,0xf2,0x24,0xcd,0xdd,0xdd,
        0xdd,0xdd,0xdd,0xdd,0xdd,0xd2,0xff,0xee,0xe2,0xf2,0x00,0x00,0x01,0xff,0x60,0x62,0xfc,0xcf,0xf6,0x0f,
        0xf1,0x1e,0xee,0xee,0x2f,0x2c,0xcf,0xff,0xf2,0x4c,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xd2,0xff,0xee,
        0xe2,0xf2,0x00,0x00,0x01,0xff,0x60,0x62,0xfc,0xcf,0xf6,0x0f,0xfe,0xee,0xee,0xee,0x2f,0x2c,0xcf,0xf2,
        0xff,0x43,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xd2,0xff,0xee,0xe2,0xf2,0x00,0x00,0x01,0xff,0x60,0x62,
        0xf1,0xcf,0xf6,0x02,0xff,0xee,0xee,0xee,0xff,0xcc,0xcf,0x24,0x22,0x44,0x3d,0xdd,0xdd,0xdd,0xdd,0xdd,
        0xdd,0xd2,0xff,0xee,0xe2,0xf2,0x00,0x00,0x01,0xff,0x60,0x62,0xf3,0x3f,0xf3,0x01,0xff,0x2e,0xee,0xef,
        0xf2,0xcc,0x2f,0x24,0x44,0x44,0x4c,0xdd,0xdd,0xdd,0xdd,0xdd,0xdd,0xd2,0xff,0xee,0xef,0xf2,0x00,0x00,
        0x01,0xff,0x60,0x62,0xff,0xff,0xf2,0x00,0x4f,0xff,0xff,0xff,0xfc,0xcc,0xff,0xf4,0x44,0x44,0x4c,0xdd,
        0xdd,0xdd,0xdd,0xdd,0xdd,0xdf,0xf2,0xee,0xef,0xf2,0x00,0x00,0x01,0xff,0x60,0x62,0xff,0xff,0xff,0x30,
        0x02,0xff,0xff,0xff,0xcc,0xc2,0xff,0xff,0x24,0x44,0xcd,0xdd,0xdd,0xdd,0xdd,0xdd,0xdc,0x2f,0xfe,0x2f,
        0xef,0xf2,0x00,0x00,0x01,0xff,0x44,0x4f,0xff,0x31,0xff,0xf3,0xcc,0xc2,0x22,0xcc,0xcc,0x2f,0xff,0xff,
        0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x22,0xff,0xff,0xf4,0x00,0x00,0x06,0xff,0xff,0xff,
        0xff,0x60,0x2f,0xff,0xcc,0xcc,0xcc,0xcc,0xc2,0xff,0x22,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
        0xff,0xf2,0x2f,0xff,0xff,0xf1,0x00,0x00,0x00,0x32,0x22,0x22,0x21,0x01,0xff,0x2f,0xff,0xcc,0xcc,0xc2,
        0xff,0xff,0xcc,0xff,0xf2,0x2f,0xf2,0xff,0x2f,0xf2,0xff,0x22,0x22,0x42,0xff,0xf2,0x22,0x10,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x04,0xff,0xcc,0xff,0xff,0xff,0xff,0xff,0xff,0x22,0xff,0xf4,0x2f,0x24,0xff,
        0x4f,0xf4,0xff,0x24,0x44,0xff,0xf2,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xff,0xcc,
        0xff,0xff,0xff,0xff,0xf4,0x2f,0xff,0xff,0xf4,0x2f,0x24,0xff,0x4f,0xf4,0xff,0x24,0x42,0xff,0xf6,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0xff,0xff,0xf2,0xff,0x2c,0x2f,0xf3,0x32,0xff,0x4f,
        0xf3,0x2f,0x23,0xff,0x3f,0xf3,0xff,0x23,0x34,0xff,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x12,0xff,0xf1,0x2f,0x2c,0x2f,0xff,0x21,0x22,0x1f,0xf1,0x4f,0x21,0xff,0x1f,0x21,0xff,0x41,
        0x01,0xff,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1f,0xf1,0x3f,0xff,0xff,
        0x2f,0x21,0x22,0x1f,0xf1,0x4f,0x21,0xff,0x1f,0x21,0xff,0x41,0x01,0xff,0x60,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xf1,0x03,0xff,0xf4,0x3f,0x21,0x22,0x1f,0xf1,0x4f,0x21,0xff,
        0x1f,0x21,0xff,0x41,0x01,0xff,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,
        0xf1,0x00,0x1f,0xf1,0x3f,0x21,0x22,0x1f,0xf1,0x4f,0x21,0xff,0x1f,0x21,0xff,0x41,0x03,0xff,0x60,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xf1,0x00,0x6f,0xf1,0x1f,0x21,0x22,0x1f,
        0xf1,0x4f,0x21,0xff,0x1f,0x21,0xff,0x46,0x01,0xff,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x0f,0xf3,0x00,0x6f,0xf6,0x1f,0x21,0x22,0x1f,0xf1,0x4f,0x21,0xff,0x1f,0x21,0xff,0x40,
        0x01,0xff,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xff,0x44,0x2f,0xf2,
        0x2f,0xf2,0xff,0x2f,0xf2,0x2f,0xf2,0xff,0x2f,0xf2,0xff,0x22,0x22,0xff,0x60,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
        0xff,0xff,0xff,0xff,0xff,0xf4,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x12,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x30,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void setupTheme() {
    TcThemeBuilder themeBuilder(renderer);

    themeBuilder.menuItemOverride(menuEngine)
            .onRowCol(3, 1, 1)
            .withImage4bpp(Coord(64, 64), engineBitmap_palette0,  engineBitmap0)
            .apply();

    themeBuilder.apply();
}