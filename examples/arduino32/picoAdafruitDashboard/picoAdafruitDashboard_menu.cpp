/*
    The code in this file uses open source libraries provided by thecoderscorner

    DO NOT EDIT THIS FILE, IT WILL BE GENERATED EVERY TIME YOU USE THE UI DESIGNER
    INSTEAD EITHER PUT CODE IN YOUR SKETCH OR CREATE ANOTHER SOURCE FILE.

    All the variables you may need access to are marked extern in this file for easy
    use elsewhere.
 */

#include <tcMenu.h>
#include "picoAdafruitDashboard_menu.h"
#include "ThemeCoolBlueModern.h"
#include <Fonts/RobotoMedium24.h>

// Global variable declarations
const  ConnectorLocalInfo applicationInfo = { TC_I18N_PROJECT_NAME, "0ad4bdde-34a4-4507-912e-b495b0eac2c1" };
TcMenuRemoteServer remoteServer(applicationInfo);

Adafruit_ILI9341 gfx(20, 18, 19);
AdafruitDrawable gfxDrawable(&gfx, 40);
GraphicsDeviceRenderer renderer(30, applicationInfo.name, &gfxDrawable);
NoInitialisationNeeded serialInitializer;
SerialTagValueTransport serialTransport(&Serial);
TagValueRemoteServerConnection serialConnection(serialTransport, serialInitializer);

// Global Menu Item declarations
const BooleanMenuInfo minfoSettingsCheckBox = { "CheckBox", 9, 0xffff, 1, NO_CALLBACK, NAMING_CHECKBOX };
BooleanMenuItem menuSettingsCheckBox(&minfoSettingsCheckBox, false, nullptr, INFO_LOCATION_PGM);
const BooleanMenuInfo minfoSettingsYesNo = { TC_I18N_MENU_8_NAME, 8, 0xffff, 1, NO_CALLBACK, NAMING_YES_NO };
BooleanMenuItem menuSettingsYesNo(&minfoSettingsYesNo, false, &menuSettingsCheckBox, INFO_LOCATION_PGM);
const AnyMenuInfo minfoSettingsRGB = { TC_I18N_MENU_7_NAME, 7, 0xffff, 0, NO_CALLBACK };
Rgb32MenuItem menuSettingsRGB(&minfoSettingsRGB, RgbColor32(0, 0, 0), false, &menuSettingsYesNo, INFO_LOCATION_PGM);
const AnyMenuInfo minfoSettingsAction = { "Action", 6, 0xffff, 0, onSettingsAction };
ActionMenuItem menuSettingsAction(&minfoSettingsAction, &menuSettingsRGB, INFO_LOCATION_PGM);
const SubMenuInfo minfoSettings = { TC_I18N_MENU_5_NAME, 5, 0xffff, 0, NO_CALLBACK };
BackMenuItem menuBackSettings(&minfoSettings, &menuSettingsAction, INFO_LOCATION_PGM);
SubMenuItem menuSettings(&minfoSettings, &menuBackSettings, nullptr, INFO_LOCATION_PGM);
const BooleanMenuInfo minfoPower = { "Power", 4, 0xffff, 1, NO_CALLBACK, NAMING_ON_OFF };
BooleanMenuItem menuPower(&minfoPower, false, &menuSettings, INFO_LOCATION_PGM);
const char enumStrEnum_0[] = TC_I18N_MENU_3_ENUM_0;
const char enumStrEnum_1[] = TC_I18N_MENU_3_ENUM_1;
const char enumStrEnum_2[] = TC_I18N_MENU_3_ENUM_2;
const char* const enumStrEnum[]  = { enumStrEnum_0, enumStrEnum_1, enumStrEnum_2 };
const EnumMenuInfo minfoEnum = { TC_I18N_MENU_3_NAME, 3, 0xffff, 2, NO_CALLBACK, enumStrEnum };
EnumMenuItem menuEnum(&minfoEnum, 0, &menuPower, INFO_LOCATION_PGM);
const FloatMenuInfo minfoFloat = { TC_I18N_MENU_2_NAME, 2, 0xffff, 3, NO_CALLBACK };
FloatMenuItem menuFloat(&minfoFloat, 33.123, &menuEnum, INFO_LOCATION_PGM);
const AnalogMenuInfo minfoAnalog = { TC_I18N_MENU_1_NAME, 1, 0xffff, 1000, NO_CALLBACK, 0, 10, TC_I18N_MENU_1_UNIT };
AnalogMenuItem menuAnalog(&minfoAnalog, 222, &menuFloat, INFO_LOCATION_PGM);

void setupMenu() {
    // First we set up eeprom and authentication (if needed).
    setSizeBasedEEPROMStorageEnabled(false);
    // Code generated by plugins.
    gfx.begin();
    gfx.setRotation(1);
    renderer.setUpdatesPerSecond(5);
    switches.init(internalDigitalIo(), SWITCHES_POLL_EVERYTHING, true);
    menuMgr.initForEncoder(&renderer, &menuAnalog, 16, 17, 21);
    remoteServer.addConnection(&serialConnection);
    renderer.setTitleMode(BaseGraphicalRenderer::TITLE_FIRST_ROW);
    renderer.setUseSliderForAnalog(true);
    installCoolBlueModernTheme(renderer, MenuFontDef(&RobotoMedium24, 1), MenuFontDef(&RobotoMedium24, 1), false);
}

