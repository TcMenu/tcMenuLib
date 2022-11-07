
#include <testing/SimpleTest.h>
#include <RuntimeMenuItem.h>
#include <ScrollChoiceMenuItem.h>
#include "fixtures_extern.h"
#include <tcMenu.h>
#include <IoLogging.h>

test(testValueAtPositionAndGetters) {
    char ramStored[] = "item1item2item3";
    ScrollChoiceMenuItem choice(101, enumItemRenderFn, 0, ramStored, 5, 3, nullptr);

    // first check the three valid index positions
    char buffer[10];
    choice.valueAtPosition(buffer, sizeof buffer, 0);
    assertStringEquals("item1", buffer);

    choice.valueAtPosition(buffer, sizeof buffer, 1);
    assertStringEquals("item2", buffer);

    choice.valueAtPosition(buffer, sizeof buffer, 2);
    assertStringEquals("item3", buffer);

    // now the invalid case

    choice.valueAtPosition(buffer, sizeof buffer, 3);
    assertStringEquals("", buffer);

    // now check the positional stuff

    assertEquals(0, choice.getCurrentValue());
    assertEquals((uint8_t)3, choice.getNumberOfRows());
    choice.copyValue(buffer, sizeof buffer);
    assertStringEquals("item1", buffer);

    choice.setCurrentValue(2);
    assertEquals(2, choice.getCurrentValue());
    choice.copyValue(buffer, sizeof buffer);
    assertStringEquals("item3", buffer);

    assertEquals((uint16_t)0xFFFFU, choice.getEepromPosition());

    choice.setCurrentValue(55);
    assertEquals(2, choice.getCurrentValue());
}

test(testValueAtPositionEeeprom) {
    char buffer[16];
    strcpy(buffer, "computer123");
    eeprom.writeArrayToRom(15, reinterpret_cast<const uint8_t *>(buffer), sizeof buffer);
    strcpy(buffer, "turntable123");
    eeprom.writeArrayToRom(25, reinterpret_cast<const uint8_t *>(buffer), sizeof buffer);

    eeprom.serPrintContents(0, 35);

    menuMgr.setEepromRef(&eeprom);
    ScrollChoiceMenuItem choice(101, enumItemRenderFn, 0, 15, 10, 2, nullptr);

    // test the two valid cases

    choice.valueAtPosition(buffer, sizeof buffer, 0);
    assertStringEquals("computer12", buffer);

    choice.valueAtPosition(buffer, sizeof buffer, 1);
    assertStringEquals("turntable1", buffer);

    // and the invalid case too

    choice.valueAtPosition(buffer, sizeof buffer, 2);
    assertStringEquals("", buffer);

    choice.copyTransportText(buffer, sizeof buffer);
    assertStringEquals("0-computer12", buffer);
}

bool renderingInvoked = false;
int testEnumItemRenderFn(RuntimeMenuItem *item, uint8_t row, RenderFnMode mode, char *buffer, int bufferSize) {
    if (item->getMenuType() != MENUTYPE_SCROLLER_VALUE) return 0;
    switch (mode) {
        case RENDERFN_VALUE: {
            itoa(row, buffer, 10);
            return true;
        }
        case RENDERFN_NAME: {
            strcpy(buffer, "CustomEnum");
            return true;
        }
        case RENDERFN_EEPROM_POS:
            return 99;
        case RENDERFN_INVOKE:
            renderingInvoked = true;
            return false;
        default: return false;
    }
}

test(testValueAtPositionCustom) {
    ScrollChoiceMenuItem choice(101, testEnumItemRenderFn, 0, 100, nullptr);

    char buffer[15];
    char compBuf[15];
    for(int i = 0; i < 100; i++) {
        choice.setCurrentValue(i, true);
        assertEquals(i, choice.getCurrentValue());
        choice.copyValue(buffer, sizeof(buffer));
        itoa(i, compBuf, 10);
        assertStringEquals(compBuf, buffer);
    }

    choice.valueAtPosition(buffer, sizeof(buffer), 2);
    assertStringEquals("2", buffer);

    choice.copyNameToBuffer(buffer, sizeof buffer);
    assertStringEquals("CustomEnum", buffer);

    assertEquals((uint16_t)99U, choice.getEepromPosition());

    assertFalse(renderingInvoked);
    choice.setCurrentValue(1);
    assertTrue(renderingInvoked);
    assertEquals(1, choice.getCurrentValue());

    choice.copyTransportText(buffer, sizeof buffer);
    assertStringEquals("1-1", buffer);
}

int colorCbCount = 0;

void myCountingCallback(int id) {
    colorCbCount++;
}

RENDERING_CALLBACK_NAME_INVOKE(colorItemNoAlphaFn, rgbAlphaItemRenderFn, "RGB NoAlpha", 233, myCountingCallback);
Rgb32MenuItem colorItemNoAlpha(202, colorItemNoAlphaFn, false, nullptr);

test(testColorMenuItemNoAlpha) {
    char sz[20];
    auto originalChangeCount = colorCbCount;
    assertEquals((uint16_t)202, colorItemNoAlpha.getId());
    assertEquals((uint16_t)233, colorItemNoAlpha.getEepromPosition());
    colorItemNoAlpha.copyNameToBuffer(sz, sizeof(sz));
    assertEquals("RGB NoAlpha", sz);

    colorItemNoAlpha.setColorData(RgbColor32(100, 50, 200));

    colorItemNoAlpha.copyValue(sz, sizeof sz);
    assertEquals("R100 G50 B200", sz);

    assertEquals(3, colorItemNoAlpha.beginMultiEdit());
    assertEquals(255, colorItemNoAlpha.nextPart());
    colorItemNoAlpha.copyValue(sz, sizeof sz);
    assertEquals("R100 G50 B200", sz);
    assertTrue(checkEditorHints(1, 4, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    colorItemNoAlpha.valueChanged(145);
    assertEquals(originalChangeCount + 2, colorCbCount);

    assertEquals(255, colorItemNoAlpha.nextPart());
    colorItemNoAlpha.copyValue(sz, sizeof sz);
    assertEquals("R145 G50 B200", sz);
    assertTrue(checkEditorHints(6, 8, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    colorItemNoAlpha.valueChanged(222);
    assertEquals(originalChangeCount + 3, colorCbCount);

    assertEquals(255, colorItemNoAlpha.nextPart());
    colorItemNoAlpha.copyValue(sz, sizeof sz);
    assertEquals("R145 G222 B200", sz);
    assertTrue(checkEditorHints(11, 14, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    colorItemNoAlpha.valueChanged(1);
    assertEquals(originalChangeCount + 4, colorCbCount);

    assertEquals(0, colorItemNoAlpha.nextPart());
    colorItemNoAlpha.copyValue(sz, sizeof sz);
    assertEquals("R145 G222 B1", sz);
}

RENDERING_CALLBACK_NAME_INVOKE(colorItemWithAlphaFn, rgbAlphaItemRenderFn, "RGB Alpha", 333, myCountingCallback);
Rgb32MenuItem colorItemWithAlpha(202, colorItemWithAlphaFn, true, nullptr);

test(testColorMenuItemWithAlphaAndFn) {
    auto originalChangeCount = colorCbCount;

    char sz[25];
    assertEquals((uint16_t)202, colorItemWithAlpha.getId());
    colorItemWithAlpha.copyNameToBuffer(sz, sizeof(sz));
    assertEquals("RGB Alpha", sz);

    colorItemWithAlpha.setColorData(RgbColor32(100, 50, 200, 150));

    colorItemWithAlpha.copyValue(sz, sizeof sz);
    assertEquals("R100 G50 B200 A150", sz);

    assertEquals(4, colorItemWithAlpha.beginMultiEdit());
    assertEquals(255, colorItemWithAlpha.nextPart());
    assertEquals(255, colorItemWithAlpha.nextPart());
    assertEquals(255, colorItemWithAlpha.nextPart());
    assertEquals(255, colorItemWithAlpha.nextPart());
    colorItemWithAlpha.copyValue(sz, sizeof sz);
    assertEquals("R100 G50 B200 A150", sz);
    assertTrue(checkEditorHints(15, 18, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    colorItemWithAlpha.valueChanged(225);
    assertEquals(originalChangeCount + 2, colorCbCount);

    assertEquals(0, colorItemWithAlpha.nextPart());
    colorItemWithAlpha.copyValue(sz, sizeof sz);
    assertEquals("R100 G50 B200 A225", sz);
}

class ColorTestFixing : public SimpleTest::UnitTestExecutor {
public:
    void assertColor(const char *name, const RgbColor32 &col, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
        serdebugF2("Color test: ", name);
        assertEquals(red, col.red);
        assertEquals(green, col.green);
        assertEquals(blue, col.blue);
        assertEquals(alpha, col.alpha);
    }
};

testF(ColorTestFixing, testColor32Struct) {
    assertColor("html1", RgbColor32("#F47cbf"), 0xF4U, 0x7Cu, 0xBFU, 0xFFU);
    assertColor("html2", RgbColor32("#FFF"), 0xF0U, 0xF0u, 0xF0U, 0xFFU);
    assertColor("htmla", RgbColor32("#F3d5e8a0"), 0xF3U, 0xd5u, 0xe8U, 0xa0U);
    assertColor("html3", RgbColor32("not"), 0x00U, 0x00u, 0x00U, 0xFFU);
    assertColor("html4", RgbColor32("#ZXY"), 0x00U, 0x00u, 0x00U, 0xFFU);
    assertColor("rgb", RgbColor32(200, 100, 20), 200, 100, 20, 255);
    auto withAlpha = RgbColor32(200, 100, 20, 103);
    assertColor("rgba", withAlpha, 200, 100, 20, 103);
    assertColor("copy", RgbColor32(withAlpha), 200, 100, 20, 103);

    RgbColor32 forHtml(0xf4, 0xaa, 0x55, 0xbb);

    // ensure we can get as html for both alpha and non-alpha case
    char sz[10];
    forHtml.asHtmlString(sz, sizeof sz, true);
    assertStringEquals("#F4AA55BB", sz);
    forHtml.asHtmlString(sz, sizeof sz, false);
    assertStringEquals("#F4AA55", sz);

    // ensure string too short does not overflow.
    forHtml.asHtmlString(sz, 2, false);
    assertStringEquals("", sz);
}