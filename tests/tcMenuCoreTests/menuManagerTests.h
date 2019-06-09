#ifndef MENU_MANAGER_TEST_H
#define MENU_MANAGER_TEST_H

extern MockedIoAbstraction mockIo;
extern NoRenderer noRenderer; 
extern MockEepromAbstraction eeprom;

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
	char sz[10];
	strcpy(sz, "123456789");
    eeprom.writeArrayToRom(9, (uint8_t*)sz, 10);
    eeprom.write16(20, 50);
    menuMgr.load(eeprom);

    // now check the values we've loaded back from eeprom.
    assertEqual((int)menuAnalog.getCurrentValue(), 100);
    assertEqual((int)menuAnalog2.getCurrentValue(), 8);
    assertEqual((int)menuSubAnalog.getCurrentValue(), 50);
    assertEqual((int)menuEnum1.getCurrentValue(), 2);
    assertEqual((int)boolItem1.getBoolean(), 1);
	assertEqual(uint8_t(10), textMenuItem1.textLength());
	assertEqual(sz, textMenuItem1.getTextValue());

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
    szCompare[9]=0;
    assertEqual(szCompare, sz);

    // lastly make sure there were no errors in eeprom.
    assertFalse(eeprom.hasErrorOccurred());
}

#endif
