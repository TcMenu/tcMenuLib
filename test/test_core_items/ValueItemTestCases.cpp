
#include <unity.h>
#include "../tutils/fixtures_extern.h"
#include <tcUtil.h>

void testCoreAndBooleanMenuItem() {
    // these may seem overkill but the setters are bitwise so quite complex.
    TEST_ASSERT_EQUAL(MENUTYPE_BOOLEAN_VALUE, boolItem1.getMenuType());

    boolItem1.setChanged(true);
    TEST_ASSERT_TRUE(boolItem1.isChanged());
    boolItem1.setChanged(false);
    TEST_ASSERT_FALSE(boolItem1.isChanged());

    boolItem1.setReadOnly(true);
    TEST_ASSERT_TRUE(boolItem1.isReadOnly());
    boolItem1.setReadOnly(false);
    TEST_ASSERT_FALSE(boolItem1.isReadOnly());

    boolItem1.setLocalOnly(true);
    TEST_ASSERT_TRUE(boolItem1.isLocalOnly());
    boolItem1.setLocalOnly(false);
    TEST_ASSERT_FALSE(boolItem1.isLocalOnly());

    boolItem1.setSendRemoteNeededAll();
    TEST_ASSERT_TRUE(boolItem1.isSendRemoteNeeded(0));
    TEST_ASSERT_TRUE(boolItem1.isSendRemoteNeeded(1));
    TEST_ASSERT_TRUE(boolItem1.isSendRemoteNeeded(2));

    boolItem1.setSendRemoteNeeded(1, false);
    TEST_ASSERT_TRUE(boolItem1.isSendRemoteNeeded(0));
    TEST_ASSERT_FALSE(boolItem1.isSendRemoteNeeded(1));
    TEST_ASSERT_TRUE(boolItem1.isSendRemoteNeeded(2));

    TEST_ASSERT_EQUAL(uint16_t(4), boolItem1.getId());
    TEST_ASSERT_EQUAL(uint16_t(8), boolItem1.getEepromPosition());
    TEST_ASSERT_EQUAL(uint16_t(1), boolItem1.getMaximumValue());

    char sz[4];
    boolItem1.copyNameToBuffer(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING(sz, "Boo");

    idOfCallback = 0;
    boolItem1.triggerCallback();
    TEST_ASSERT_EQUAL(4, idOfCallback);

    boolItem1.setBoolean(false);
    TEST_ASSERT_FALSE(boolItem1.getBoolean());
    boolItem1.setBoolean(true);
    TEST_ASSERT_TRUE(boolItem1.getBoolean());

    char buffer[20];
    boolItem1.setBoolean(false);
    copyMenuItemNameAndValue(&boolItem1, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING("Bool1: FALSE", buffer);

    boolItem1.setBoolean(true);
    copyMenuItemNameAndValue(&boolItem1, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL_STRING("Bool1: TRUE", buffer);
}

bool checkWholeFraction(AnalogMenuItem* item, uint16_t whole, int16_t fract, bool neg = false) {
    WholeAndFraction wf = item->getWholeAndFraction();
    if (wf.fraction != fract || wf.whole != whole || wf.negative != neg) {
        printMenuItem(item);
        printf("Mismatch in whole fraction expected: %d %d %d\n", whole, fract, neg);
        serdebugF4("Actual values: %d, %d, %d\n", wf.whole, wf.fraction, wf.negative);
        return false;
    }
    return true;
}

void testEnumMenuItem() {
    TEST_ASSERT_EQUAL(MENUTYPE_ENUM_VALUE, menuEnum1.getMenuType());

    char sz[20];
    // try getting all the strings.
    menuEnum1.copyEnumStrToBuffer(sz, sizeof(sz), 0);
    TEST_ASSERT_EQUAL_STRING(sz, "ITEM1");
    menuEnum1.copyEnumStrToBuffer(sz, sizeof(sz), 1);
    TEST_ASSERT_EQUAL_STRING(sz, "ITEM2");
    menuEnum1.copyEnumStrToBuffer(sz, sizeof(sz), 2);
    TEST_ASSERT_EQUAL_STRING(sz, "ITEM3");

    menuEnum1.setCurrentValue(1);
    copyMenuItemNameAndValue(&menuEnum1, sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("Enum1: ITEM2", sz);

    // try with limited string buffer and ensure properly terminated
    menuEnum1.copyEnumStrToBuffer(sz, 4, 2);
    TEST_ASSERT_EQUAL_STRING(sz, "ITE");

    // verify the others.
    TEST_ASSERT_EQUAL(5, menuEnum1.getLengthOfEnumStr(0));
    TEST_ASSERT_EQUAL(5, menuEnum1.getLengthOfEnumStr(1));
    TEST_ASSERT_EQUAL(uint16_t(2), menuEnum1.getMaximumValue());
}

void testAnalogMenuItem() {
    TEST_ASSERT_EQUAL(MENUTYPE_INT_VALUE, menuAnalog.getMenuType());

    char sz[20];
    menuAnalog.setCurrentValue(25);
    copyMenuItemNameAndValue(&menuAnalog, sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING(sz, "Analog: 25AB");

    TEST_ASSERT_EQUAL((uint16_t)255U, menuAnalog.getMaximumValue());
    TEST_ASSERT_EQUAL(0, menuAnalog.getOffset());
    TEST_ASSERT_EQUAL((uint16_t)1U, menuAnalog.getDivisor());
    TEST_ASSERT_EQUAL(2, menuAnalog.unitNameLength());
    menuAnalog.copyUnitToBuffer(sz);
    TEST_ASSERT_EQUAL_STRING("AB", sz);

    TEST_ASSERT_EQUAL(uint8_t(0), menuAnalog.getDecimalPlacesForDivisor());

    menuAnalog.setCurrentValue(192);
    TEST_ASSERT_EQUAL(192, menuAnalog.getIntValueIncludingOffset());
    TEST_ASSERT_EQUAL((uint16_t)192U, menuAnalog.getCurrentValue());
    TEST_ASSERT_TRUE(checkWholeFraction(&menuAnalog, 192, 0));
    TEST_ASSERT_EQUAL((uint16_t)192U, menuAnalog.getCurrentValue());
    TEST_ASSERT_FLOAT_WITHIN(float(192.0), menuAnalog.getAsFloatingPointValue(), float(0.0001));
    menuAnalog.setCurrentValue(0);
    TEST_ASSERT_TRUE(checkWholeFraction(&menuAnalog, 0, 0));
    TEST_ASSERT_FLOAT_WITHIN(float(0.0), menuAnalog.getAsFloatingPointValue(), float(0.0001));
    menuAnalog.setFromFloatingPointValue(21.3);
    TEST_ASSERT_FLOAT_WITHIN(float(21.0), menuAnalog.getAsFloatingPointValue(), float(0.0001));
    TEST_ASSERT_TRUE(checkWholeFraction(&menuAnalog, 21, 0));
    menuAnalog.copyValue(sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("21AB", sz);
    menuAnalog.setFromFloatingPointValue(21.3);
    TEST_ASSERT_FLOAT_WITHIN(float(21.0), menuAnalog.getAsFloatingPointValue(), float(0.0001));
    TEST_ASSERT_TRUE(checkWholeFraction(&menuAnalog, 21, 0));

    menuAnalog2.setFromFloatingPointValue(-0.2);
    TEST_ASSERT_FLOAT_WITHIN(-0.2F, menuAnalog2.getAsFloatingPointValue(), 0.0001F);
}

void testAnalogItemNegativeInteger() {
    AnalogMenuInfo myInfo = { "Analog12", 1055, 0xffff, 255, nullptr, -20, 1, "dB" };
    AnalogMenuItem localAnalog(&myInfo, 0, nullptr, INFO_LOCATION_RAM);

    TEST_ASSERT_EQUAL(-20, localAnalog.getIntValueIncludingOffset());
    TEST_ASSERT_EQUAL((uint16_t)0, localAnalog.getCurrentValue());
    TEST_ASSERT_TRUE(checkWholeFraction(&localAnalog, 20, 0, true));
    TEST_ASSERT_FLOAT_WITHIN(float(-20.0), localAnalog.getAsFloatingPointValue(), float(0.0001));

    localAnalog.setCurrentValue(255);
    TEST_ASSERT_TRUE(checkWholeFraction(&localAnalog, 235, 0));
    TEST_ASSERT_FLOAT_WITHIN(float(235.0), localAnalog.getAsFloatingPointValue(), float(0.0001));
    TEST_ASSERT_EQUAL(235, localAnalog.getIntValueIncludingOffset());
}

void testGetIntValueIncudingOffset() {
    AnalogMenuInfo myInfo = { "Analog12", 102, 0xffff, 200, nullptr, 100, 1, "xyz" };
    AnalogMenuItem localAnalog(&myInfo, 0, nullptr, INFO_LOCATION_RAM);
    TEST_ASSERT_EQUAL(100, localAnalog.getIntValueIncludingOffset());
    localAnalog.setCurrentValue(123);
    TEST_ASSERT_EQUAL(223, localAnalog.getIntValueIncludingOffset());
    localAnalog.setCurrentValue(123);
    TEST_ASSERT_EQUAL(223, localAnalog.getIntValueIncludingOffset());
    localAnalog.setCurrentValue(50);
    TEST_ASSERT_EQUAL(150, localAnalog.getIntValueIncludingOffset());
}

void testAnalogValuesWithFractions() {
    char sz[20];

    TEST_ASSERT_EQUAL(uint8_t(2), menuNumTwoDp.getDecimalPlacesForDivisor());
    menuNumTwoDp.setFromFloatingPointValue(98.234);
    TEST_ASSERT_FLOAT_WITHIN(float(98.23), menuNumTwoDp.getAsFloatingPointValue(), float(0.0001));
    TEST_ASSERT_EQUAL(uint16_t(9823), menuNumTwoDp.getCurrentValue());
    TEST_ASSERT_TRUE(checkWholeFraction(&menuNumTwoDp, 98, 23));
    menuNumTwoDp.copyValue(sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("98.23", sz);

    menuNumTwoDp.setFromWholeAndFraction(WholeAndFraction(22, 99, false));
    TEST_ASSERT_FLOAT_WITHIN(float(22.99), menuNumTwoDp.getAsFloatingPointValue(), float(0.0001));
    TEST_ASSERT_EQUAL(uint16_t(2299), menuNumTwoDp.getCurrentValue());
    TEST_ASSERT_TRUE(checkWholeFraction(&menuNumTwoDp, 22, 99));

    menuNumTwoDp.copyValue(sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("22.99", sz);

    TEST_ASSERT_EQUAL(uint8_t(1), menuHalvesOffs.getDecimalPlacesForDivisor());
    menuHalvesOffs.setCurrentValue(21);
    TEST_ASSERT_TRUE(checkWholeFraction(&menuHalvesOffs, 39, 5, true));
    TEST_ASSERT_FLOAT_WITHIN(float(-39.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));
    menuHalvesOffs.copyValue(sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("-39.5dB", sz);

    menuHalvesOffs.setCurrentValue(103);
    TEST_ASSERT_TRUE(checkWholeFraction(&menuHalvesOffs, 1, 5));
    TEST_ASSERT_FLOAT_WITHIN(float(1.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));

    menuHalvesOffs.setFromFloatingPointValue(50.5);
    TEST_ASSERT_TRUE(checkWholeFraction(&menuHalvesOffs, 50, 5));
    TEST_ASSERT_FLOAT_WITHIN(float(50.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));
    TEST_ASSERT_EQUAL(uint16_t(201), menuHalvesOffs.getCurrentValue());
    menuHalvesOffs.copyValue(sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("50.5dB", sz);

    menuHalvesOffs.setFromWholeAndFraction(WholeAndFraction(10, 5, false));
    TEST_ASSERT_EQUAL(uint16_t(121), menuHalvesOffs.getCurrentValue());
    TEST_ASSERT_TRUE(checkWholeFraction(&menuHalvesOffs, 10, 5));
    TEST_ASSERT_FLOAT_WITHIN(float(10.5), menuHalvesOffs.getAsFloatingPointValue(), float(0.0001));
    menuHalvesOffs.copyValue(sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("10.5dB", sz);

    menuHalvesOffs.setFromFloatingPointValue(-0.5);
    TEST_ASSERT_TRUE(checkWholeFraction(&menuHalvesOffs, 0, 5, true));
    menuHalvesOffs.copyValue(sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("-0.5dB", sz);
}

int calleeCount123 = 0;
int calleeCount321 = 0;
void onTest123(int id) {
    if(id == 123) calleeCount123++;
    if(id == 321) calleeCount321++;
}

void testAnalogValueItemInMemory() {
    AnalogMenuInfo analogInfo = { "Test 123", 123, 0xffff, 234, onTest123, -180, 10, "dB" };
    AnalogMenuItem analogItem(&analogInfo, 0, nullptr, INFO_LOCATION_RAM);

    char sz[20];
    analogItem.copyNameToBuffer(sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("Test 123", sz);

    analogItem.copyUnitToBuffer(sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("dB", sz);

    TEST_ASSERT_EQUAL((uint16_t)10, analogItem.getDivisor());
    TEST_ASSERT_EQUAL((uint8_t)1, analogItem.getDecimalPlacesForDivisor());
    TEST_ASSERT_EQUAL((uint16_t)10, analogItem.getActualDecimalDivisor());
    TEST_ASSERT_EQUAL((uint16_t)234, analogItem.getMaximumValue());
    TEST_ASSERT_EQUAL(-180, analogItem.getOffset());
    TEST_ASSERT_EQUAL((uint16_t)123, analogItem.getId());
    TEST_ASSERT_EQUAL((uint16_t)0xffff, analogItem.getEepromPosition());
    TEST_ASSERT_EQUAL(MENUTYPE_INT_VALUE, analogItem.getMenuType());

    analogItem.setCurrentValue(190);
    TEST_ASSERT_EQUAL(1, calleeCount123);
    TEST_ASSERT_TRUE(analogItem.isChanged());
    TEST_ASSERT_TRUE(analogItem.isSendRemoteNeeded(0));

    copyMenuItemNameAndValue(&analogItem, sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("Test 123: 1.0dB", sz);
}

void testBooleanItemInMemory() {
    BooleanMenuInfo boolInfo = { "Boolio", 321, 22, 1, onTest123, NAMING_ON_OFF};
    BooleanMenuItem boolItem(&boolInfo, false, nullptr, INFO_LOCATION_RAM);

    char sz[20];
    boolItem.copyNameToBuffer(sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("Boolio", sz);

    TEST_ASSERT_EQUAL((uint16_t)1, boolItem.getMaximumValue());
    TEST_ASSERT_EQUAL((uint16_t)321, boolItem.getId());
    TEST_ASSERT_EQUAL(NAMING_ON_OFF, boolItem.getBooleanNaming());
    TEST_ASSERT_EQUAL((uint16_t)22, boolItem.getEepromPosition());
    TEST_ASSERT_EQUAL(MENUTYPE_BOOLEAN_VALUE, boolItem.getMenuType());

    boolItem.setBoolean(true);
    TEST_ASSERT_EQUAL(1, calleeCount321);
    TEST_ASSERT_TRUE(boolItem.isChanged());
    TEST_ASSERT_TRUE(boolItem.isSendRemoteNeeded(0));

    copyMenuItemNameAndValue(&boolItem, sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("Boolio: ON", sz);
}

void testFloatItemInMemory() {
    FloatMenuInfo fltInfo = { "Floater", 122, 0xffff, 3, nullptr};
    FloatMenuItem fltItem(&fltInfo, nullptr, INFO_LOCATION_RAM);

    char sz[20];
    TEST_ASSERT_EQUAL(3, fltItem.getDecimalPlaces());
    TEST_ASSERT_EQUAL((uint16_t)122, fltItem.getId());
    TEST_ASSERT_EQUAL((uint16_t)0xffff, fltItem.getEepromPosition());
    TEST_ASSERT_EQUAL(MENUTYPE_FLOAT_VALUE, fltItem.getMenuType());

    fltItem.setFloatValue(223.2341);
    TEST_ASSERT_TRUE(fltItem.isChanged());
    TEST_ASSERT_TRUE(fltItem.isSendRemoteNeeded(0));

    copyMenuItemNameAndValue(&fltItem, sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("Floater: 223.234", sz);
}
