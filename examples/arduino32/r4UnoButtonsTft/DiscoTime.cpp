/**
 * Create an event that will run our LED Matrix as something akin to disco lights. When the disco is running it will
 * schedule very frequently and use an AdaFruit_GFX GFXCanvas1 to represent the 12x8 display, and with that you can
 * draw anything onto it that is possible with Adafruit_GFX. Feel free to make it better!
 *
 * This also shows how to create custom messages, we have two custom "disco messages" that the Java API can receive and
 * you can easily create additional protocol handlers to process them.
 */

#include "DiscoTime.h"
#include <Arduino_LED_Matrix.h>
#include <Fonts/OpenSansRegular7pt.h>

//
// Here we pre-define a few custom tags for the custom tag val message we send later on. Each key in a tag val message
// is two characters. Use 'Z' and 'z' for you own custom ones. This gives you around 100 possibilities.
//
#define MY_CUSTOM_DISCO_MSG msgFieldToWord('Z', 'D')
#define MY_CUSTOM_XBMP_MSG msgFieldToWord('Z', 'X')
#define MY_CUSTOM_DISCO_NAME_FIELD msgFieldToWord('Z', 'N')
#define MY_CUSTOM_DISCO_STATUS_FIELD msgFieldToWord('Z', 'B')

ArduinoLEDMatrix matrix;

void DiscoTime::stop() {
    serlogF(SER_DEBUG, "Stop disco");
    discoIsRunning = false;
    sendDiscoMsg();

}

void DiscoTime::init() {
    matrix.begin();
    taskManager.registerEvent(this);
}

void DiscoTime::start(int sp) {
    serlogF(SER_DEBUG, "Init disco");
    discoIsRunning = true;
    speed = sp;
    switchMode(SQUARES);
    sendDiscoMsg();
}

// This is called during event handling, we can set here when we want to be asked again if the event is triggered.
// https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/tcmenu-iot/embed-control-tagval-wire-protocol/
uint32_t DiscoTime::timeOfNextCheck() {
    setTriggered(discoIsRunning);
    return discoIsRunning ? millisToMicros(speed) : secondsToMicros(1);
}

// Again this is part of the above event, if we are considered triggered, then this method is called, it works out
// which animation is running, and calls the right one.
void DiscoTime::exec() {
    currPos++;
    if(mode == TEXT) {
        drawText();
    } else if(mode == SQUARES) {
        runSquares();
    } else if(mode == STROBE) {
        runStrobe();
    } else if(mode == CIRCLES){
        runCircles();
    }
}

// Put an XBMP image onto the display using the canvas.
void DiscoTime::picture(const uint8_t* xbmpData) {
    stop();
    switchMode(PICTURE);
    serlogF(SER_DEBUG, "Picture");

    canvas.fillScreen(0);
    canvas.drawXBitmap(0, 0, xbmpData, 12, 8, 1);
    sendXbmpData(xbmpData, 16);
    drawCanvasToLeds();
}

// schedule text to be drawn onto the display
void DiscoTime::text(const char *data) {
    start(500);
    mode = TEXT;
    currPos = 0;
    strncpy(textBuffer, data, sizeof textBuffer);
    serlogF3(SER_DEBUG, "Start Text", data, textBuffer);
    textBuffer[sizeof(textBuffer)-1] = 0;
}

// draw one character at a time onto the display
void DiscoTime::drawText() {
    if(currPos >= strlen(textBuffer)) {
        currPos = 0;
    }
    serlogF3(SER_DEBUG, "Text: ", (int)textBuffer[currPos], currPos);
    canvas.fillScreen(0);
    unicodeHelper.setCursor(0,7);
    unicodeHelper.setDrawColor(1);
    unicodeHelper.setFont(OpenSansRegular7pt);
    unicodeHelper.print(textBuffer[currPos]);
    drawCanvasToLeds();
}

// This actually maps every pixel on the matrix to the canvas.
void DiscoTime::drawCanvasToLeds() {
    uint32_t output[3] = {0};
    int position = 0;
    for(int row=0; row<8; row++) {
        for(int col=0; col<12; col++) {
            auto pix = canvas.getPixel(col, row);
            if(pix) {
                output[position / 32] |= (1 << ((31 - (position % 32))));
                // enable only for debugging
                //serlogF4(SER_DEBUG, "Pix/y pos on ", col, row, position);
            }
            position++;
        }
    }
    // enable only for debugging
    //serlogHexDump(SER_DEBUG, "Output=", output, 8);
    matrix.loadFrame(output);
}

void DiscoTime::runStrobe() {
    auto col = (currPos % 2 == 1);
    canvas.fillScreen(col);
    drawCanvasToLeds();

    if(loopTimer++ > 100) switchMode(SQUARES);
}

void DiscoTime::runCircles() {
    canvas.fillScreen(0);
    int rad = currPos % 7;
    if(currPos > 36) {
        canvas.fillCircle(6, 4, rad, 1);
    } else {
        canvas.drawCircle(6, 4, rad, 1);
    }
    drawCanvasToLeds();
    if(loopTimer++ > 100) switchMode(STROBE);
}

void DiscoTime::runSquares() {
    canvas.fillScreen(0);
    int rad = currPos % 12;
    if(currPos > 36) {
        canvas.fillRect(0, 0, rad + 1, rad + 1, 1);
    } else {
        canvas.drawRect(0, 0, rad + 1, rad + 1, 1);
    }
    drawCanvasToLeds();
    if(loopTimer++ > 100) switchMode(CIRCLES);
}

void DiscoTime::switchMode(DiscoMode m) {
    mode = m;
    loopTimer = currPos = 0;
}

DiscoTime::DiscoTime() : canvas(12, 8), textBuffer{}, unicodeHelper(newAdafruitTextPipeline(&canvas), ENCMODE_UTF8) {
}

//
// ADVANCED
// Here is how we send a custom tag val message to the remote server, we call the `encodeCustomTagValMessage` method
// providing a lambda, we only get into the lambda if the connection is made, and we then just write the fields we
// need to write.
// For the second case of binary message, we simply encode the binary data within the lambda callback instead of
// tag val fields.
// https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/tcmenu-iot/embed-control-tagval-wire-protocol/
//
void DiscoTime::sendDiscoMsg() {
    auto tagValConnector = remoteServer.getRemoteConnector(0);
    tagValConnector->encodeCustomTagValMessage(MY_CUSTOM_DISCO_MSG, [](TagValueTransport* transport, void* data) {
        auto myDisco = (DiscoTime*)data;
        transport->writeField(MY_CUSTOM_DISCO_NAME_FIELD, "Disco");
        transport->writeFieldInt(MY_CUSTOM_DISCO_STATUS_FIELD, myDisco->discoIsRunning);
    }, this);
}

void DiscoTime::sendXbmpData(const uint8_t* xbmpData, size_t length) {
    auto tagValConnector = remoteServer.getRemoteConnector(0);
    tagValConnector->encodeCustomBinaryMessage(MY_CUSTOM_XBMP_MSG, length, [](TagValueTransport* transport, void* data, size_t len) {
        auto xbmpData = (uint8_t*)data;
        for(auto i=0; i< len; ++i) {
            transport->writeChar(xbmpData[i]);
        }
    }, (void*)xbmpData);


}
