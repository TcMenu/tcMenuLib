## Uno SSD1306 ASCII: Take over the display

This shows an example of using the SSD1306Ascii library available from package manager with tcMenu. It is a very simple menu that's built for Uno with Serial remote capabilities, it is assumed that because the connection is over serial, there is no need for Authentication.

To get started just connect a suitable display to your Arduino, in this case I used an 1106 over Wire, and connect a rotary encoder to pins 2 and 3, connect the click button to A3. With this the demo should work out of the box.

## Files in the example and their purpose:

unoSsd1306Ascii.ino - the function definitions will be placed here.

## Using the designer

This menu was generated using the "tcMenu Designer" which can be obtained from the Windows Store, or from https://github.com/davetcc/tcMenu

You can get detailed instructions about tcMenu from the main page: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/.
