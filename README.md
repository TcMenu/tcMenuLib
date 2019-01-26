# TcMenu library for Arduino platform.

## Summary

TcMenuLib is the Arduino menu library for part of tcMenu, that supports different displays, remote control using a simple protocol over Ethernet and Serial, a 
Java API and rotary encoder or switch input. Target platform is anything from Arduino Uno upward. Tested on ATMEGA328 (Uno), Mega2560 and SAMD (MKR1300).

This is the Arduino library part of the package, it also comes with a Java designer and Java API. You can get the whole lot packaged from the URL below, it is recommended to use this with the designer.

* [TcMenuAPI, Designer UI and Example UI](https://github.com/davetcc/tcMenu)
* [TCC Community forum](https://www.thecoderscorner.com/jforum/)

## Installation

This is the native library that goes along with tcMenu. It's in a separate repository because of the way that Arduino libraries need to be packaged. Install by unzipping into the Arduino/libraries directory and rename the folder to tcMenu. You can also get the full package with generator UI and examples, see below. This library depends upon:

* IoAbstraction [https://github.com/davetcc/IoAbstraction] in all cases.
* LiquidCrystalIO fork [https://github.com/davetcc/LiquidCrystalIO] that supports task manager when using the LiquidCrystal support.
* Adafruit_GFX if you are using the Adafruit graphics driver.

## Package structure and installation:

Usually the best way to get started with tcMenu is to use the TcMenu designer, this is packaged for Windows and Mac, it is also available for Linux.

If you are manually using tcMenu, without the designer, this page fully documents the types used, along with how to create them.

* [Describing the TcMenu type system](https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-menu-item-types-tutorial/)

