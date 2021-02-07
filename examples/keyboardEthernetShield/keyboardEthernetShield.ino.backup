#include "keyboardEthernetShield_menu.h"
#include <IoAbstractionWire.h>
#include <RemoteAuthentication.h>
#include <RemoteMenuItem.h>
#include <KeyboardManager.h>
#include <tcMenuKeyboard.h>
#include <EepromAbstraction.h>
#include <Ethernet.h>

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

// In the designer UI we configured io23017 as an IoExpander variable for both the input and display.
// We must now create it as an MCP23017 expander. Address is 0x20 with interrupt pin connected to pin 2.
// make sure you've arranged for !RESET pin to be held HIGH!!
IoAbstractionRef io23017 = ioFrom23017(0x20, ACTIVE_LOW_OPEN, 2);

// we want to authenticate connections, the easiest and quickest way is to use the EEPROM
// authenticator where pairing requests add a new item into the EEPROM. Any authentication
// requests are then handled by looking in the EEPROM.
EepromAuthenticatorManager authManager;

// here we initialise an AVR eeprom, there are lots of abstractions that cover most types of EEPROM
AvrEeprom eeprom;

// Here we create two additional menus, that will be added manually to handle the connectivity
// status and authentication keys. In a future version these will be added to th desinger.
RemoteMenuItem menuRemoteMonitor(1001, 2);
EepromAuthenicationInfoMenuItem menuAuthKeyMgr(1002, &authManager, &menuRemoteMonitor);

// 
// Here we add the keyboard components to the menu. For more general documentation of keyboards
// https://www.thecoderscorner.com/products/arduino-libraries/io-abstraction/matrix-keyboard-keypad-manager/
//

// First we create a standard layout, in this case the 3x4 layout but there's also a 4x4 one
// You can also generate your own layouts for this. The class is KeyboardLayout.
MAKE_KEYBOARD_LAYOUT_3X4(keyboardLayout)
//MAKE_KEYBOARD_LAYOUT_4X4(keyboardLayout)

// The matrix keyboard manager itself, looks after the keyboard, notifies of changes etc.
MatrixKeyboardManager keyboard;

// This is the tcMenu standard keyboard listener. It allows control of the menu by keyboard
// You could also write your own keyboard interface if this didn't work as expected for you.
MenuEditingKeyListener menuKeyListener;

//
// This function is called by setup() further down to initialise the keyboard.
//
void setupKeyboard() {
	// set up the pin mappings.
	keyboardLayout.setRowPin(0, 22);
	keyboardLayout.setRowPin(1, 23);
	keyboardLayout.setRowPin(2, 24);
	keyboardLayout.setRowPin(3, 25);
	keyboardLayout.setColPin(0, 26);
	keyboardLayout.setColPin(1, 27);
	keyboardLayout.setColPin(2, 28);

	// initialise and set the repeat key intervals.
	keyboard.initialise(ioUsingArduino(), &keyboardLayout, &menuKeyListener);
	keyboard.setRepeatKeyMillis(850, 350);

}

void setup() {
	//
	// If you are using serial (connectivity or logging) and wire they must be initialised 
	// before their first use.
	//
	Serial.begin(115200);
	Wire.begin();
	menuMgr.setEepromRef(&eeprom);

	// now we enable authentication using EEPROM authentication. Where the EEPROM is
	// queried for authentication requests, and any additional pairs are stored there too.
	// first we initialise the authManager, then pass it to the class.
	// Always call BEFORE setupMenu()
	authManager.initialise(&eeprom, 100);
	remoteServer.setAuthenticator(&authManager);
    menuMgr.setAuthenticator(&authManager);

	// Here we add two additional menus for managing the connectivity and authentication keys.
	// In the future, there will be an option to autogenerate these from the designer.
	menuConnectivitySaveToEEPROM.setNext(&menuAuthKeyMgr);
	menuRemoteMonitor.addConnector(remoteServer.getRemoteConnector(0));
	menuAuthKeyMgr.setLocalOnly(true);    

    // now we turn off the title and change the editor characters
    renderer.setTitleRequired(false);
    
    // Here we set the character to be used for back, next and editing for the "cursor".
    renderer.setEditorChars(0b01111111, 0b01111110, '=');

	setupMenu();

	setupKeyboard();

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
    authManager.copyPinToBuffer(sz, sizeof(sz));
    menuConnectivityChangePin.setTextValue(sz);
    menuConnectivityChangePin.setPasswordField(true);

    menuLargeNum.getLargeNumber()->setFromFloat(1234.567);
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
	menuMgr.save(eeprom, 0xf8f3);
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
        authManager.changePin(newPin);
    }
}

void CALLBACK_FUNCTION onItemChange(int id) {
    auto itemNo = menuRomChoicesItemNum.getCurrentValue();
    char sz[12];
    auto itemSize = menuAdditionalRomChoice.getItemWidth();
    eeprom.readIntoMemArray((uint8_t*)sz, menuAdditionalRomChoice.getEepromStart() + (itemNo * itemSize), 10);
    menuRomChoicesValue.setTextValue(sz);
}


void CALLBACK_FUNCTION onSaveValue(int id) {
    auto itemNo = menuRomChoicesItemNum.getCurrentValue();
    auto itemSize = menuAdditionalRomChoice.getItemWidth();
    auto position = menuAdditionalRomChoice.getEepromStart() + (itemNo * itemSize);
    eeprom.writeArrayToRom(position, (const uint8_t*)menuRomChoicesValue.getTextValue(), 10);

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




