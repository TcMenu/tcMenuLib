#line 2 "tcMenuCoreTests.ino"

#include <AUnit.h>
#include <tcMenu.h>
#include "tcMenuFixtures.h"
#include <BaseRenderers.h>
#include <MockEepromAbstraction.h>
#include <MockIoAbstraction.h>

#include "menuManagerTests.h"
#include "baseRemoteTests.h"

using namespace aunit;

MockedIoAbstraction mockIo;
NoRenderer noRenderer; 
MockEepromAbstraction eeprom;
char szData[10] = { "123456789" };
const char PROGMEM pgmMyName[]  = "UnitTest";

void setup() {
    Serial.begin(115200);
    while(!Serial);
}

void loop() {
    TestRunner::run();
}


test(testTcUtilIntegerConversions) {
    char szBuffer[20];
    
    // first check the basic cases for the version that always starts at pos 0
    strcpy(szBuffer, "abc");
    ltoaClrBuff(szBuffer, 1234, 4, ' ', sizeof(szBuffer));
    assertEqual(szBuffer, "1234");
    ltoaClrBuff(szBuffer, 907, 4, ' ', sizeof(szBuffer));
    assertEqual(szBuffer, " 907");
    ltoaClrBuff(szBuffer, 22, 4, '0', sizeof(szBuffer));
    assertEqual(szBuffer, "0022");

    // and now test the appending version with zero padding
    strcpy(szBuffer, "val = ");
    fastltoa(szBuffer, 22, 4, '0', sizeof(szBuffer));
    assertEqual(szBuffer, "val = 0022");

    // and now test the appending version with an absolute divisor.
    strcpy(szBuffer, "val = ");
    fastltoa_mv(szBuffer, 22, 1000, '0', sizeof(szBuffer));
    assertEqual(szBuffer, "val = 022");

    // and lasty try the divisor version without 0.
    strcpy(szBuffer, "val = ");
    fastltoa_mv(szBuffer, 22, 10000, NOT_PADDED, sizeof(szBuffer));
    assertEqual(szBuffer, "val = 22");
}
