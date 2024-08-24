
#include <unity.h>
#include <RuntimeMenuItem.h>
#include <RemoteMenuItem.h>
#include <RemoteAuthentication.h>
#include "../tutils/fixtures_extern.h"
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

void testBasicRuntimeMenuItem() {
    RuntimeMenuItem item(MENUTYPE_RUNTIME_VALUE, 22, testBasicRuntimeFn, 222, 1, nullptr);

    TEST_ASSERT_EQUAL(item.getId(), uint16_t(22));
    TEST_ASSERT_EQUAL(item.getEepromPosition(), uint16_t(44));
    char sz[20];
    item.copyNameToBuffer(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("hello", sz);
    item.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("222", sz);

    renderActivateCalled = false;
    item.runCallback();
    TEST_ASSERT_TRUE(renderActivateCalled);
}

void testListRuntimeItem() {
    ListRuntimeMenuItem item(22, 2, testBasicRuntimeFn, nullptr);

    // check the name and test on the "top level" or parent item
    char sz[20];

    // ensure there are two parts
    TEST_ASSERT_EQUAL(uint8_t(2), item.getNumberOfParts());

    RuntimeMenuItem* child = item.getChildItem(0);
    TEST_ASSERT_EQUAL(MENUTYPE_RUNTIME_LIST, child->getMenuType());
    child->copyNameToBuffer(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("name0", sz);
    child->copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("0", sz);

    child = item.getChildItem(1);
    TEST_ASSERT_EQUAL(MENUTYPE_RUNTIME_LIST, child->getMenuType());
    child->copyNameToBuffer(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("name1", sz);
    child->copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("1", sz);

    RuntimeMenuItem* back = item.asBackMenu();
    TEST_ASSERT_EQUAL(MENUTYPE_BACK_VALUE, back->getMenuType());

    RuntimeMenuItem* parent = item.asParent();
    TEST_ASSERT_EQUAL(MENUTYPE_RUNTIME_LIST, back->getMenuType());
    TEST_ASSERT_EQUAL(parent->getId(), uint16_t(22));
    TEST_ASSERT_EQUAL(parent->getEepromPosition(), uint16_t(44));
    parent->copyNameToBuffer(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("hello", sz);
    item.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("255", sz);
}

void renderCallback(int id) {
    renderActivateCalled = true;
}

RENDERING_CALLBACK_NAME_INVOKE(textMenuItemTestCb, textItemRenderFn, "HelloWorld", 99, renderCallback)

void testTextMenuItemFromEmpty() {
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
    TEST_ASSERT_EQUAL_STRING("", sz);

    // ensure we can edit an empty string position
    TEST_ASSERT_EQUAL(uint8_t(10), textItem.beginMultiEdit());
    TEST_ASSERT_EQUAL(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    TEST_ASSERT_EQUAL(0, textItem.getPartValueAsInt());

    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("", sz);
    TEST_ASSERT_TRUE(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    // add char to empty string
    textItem.valueChanged(findPositionInEditorSet('N'));
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("N", sz);
    TEST_ASSERT_TRUE(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    // add another char to empty string
    TEST_ASSERT_EQUAL(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('E'));
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("NE", sz);
    TEST_ASSERT_TRUE(checkEditorHints(1, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    TEST_ASSERT_EQUAL(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('T'));
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("NET", sz);
    TEST_ASSERT_TRUE(checkEditorHints(2, 3, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    TEST_ASSERT_EQUAL(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('_'));
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("NET_", sz);
    TEST_ASSERT_TRUE(checkEditorHints(3, 4, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    TEST_ASSERT_EQUAL(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());

    // check that the edit worked ok
    textItem.stopMultiEdit();
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("NET_", sz);

    // now start editing again and clear down the string to zero terminated at position 0
    TEST_ASSERT_EQUAL(uint8_t(10), textItem.beginMultiEdit());
    TEST_ASSERT_EQUAL(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(0);
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("", sz);
    TEST_ASSERT_TRUE(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    // should be empty now
    textItem.stopMultiEdit();
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("", sz);

    // check every byte of buffer is 0.
    for (int i = 0; i < textItem.textLength(); i++) TEST_ASSERT_EQUAL(int(data[i]), 0);
}

void testFindEditorSetFunction() {
    TEST_ASSERT_EQUAL(13, findPositionInEditorSet('9'));
    TEST_ASSERT_EQUAL(24, findPositionInEditorSet('K'));
    TEST_ASSERT_EQUAL(94, findPositionInEditorSet('~'));
    TEST_ASSERT_EQUAL(1, findPositionInEditorSet(' '));
    TEST_ASSERT_EQUAL(2, findPositionInEditorSet('.'));
    TEST_ASSERT_EQUAL(0, findPositionInEditorSet(0));
}

void testTextPasswordItem() {
    // this menu item is fully tested in the main tests
    // here we concentrate on password functions
    TextMenuItem textItem(textMenuItemTestCb, 33, 5, nullptr);
    textItem.setPasswordField(true);
    textItem.setTextValue("1234");

    char sz[20];
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("****", sz);

    TEST_ASSERT_EQUAL(uint8_t(5), textItem.beginMultiEdit());
    TEST_ASSERT_EQUAL(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.valueChanged(findPositionInEditorSet('9'));
    TEST_ASSERT_EQUAL(findPositionInEditorSet('9'), textItem.getPartValueAsInt());
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("9***", sz);
    TEST_ASSERT_TRUE(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));
    TEST_ASSERT_EQUAL_STRING("9234", textItem.getTextValue());
}

void testTextRuntimeItem() {
    TextMenuItem textItem(textMenuItemTestCb, 33, 10, nullptr);
    textItem.setTextValue("Goodbye");

    TEST_ASSERT_EQUAL(textItem.getId(), uint16_t(33));
    TEST_ASSERT_EQUAL(textItem.getEepromPosition(), uint16_t(99));

    // check the name and test on the "top level" or parent item
    char sz[20];
    textItem.copyNameToBuffer(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("HelloWorld", sz);
    copyMenuItemValue(&textItem, sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("Goodbye", sz);

    TEST_ASSERT_EQUAL(uint8_t(10), textItem.beginMultiEdit());
    TEST_ASSERT_EQUAL(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    TEST_ASSERT_EQUAL(findPositionInEditorSet('G'), textItem.getPartValueAsInt());

    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("Goodbye", sz);
    TEST_ASSERT_TRUE(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    textItem.valueChanged(findPositionInEditorSet('0'));
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("0oodbye", sz);
    TEST_ASSERT_TRUE(checkEditorHints(0, 1, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    TEST_ASSERT_EQUAL(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    TEST_ASSERT_EQUAL(findPositionInEditorSet('o'), textItem.getPartValueAsInt());
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("0oodbye", sz);
    TEST_ASSERT_TRUE(checkEditorHints(1, 2, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    TEST_ASSERT_EQUAL(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("0oodbye", sz);
    TEST_ASSERT_TRUE(checkEditorHints(2, 3, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    textItem.valueChanged(findPositionInEditorSet('1'));
    TEST_ASSERT_EQUAL(ALLOWABLE_CHARS_ENCODER_SIZE, textItem.nextPart());
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("0o1dbye", sz);
    TEST_ASSERT_TRUE(checkEditorHints(3, 4, CurrentEditorRenderingHints::EDITOR_RUNTIME_TEXT));

    renderActivateCalled = false;
    textItem.stopMultiEdit();
    TEST_ASSERT_TRUE(renderActivateCalled);
    textItem.copyValue(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("0o1dbye", sz);
}

RENDERING_CALLBACK_NAME_INVOKE(rtSubMenuFn, backSubItemRenderFn, "My Sub", 0xffff, nullptr)
SubMenuItem rtSubMenu(101, rtSubMenuFn, &menuVolume, &menuContrast);

void testSubMenuItem() {
    char sz[20];
    menuSub.copyNameToBuffer(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("Settings", sz);
    TEST_ASSERT_TRUE(&menuBackSub == menuSub.getChild());
    TEST_ASSERT_TRUE(isMenuRuntime(&menuSub));
    TEST_ASSERT_TRUE(menuSub.getMenuType() == MENUTYPE_SUB_VALUE);
    TEST_ASSERT_EQUAL((uint16_t)7, menuSub.getId());
    TEST_ASSERT_EQUAL((uint16_t)-1, menuSub.getEepromPosition());

    rtSubMenu.copyNameToBuffer(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("My Sub", sz);
    TEST_ASSERT_TRUE(&menuVolume == rtSubMenu.getChild());
    TEST_ASSERT_TRUE(&menuContrast == rtSubMenu.getNext());
    TEST_ASSERT_TRUE(isMenuRuntime(&rtSubMenu));
    TEST_ASSERT_TRUE(rtSubMenu.getMenuType() == MENUTYPE_SUB_VALUE);
    TEST_ASSERT_EQUAL((uint16_t)101, rtSubMenu.getId());
    TEST_ASSERT_EQUAL((uint16_t)-1, rtSubMenu.getEepromPosition());
}

int actionCbCount = 0;
void myActionCb(int id) {
    actionCbCount++;
}

void testActionMenuItem() {
    char sz[20];
    menuPressMe.copyNameToBuffer(sz, sizeof(sz));
    TEST_ASSERT_EQUAL_STRING("Press Me", sz);
    TEST_ASSERT_TRUE(!isMenuRuntime(&menuPressMe));
    TEST_ASSERT_TRUE(menuPressMe.getMenuType() == MENUTYPE_ACTION_VALUE);
    TEST_ASSERT_EQUAL((uint16_t)7, menuSub.getId());
    TEST_ASSERT_EQUAL((uint16_t)-1, menuSub.getEepromPosition());
    auto oldCbCount = actionCbCount;
    menuPressMe.triggerCallback();
    TEST_ASSERT_EQUAL(oldCbCount + 1, actionCbCount);

    copyMenuItemNameAndValue(&menuPressMe, sz, sizeof sz);
    TEST_ASSERT_EQUAL_STRING("Press Me: >>", sz);
}
