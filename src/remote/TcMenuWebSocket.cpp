/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>
#include "TcMenuWebSocket.h"

// byte 1
#define WS_FIN              0
#define WS_RSV1             1
#define WS_RSV2             2
#define WS_RSV3             3
#define WS_OPCODE_STRT      4
#define WS_OPCODE_MASK      0x0f

// byte 2
#define WS_MASK             7
#define WS_EXTENDED_PAYLOAD 126
#define WS_FAIL_PAYLOAD_LEN 127

/**
 * When a websocket is upgraded from HTTP to WS protocol, it must answer with a security key, this is made up of the
 * key provided, this UUID, turned into a sha1 and base64 encoded.
 */
const char pgmWebSockUuid[] PROGMEM = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
/**
 * This is the HTTP message to allow upgrade to the WS protocol, up to the security key which will be appended.
 */
const char pgmWebSockHeader[] PROGMEM = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: ";
/**
 * This comes after the security key in the response header, it is the EOL and blank line needed to terminate the HTTP side.
 */
const char pgmWebSockHeaderPostfix[] PROGMEM = "\r\n\r\n";

namespace tc_b64 {
    /**
     * A lightweight base 64 facility that takes in some data and writes out base 64 into the buffer
     * @param data input data
     * @param dataSize input data size
     * @param buffer the buffer to write to
     * @param bufferSize buffer size
     * @return the number of bytes written.
     */
    int base64(const uint8_t *data, int dataSize, uint8_t *buffer, int bufferSize);
}

// See the associated C file in this directory
extern "C" {
int sha1digest(uint32_t *hexDigest, const uint8_t *data, size_t databytes);
}

using namespace tcremote;

// initialisation class

char AbstractWebSocketTcMenuInitialisation::readCharFromTransport() {
    while(transport->connected()) {
        if(transport->readAvailable()) {
            return transport->readByte();
        } else {
            taskManager.yieldForMicros(100);
        }
    }
    return 0;
}

bool AbstractWebSocketTcMenuInitialisation::readWordUntilTrim(char* buffer, size_t bufferSize) {
    unsigned int pos = 0;
    bool wordRead = false;
    bool endOfLine = false;
    buffer[0] = 0;

    while(pos < bufferSize && !wordRead && transport->connected()) {
        auto ch = readCharFromTransport();
        if(ch == ':') wordRead = true;
        else if(ch == '\n') {
            wordRead = true;
            endOfLine = true;
        } else if(ch == '\r') {
            // ignore the \r, we are supposed to read \r\n
        } else {
            buffer[pos] = ch;
        }
        pos++;
    }

    buffer[min(pos, bufferSize -1)] = 0;
    return endOfLine;
}

WebSocketHeader AbstractWebSocketTcMenuInitialisation::processHeader(char* buffer, size_t bufferSize) {
    if(readWordUntilTrim(buffer, bufferSize)) return WSH_FINISHED;
    if(strncmp(buffer, "GET ", 4) == 0) {
        bool eol = readWordUntilTrim(buffer, bufferSize);
        if(!strcmp_P(buffer, expectedPath)) return WSH_ERROR;
        if(!eol) readWordUntilTrim(buffer, bufferSize);
        return WSH_GET;
    } else {
        if(readWordUntilTrim(buffer, bufferSize)) return WSH_ERROR;
        if(strcmp(buffer, "Upgrade")==0) {
            if(!readWordUntilTrim(buffer, bufferSize)) return WSH_ERROR;
            if(strcmp(buffer, "websocket") == 0) {
                return WSH_UPGRADE_TO_WEBSOCKET;
            }
        } else if(strcmp(buffer, "Connection") == 0) {
            readWordUntilTrim(buffer, bufferSize);
            if(strcmp(buffer, "Upgrade") == 0) {
                return WSH_UPGRADE_TO_WEBSOCKET;
            }
        } else if(strcmp(buffer, "Sec-WebSocket-Key") == 0) {
            if(!readWordUntilTrim(buffer, bufferSize)) return WSH_ERROR;
            return WSH_SEC_WS_KEY;
        }
    }
    return WSH_UNPROCESSED;
}

void writePgmStrToTransport(TagValueTransport* transport, const char* pgmData) {
    while(*pgmData) {
        transport->writeChar(*pgmData);
        pgmData++;
    }
}

bool AbstractWebSocketTcMenuInitialisation::performUpgradeOnClient(AbstractWebSocketTcMenuTransport* t) {
    this->transport = t;
    this->transport->setState(WSS_UPGRADING);
    bool reachedEndOfHeader = false;
    bool getReceivedOK = false;
    bool hasUpgraded = false;
    char sz[64];
    char sha1Space[32];
    while(!reachedEndOfHeader && transport->connected()) {
        auto hdrName = processHeader(sz, sizeof sz);
        if(hdrName == WSH_ERROR) {
            reachedEndOfHeader = true;
        } else if(hdrName == WSH_UPGRADE_TO_WEBSOCKET) {
            hasUpgraded = true;
        } else if(hdrName == WSH_GET) {
            getReceivedOK = true;
        } else if(hdrName == WSH_FINISHED) {
            reachedEndOfHeader = true;
        } else if(hdrName == WSH_SEC_WS_KEY) {
            unsigned long shaRaw[5];
            strncat_P(sz, pgmWebSockUuid, sizeof(sz) - strlen(sz) - 1);
            sha1digest(shaRaw, (uint8_t*)sz, strlen(sz));
            tc_b64::base64((const uint8_t*)shaRaw, sizeof(shaRaw), (uint8_t*)sha1Space, sizeof sha1Space);
        }
    }

    if(transport->connected() && reachedEndOfHeader && getReceivedOK && hasUpgraded) {
        writePgmStrToTransport(transport, pgmWebSockHeader);
        transport->writeStr(sha1Space);
        writePgmStrToTransport(transport, pgmWebSockHeaderPostfix);
        transport->flush();
        this->transport->setState(WSS_IDLE);
        return true;
    } else {
        transport->close();
        return false;
    }
}

namespace tc_b64 {
    const char *b64Dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    void innerBase64(const uint8_t *data, int size, uint8_t *buffer) {
        if (size == 3) {
            buffer[0] = b64Dictionary[data[0] >> 2];
            buffer[1] = b64Dictionary[(data[0] & 0x3) << 4 | (data[1] >> 4)];
            buffer[2] = b64Dictionary[(data[1] & 0x0F) << 2 | (data[2] >> 6)];
            buffer[3] = b64Dictionary[data[2] & 0x3F];
        } else if (size == 2) {
            buffer[0] = b64Dictionary[data[0] >> 2];
            buffer[1] = b64Dictionary[(data[0] & 0x3) << 4 | (data[1] >> 4)];
            buffer[2] = b64Dictionary[(data[1] & 0x0F) << 2];
            buffer[3] = '=';
        } else if (size == 1) {
            buffer[0] = b64Dictionary[data[0] >> 2];
            buffer[1] = b64Dictionary[(data[0] & 0x3) << 4];
            buffer[2] = '=';
            buffer[3] = '=';
        }
    }

    int base64(const uint8_t *data, int dataSize, uint8_t *buffer, int bufferSize) {
        // If we get here we've got enough space to do the encoding

        int writtenBytes = 0;
        // Break the input into 3-byte chunks and process each of them
        int i;
        for (i = 0; i < dataSize / 3; i++) {
            writtenBytes += 4;
            if(writtenBytes >= bufferSize) return -1;
            innerBase64(&data[i * 3], 3, &buffer[i * 4]);
        }
        if (dataSize % 3 > 0) {
            // It doesn't fit neatly into a 3-byte chunk, so process what's left
            innerBase64(&data[i * 3], dataSize % 3, &buffer[i * 4]);
        }

        writtenBytes++;
        if(writtenBytes < bufferSize) {
            buffer[writtenBytes] = 0;
        }
        return writtenBytes;
    }

}


inline WebSocketOpcode getOpcode(uint8_t header) {
    return (WebSocketOpcode)((header >> WS_OPCODE_STRT) & WS_OPCODE_MASK);
}

void AbstractWebSocketTcMenuTransport::close() {
    uint8_t sz[2];
    sendMessageOnWire(OPC_CLOSE, sz, 0);
    bytesLeftInCurrentMsg = 0;
    frameMaskingPosition = 0;
    writePosition = 0;
    readAvailable = 0;
    readPosition = 0;
    currentState = WSS_NOT_CONNECTED;
}

uint8_t AbstractWebSocketTcMenuTransport::readByte() {
    if(currentState == WSS_UPGRADING) {
        return readRawByte();
    } else {
        auto data = readBuffer[readPosition] ^ frameMask[frameMaskingPosition];
        readPosition++;
        bytesLeftInCurrentMsg--;
        frameMaskingPosition = (frameMaskingPosition + 1) % 4;
        return data;
    }
}

bool AbstractWebSocketTcMenuTransport::available() {
    if(currentState == WSS_UPGRADING) {
        return rawAvailable();
    }
    else if(bytesLeftInCurrentMsg > 0) {
        if(readPosition >= readAvailable) {
            // here we read but not past the end of the current message, each message has new framing
            readAvailable = performRawRead(readBuffer, bytesLeftInCurrentMsg);
            readPosition = 0;
            return readPosition < readAvailable;
        }
    }
    else {
        // we need to read a new frame

        readAvailable = performRawRead(&readBuffer[readPosition], WS_BUFFER_SIZE - readPosition);
        readPosition = 0;

        if(!rawAvailable()) return false;
        uint8_t flags = readRawByte();
        bsize_t overallLength;
        uint8_t len = readRawByte();
        if(!bitRead(len, WS_MASK)) {
            close();
             return false;
        }
        len = len & 0x7f;
        currentState = WSS_FLAGS_READ;

            size_t offset;
            if(len == WS_FAIL_PAYLOAD_LEN) return 0;
            if(len == WS_EXTENDED_PAYLOAD) {
                uint8_t extLenHi = readByte();
                uint8_t extLenLo = readByte();
                overallLength = ((size_t)extLenHi << 8U) | extLenLo;
                offset = 2;
            } else {
                overallLength = len;
                offset = 4;
            }

            // we must be masked as it is a read op
            for(int i=0;i<4;i++) {
                frameMask[i]=readByte();
            }

            bytesLeftInCurrentMsg = overallLength - offset;
            frameMaskingPosition = 0;

            auto op = getOpcode(flags);

            if(op == OPC_PING) {
                uint8_t sz[2];
                sendMessageOnWire(OPC_PONG, sz, 0);
            }
            else if(op == OPC_CLOSE) {
                close();
                return 0;
            }


            return overallLength;
        }

        return data;
    }
}

int AbstractWebSocketTcMenuTransport::writeChar(char data) {
    if(writePosition >= (WS_BUFFER_SIZE - 2)) {
        // we've exceeded the buffer size so we must flush, and then ensure
        // that flush actually did something and there is now capacity.
        flush();
        if(writePosition != 0) return 0;// we did not write so return an error condition.
    }
    writeBuffer[writePosition + 2] = data;
    writePosition++;
    return 1;
}

int AbstractWebSocketTcMenuTransport::writeStr(const char *data) {
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

void AbstractWebSocketTcMenuTransport::flush() {
    if(writePosition == 0) return;

    sendMessageOnWire(OPC_TEXT, writeBuffer, writePosition);
    serdebugF2("Buffer written ", writePosition);
    writePosition = 0;
}

void AbstractWebSocketTcMenuTransport::sendMessageOnWire(WebSocketOpcode opcode, uint8_t* buffer, size_t size) {
    buffer[0] = (uint8_t)(WS_FIN | (opcode << 4));
    buffer[1] = (uint8_t)size;
    performRawWrite(buffer, size + 2);
}

void AbstractWebSocketTcMenuTransport::endMsg() {
    TagValueTransport::endMsg();
    flush();
}

uint8_t AbstractWebSocketTcMenuTransport::readRawByte() {
    uint8_t sz[1];
    performRawRead(sz, 1);
    return sz[0];
}
