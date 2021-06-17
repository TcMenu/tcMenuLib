/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "BaseBufferedRemoteTransport.h"

using namespace tcremote;

BaseBufferedRemoteTransport::BaseBufferedRemoteTransport(BufferingMode bufferMode, int8_t readBufferSize, int writeBufferSize)
        : writeBufferSize(writeBufferSize), writeBufferPos(0), readBufferSize(readBufferSize), readBufferPos(0),
          readBufferAvail(0), mode(bufferMode) {
    readBuffer = new uint8_t[readBufferSize];
    writeBuffer = new uint8_t[writeBufferSize];
}

BaseBufferedRemoteTransport::~BaseBufferedRemoteTransport() {
    delete[] readBuffer;
    delete[] writeBuffer;
}

void BaseBufferedRemoteTransport::endMsg() {
    TagValueTransport::endMsg();
    if(mode == BUFFER_ONE_MESSAGE) flush();
}

uint8_t BaseBufferedRemoteTransport::readByte() {
    if(!readAvailable()) return -1;
    auto ch = readBuffer[readBufferPos];
    readBufferPos += 1;
    // only uncomment the below for debugging.
    serdebugF2("readByte ", ch);
    return ch;
}

bool BaseBufferedRemoteTransport::readAvailable() {
    if(readBufferAvail && readBufferPos < readBufferAvail) {
        serdebugF("avl");
        return true;
    }

    readBufferAvail = (int8_t)fillReadBuffer(readBuffer, readBufferSize);
    return readBufferPos < readBufferAvail;
}

int BaseBufferedRemoteTransport::writeChar(char data) {
    if(writeBufferPos >= writeBufferSize) {
        flush(); // exceeded the buffer, must try to flush now.
        if (writeBufferPos != 0) return 0;
    }
    writeBuffer[writeBufferPos++] = data;
    return 1;
}

int BaseBufferedRemoteTransport::writeStr(const char *data) {
    // only uncomment below for worst case debugging..
    //	serdebug2("writing ", data);

    size_t len = strlen(data);
    for(size_t i = 0; i < len; ++i) {
        if(writeChar(data[i]) == 0) {
            return 0;
        }
    }
    return (int)len;
}

void BaseBufferedRemoteTransport::close() {
    writeBufferPos = 0;
    readBufferPos = 0;
    readBufferAvail = 0;
}
