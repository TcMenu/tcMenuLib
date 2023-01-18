/**
 * This script demonstrates the use of a matrix keyboard with a rotary encoder also attached, this allows for the
 * best of both worlds where the encoder is used for rotational type editing, and the keyboard where it is appropriate.
 * This sketch is setup for MEGA2560 AVR with a 20x4 LCD on I2C and also an encoder on I2C. It also assumes an I2C ROM.
 * However, you can take the ideas from this sketch and apply them in your own designs.
 *
 * More information about matrix keyboard support:
 * https://www.thecoderscorner.com/products/arduino-libraries/io-abstraction/matrix-keyboard-keypad-manager/
 *
 * Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/
 */
#include "keyboardEthernetShield_menu.h"
#include <IoAbstractionWire.h>
#include <RemoteAuthentication.h>
#include <RemoteMenuItem.h>
#include <KeyboardManager.h>
#include <tcMenuKeyboard.h>
#include <EepromAbstraction.h>
#include <Ethernet.h>
#include <IoLogging.h>
#include <stockIcons/wifiAndConnectionIconsLCD.h>

using namespace tcremote;

// Set up ethernet, the usual default settings are chosen. Change to your preferred values or use DHCP.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};

//
// Some constant character definitions that we use around the code
//
const char pgmListPressed[] PROGMEM = "List Item Pressed";
const char pgmHeaderSavedItem[] PROGMEM = "Saved Item";
const char * romSpaceNames = "item 01item 02item 03item 04item 05item 06item 07item 08item 09item 10 ";

// We add a title widget that shows when a user is connected to the device. Connection icons
// are in the standard icon set we included at the top.
// Yes even on LCD we now support title widgets, but they eat up a few of your custom chars.
// The width must always be 1, and the height is the first custom character that is used.
TitleWidget connectedWidget(iconsConnection, 2, 1, 0);

// used by the take over display logic.
int counter = 0;

//
// Creating a grid layout just for a specific menu item. The flags menu under additional is laid out in a grid format,
// where the menu items are presented two per row.
//
void prepareLayout() {
    auto& factory = renderer.getLcdDisplayPropertiesFactory();
    // we now create a grid for the two led controls in the submenu, this shows how to override the default grid for a few items.
    // In most cases the automatic rendering works fine when a few items are overriden as long as the row you request hasn't yet been taken.
    factory.addGridPosition(&menuAdditionalBoolFlagFlag1,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE, 2, 1, 1, 1));
    factory.addGridPosition(&menuAdditionalBoolFlagFlag2,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_RIGHT_WITH_VALUE, 2, 2, 1, 1));
    factory.addGridPosition(&menuAdditionalBoolFlagFlag3,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_TITLE_LEFT_WITH_VALUE, 2, 1, 2, 1));
    factory.addGridPosition(&menuAdditionalBoolFlagFlag4,
                            GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_RIGHT_WITH_VALUE, 2, 2, 2, 1));
}

// when there's a change in communication status (client connects for example) this gets called.
void onCommsChange(CommunicationInfo info) {
    if(info.remoteNo == 0) {
        connectedWidget.setCurrentState(info.connected ? 1 : 0);
    }
    // this relies on logging in IoAbstraction's ioLogging.h, to turn it on visit the file for instructions.
    serdebugF4("Comms notify (rNo, con, enum)", info.remoteNo, info.connected, info.errorMode);
}

void setup() {
	//
	// If you are using serial (connectivity or logging) and wire they must be initialised 
	// before their first use.
	//
	Serial.begin(115200);
	Wire.begin();
    Wire.setClock(400000);
    lcd.setDelayTime(0, 20);

    serEnableLevel(SER_TCMENU_DEBUG, true);

    // you can turn off the title line (but also removes title widget).
    //renderer.setTitleRequired(false);

    // Here we set the character to be used for back, next and editing for the "cursor".
    renderer.setEditorChars(0b01111111, 0b01111110, '=');

    // You can also have title widgets on LCDs, maximum of 7 states. They use up the custom chars 0..7. How it works
    // is that you define a custom character for each icon, and treat them just as you would a regular icon. The height
    // of the widget is fixed, and the height field instead indicates the first custom character for the first icon.
    renderer.setFirstWidget(&connectedWidget);

	setupMenu();

	auto* authenticator = reinterpret_cast<EepromAuthenticatorManager*>(menuMgr.getAuthenticator());

	// Here you could have a button internal to the device somewhere that reset the pins and remotes, in this case
	// if the button is held at start up it trigger a key reset.
    if(ioDeviceDigitalReadS(ioexp_io23017, 5) == LOW) {
        Serial.println("Resetting all keys and pin");
        authenticator->resetAllKeys();
    }

	// here we use the EEPROM to load back the last set of values.
	menuMgr.load(0xf8f3);

	// and print out the IP address
	char sz[20];
	menuConnectivityIpAddress.copyValue(sz, sizeof(sz));
	Serial.print("Device IP is: "); Serial.println(sz);

	// spin up the Ethernet library, get the IP address from the menu
	byte* rawIp = menuConnectivityIpAddress.getIpAddress();
	IPAddress ip(rawIp[0], rawIp[1], rawIp[2], rawIp[3]);
	Ethernet.begin(mac, ip);

    // copy the pin from the authenticator into the change pin field.
    // and make it a password field so characters are not visible unless edited.
    authenticator->copyPinToBuffer(sz, sizeof(sz));
    menuConnectivityChangePin.setTextValue(sz);
    menuConnectivityChangePin.setPasswordField(true);

    // here's an example of setting an item to a value manually before the loop starts.
    menuLargeNum.getLargeNumber()->setFromFloat(1234.567);

    // here we customize the LCD layout for one menu, to have two items per line.
    prepareLayout();

    // and lastly we register a communication listener, it updates the title widget
    // that shows connectivity state on the right corner.
    menuConnectivityIoTMonitor.registerCommsNotification(onCommsChange);

    // and finally, when the display times out, take over and draw a custom screen
    renderer.setResetCallback([] {
        onTakeOverDisplay(-1);
    });
}

void loop() {
    taskManager.runLoop();
}


void CALLBACK_FUNCTION onFiths(int /*id*/) {
	Serial.println("Fiths changed");
}


void CALLBACK_FUNCTION onInteger(int /*id*/) {
	Serial.println("Integer changed");
}


void CALLBACK_FUNCTION onAnalog1(int /*id*/) {
	Serial.println("Analog1 changed");
}


const char pgmSavedText[] PROGMEM = "Saved all setttings";

void CALLBACK_FUNCTION onSaveToEeprom(int /*id*/) {
	menuMgr.save(0xf8f3);
	auto dlg = renderer.getDialog();
	if(dlg && !dlg->isInUse()) {
	    dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
	    dlg->show(pgmSavedText, true, nullptr);
	    dlg->copyIntoBuffer("");
	}
}

const char pgmPinTooShort[] PROGMEM = "Pin too short";

void CALLBACK_FUNCTION onChangePin(int /*id*/) {
    // Here we check if the pin that's just been entered is too short.
    // Diallowing setting and showing a dialog if it is.
    const char* newPin = menuConnectivityChangePin.getTextValue();
    if(strlen(newPin) < 4) {
        BaseDialog* dlg = renderer.getDialog();
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
        dlg->show(pgmPinTooShort, false);
        dlg->copyIntoBuffer(newPin);
    }
    else {
        auto* authenticator = reinterpret_cast<EepromAuthenticatorManager*>(menuMgr.getAuthenticator());
        authenticator->changePin(newPin);
    }
}

void CALLBACK_FUNCTION onItemChange(int id) {
    auto itemNo = menuRomChoicesItemNum.getCurrentValue();
    char sz[12];
    auto itemSize = menuAdditionalRomChoice.getItemWidth();
    menuMgr.getEepromAbstraction()->readIntoMemArray((uint8_t*)sz, menuAdditionalRomChoice.getEepromStart() + (itemNo * itemSize), 10);
    menuRomChoicesValue.setTextValue(sz);
}


void CALLBACK_FUNCTION onSaveValue(int id) {
    auto itemNo = menuRomChoicesItemNum.getCurrentValue();
    auto itemSize = menuAdditionalRomChoice.getItemWidth();
    auto position = menuAdditionalRomChoice.getEepromStart() + (itemNo * itemSize);
    menuMgr.getEepromAbstraction()->writeArrayToRom(position, (const uint8_t*)menuRomChoicesValue.getTextValue(), itemSize);

    if(renderer.getDialog()->isInUse()) return;
    renderer.getDialog()->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
    renderer.getDialog()->show(pgmHeaderSavedItem, true);
    renderer.getDialog()->copyIntoBuffer(menuRomChoicesValue.getTextValue());
}

//
// Here we handle the custom rendering for the number choices Scroll Choice menu item. We basically get called back
// every time it needs more data. For example when the name is required, when any index value is required or when
// it will be saved to EEPROM or invoked.
//
int CALLBACK_FUNCTION fnAdditionalNumChoicesRtCall(RuntimeMenuItem * item, uint8_t row, RenderFnMode mode, char * buffer, int bufferSize) {
    switch(mode) {
        case RENDERFN_EEPROM_POS:
           return 32;
        case RENDERFN_INVOKE:
            serdebugF2("Num Choice changed to ", row);
            return true;
        case RENDERFN_NAME:
            strcpy(buffer, "Num Choices");
            return true;
        case RENDERFN_VALUE:
            buffer[0] = 'V'; buffer[1]=0;
            fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
            return true;
        default: return false;
    }
}

//
// Here we handle the custom rendering for the runtime list menu item that just counts. We basically get called back
// every time it needs more data. For example when the name is required, when any index value is required or when
// it will be saved to EEPROM or invoked.
//
int CALLBACK_FUNCTION fnAdditionalCountListRtCall(RuntimeMenuItem * item, uint8_t row, RenderFnMode mode, char * buffer, int bufferSize) {
    switch(mode) {
        case RENDERFN_INVOKE: {
            // when the user selects an item in the list, we get this callback. Row is the offset of the selection
            renderer.getDialog()->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
            renderer.getDialog()->show(pgmListPressed, true);
            char sz[10];
            ltoaClrBuff(sz, row, 3, NOT_PADDED, sizeof sz);
            renderer.getDialog()->copyIntoBuffer(sz);
            return true;
        }
        case RENDERFN_NAME:
            // Called whenever the name of the menu item is needed
            strcpy(buffer, "List Item");
            return true;
        case RENDERFN_VALUE:
            // Called whenever the value of a given row is needed.
            buffer[0] = 'V'; buffer[1]=0;
            fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
            return true;
        case RENDERFN_EEPROM_POS: return 0xFFFF; // lists are generally not saved to EEPROM
        default: return false;
    }
}

//
// this is the function called by the renderer every 1/5 second once the display is
// taken over, we pass this function to takeOverDisplay below.
//
void myDisplayFunction(unsigned int encoderValue, RenderPressMode clicked) {
    // we initialise the display on the first call.
    if(counter == 0) {
        switches.changeEncoderPrecision(999, 50);
        lcd.clear();
        lcd.print("We have the display!");
        lcd.setCursor(0, 1);
        lcd.print("OK button for menu..");
    }

    // We are told when the button is pressed in by the boolean parameter.
    // When the button is clicked, we give back to the menu..
    if(clicked) {
        renderer.giveBackDisplay();
        counter = 0;
    }
    else {
        char buffer[5];
        // otherwise update the counter.
        lcd.setCursor(0, 2);
        ltoaClrBuff(buffer, ++counter, 4, ' ', sizeof(buffer));
        lcd.print(buffer);
        lcd.setCursor(12, 2);
        ltoaClrBuff(buffer, encoderValue, 4, '0', sizeof(buffer));
        lcd.print(buffer);
    }
}

//
// We have an option on the menu to take over the display, this function is called when that
// option is chosen.
//
void CALLBACK_FUNCTION onTakeOverDisplay(int /*id*/) {
    // in order to take over rendering onto the display we just request the display
    // at which point tcMenu will stop rendering until the display is "given back".
    // Don't forget that LiquidCrystalIO uses task manager and things can be happening
    // in the background. Always ensure all operations with the LCD occur on the rendering
    // call back.

    counter = 0;
    renderer.takeOverDisplay(myDisplayFunction);
}

//
// For Item: menuAdditionalCustomHex
//
// Here we present how to customize a runtime menu item such as a text item, RGB or Large number. You can take full
// control of the callback, or just override a few of the features to customize it.
//
// In this case we start with the regular text item and then we restrict the values that can be put into each char down
// to only 0-9 and A-F. This shows how to quickly provide a simple custom text editor.
//
// Steps
// 1. In the Designer of a text item, select "edit" next to the function callback text field
// 2. Select the "Runtime RenderFn override implementation" option
// 3. Once code generation the function is generated (exactly as below)
//
int CALLBACK_FUNCTION customHexEditorRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    auto textItem = reinterpret_cast<TextMenuItem*>(item);
    // See https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/based-on-runtimemenuitem/
    switch(mode) {
        case RENDERFN_NAME:
            return false; // use default
        case RENDERFN_GETRANGE:
            return 15; // range is 0..15 - IE '0'-'F'.
        case RENDERFN_GETPART: {
            int partVal = textItem->getTextValue()[row - 1];
            return (partVal >= 65) ? partVal - 55 : partVal - 48;
        }
        case RENDERFN_SET_VALUE: {
            // value is from a rotary encoder or other linear range based input that emulates an encoder. Here we need
            // to just set the character value that represents the current value.
            char val = buffer[0];
            textItem->setCharValue(row - 1, val > 10 ? char(val + 55) : char(val + 48));
            return true;
        }
        case RENDERFN_SET_TEXT_VALUE: {
            // value is from a keyboard and in this case the actual character to store is passed in, we validate it
            // against our allowed "hex" characters, returning false prevents the keyboard input being accepted.
            char val = buffer[0];
            if ((val >= '0' && val <= '9') || (val >= 'A' && val <= 'F')) {
                textItem->setCharValue(row - 1, val);
            } else {
                return false;
            }
            return true;
        }
    }
    return textItemRenderFn(item, row, mode, buffer, bufferSize);
}
