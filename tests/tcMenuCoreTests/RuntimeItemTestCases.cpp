
#include <testing/SimpleTest.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <RemoteAuthentication.h>
#include "fixtures_extern.h"
#include <tcUtil.h>

bool renderActivateCalled = false;

bool checkEditorHints(int start, int end, CurrentEditorRenderingHints::EditorRenderingType ty) {
    if(ty != menuMgr.getEditorHints().getEditorRenderingType()) {
        serlogF2(SER_DEBUG, "Edit hint type not matching was ", menuMgr.getEditorHints().getEditorRenderingType());
        return false;
    }

    int startIndex = menuMgr.getEditorHints().getStartIndex();
    int endIndex = menuMgr.getEditorHints().getEndIndex();
    if(start != startIndex || end != endIndex) {
        serlogF3(SER_DEBUG, "Editor hint start/end out, was ", startIndex, endIndex);
        return false;
    }
    return true;
}

int testBasicRuntimeFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    switch (mode) {
        case RENDERFN_NAME: {
            if (row < 10) {
                strcpy(buffer, "name");
                fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
            }
            else {
                strcpy(buffer, "hello");
            }
            break;
        }
        case RENDERFN_VALUE:
            ltoaClrBuff(buffer, row, row, NOT_PADDED, bufferSize);
            break;
        case RENDERFN_EEPROM_POS:
            return 44;
        case RENDERFN_INVOKE:
            renderActivateCalled = true;
            break;
        default: break;
    }
    return true;
}

test(testBasicRuntimeMenuItem) {
    RuntimeMenuItem item(MENUTYPE_RUNTIME_VALUE, 22, testBasicRuntimeFn, 222, 1, nullptr);

    assertEquals(item.getId(), uint16_t(22));
    assertEquals(item.getEepromPosition(), uint16_t(44));
    char sz[20];
    item.copyNameToBuffer(sz, sizeof(sz));
    assertStringEquals("hello", sz);
    item.copyValue(sz, sizeof(sz));
    assertStringEquals("222", sz);

    renderActivateCalled = false;
    item.runCallback();
    assertTrue(renderActivateCalled);
}

test(testListRuntimeItem) {
    ListRuntimeMenuItem item(22, 2, testBasicRuntimeFn, nullptr);

    // check the name and test on the "top level" or parent item
    char sz[20];

    // ensure there are two parts
    assertEquals(uint8_t(2), item.getNumberOfParts());

    RuntimeMenuItem* child = item.getChildItem(0);
    assertEquals(MENUTYPE_RUNTIME_LIST, child->getMenuType());
    child->copyNameToBuffer(sz, sizeof(sz));
    assertStringEquals("name0", sz);
    child->copyValue(sz, sizeof(sz));
    assertStringEquals("0", sz);

    child = item.getChildItem(1);
    assertEquals(MENUTYPE_RUNTIME_LIST, child->getMenuType());
    child->copyNameToBuffer(sz, sizeof(sz));
    assertStringEquals("name1", sz);
    child->copyValue(sz, sizeof(sz));
    assertStringEquals("1", sz);

    RuntimeMenuItem* back = item.asBackMenu();
    assertEquals(MENUTYPE_BACK_VALUE, back->getMenuType());

    RuntimeMenuItem* parent = item.asParent();
    assertEquals(MENUTYPE_RUNTIME_LIST, back->getMenuType());
    assertEquals(parent->getId(), uint16_t(22));
    assertEquals(parent->getEepromPosition(), uint16_t(44));
    parent->copyNameToBuffer(sz, sizeof(sz));
    assertStringEquals("hello", sz);
    item.copyValue(sz, sizeof(sz));
    assertStringEquals("255", sz);
}

void renderCallback(int id) {
    renderActivateCalled = true;
}

RENDERING_CALLBACK_NAME_INVOKE(textMenuItemTestCb, textItemRenderFn, "HelloWorld", 99, renderCallback)

test(testTextMenuItemFromEmpty) {
    TextMenuItem textItem(textMenuItemTestCb, 33, 10, nullptr);

    // first simulate eeprom loading back from storage.
    uint8_t* data = (uint8_t*)textItem.getTextValue();
    data[0] = 0;
    data[1] = 'Z';
    data[2] = 'Y';
    data[3] = 'X';
    data[4] = '[';
    data[5] = ']';
    textItem.cleanUpArray();

    // start off with an empty string
    char sz[20];
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("", sz);

    // ensure we can edit an empty string position
    assertEquals(uint8_t(10), textItem.beginMultiEdit());
    assertEquals(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    assertEquals(0, textItem.getPartValueAsInt());

    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("", sz);
    assertTrue(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    // add char to empty string
    textItem.valueChanged(findPositionInEditorSet('N'));
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("N", sz);
    assertTrue(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    // add another char to empty string
    assertEquals(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('E'));
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("NE", sz);
    assertTrue(checkEditorHints(1, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('T'));
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("NET", sz);
    assertTrue(checkEditorHints(2, 3, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('_'));
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("NET_", sz);
    assertTrue(checkEditorHints(3, 4, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());

    // check that the edit worked ok
    textItem.stopMultiEdit();
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("NET_", sz);

    // now start editing again and clear down the string to zero terminated at position 0
    assertEquals(uint8_t(10), textItem.beginMultiEdit());
    assertEquals(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(0);
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("", sz);
    assertTrue(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    // should be empty now
    textItem.stopMultiEdit();
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("", sz);

    // check every byte of buffer is 0.
    for (int i = 0; i < textItem.textLength(); i++) assertEquals(int(data[i]), 0);
}

test(testFindEditorSetFunction) {
    assertEquals(13, findPositionInEditorSet('9'));
    assertEquals(24, findPositionInEditorSet('K'));
    assertEquals(94, findPositionInEditorSet('~'));
    assertEquals(1, findPositionInEditorSet(' '));
    assertEquals(2, findPositionInEditorSet('.'));
    assertEquals(0, findPositionInEditorSet(0));
}

test(testTextPasswordItem) {
    // this menu item is fully tested in the main tests
    // here we concentrate on password functions
    TextMenuItem textItem(textMenuItemTestCb, 33, 5, nullptr);
    textItem.setPasswordField(true);
    textItem.setTextValue("1234");

    char sz[20];
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("****", sz);

    assertEquals(uint8_t(5), textItem.beginMultiEdit());
    assertEquals(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('9'));
    assertEquals(findPositionInEditorSet('9'), textItem.getPartValueAsInt());
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("9***", sz);
    assertTrue(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    assertStringEquals("9234", textItem.getTextValue());
}

test(testTextRuntimeItem) {
    TextMenuItem textItem(textMenuItemTestCb, 33, 10, nullptr);
    textItem.setTextValue("Goodbye");

    assertEquals(textItem.getId(), uint16_t(33));
    assertEquals(textItem.getEepromPosition(), uint16_t(99));

    // check the name and test on the "top level" or parent item
    char sz[20];
    textItem.copyNameToBuffer(sz, sizeof(sz));
    assertStringEquals("HelloWorld", sz);
    copyMenuItemValue(&textItem, sz, sizeof(sz));
    assertStringEquals("Goodbye", sz);

    assertEquals(uint8_t(10), textItem.beginMultiEdit());
    assertEquals(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    assertEquals(findPositionInEditorSet('G'), textItem.getPartValueAsInt());

    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("Goodbye", sz);
    assertTrue(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    textItem.valueChanged(findPositionInEditorSet('0'));
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("0oodbye", sz);
    assertTrue(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    assertEquals(findPositionInEditorSet('o'), textItem.getPartValueAsInt());
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("0oodbye", sz);
    assertTrue(checkEditorHints(1, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    assertEquals(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("0oodbye", sz);
    assertTrue(checkEditorHints(2, 3, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    textItem.valueChanged(findPositionInEditorSet('1'));
    assertEquals(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("0o1dbye", sz);
    assertTrue(checkEditorHints(3, 4, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    renderActivateCalled = false;
    textItem.stopMultiEdit();
    assertTrue(renderActivateCalled);
    textItem.copyValue(sz, sizeof(sz));
    assertStringEquals("0o1dbye", sz);
}

RENDERING_CALLBACK_NAME_INVOKE(rtSubMenuFn, backSubItemRenderFn, "My Sub", 0xffff, nullptr)
SubMenuItem rtSubMenu(101, rtSubMenuFn, &menuVolume, &menuContrast);

test(testSubMenuItem) {
    char sz[20];
    menuSub.copyNameToBuffer(sz, sizeof(sz));
    assertStringEquals("Settings", sz);
    assertTrue(&menuBackSub == menuSub.getChild());
    assertTrue(isMenuRuntime(&menuSub));
    assertTrue(menuSub.getMenuType() == MENUTYPE_SUB_VALUE);
    assertEquals((uint16_t)7, menuSub.getId());
    assertEquals((uint16_t)-1, menuSub.getEepromPosition());

    rtSubMenu.copyNameToBuffer(sz, sizeof(sz));
    assertStringEquals("My Sub", sz);
    assertTrue(&menuVolume == rtSubMenu.getChild());
    assertTrue(&menuContrast == rtSubMenu.getNext());
    assertTrue(isMenuRuntime(&rtSubMenu));
    assertTrue(rtSubMenu.getMenuType() == MENUTYPE_SUB_VALUE);
    assertEquals((uint16_t)101, rtSubMenu.getId());
    assertEquals((uint16_t)-1, rtSubMenu.getEepromPosition());
}

int actionCbCount = 0;
void myActionCb(int id) {
    actionCbCount++;
}

test(testActionMenuItem) {
    char sz[20];
    menuPressMe.copyNameToBuffer(sz, sizeof(sz));
    assertStringEquals("Press Me", sz);
    assertTrue(!isMenuRuntime(&menuPressMe));
    assertTrue(menuPressMe.getMenuType() == MENUTYPE_ACTION_VALUE);
    assertEquals((uint16_t)7, menuSub.getId());
    assertEquals((uint16_t)-1, menuSub.getEepromPosition());
    auto oldCbCount = actionCbCount;
    menuPressMe.triggerCallback();
    assertEquals(oldCbCount + 1, actionCbCount);

    copyMenuItemNameAndValue(&menuPressMe, sz, sizeof sz);
    assertStringEquals("Press Me: >>", sz);
}
