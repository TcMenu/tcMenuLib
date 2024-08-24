#include <unity.h>
#include <graphics/GfxMenuConfig.h>
#include <graphics/BaseGraphicalRenderer.h>
#include "../tutils/fixtures_extern.h"
#include "../tutils/TestCapturingRenderer.h"

using namespace tcgfx;

color_t palette1[] = {RGB(0,0,0),RGB(255,255,255),RGB(1,2,3),RGB(3,2,1)};
color_t palette2[] = {RGB(55,55,55),RGB(66,66,66),RGB(77,77,77),RGB(0,0,0)};
color_t palette3[] = {RGB(0,0,0),RGB(22,22,22),RGB(33,44,55),RGB(6,5,4)};

const uint8_t pointer1[] = { 0x01, 0x02, 0x03, 0x04 };
const uint8_t pointer2[] = { 0x01, 0x02, 0x03, 0x04 };
const uint8_t pointer3[] = { 0x01, 0x02, 0x03, 0x04 };

bool checkPadding(MenuPadding padding, uint16_t top, uint16_t right, uint16_t bottom, uint16_t left) {
    if (padding.top != top) {
        serdebugF3("Padding top mismatch: ", top, padding.top);
        return false;
    } else if (padding.right != right) {
        serdebugF3("Padding right mismatch: ", right, padding.right);
        return false;
    } else if (padding.left != left) {
        serdebugF3("Padding left mismatch: ", left, padding.left);
        return false;
    } else if (padding.bottom != bottom) {
        serdebugF3("Padding bottom mismatch: ", bottom, padding.bottom);
        return false;
    }
    return true;
}

bool checkPropertiesBasics(ItemDisplayProperties* props, const char* name, const void* expectedFont, uint8_t mag,
                           color_t colorBg, color_t colorFg, uint8_t spacing, uint8_t height, GridPosition::GridJustification just) {
    serdebugF2("Checking properties ", name);
    if(props == nullptr) {
        serdebugF("Props null");
        return false;
    }
    if(expectedFont != props->getFont() || mag != props->getFontMagnification()) {
        serdebugF3("Font or mag mismatch ", mag, props->getFontMagnification());
        return false;
    }
    if(colorBg != props->getColor(ItemDisplayProperties::BACKGROUND)) {
        serdebugFHex2("Mismatch on BG ", colorBg, props->getColor(ItemDisplayProperties::BACKGROUND));
        return false;
    }
    if(colorFg != props->getColor(ItemDisplayProperties::TEXT)) {
        serdebugFHex2("Mismatch on FG ", colorBg, props->getColor(ItemDisplayProperties::BACKGROUND));
        return false;
    }
    if(spacing != props->getSpaceAfter()) {
        serdebugF3("Spacing out ", spacing, props->getSpaceAfter());
        return false;
    }
    if(height != props->getRequiredHeight()) {
        serdebugF3("Height out ", height, props->getRequiredHeight());
        return false;
    }
    if(just != props->getDefaultJustification()) {
        serdebugF3("Justification out ", just, props->getDefaultJustification());
    }
    return true;
}

void testEmptyItemPropertiesFactory() {
    taskManager.reset();

    TEST_ASSERT_EQUAL(4, (int)sizeof(GridPosition));
    TEST_ASSERT_EQUAL(4, (int)sizeof(Coord));
    TEST_ASSERT_EQUAL(2, (int)sizeof(MenuPadding));
    TEST_ASSERT_EQUAL(1, (int)sizeof(MenuBorder));

    ConfigurableItemDisplayPropertiesFactory factory;
    auto *config = factory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);
    TEST_ASSERT_TRUE(checkPropertiesBasics(config, "empty not null", nullptr, 1, RGB(0, 0, 0), RGB(255, 255, 255), 2, 12, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT));
    TEST_ASSERT_TRUE(checkPadding(config->getPadding(), 2, 2, 2, 2));
}

void populatePropsWithDefaults(ConfigurableItemDisplayPropertiesFactory& factory) {
    factory.setSelectedColors(RGB(1, 2, 3), RGB(3, 2, 1));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, palette1, MenuPadding(4, 3, 2, 1), pointer1, 4, 22, 30, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, palette2, MenuPadding(2), pointer2, 1, 4, 40, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, palette2, MenuPadding(1), pointer3, 2, 2, 50, GridPosition::JUSTIFY_LEFT_NO_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesAllInSub(ItemDisplayProperties::COMPTYPE_ITEM, menuSub.getId(), palette3, MenuPadding(3), pointer1, 3, 10, 60, GridPosition::JUSTIFY_CENTER_NO_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesForItem(ItemDisplayProperties::COMPTYPE_ITEM, menuSubAnalog.getId(), palette1, MenuPadding(6), pointer2, 3, 12, 80, GridPosition::JUSTIFY_CENTER_WITH_VALUE, MenuBorder(0));
    factory.addGridPosition(&menuVolume, GridPosition(GridPosition::DRAW_INTEGER_AS_UP_DOWN, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 2, 100));
    menuMgr.getNavigationStore().clearNavigationListeners();
    menuMgr.initWithoutInput(&noRenderer, &textMenuItem1);
    taskManager.reset();
}

void testDefaultItemPropertiesFactory() {
    ConfigurableItemDisplayPropertiesFactory factory;
    populatePropsWithDefaults(factory);

    // check that when we request with null item we get the default
    auto* config = factory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM);
    TEST_ASSERT_TRUE(checkPropertiesBasics(config, "default item", pointer2, 1, palette2[1], palette2[0], 4, 40, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT));
    TEST_ASSERT_TRUE(checkPadding(config->getPadding(), 2,2,2,2));

    // check that when we request with null title we get the default
    config = factory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_TITLE);
    TEST_ASSERT_TRUE(checkPropertiesBasics(config, "default title", pointer1, 4, palette1[1], palette1[0], 22, 30, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT));
    TEST_ASSERT_TRUE(checkPadding(config->getPadding(), 4,3,2,1));

    // check that when we request with null action we get the default
    config = factory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ACTION);
    TEST_ASSERT_TRUE(checkPropertiesBasics(config, "default action", pointer3, 2, palette2[1], palette2[0], 2, 50, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT));
    TEST_ASSERT_TRUE(checkPadding(config->getPadding(), 1,1,1,1));

    // now change the default item and ensure that it picks up the new settings.
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, palette1, MenuPadding(2,3,4,5), pointer1, 2, 14, 15, GridPosition::JUSTIFY_RIGHT_NO_VALUE, MenuBorder(0));
    config = factory.configFor(nullptr, ItemDisplayProperties::COMPTYPE_ITEM);
    TEST_ASSERT_TRUE(checkPropertiesBasics(config, "default changed item", pointer1, 2, palette1[1], palette1[0], 14, 15, GridPosition::JUSTIFY_RIGHT_NO_VALUE));
    TEST_ASSERT_TRUE(checkPadding(config->getPadding(), 2,3,4,5));
}

void testSubAndItemSelectionPropertiesFactory() {
    ConfigurableItemDisplayPropertiesFactory factory;
    populatePropsWithDefaults(factory);

    menuMgr.navigateToMenu(menuSub.getChild());

    // using the submenu level settings because it is in menuSub, with no item level override.
    auto* config = factory.configFor(&menuIpAddr, ItemDisplayProperties::COMPTYPE_ITEM);
    TEST_ASSERT_TRUE(checkPropertiesBasics(config, "override sub", pointer1, 3, palette3[1], palette3[0], 10, 60, GridPosition::JUSTIFY_CENTER_NO_VALUE));
    TEST_ASSERT_TRUE(checkPadding(config->getPadding(), 3,3,3,3));

    // using the item level settings because it has an override at item and sub, item takes priority.
    config = factory.configFor(&menuSubAnalog, ItemDisplayProperties::COMPTYPE_ITEM);
    TEST_ASSERT_TRUE(checkPropertiesBasics(config, "override item", pointer2, 3, palette1[1], palette1[0], 12, 80, GridPosition::JUSTIFY_CENTER_WITH_VALUE));
    TEST_ASSERT_TRUE(checkPadding(config->getPadding(), 6,6,6,6));
}

void testIconStorageAndRetrival() {
    ConfigurableItemDisplayPropertiesFactory factory;
    factory.addImageToCache(DrawableIcon(menuVolume.getId(), Coord(1, 3), DrawableIcon::ICON_XBITMAP, pointer1, pointer2));
    factory.addImageToCache(DrawableIcon(menuSub.getId(), Coord(2, 1), DrawableIcon::ICON_NATIVE, pointer3));

    TEST_ASSERT_TRUE(nullptr == factory.iconForMenuItem(menuPressMe.getId()));

    auto* icon = factory.iconForMenuItem(menuVolume.getId());
    TEST_ASSERT_TRUE(icon != nullptr);
    TEST_ASSERT_EQUAL(icon->getIconType(), DrawableIcon::ICON_XBITMAP);
    int iconX = (int)(icon->getDimensions().x);
    int iconY = (int)(icon->getDimensions().y);
    TEST_ASSERT_EQUAL(1, iconX);
    TEST_ASSERT_EQUAL(3, iconY);
    TEST_ASSERT_EQUAL(pointer1, icon->getIcon(false));
    TEST_ASSERT_EQUAL(pointer2, icon->getIcon(true));

    icon = factory.iconForMenuItem(menuSub.getId());
    TEST_ASSERT_TRUE(icon != nullptr);
    TEST_ASSERT_EQUAL(icon->getIconType(), DrawableIcon::ICON_NATIVE);
    iconX = (int)(icon->getDimensions().x);
    iconY = (int)(icon->getDimensions().y);
    TEST_ASSERT_EQUAL(2, iconX);
    TEST_ASSERT_EQUAL(1, iconY);
    TEST_ASSERT_EQUAL(pointer3, icon->getIcon(false));
    TEST_ASSERT_EQUAL(pointer3, icon->getIcon(true));
}

void testGridPositionStorageAndRetrival() {
    ConfigurableItemDisplayPropertiesFactory factory;
    factory.addGridPosition(&menuVolume, GridPosition(GridPosition::DRAW_INTEGER_AS_UP_DOWN, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 2, 10));
    factory.addGridPosition(&menuStatus, GridPosition(GridPosition::DRAW_AS_ICON_ONLY, GridPosition::JUSTIFY_CENTER_NO_VALUE, 3, 2, 4, 0));

    TEST_ASSERT_TRUE(nullptr == factory.gridPositionForItem(&menuSub));

    auto* grid = factory.gridPositionForItem(&menuVolume);
    TEST_ASSERT_EQUAL(grid->getPosition().getDrawingMode(), GridPosition::DRAW_INTEGER_AS_UP_DOWN);
    TEST_ASSERT_EQUAL(grid->getPosition().getRow(), 2);
    TEST_ASSERT_EQUAL(grid->getPosition().getGridPosition(), 1);
    TEST_ASSERT_EQUAL(grid->getPosition().getGridSize(), 1);
    TEST_ASSERT_EQUAL(grid->getPosition().getGridHeight(), 10);
    TEST_ASSERT_EQUAL(grid->getPosition().getJustification(), GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT);

    grid = factory.gridPositionForItem(&menuStatus);
    TEST_ASSERT_EQUAL(grid->getPosition().getDrawingMode(), GridPosition::DRAW_AS_ICON_ONLY);
    TEST_ASSERT_EQUAL(grid->getPosition().getRow(), 4);
    TEST_ASSERT_EQUAL(grid->getPosition().getGridPosition(), 2);
    TEST_ASSERT_EQUAL(grid->getPosition().getGridSize(), 3);
    TEST_ASSERT_EQUAL(grid->getPosition().getGridHeight(), 0);
    TEST_ASSERT_EQUAL(grid->getPosition().getJustification(), GridPosition::JUSTIFY_CENTER_NO_VALUE);
}

const uint8_t* const icons1[] PROGMEM {pointer1, pointer2};
const uint8_t* const icons2[] PROGMEM {pointer1, pointer2, pointer3};

TitleWidget widget2 = TitleWidget(icons2, 3, 8, 4, nullptr);
TitleWidget widget1 = TitleWidget(icons1, 2, 8, 3, &widget2);
const char pgmName[] PROGMEM = "Test";

void testWidgetFunctionality() {
    TEST_ASSERT_EQUAL((uint8_t)8, widget1.getWidth());
    TEST_ASSERT_EQUAL((uint8_t)3, widget1.getHeight());
    TEST_ASSERT_EQUAL((uint8_t)2, widget1.getMaxValue());
    TEST_ASSERT_EQUAL((uint8_t)8, widget2.getWidth());
    TEST_ASSERT_EQUAL((uint8_t)4, widget2.getHeight());
    TEST_ASSERT_EQUAL((uint8_t)3, widget2.getMaxValue());

    widget1.setCurrentState(0);
    TEST_ASSERT_EQUAL((uint8_t)0, widget1.getCurrentState());
    TEST_ASSERT_EQUAL(pointer1, widget1.getCurrentIcon());
    widget1.setCurrentState(19);
    TEST_ASSERT_EQUAL((uint8_t)0, widget1.getCurrentState());
    widget1.setCurrentState(1);
    TEST_ASSERT_EQUAL((uint8_t)1, widget1.getCurrentState());
    TEST_ASSERT_EQUAL(pointer2, widget1.getCurrentIcon());

    widget2.setCurrentState(2);
    TEST_ASSERT_EQUAL((uint8_t)2, widget2.getCurrentState());
    TEST_ASSERT_EQUAL(pointer3, widget2.getCurrentIcon());
    TEST_ASSERT_EQUAL(pointer3, widget2.getIcon(2));
    TEST_ASSERT_EQUAL(pointer1, widget2.getIcon(20));
}

bool checkWidget(int widNum, WidgetDrawingRecord* item, Coord where, color_t* palette, int updatesExpected) {
    serdebugF2("check widget", widNum);
    if(item == nullptr) {
        serdebugF("widget not found");
        return false;
    }
    if(where.x != item->where.x || where.y != item->where.y) {
        serdebugF3("widget pos wrong ", item->where.x, item->where.y);
        return false;
    }
    if(item->bg != palette[1] || item->fg != palette[2] || item->updated != updatesExpected) {
        serdebugF4("clr/update wrong", item->fg, item->bg, item->updated);
        return false;
    }
    return true;
}

void checkItem(const char* itemNm, MenuDrawingRecord* record, Coord where, Coord size, const void* font, GridPosition::GridDrawingMode mode, GridPosition::GridJustification justification, int expectedUpdates, MenuItem* pItem = nullptr) {
    TEST_ASSERT_NOT_NULL(record);
    printf("check item %s, %d, %d\n", itemNm, record->position.getRow(), record->position.getGridPosition());

    TEST_ASSERT_EQUAL(where.x, record->where.x);
    TEST_ASSERT_EQUAL(where.y, record->where.y);
    TEST_ASSERT_EQUAL(size.x, record->size.x);
    TEST_ASSERT_EQUAL(size.y, record->size.y);

    TEST_ASSERT_EQUAL_PTR(font, record->properties->getFont());
    TEST_ASSERT_EQUAL(mode, record->position.getDrawingMode());
    TEST_ASSERT_EQUAL(justification, record->position.getJustification());

    TEST_ASSERT_TRUE(pItem == nullptr || pItem == record->theItem);

    TEST_ASSERT_TRUE(expectedUpdates == record->updated);
}

void testBaseRendererWithDefaults() {

    TestCapturingRenderer renderer(320, 120, false, pgmName);

    renderer.setFirstWidget(&widget1);
    auto& factory = reinterpret_cast<ConfigurableItemDisplayPropertiesFactory &>(renderer.getDisplayPropertiesFactory());
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, palette1, MenuPadding(4), pointer2, 1, 10, 30, GridPosition::JUSTIFY_CENTER_NO_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ACTION, palette1, MenuPadding(4), pointer1, 1, 5, 25, GridPosition::JUSTIFY_LEFT_NO_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, palette1, MenuPadding(4), pointer1, 1, 5, 20, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder(0));

    menuMgr.getNavigationStore().clearNavigationListeners();
    menuMgr.initWithoutInput(&renderer, &textMenuItem1);
    taskManager.reset();

    widget1.setCurrentState(0);
    menuEnum1.setCurrentValue(0);
    renderer.resetCommandStates();
    renderer.exec();

    TEST_ASSERT_TRUE(renderer.checkCommands(true, true, true));

    TEST_ASSERT_EQUAL((bsize_t)2, renderer.getWidgetRecordings().count());
    TEST_ASSERT_TRUE(checkWidget(1, renderer.getWidgetRecordings().getByKey((unsigned long)&widget1), Coord(320 - 8 - 4, 4), palette1, 0));
    TEST_ASSERT_TRUE(checkWidget(2, renderer.getWidgetRecordings().getByKey((unsigned long)&widget2), Coord(320 - 16 - 8, 4), palette1, 0));

    TEST_ASSERT_EQUAL((bsize_t)2, renderer.getWidgetRecordings().count());
    checkItem("BaseTitle", renderer.getMenuItemRecordings().getByKey(0), Coord(0, 0), Coord(320, 30), pointer2, GridPosition::DRAW_TITLE_ITEM, GridPosition::JUSTIFY_CENTER_NO_VALUE, 0);
    checkItem("Base R1", renderer.getMenuItemRecordings().getByKey(1), Coord(0, 40), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &textMenuItem1);
    checkItem("Base R2", renderer.getMenuItemRecordings().getByKey(2), Coord(0, 65), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &boolItem1);
    checkItem("Base R3", renderer.getMenuItemRecordings().getByKey(3), Coord(0, 90), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_UP_DOWN, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &menuEnum1);
    checkItem("Base R4", renderer.getMenuItemRecordings().getByKey(4), Coord(0, 115), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_SCROLL, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &menuAnalog2);
    TEST_ASSERT_FALSE(textMenuItem1.isChanged());
    TEST_ASSERT_FALSE(boolItem1.isChanged());
    TEST_ASSERT_FALSE(menuEnum1.isChanged());
    TEST_ASSERT_FALSE(menuAnalog2.isChanged());

    widget1.setCurrentState(1);
    menuEnum1.setCurrentValue(1);
    renderer.resetCommandStates();
    renderer.exec();

    TEST_ASSERT_TRUE(renderer.checkCommands(false, true, true));
    TEST_ASSERT_TRUE(checkWidget(1, renderer.getWidgetRecordings().getByKey((unsigned long)&widget1), Coord(320 - 8 - 4, 4), palette1, 1));
    TEST_ASSERT_TRUE(checkWidget(2, renderer.getWidgetRecordings().getByKey((unsigned long)&widget2), Coord(320 - 16 - 8, 4), palette1, 0));
    checkItem("I2.BaseTitle", renderer.getMenuItemRecordings().getByKey(1), Coord(0, 40), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &textMenuItem1);
    checkItem("I2.R1", renderer.getMenuItemRecordings().getByKey(0), Coord(0, 0), Coord(320, 30), pointer2, GridPosition::DRAW_TITLE_ITEM, GridPosition::JUSTIFY_CENTER_NO_VALUE, 0);
    checkItem("I2.R2", renderer.getMenuItemRecordings().getByKey(2), Coord(0, 65), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &boolItem1);
    checkItem("I2.R3", renderer.getMenuItemRecordings().getByKey(3), Coord(0, 90), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_UP_DOWN, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 1, &menuEnum1);
    checkItem("I2.R4", renderer.getMenuItemRecordings().getByKey(4), Coord(0, 115), Coord(320, 20), pointer1, GridPosition::DRAW_INTEGER_AS_SCROLL, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &menuAnalog2);
    TEST_ASSERT_FALSE(menuEnum1.isChanged());
}

class DisplayDrawing : public CustomDrawing {
private:
    bool hasStarted = false;
    bool resetCalled = false;
    int ticks = 0;
public:
    void started(BaseMenuRenderer *currentRenderer) override {
        hasStarted = true;
    }

    void reset() override {
        resetCalled = true;
    }

    void renderLoop(unsigned int currentValue, RenderPressMode userClick) override {
        ticks++;
    }

    bool didStart() const { return hasStarted; }
    bool didReset() const { return resetCalled; }
    int getTicks() const { return ticks; }

    void clearFlags() {
        hasStarted = false;
        resetCalled = false;
    }
};

void testTakeOverDisplay() {
    TestCapturingRenderer renderer(320, 100, false, pgmName);
    menuMgr.getNavigationStore().clearNavigationListeners();
    menuMgr.initWithoutInput(&renderer, &textMenuItem1);
    DisplayDrawing drawingTest;
    renderer.setCustomDrawingHandler(&drawingTest);
    for(int i=0; i<400 ;i++) {
        renderer.exec();
    }
    TEST_ASSERT_TRUE(drawingTest.didReset());
    TEST_ASSERT_FALSE(drawingTest.didStart());

    renderer.takeOverDisplay();
    renderer.exec();
    TEST_ASSERT_TRUE(drawingTest.didStart());
    TEST_ASSERT_EQUAL(0, drawingTest.getTicks());
    drawingTest.clearFlags();
    for(int i=0;i<500;i++) renderer.exec();
    TEST_ASSERT_EQUAL(500, drawingTest.getTicks());
    TEST_ASSERT_FALSE(drawingTest.didReset());

    renderer.giveBackDisplay();
    for(int i=0;i<500;i++) renderer.exec();
    TEST_ASSERT_TRUE(drawingTest.didReset());

    TEST_ASSERT_EQUAL(500, drawingTest.getTicks());
}

extern int testBasicRuntimeFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

void testListRendering() {
    ListRuntimeMenuItem runtimeItem(101, 20, testBasicRuntimeFn, nullptr);
    TestCapturingRenderer renderer(320, 100, false, pgmName);
    renderer.setTitleMode(tcgfx::BaseGraphicalRenderer::TITLE_ALWAYS);
    DisplayDrawing drawingTest;
    menuMgr.getNavigationStore().clearNavigationListeners();
    menuMgr.initWithoutInput(&renderer, &runtimeItem);
    auto& factory = reinterpret_cast<ConfigurableItemDisplayPropertiesFactory &>(renderer.getDisplayPropertiesFactory());
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_TITLE, palette1, MenuPadding(4), pointer2, 1, 10, 30, GridPosition::JUSTIFY_CENTER_NO_VALUE, MenuBorder(0));
    factory.setDrawingPropertiesDefault(ItemDisplayProperties::COMPTYPE_ITEM, palette1, MenuPadding(4), pointer1, 1, 5, 20, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, MenuBorder(0));
    taskManager.reset();

    renderer.resetCommandStates();
    renderer.exec();
    TEST_ASSERT_EQUAL((bsize_t)4, renderer.getMenuItemRecordings().count());
    checkItem("List.R0", renderer.getMenuItemRecordings().getByKey(0), Coord(0, 0), Coord(320, 30), pointer2, GridPosition::DRAW_TITLE_ITEM, GridPosition::JUSTIFY_CENTER_NO_VALUE, 0, &runtimeItem);
    checkItem("List.R1", renderer.getMenuItemRecordings().getByKey(1), Coord(0, 40), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &runtimeItem);
    checkItem("List.R2", renderer.getMenuItemRecordings().getByKey(2), Coord(0, 65), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &runtimeItem);
    checkItem("List.R3", renderer.getMenuItemRecordings().getByKey(3), Coord(0, 90), Coord(320, 20), pointer1, GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_VALUE_RIGHT, 0, &runtimeItem);
}
