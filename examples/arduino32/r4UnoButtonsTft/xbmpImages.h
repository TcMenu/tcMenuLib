#ifndef XBMP_IMAGES_H
#define XBMP_IMAGES_H

// To use palette and size constants we need to use tcgfx types
#include <Arduino.h>
#include <graphics/DrawingPrimitives.h>

using namespace tcgfx;

// XBM_LSB_FIRST width=12, height=8, size=16
// auto size = Coord(12, 8);
const uint8_t faceIconXbm12x8[] PROGMEM = {
        0x09,0x09,0x95,0x0a,0x62,0x04,0x0a,0x05,0x02,0x04,0x64,0x02,0x98,0x01,0x04,0x02
};

// XBM_LSB_FIRST width=12, height=8, size=16
// auto size = Coord(12, 8);
const uint8_t logoIconXbm12x8[] PROGMEM = {
        0x28,0x00,0x28,0x00,0x54,0x0c,0x54,0x02,0xca,0x02,0x4a,0x02,0x45,0x02,0x85,0x0d
};

/**
 * A really simple value class that represents each of the icons declared above to be stored in a simple array
 */
class XBmpImageWithDescription {
private:
    const uint8_t* data;
    char name[20];
public:
    XBmpImageWithDescription(const uint8_t* data, const char* n): name{}, data(data) {
        strncpy(name, n, sizeof name);
    }

    const uint8_t* getData() { return data; }
    const char* getName() { return name; }
};

XBmpImageWithDescription imageWithDescription[] = {
        XBmpImageWithDescription(logoIconXbm12x8, "Tc Logo"),
        XBmpImageWithDescription(faceIconXbm12x8, "Face")
};

#define NUMBER_OF_XBMPS 2

#endif //XBMP_IMAGES_H