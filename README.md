# TcMenu library for Arduino and mbed.

## Summary

TcMenu is a modular, IoT ready menu library for the Arduino and mbed platform, it uses plugins to support many displays, input devices and provides remote control using a simple protocol over Ethernet and Serial. Menu designs are built using a designer UI and then generated for the platform. Target platform is anything from Arduino Uno upward to ST32F4 boards and beyond. Tested on many Arduino and mbed boards including Uno, Mega2560, SAMD, Nano, STM32F4 and ESP8266/ESP32 boards. Note that this repository contains just the Arduino/mbed library to meet the requirements in the Arduino specification. For the main repository see the links below.

* [TcMenu main repo](https://github.com/davetcc/tcMenu)
* [TcMenu main page at TheCodersCorner website](https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/)

## Arduino Library Installation

For most people, the best way to proceed is via library manager from Arduino IDE. Simply choose tcMenu and this will include it's core dependencies (IoAbstraction and LiquidCrystalIO). The only additions you'll need are:

* Adafruit_GFX if you are using the Adafruit graphics driver.
* U8G2 if you are using the U8G2 driver.
* UIP Ethernet, if you are using the UIP driver, take special care that this is GPL licensed.

## PlatformIO Installation

Install the [tcMenu library dependency](https://platformio.org/lib/show/7316/tcMenu) into your sketch. This will automatically include IoAbstraction but not LiquidCrystalIO, if you are using an LCD, also [include a LiquidCrystalIO dependency](https://platformio.org/lib/show/7242/LiquidCrystalIO).

### Installing tcMenu Designer UI, recommended

For designing menu structures we recommend using the TcMenu Designer which can design your menu in a round trip way, generate the code including the correct plugins for your hardware setup.

* On Windows 10, an *early* access of the app is available in the Windows Store, **it's currently in late BETA state, please report anything found on the forum** - https://www.microsoft.com/store/apps/9NHJNH9BCNJN, forum topic for reporting issues https://www.thecoderscorner.com/jforum/posts/list/47.page
* On MacOS a native app is probably now weeks away, in the mean time use the original packaged version below.
* Linux, Windows 7/8 (and MacOS for now), continue to use the original packaged application updated for 1.4.x - https://github.com/davetcc/tcMenu/releases

## Tested Configurations

We test each release of the software on a wide range of hardware. Here's what's tested with nearly every release. Our tests include generating the menu for each of the boards, compiling, uploading and testing on device, then testing remote control against the latest embedCONTROL UI.

* MKR 1300, MKR Ethernet Shield, AdaFruit_GFX ST7735 and IL, Rotary Encoder connected to PCF8574, i2c EEPROM
* MEGA 2560, DF Robot shield for input and output. Internal EEPROM
* MEGA 2560, Nokia 5110, Rotary Encoder, UIP Ethernet, Bluetooth Serial Module. Internal EEPROM
* MEGA 2560, Matrix keyboard, Rotary Encoder on MCP23017, display 20x4 sharing MCP23017, i2c EEPROM 
* ESP8266, OLED SSD1106 display, rotary encoder on PCF8574, ESP WiFi remote using DHCP.
* ESP32, OLED SSD1306 display, minimal input, ESP WiFi remote using DHCP. (Heltek WiFi Kit 32)
* Nano 33 BLE sense board with rotary encoder and I2C LCD display
* mbed STM32F439 based board with Ethernet and I2C LCD display

### Controlling menu items remotely

You can use the embedCONTROL UI (coming soon), Java Example UI and TcMenu Java API (soon TcMenu C# API too) to remotely control your menu design. Connectivity is near automatic, connection establishment and loss detection is managed by the API, and it's quite straightforward to set up authentication. At the moment we support Ethernet2, UipEthernet, most serial devices including bluetooth, ESP8266 WiFi and ESP32 WiFi.

### Manual usage of library, NOT recommended

If you are manually using tcMenu, without the designer, this page fully documents the types used, along with how to create them. Once you've created a menu structure, at this point you will need to include the right plugins. The plugins are hosted in the main repository and each one is documented to some extent in the second link below:

* [Describing the TcMenu type system](https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-menu-item-types-tutorial/)
* [The core plugins that you can use](https://github.com/davetcc/tcMenu/tree/master/CoreXmlPlugins)

## Asking questions

We try our best to answer all questions in our hosted forum that covers our libraries, we also provide commercial support for companies trying to use our libraries in a commercial design. Further, you can also ask questions related to Arduino in the regular Arduino forum, we try to monitor that on a best efforts basis, please include the library name in the forum topic if you use Arduino forum.

* [TCC Community forum](https://www.thecoderscorner.com/jforum/)
* [Commercial support](https://www.thecoderscorner.com/all-contact)
* I also monitor the Arduino forum [https://forum.arduino.cc/], Arduino related questions can be asked there too, please make sure the library name is in the subject.
