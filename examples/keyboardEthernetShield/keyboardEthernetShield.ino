#include "keyboardEthernetShield_menu.h"
#include <IoAbstractionWire.h>
#include <RemoteAuthentication.h>
#include <RemoteMenuItem.h>
#include <KeyboardManager.h>
#include <tcMenuKeyboard.h>
#include <EepromAbstraction.h>

// Set up ethernet, the usual default settings are chosen. Change to your preferred values or use DHCP.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};

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

	// now we enable authentication using EEPROM authentication. Where the EEPROM is
	// queried for authentication requests, and any additional pairs are stored there too.
	// first we initialise the authManager, then pass it to the class.
	// Always call BEFORE setupMenu()
	authManager.initialise(&eeprom, 100);
	remoteServer.setAuthenticator(&authManager);

	// Here we add two additional menus for managing the connectivity and authentication keys.
	// In the future, there will be an option to autogenerate these from the designer.
	menuConnectivitySaveToEEPROM.setNext(&menuAuthKeyMgr);
	menuRemoteMonitor.addConnector(remoteServer.getRemoteConnector(0));
	menuAuthKeyMgr.setLocalOnly(true);

	setupMenu();

	setupKeyboard();

	// here we use the EEPROM to load back the last set of values.
	menuMgr.load(eeprom, 0xf8f3);

	// and print out the IP address
	char sz[20];
	menuConnectivityIpAddress.copyValue(sz, sizeof(sz));
	Serial.print("Device IP is: "); Serial.println(sz);

	// spin up the Ethernet library, get the IP address from the menu
	byte* rawIp = menuConnectivityIpAddress.getIpAddress();
	IPAddress ip(rawIp[0], rawIp[1], rawIp[2], rawIp[3]);
	Ethernet.begin(mac, ip);

}

void loop() {
    taskManager.runLoop();
}


void CALLBACK_FUNCTION onFiths(int id) {
	Serial.println("Fiths changed");
}


void CALLBACK_FUNCTION onInteger(int id) {
	Serial.println("Integer changed");
}


void CALLBACK_FUNCTION onAnalog1(int id) {
	Serial.println("Analog1 changed");
}


void CALLBACK_FUNCTION onSaveToEeprom(int id) {
	menuMgr.save(eeprom, 0xf8f3);
}
