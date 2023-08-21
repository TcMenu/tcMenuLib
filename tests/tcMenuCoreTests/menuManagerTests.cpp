
#include <testing/SimpleTest.h>
#include <MockIoAbstraction.h>
#include <BaseRenderers.h>
#include <MockEepromAbstraction.h>
#include "fixtures_extern.h"
#include "TestCapturingRenderer.h"

extern MockedIoAbstraction mockIo;
extern NoRenderer noRenderer; 
extern MockEepromAbstraction eeprom;

const char szCompareData[] = "123456789";

test(saveAndLoadFromMenuSized) {
    switches.initialise(&mockIo, true);
    setSizeBasedEEPROMStorageEnabled(true);
    menuMgr.initForUpDownOk(&noRenderer, &textMenuItem1, 0, 1, 2);
    menuAnalog.setCurrentValue(0);
    menuAnalog2.setCurrentValue(0);
    menuEnum1.setCurrentValue(0);
    menuSubAnalog.setCurrentValue(0);
    menuSubAnalog.setCurrentValue(0);

    eeprom.write16(0, 0xfade);
    eeprom.write16(2, 8); // limit is location 8, shouldn't load past there.
    eeprom.write16(4, 8);
    eeprom.write16(6, 2);
    eeprom.write8(8, 1);
    eeprom.write16(20, 99);
    eeprom.write16(34, 99);
    menuMgr.load(eeprom);

    // now check the values we've loaded back from eeprom.
    assertEquals((int)menuAnalog2.getCurrentValue(), 8);
    assertEquals((int)menuEnum1.getCurrentValue(), 2);
    assertTrue(boolItem1.getBoolean());
    // these items exceed position 8 in the rom and wont load
    assertEquals((int)menuAnalog.getCurrentValue(), 0);
    assertEquals((int)menuSubAnalog.getCurrentValue(), 0);

    menuSubAnalog.setCurrentValue(42);

    // save and then make sure the header is right
    menuMgr.save(eeprom);
    assertEquals(uint16_t(0xfade), eeprom.read16(0));
    assertEquals(uint16_t(34), eeprom.read16(2));

    // and now the values
    assertEquals(eeprom.read16(20), (uint16_t)42);
    assertEquals(eeprom.read16(4), (uint16_t)8);
    assertEquals(eeprom.read16(6), (uint16_t)2);
    assertEquals(eeprom.read8(8), (uint8_t)1);
    switches.resetAllSwitches();
}

test(saveAndLoadFromMenuUnsized) {
    // initialise the menu manager and switches with basic configuration
    switches.initialise(&mockIo, true);
    setSizeBasedEEPROMStorageEnabled(false);
    menuMgr.initForUpDownOk(&noRenderer, &textMenuItem1, 0, 1, 2);

    // now set up the eeprom ready to load.
    eeprom.write16(0, 0xfade);
    eeprom.write16(34, 100);
    eeprom.write16(4, 8);
    eeprom.write16(6, 2);
    eeprom.write8(8, 1);
    eeprom.writeArrayToRom(9, (uint8_t*)szCompareData, 10);
    eeprom.write16(20, 50);
	// and the Ip address
	eeprom.write8(22, 192);
	eeprom.write8(23, 168);
	eeprom.write8(24, 90);
	eeprom.write8(25, 88);
	menuMgr.load(eeprom);

    // now check the values we've loaded back from eeprom.
    assertEquals((int)menuAnalog.getCurrentValue(), 100);
    assertEquals((int)menuAnalog2.getCurrentValue(), 8);
    assertEquals((int)menuSubAnalog.getCurrentValue(), 50);
    assertEquals((int)menuEnum1.getCurrentValue(), 2);
    assertEquals((int)boolItem1.getBoolean(), 1);
	assertEquals(uint8_t(10), textMenuItem1.textLength());
	assertEquals(szCompareData, textMenuItem1.getTextValue());
	
	char sz[20];
	menuIpAddr.copyValue(sz, sizeof(sz));
	assertEquals("192.168.90.88", sz);

    // clear out the eeprom and then save the present structure.
    eeprom.reset();
    menuMgr.save(eeprom);

    // now compare back from eeprom what we saved.
    assertEquals(eeprom.read16(0), (uint16_t)0xfade);
    assertEquals(eeprom.read16(34), (uint16_t)100);
    assertEquals(eeprom.read16(20), (uint16_t)50);
    assertEquals(eeprom.read16(4), (uint16_t)8);
    assertEquals(eeprom.read16(6), (uint16_t)2);
    assertEquals(eeprom.read8(8), (uint8_t)1);
    
    eeprom.readIntoMemArray((uint8_t*)sz, 9, 10);
    sz[9]=0;
    assertEquals(szCompareData, sz);

	assertEquals(eeprom.read8(22), (uint8_t)192);
	assertEquals(eeprom.read8(23), (uint8_t)168);
	assertEquals(eeprom.read8(24), (uint8_t)90);
	assertEquals(eeprom.read8(25), (uint8_t)88);

    // lastly make sure there were no errors in eeprom.
    assertFalse(eeprom.hasErrorOccurred());
    switches.resetAllSwitches();
}

class TestMenuMgrObserver : public MenuManagerObserver {
private:
    bool structureChangedCalled{};
    bool editStartedCalled{};
    bool editEndedCalled{};
    bool startReturnValue{};
    bool originalCommitCalled{};
public:
    TestMenuMgrObserver() = default;

    void setStartReturn(bool toReturn) { startReturnValue = toReturn; }

    void reset() {
        structureChangedCalled = editStartedCalled = editEndedCalled = originalCommitCalled = false;
    }

    void structureHasChanged() override {
        structureChangedCalled = true;
    }

    bool menuEditStarting(MenuItem *item) override {
        editStartedCalled = true;
        return startReturnValue;
    }

    void menuEditEnded(MenuItem *item) override {
        editEndedCalled = true;
    }

    void originalCommitWasCalled() {
        originalCommitCalled = true;
    }

    bool didTriggerStructure() const { return structureChangedCalled; }
    bool didTriggerStartEdit() const { return editStartedCalled; }
    bool didTriggerEndEdit() const { return editEndedCalled; }
    bool didOriginalCommitTrigger() const { return originalCommitCalled; }
} menuMgrObserver;

void originalCommitCb(int itemId) {
    menuMgrObserver.originalCommitWasCalled();
}

const PROGMEM AnyMenuInfo testActionInfo = { "ActTest", 2394, 0xffff,  0, NO_CALLBACK };
ActionMenuItem testActionItem(&testActionInfo, nullptr);

const PROGMEM AnyMenuInfo testActionInfo2 = { "ActTest", 2394, 0xffff,  0, NO_CALLBACK };
ActionMenuItem testActionItem2(&testActionInfo2, nullptr);

test(addingItemsAndMenuCallbacks) {
    menuMgr.resetObservers();
    TestCapturingRenderer testRenderer(320, 200, true, "hello");
    menuMgr.setRootMenu(&textMenuItem1);
    menuMgr.initWithoutInput(&testRenderer, &textMenuItem1);
    menuMgr.addChangeNotification(&menuMgrObserver);
    menuMgr.setItemCommittedHook(originalCommitCb);

    // first we add some menu items at the end of the menu and test the structure change call is made
    menuMgr.addMenuAfter(&menuNumTwoDp, &testActionItem, true);
    assertFalse(menuMgrObserver.didTriggerStructure());
    menuMgr.addMenuAfter(&menuNumTwoDp, &testActionItem2);
    assertTrue(menuMgrObserver.didTriggerStructure());

    assertEquals(&testActionItem2, menuNumTwoDp.getNext());
    assertEquals(&testActionItem, testActionItem2.getNext());
    assertTrue(testActionItem.getNext() == nullptr);

    // put back as it was before.
    menuNumTwoDp.setNext(nullptr);

    // now we test that when editing a bool, we get a commit callback.
    menuMgrObserver.reset();
    menuMgrObserver.setStartReturn(true);
    auto currentMenuValue = boolItem1.getBoolean();
    menuMgr.valueChanged(2); // boolItem1
    menuMgr.onMenuSelect(false);

    assertTrue(menuMgrObserver.didTriggerEndEdit());
    assertTrue(menuMgrObserver.didOriginalCommitTrigger());
    assertTrue(menuMgrObserver.didTriggerStartEdit());
    assertTrue(boolItem1.getBoolean() != currentMenuValue);

    // and now we test that when we return false in the start edit callback, we do not start editing.
    menuMgrObserver.reset();
    menuMgrObserver.setStartReturn(false);
    menuMgr.onMenuSelect(false);
    assertTrue(menuMgrObserver.didTriggerStartEdit());
    assertFalse(menuMgrObserver.didTriggerEndEdit());
    assertTrue(boolItem1.getBoolean() != currentMenuValue);

    // now we move on to test an enum (integer) item and ensure we get the edit callbacks.
    menuMgrObserver.reset();
    menuMgrObserver.setStartReturn(true);
    menuMgr.valueChanged(3); //  select menuEnum1
    menuMgr.onMenuSelect(false); // start edit
    assertTrue(menuMgrObserver.didTriggerStartEdit());
    assertTrue(&menuEnum1 == menuMgr.getCurrentEditor());

    menuMgr.valueChanged(1); // we are now editing, change the actual enum
    menuMgr.onMenuSelect(false); // stop editing
    assertTrue(menuMgrObserver.didTriggerEndEdit());
    assertTrue(menuMgrObserver.didOriginalCommitTrigger());
    assertEquals((uint16_t)1, menuEnum1.getCurrentValue());

    // lastly try an enum item that does not go into editing because the callback returned false.
    menuMgrObserver.reset();
    menuMgrObserver.setStartReturn(false);
    menuMgr.valueChanged(3); // menuEnum1
    menuMgr.onMenuSelect(false);
    assertTrue(menuMgrObserver.didTriggerStartEdit());
    assertTrue(&menuEnum1 == menuMgr.getCurrentEditor());
}

