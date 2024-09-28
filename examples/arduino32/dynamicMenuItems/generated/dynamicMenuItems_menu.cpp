/*
    The code in this file uses open source libraries provided by thecoderscorner

    DO NOT EDIT THIS FILE, IT WILL BE GENERATED EVERY TIME YOU USE THE UI DESIGNER
    INSTEAD EITHER PUT CODE IN YOUR SKETCH OR CREATE ANOTHER SOURCE FILE.

    All the variables you may need access to are marked extern in this file for easy
    use elsewhere.
 */

// Generated for Arduino 32bit ARM by TcMenu 4.3.1 on 2024-09-28T08:15:53.193774100Z.

#include <tcMenu.h>
#include "dynamicMenuItems_menu.h"
#include "../ThemeCoolBlueTraditionalBuilder.h"
#include <Fonts/OpenSansRegular8pt.h>
#include <Fonts/OpenSansCyrillicLatin12.h>

// Global variable declarations
const  ConnectorLocalInfo applicationInfo = { "Dynamic Menus", "5f22995e-8da2-49c4-9ec8-d055901003af" };
IoAbstractionRef ioexp_io23017 = ioFrom23017(0x20, ACTIVE_LOW_OPEN, 10);
Adafruit_ST7735 gfx(1, 0, -1);
AdafruitDrawable gfxDrawable(&gfx, 40);
GraphicsDeviceRenderer renderer(30, applicationInfo.name, &gfxDrawable);
MatrixKeyboardManager keyboard;
const char keyboardKeys[]  = "123A456B789C*0#D";
KeyboardLayout keyboardLayout(4, 4, keyboardKeys);
MenuEditingKeyListener tcMenuKeyListener('*', '#', 'A', 'B');

// Global Menu Item declarations
RENDERING_CALLBACK_NAME_OVERRIDDEN(fnRuntimesOctalRtCall, octalOnlyRtCall, "Octal", -1)
TextMenuItem menuRuntimesOctal(fnRuntimesOctalRtCall, "", 22, 5, nullptr);
const AnyMenuInfo minfoIpItem = { "IpItem", 21, 0xffff, 0, NO_CALLBACK };
IpAddressMenuItem menuIpItem(&minfoIpItem, IpAddressStorage(127, 0, 0, 1), &menuRuntimesOctal, INFO_LOCATION_PGM);
const AnyMenuInfo minfoTextItem = { "TextItem", 20, 0xffff, 0, NO_CALLBACK };
TextMenuItem menuTextItem(&minfoTextItem, "", 5, &menuIpItem, INFO_LOCATION_PGM);
const SubMenuInfo minfoRuntimes = { "Runtimes", 19, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackRuntimes(&minfoRuntimes, &menuTextItem, INFO_LOCATION_PGM);
SubMenuItem menuRuntimes(&minfoRuntimes, &menuBackRuntimes, nullptr, INFO_LOCATION_PGM);
const AnyMenuInfo minfoList = { "List", 13, 0xffff, 0, NO_CALLBACK };
ListRuntimeMenuItem menuList(&minfoList, 116, fnListRtCall, &menuRuntimes, INFO_LOCATION_PGM);
const BooleanMenuInfo minfoDialogsBlockedBool = { "Blocked Bool", 18, 0xffff, 1, NO_CALLBACK, NAMING_TRUE_FALSE };
BooleanMenuItem menuDialogsBlockedBool(&minfoDialogsBlockedBool, false, nullptr, INFO_LOCATION_PGM);
const AnyMenuInfo minfoDialogsBlockedAction = { "Blocked Action", 17, 0xffff, 0, NO_CALLBACK };
ActionMenuItem menuDialogsBlockedAction(&minfoDialogsBlockedAction, &menuDialogsBlockedBool, INFO_LOCATION_PGM);
const SubMenuInfo minfoDialogsBlockedSub = { "Blocked Sub", 15, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackDialogsBlockedSub(&minfoDialogsBlockedSub, nullptr, INFO_LOCATION_PGM);
SubMenuItem menuDialogsBlockedSub(&minfoDialogsBlockedSub, &menuBackDialogsBlockedSub, &menuDialogsBlockedAction, INFO_LOCATION_PGM);
const BooleanMenuInfo minfoDialogsDialogBack = { "Allow Observer", 14, 0xffff, 1, onDialogBack, NAMING_ON_OFF };
BooleanMenuItem menuDialogsDialogBack(&minfoDialogsDialogBack, false, &menuDialogsBlockedSub, INFO_LOCATION_PGM);
const AnyMenuInfo minfoDialogsController = { "Controller", 12, 0xffff, 0, onDialogController };
ActionMenuItem menuDialogsController(&minfoDialogsController, &menuDialogsDialogBack, INFO_LOCATION_PGM);
const AnyMenuInfo minfoDialogsInformation = { "Information", 11, 0xffff, 0, onDialogInfo };
ActionMenuItem menuDialogsInformation(&minfoDialogsInformation, &menuDialogsController, INFO_LOCATION_PGM);
AnyMenuInfo minfoDialogsQuestion = { "Question", 10, 0xffff, 0, onDialogQuestion };
ActionMenuItem menuDialogsQuestion(&minfoDialogsQuestion, &menuDialogsInformation, INFO_LOCATION_RAM);
const SubMenuInfo minfoDialogs = { "Dialogs", 9, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackDialogs(&minfoDialogs, &menuDialogsQuestion, INFO_LOCATION_PGM);
SubMenuItem menuDialogs(&minfoDialogs, &menuBackDialogs, &menuList, INFO_LOCATION_PGM);
const AnyMenuInfo minfoPizzaMakerStartCooking = { "Start Cooking", 8, 0xffff, 0, onStartCooking };
ActionMenuItem menuPizzaMakerStartCooking(&minfoPizzaMakerStartCooking, nullptr, INFO_LOCATION_PGM);
extern char pizzaToppings[];
const AnyMenuInfo minfoPizzaMakerTopping3 = { "Topping 3", 7, 0xffff, 0, NO_CALLBACK };
ScrollChoiceMenuItem menuPizzaMakerTopping3(&minfoPizzaMakerTopping3, 0, pizzaToppings, 10, 6, &menuPizzaMakerStartCooking, INFO_LOCATION_PGM);
extern char pizzaToppings[];
const AnyMenuInfo minfoPizzaMakerTopping2 = { "Topping 2", 6, 0xffff, 0, NO_CALLBACK };
ScrollChoiceMenuItem menuPizzaMakerTopping2(&minfoPizzaMakerTopping2, 0, pizzaToppings, 10, 6, &menuPizzaMakerTopping3, INFO_LOCATION_PGM);
extern char pizzaToppings[];
const AnyMenuInfo minfoPizzaMakerTopping1 = { "Topping 1", 5, 0xffff, 0, NO_CALLBACK };
ScrollChoiceMenuItem menuPizzaMakerTopping1(&minfoPizzaMakerTopping1, 0, pizzaToppings, 10, 6, &menuPizzaMakerTopping2, INFO_LOCATION_PGM);
const AnalogMenuInfo minfoPizzaMakerOvenTemp = { "Oven Temp", 4, 0xffff, 300, NO_CALLBACK, 0, 1, "C" };
AnalogMenuItem menuPizzaMakerOvenTemp(&minfoPizzaMakerOvenTemp, 0, &menuPizzaMakerTopping1, INFO_LOCATION_PGM);
const SubMenuInfo minfoPizzaMaker = { "Pizza Maker", 3, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackPizzaMaker(&minfoPizzaMaker, &menuPizzaMakerOvenTemp, INFO_LOCATION_PGM);
SubMenuItem menuPizzaMaker(&minfoPizzaMaker, &menuBackPizzaMaker, &menuDialogs, INFO_LOCATION_PGM);
const SubMenuInfo minfoOven = { "Oven", 2, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackOven(&minfoOven, nullptr, INFO_LOCATION_PGM);
SubMenuItem menuOven(&minfoOven, &menuBackOven, &menuPizzaMaker, INFO_LOCATION_PGM);
const BooleanMenuInfo minfoMainPower = { "Main Power", 1, 0xffff, 1, NO_CALLBACK, NAMING_ON_OFF };
BooleanMenuItem menuMainPower(&minfoMainPower, false, &menuOven, INFO_LOCATION_PGM);

void setupMenu() {
    // First we set up eeprom and authentication (if needed).
    setSizeBasedEEPROMStorageEnabled(false);
    // Code generated by plugins and new operators.
    gfx.initR(INITR_BLACKTAB);
    gfx.setRotation(1);
    renderer.setUpdatesPerSecond(5);
    switches.init(internalDigitalIo(), SWITCHES_POLL_KEYS_ONLY, true);
    menuMgr.initForEncoder(&renderer, &menuMainPower, 6, 5, 9, FULL_CYCLE);
    keyboardLayout.setRowPin(0, 11);
    keyboardLayout.setRowPin(1, 10);
    keyboardLayout.setRowPin(2, 9);
    keyboardLayout.setRowPin(3, 8);
    keyboardLayout.setColPin(0, 15);
    keyboardLayout.setColPin(1, 14);
    keyboardLayout.setColPin(2, 13);
    keyboardLayout.setColPin(3, 12);
    keyboard.initialise(ioexp_io23017, &keyboardLayout, &tcMenuKeyListener, true);
    keyboard.setRepeatKeyMillis(850, 350);
    installCoolBlueTraditionalTheme(renderer, MenuFontDef(&OpenSansRegular8pt, 0), MenuFontDef(&OpenSansCyrillicLatin12, 0), true, BaseGraphicalRenderer::TITLE_FIRST_ROW, true);
}

