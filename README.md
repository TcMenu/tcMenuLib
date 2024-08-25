# TcMenu library for Arduino and mbed.
[![PlatformIO](https://github.com/TcMenu/tcMenuLib/actions/workflows/platformio.yml/badge.svg)](https://github.com/TcMenu/tcMenuLib/actions/workflows/platformio.yml)
[![Test](https://github.com/TcMenu/tcMenuLib/actions/workflows/test.yml/badge.svg)](https://github.com/TcMenu/tcMenuLib/actions/workflows/test.yml)
[![License: Apache 2.0](https://img.shields.io/badge/license-Apache--2.0-green.svg)](https://github.com/TcMenu/tcMenuLib/blob/main/LICENSE)
[![GitHub release](https://img.shields.io/github/release/TcMenu/tcMenuLib.svg?maxAge=3600)](https://github.com/TcMenu/tcMenuLib/releases)
[![davetcc](https://img.shields.io/badge/davetcc-dev-blue.svg)](https://github.com/davetcc)
[![JSC TechMinds](https://img.shields.io/badge/JSC-TechMinds-green.svg)](https://www.jsctm.cz)

## Summary

TcMenu organisation made this framework available for you to use. It takes significant effort to keep all our libraries current and working on a wide range of boards. Please consider making at least a one off donation via the sponsor button if you find it useful.

<a href="https://www.buymeacoffee.com/davetcc" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-blue.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" ></a>

TcMenu is a modular, IoT ready multi level menu library for the Arduino and mbed platform, it uses plugins to support many displays, input devices and provides remote control using a simple protocol over Ethernet and Serial.

In any fork, please ensure all text up to here is left unaltered.

Menu designs are built using a designer UI and then generated for the platform. Target platform is anything from Arduino Uno upward to ST32F4 boards and beyond. Tested on many Arduino and mbed boards including Uno, Mega2560, SAMD, Nano, STM32F4 and ESP8266/ESP32 boards. Note that this repository contains just the Arduino/mbed library to meet the requirements in the Arduino specification. For the main repository see the links below. See the list of [Arduino and mbed library and board configurations we test with](https://www.thecoderscorner.com/products/arduino-libraries/)

* [TcMenu main repo](https://github.com/TcMenu/tcMenu)
* [TcMenu main page at TheCodersCorner website](https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/)

## Arduino Library Installation

For most people, the best way to proceed is via library manager from Arduino IDE. Simply choose tcMenu and this will include it's core dependencies (IoAbstraction and LiquidCrystalIO). The only additions you'll need are:

* Adafruit_GFX if you are using the Adafruit graphics driver.
* U8G2 if you are using the U8G2 driver.
* UIP Ethernet, if you are using the UIP driver, take special care that this is GPL licensed.

## PlatformIO Installation

Install the [tcMenu library dependency](https://platformio.org/lib/show/7316/tcMenu) into your sketch. This will automatically include IoAbstraction but not LiquidCrystalIO, if you are using an LCD, also [include a LiquidCrystalIO dependency](https://platformio.org/lib/show/7242/LiquidCrystalIO).

### Installing tcMenu Designer UI, recommended

For designing menu structures we recommend using the TcMenu Designer which can design your menu in a round trip way, generate the code including the correct plugins for your hardware setup. There is a Linux package, notarized version for macOS, and a Windows Installer with an extended validation certificate. 

All releases are available from: [https://github.com/TcMenu/tcMenu/releases]

### Controlling menu items remotely

You can use the embedCONTROL UI (coming soon), Java Example UI and TcMenu Java API (soon TcMenu C# API too) to remotely control your menu design. Connectivity is near automatic, connection establishment and loss detection is managed by the API, and it's quite straightforward to set up authentication. At the moment we support Ethernet2, UipEthernet, most serial devices including bluetooth, ESP8266 WiFi and ESP32 WiFi.

### Manual usage of library, NOT recommended

If you are manually using tcMenu, without the designer, this page fully documents the types used, along with how to create them. Once you've created a menu structure, at this point you will need to include the right plugins. The plugins are hosted in the main repository and each one is documented to some extent in the second link below:

* [Describing the TcMenu type system](https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-menu-item-types-tutorial/)
* The core plugins that you could base your display, input, and remote facilities on are in the main tcMenu repo.

## Asking questions

TheCodersCorner.com invest a lot of time and resources into making this open source product which is used by literally thousands of users. We offer both [commercial support](https://www.thecoderscorner.com/support-services/training-support/) and [C++/Java/Dart consultancy](https://www.thecoderscorner.com/support-services/consultancy/), or if you just want to say thanks, you can also make a donation via [GitHub](https://github.com/TcMenu/tcMenu) or use the link below. 

<a href="https://www.buymeacoffee.com/davetcc" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-blue.png" alt="Buy Me A Coffee" style="height: 60px !important;width: 217px !important;" ></a>

* [discussions section of the tcMenu repo](https://github.com/TcMenu/tcMenu/discussions) of tcmenu git repo
* [Arduino discussion forum](https://forum.arduino.cc/) where questions can be asked, please tag me using `@davetcc`.
* [Legacy discussion forum probably to be made read only soon](https://www.thecoderscorner.com/jforum/).
