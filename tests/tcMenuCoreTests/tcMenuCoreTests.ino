#line 2 "tcMenuCoreTests.ino"

#include <AUnit.h>
#include <tcMenu.h>
#include "tcMenuFixtures.h"
#include <BaseRenderers.h>
#include "MockEepromAbstraction.h"
#include "MockIoAbstraction.h"

using namespace aunit;

MockedIoAbstraction mockIo;
NoRenderer noRenderer; 
MockEepromAbstraction eeprom;

void setup() {
    Serial.begin(115200);
    while(!Serial);
}

void loop() {
    TestRunner::run();
}

char szData[10] = { "123456789" };

test(saveAndLoadFromMenu) {
    // initialise the menu manager and switches with basic configuration
    switches.initialise(&mockIo, true);
    menuMgr.initForUpDownOk(&noRenderer, &textMenuItem1, 0, 1, 2);

    // now set up the eeprom ready to load.
    eeprom.write16(0, 0xfade);
    eeprom.write16(2, 100);
    eeprom.write16(4, 8);
    eeprom.write16(6, 2);
    eeprom.write8(8, 1);
    eeprom.writeArrayToRom(9, (uint8_t*)szData, 10);
    eeprom.write16(20, 50);
    menuMgr.load(eeprom);

    // now check the values we've loaded back from eeprom.
    assertEqual((int)menuAnalog.getCurrentValue(), 100);
    assertEqual((int)menuAnalog2.getCurrentValue(), 8);
    assertEqual((int)menuSubAnalog.getCurrentValue(), 50);
    assertEqual((int)menuEnum1.getCurrentValue(), 2);
    assertEqual((int)boolItem1.getBoolean(), 1);
    assertEqual(szData, textMenuItem1.getTextValue());

    // clear out the eeprom and then save the present structure.
    eeprom.reset();
    menuMgr.save(eeprom);

    // now compare back from eeprom what we saved.
    assertEqual(eeprom.read16(0), (uint16_t)0xfade);
    assertEqual(eeprom.read16(2), (uint16_t)100);
    assertEqual(eeprom.read16(20), (uint16_t)50);
    assertEqual(eeprom.read16(4), (uint16_t)8);
    assertEqual(eeprom.read16(6), (uint16_t)2);
    assertEqual(eeprom.read8(8), (uint8_t)1);
    
    char szCompare[10];
    eeprom.readIntoMemArray((uint8_t*)szCompare, 9, sizeof szCompare);
    assertEqual(szCompare, szData);

    // lastly make sure there were no errors in eeprom.
    assertFalse(eeprom.hasErrorOccurred());
}
