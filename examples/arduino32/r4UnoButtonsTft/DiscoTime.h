
#ifndef TCLIBRARYDEV_DISCOTIME_H
#define TCLIBRARYDEV_DISCOTIME_H

#include "r4UnoButtonsTft_menu.h"
#include <Adafruit_GFX.h>
#include <tcUnicodeAdaGFX.h>

/**
 * Create an event that will run our LED Matrix as something akin to disco lights. When the disco is running it will
 * schedule very frequently and use an AdaFruit_GFX GFXCanvas1 to represent the 12x8 display, and with that you can
 * draw anything onto it that is possible with Adafruit_GFX. Feel free to make it better!
 *
 * This also shows how to create custom messages, we have two custom "disco messages" that the Java API can receive and
 * you can easily create additional protocol handlers to process them.
 */
class DiscoTime : public BaseEvent {
public:
    // the types of disco lights we have!
    enum DiscoMode { TEXT, CIRCLES, SQUARES, STROBE, PICTURE };
private:
    // when the disco is running, this is true
    bool discoIsRunning = false;
    // the speed of our disco in milliseconds
    int speed = 250;
    // the canvas we will draw into
    GFXcanvas1 canvas;
    // the text area to store the text to print in text mode
    char textBuffer[32];
    // the font handler that draws unicode fonts using TcUnicode format
    UnicodeFontHandler unicodeHelper;
    // the current position in the animation
    int currPos = 0;
    // the amount of time the loop has been running
    int loopTimer = 0;
    // the current mode
    DiscoMode mode = CIRCLES;
public:
    explicit DiscoTime();
    void init();

    //
    // These two are part of TaskManagerIO BaseEvent processing, see TaskManagerIO event handling for docs:
    // https://tcmenu.github.io/documentation/arduino-libraries/taskmanager-io/
    //
    void exec() override;
    uint32_t timeOfNextCheck() override;

    // start the disco on the matrix
    void start(int speed);
    // draw a picture on the matrix in 12x8 XBMP format
    void picture(const uint8_t* xbmpData);
    // draw text on the matrix
    void text(const char* data);
    // stop the animation
    void stop();

    // These two send non-standard custom messages over the wire, this shows how easy it is to define your own.
    // https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/tcmenu-iot/embed-control-tagval-wire-protocol/
    void sendDiscoMsg();
    void sendXbmpData(const uint8_t* xbmpData, size_t length);

    // these are the animation methods
    void runSquares();
    void drawText();
    void runStrobe();

    // and lastly we convert the canvas to LED output with this
    void drawCanvasToLeds();

    void runCircles();

    void switchMode(DiscoMode mode);
};


#endif //TCLIBRARYDEV_DISCOTIME_H
