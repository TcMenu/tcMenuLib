
#include <testing/SimpleTest.h>
#include <RuntimeMenuItem.h>
#include "fixtures_extern.h"
#include <tcUtil.h>

test(testCoreAndBooleanMenuItem) {
    // these may seem overkill but the setters are bitwise so quite complex.
    assertEquals(MENUTYPE_BOOLEAN_VALUE, boolItem1.getMenuType());

    boolItem1.setChanged(true);
    assertTrue(boolItem1.isChanged());
    boolItem1.setChanged(false);
    assertFalse(boolItem1.isChanged());

    boolItem1.setReadOnly(true);
    assertTrue(boolItem1.isReadOnly());
    boolItem1.setReadOnly(false);
    assertFalse(boolItem1.isReadOnly());

    boolItem1.setLocalOnly(true);
    assertTrue(boolItem1.isLocalOnly());
    boolItem1.setLocalOnly(false);
    assertFalse(boolItem1.isLocalOnly());

    boolItem1.setSendRemoteNeededAll();
    assertTrue(boolItem1.isSendRemoteNeeded(0));
    assertTrue(boolItem1.isSendRemoteNeeded(1));
    assertTrue(boolItem1.isSendRemoteNeeded(2));

    boolItem1.setSendRemoteNeeded(1, false);
    assertTrue(boolItem1.isSendRemoteNeeded(0));
    assertFalse(boolItem1.isSendRemoteNeeded(1));
    assertTrue(boolItem1.isSendRemoteNeeded(2));

    assertEquals(uint16_t(4), boolItem1.getId());
    assertEquals(uint16_t(8), boolItem1.getEepromPosition());
    assertEquals(uint16_t(1), boolItem1.getMaximumValue());

    char sz[4];
    boolItem1.copyNameToBuffer(sz, sizeof(sz));
    assertStringEquals(sz, "Boo");

    idOfCallback = 0;
    boolItem1.triggerCallback();
    assertEquals(4, idOfCallback);

    boolItem1.setBoolean(false);
    assertFalse(boolItem1.getBoolean());
    boolItem1.setBoolean(true);
    assertTrue(boolItem1.getBoolean());

    char buffer[20];
    boolItem1.setBoolean(false);
    copyMenuItemNameAndValue(&boolItem1, buffer, sizeof(buffer));
    assertStringEquals("Bool1: FALSE", buffer);

    boolItem1.setBoolean(true);
    copyMenuItemNameAndValue(&boolItem1, buffer, sizeof(buffer));
    assertStringEquals("Bool1: TRUE", buffer);
}

bool checkWholeFraction(AnalogMenuItem* item, uint16_t whole, int16_t fract, bool neg = false) {
    WholeAndFraction wf = item->getWholeAndFraction();
    if (wf.fraction != fract || wf.whole != whole || wf.negative != neg) {
        printMenuItem(item);
        serdebugF4("Mismatch in whole fraction expected: ", whole, fract, neg);
        serdebugF4("Actual values: ", wf.whole, wf.fraction, wf.negative);
        return false;
    }
    return true;
}

test(testEnumMenuItem) {
    assertEquals(MENUTYPE_ENUM_VALUE, menuEnum1.getMenuType());

    char sz[20];
    // try getting all the strings.
    menuEnum1.copyEnumStrToBuffer(sz, sizeof(sz), 0);
    assertStringEquals(sz, "ITEM1");
    menuEnum1.copyEnumStrToBuffer(sz, sizeof(sz), 1);
    assertStringEquals(sz, "ITEM2");
    menuEnum1.copyEnumStrToBuffer(sz, sizeof(sz), 2);
    assertStringEquals(sz, "ITEM3");

    menuEnum1.setCurrentValue(1);
    copyMenuItemNameAndValue(&menuEnum1, sz, sizeof sz);
    assertStringEquals("Enum1: ITEM2", sz);

    // try with limited string buffer and ensure properly terminated
    menuEnum1.copyEnumStrToBuffer(sz, 4, 2);
    assertStringEquals(sz, "ITE");

    // verify the others.
    assertEquals(5, menuEnum1.getLengthOfEnumStr(0));
    assertEquals(5, menuEnum1.getLengthOfEnumStr(1));
    assertEquals(uint16_t(2), menuEnum1.getMaximumValue());
}

test(testAnalogMenuItem) {
    assertEquals(MENUTYPE_INT_VALUE, menuAnalog.getMenuType());

    char sz[20];
    menuAnalog.setCurrentValue(25);
    copyMenuItemNameAndValue(&menuAnalog, sz, sizeof(sz));
    assertStringEquals(sz, "Analog: 25AB");

    assertEquals((uint16_t)255U, menuAnalog.getMaximumValue());
    assertEquals(0, menuAnalog.getOffset());
    assertEquals((uint16_t)1U, menuAnalog.getDivisor());
    assertEquals(2, menuAnalog.unitNameLength());
    menuAnalog.copyUnitToBuffer(sz);
    assertStringEquals("AB", sz);

    assertEquals(uint8_t(0), menuAnalog.getDecimalPlacesForDivisor());

    menuAnalog.setCurrentValue(192);
    assertEquals(192, menuAnalog.getIntValueIncludingOffset());
    assertEquals((uint16_t)192U, menuAnalog.getCurrentValue());
    assertTrue(checkWholeFraction(&menuAnalog, 192, 0));
    assertEquals((uint16_t)192U, menuAnalog.getCurrentValue());
    assertFloatNear(float(192.0), menuAnalog.getAsFloatingPointValue(), float(0.0001));
    menuAnalog.setCurrentValue(0);
    assertTrue(checkWholeFraction(&menuAnalog, 0, 0));
    assertFloatNear(float(0.0), menuAnalog.getAsFloatingPointValue(), float(0.0001));
    menuAnalog.setFromFloatingPointValue(21.3);
    assertFloatNear(float(21.0), menuAnalog.getAsFloatingPointValue(), float(0.0001));
    assertTrue(checkWholeFraction(&menuAnalog, 21, 0));
    menuAnalog.copyValue(sz, sizeof sz);
    assertStringEquals("21AB", sz);
    menuAnalog.setFromFloatingPointValue(21.3);
    assertFloatNear(float(21.0), menuAnalog.getAsFloatingPointValue(), float(0.0001));
    assertTrue(checkWholeFraction(&menuAnalog, 21, 0));

    menuAnalog2.setFromFloatingPointValue(-0.2);
    assertFloatNear(-0.2F, menuAnalog2.getAsFloatingPointValue(), 0.0001F);
}

test(testAnalogItemNegativeInteger) {
    AnalogMenuInfo myInfo = { "Analog12", 1055, 0xffff, 255, nullptr, -20, 1, "dB" };
    AnalogMenuItem localAnalog(&myInfo, 0, nullptr, INFO_LOCATION_RAM);

    assertEquals(-20, localAnalog.getIntValueIncludingOffset());
    assertEquals((uint16_t)0, localAnalog.getCurrentValue());
    assertTrue(checkWholeFraction(&localAnalog, 20, 0, true));
    assertFloatNear(float(-20.0), localAnalog.getAsFloatingPointValue(), float(0.0001));

    localAnalog.setCurrentValue(255);
    assertTrue(checkWholeFraction(&localAnalog, 235, 0));
    assertFloatNear(float(235.0), localAnalog.getAsFloatingPointValue(), float(0.0001));
    assertEquals(235, localAnalog.getIntValueIncludingOffset());
}

test(testGetIntValueIncudingOffset) {
    AnalogMenuInfo myInfo = { "Analog12", 102, 0xffff, 200, nullptr, 100, 1, "xyz" };
    AnalogMenuItem localAnalog(&myInfo, 0, nullptr, INFO_LOCATION_RAM);
    assertEquals(100, localAnalog.getIntValueIncludingOffset());
    localAnalog.setCurrentValue(123);
    assertEquals(223, localAnalog.getIntValueIncludingOffset());
    localAnalog.setCurrentValue(123);
    assertEquals(223, localAnalog.getIntValueIncludingOffset());
    localAnalog.setCurrentValue(50);
    assertEquals(150, localAnalog.getIntValueIncludingOffset());
}

test(testAnalogValuesWithFractions) {
    char sz[20];

    assertEquals(uint8_t(2), menuNumTwoDp.getDecimalPlacesForDivisor());
    menuNumTwoDp.setFromFloatingPointValue(98.234);
    assertFloatNear(float(98.23), menuNumTwoDp.getAsFloatingPointValue(), float(0.0001));
    assertEquals(uint16_t(9823), menuNumTwoDp.getCurrentValue());
    assertTrue(checkWholeFraction(&menuNumTwoDp, 98, 23));
    menuNumTwoDp.copyValue(sz, sizeof sz);
    assertStringEquals("98.23", sz);

    menuNumTwoDp.setFromWholeAndFraction(WholeAndFraction(22, 99, false));
    assertFloatNear(float(22.99), menuNumTwoDp.getAsFloatingPointValue(), float(0.0001));
    assertEquals(uint16_t(2299), menuNumTwoDp.getCurrentValue());
    assertTrue(checkWholeFraction(&menuNumTwoDp, 22, 99));

    menuNumTwoDp.copyValue(sz, sizeof sz);
    assertStringEquals("22.99", sz);

    assertEquals(uint8_t(1), menuHalvesOffs.getDecimalPlacesForDivisor());
    menuHalvesOffs.setCurrentValue(21);
    assertTrue(checkWholeFraction(&menuHalvesOffs, 39, 5, true));
    assertFloatNear(float(-39.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));
    menuHalvesOffs.copyValue(sz, sizeof sz);
    assertStringEquals("-39.5dB", sz);

    menuHalvesOffs.setCurrentValue(103);
    assertTrue(checkWholeFraction(&menuHalvesOffs, 1, 5));
    assertFloatNear(float(1.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));

    menuHalvesOffs.setFromFloatingPointValue(50.5);
    assertTrue(checkWholeFraction(&menuHalvesOffs, 50, 5));
    assertFloatNear(float(50.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));
    assertEquals(uint16_t(201), menuHalvesOffs.getCurrentValue());
    menuHalvesOffs.copyValue(sz, sizeof sz);
    assertStringEquals("50.5dB", sz);

    menuHalvesOffs.setFromWholeAndFraction(WholeAndFraction(10, 5, false));
    assertEquals(uint16_t(121), menuHalvesOffs.getCurrentValue());
    assertTrue(checkWholeFraction(&menuHalvesOffs, 10, 5));
    assertFloatNear(float(10.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));
    menuHalvesOffs.copyValue(sz, sizeof sz);
    assertStringEquals("10.5dB", sz);

    menuHalvesOffs.setFromFloatingPointValue(-0.5);
    assertTrue(checkWholeFraction(&menuHalvesOffs, 0, 5, true));
    menuHalvesOffs.copyValue(sz, sizeof sz);
    assertStringEquals("-0.5dB", sz);
}

int calleeCount123 = 0;
int calleeCount321 = 0;
void onTest123(int id) {
    if(id == 123) calleeCount123++;
    if(id == 321) calleeCount321++;
}

test(testAnalogValueItemInMemory) {
    AnalogMenuInfo analogInfo = { "Test 123", 123, 0xffff, 234, onTest123, -180, 10, "dB" };
    AnalogMenuItem analogItem(&analogInfo, 0, nullptr, INFO_LOCATION_RAM);

    char sz[20];
    analogItem.copyNameToBuffer(sz, sizeof sz);
    assertStringEquals("Test 123", sz);

    analogItem.copyUnitToBuffer(sz, sizeof sz);
    assertStringEquals("dB", sz);

    assertEquals((uint16_t)10, analogItem.getDivisor());
    assertEquals((uint8_t)1, analogItem.getDecimalPlacesForDivisor());
    assertEquals((uint16_t)10, analogItem.getActualDecimalDivisor());
    assertEquals((uint16_t)234, analogItem.getMaximumValue());
    assertEquals(-180, analogItem.getOffset());
    assertEquals((uint16_t)123, analogItem.getId());
    assertEquals((uint16_t)0xffff, analogItem.getEepromPosition());
    assertEquals(MENUTYPE_INT_VALUE, analogItem.getMenuType());

    analogItem.setCurrentValue(190);
    assertEquals(1, calleeCount123);
    assertTrue(analogItem.isChanged());
    assertTrue(analogItem.isSendRemoteNeeded(0));

    copyMenuItemNameAndValue(&analogItem, sz, sizeof sz);
    assertStringEquals("Test 123: 1.0dB", sz);
}

test(testBooleanItemInMemory) {
    BooleanMenuInfo boolInfo = { "Boolio", 321, 22, 1, onTest123, NAMING_ON_OFF};
    BooleanMenuItem boolItem(&boolInfo, false, nullptr, INFO_LOCATION_RAM);

    char sz[20];
    boolItem.copyNameToBuffer(sz, sizeof sz);
    assertStringEquals("Boolio", sz);

    assertEquals((uint16_t)1, boolItem.getMaximumValue());
    assertEquals((uint16_t)321, boolItem.getId());
    assertEquals(NAMING_ON_OFF, boolItem.getBooleanNaming());
    assertEquals((uint16_t)22, boolItem.getEepromPosition());
    assertEquals(MENUTYPE_BOOLEAN_VALUE, boolItem.getMenuType());

    boolItem.setBoolean(true);
    assertEquals(1, calleeCount321);
    assertTrue(boolItem.isChanged());
    assertTrue(boolItem.isSendRemoteNeeded(0));

    copyMenuItemNameAndValue(&boolItem, sz, sizeof sz);
    assertStringEquals("Boolio: ON", sz);
}

test(testFloatItemInMemory) {
    FloatMenuInfo fltInfo = { "Floater", 122, 0xffff, 3, nullptr};
    FloatMenuItem fltItem(&fltInfo, nullptr, INFO_LOCATION_RAM);

    char sz[20];
    assertEquals(3, fltItem.getDecimalPlaces());
    assertEquals((uint16_t)122, fltItem.getId());
    assertEquals((uint16_t)0xffff, fltItem.getEepromPosition());
    assertEquals(MENUTYPE_FLOAT_VALUE, fltItem.getMenuType());

    fltItem.setFloatValue(223.2341);
    assertTrue(fltItem.isChanged());
    assertTrue(fltItem.isSendRemoteNeeded(0));

    copyMenuItemNameAndValue(&fltItem, sz, sizeof sz);
    assertStringEquals("Floater: 223.234", sz);
}
