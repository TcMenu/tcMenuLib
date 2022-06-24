/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <Arduino.h>
#include "TcMenuWebSocket.h"

// byte 1
#define WS_FIN              0x80
#define WS_RSV1             6
#define WS_RSV2             5
#define WS_RSV3             4
#define WS_OPCODE_MASK      0x0f

// byte 2
#define WS_MASKED_PAYLOAD   7
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
//                                              0123456789012345678901234567890123456
const char pgmWebSockHeaderSwitch[] PROGMEM =  "HTTP/1.1 101 Switching Protocols\r\n";
const char pgmWebSockHeaderUpgrade[] PROGMEM = "Upgrade: websocket\r\nConnection: Upgrade\r\n";
const char pgmWsAcceptHeaderStart[] = "Sec-WebSocket-Accept: ";
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
int sha1digest(uint8_t *hexDigest, const uint8_t *data, size_t databytes);
}

using namespace tcremote;

// initialisation class

char AbstractWebSocketTcMenuInitialisation::readCharFromTransport() {
    while(transport->connected()) {
        if(transport->available()) {
            return transport->readByte();
        } else {
            taskManager.yieldForMicros(100);
        }
    }
    return 0;
}

bool AbstractWebSocketTcMenuInitialisation::readWordUntilTrim(char* buffer, size_t bufferSize, bool skipSeparator) {
    unsigned int pos = 0;
    bool wordRead = false;
    bool trimming = true;
    bool endOfLine = false;
    buffer[0] = 0;

    while(pos < bufferSize && !wordRead && transport->connected()) {
        auto ch = readCharFromTransport();
        if(trimming && ch == ' ') continue;
        if(!skipSeparator && (ch == ':' || ch == ' ')) wordRead = true;
        else if(ch == '\n') {
            wordRead = true;
            endOfLine = true;
        } else if(ch == '\r') {
            // ignore the \r, we are supposed to read \r\n
        } else {
            trimming = false;
            buffer[pos] = ch;
            pos++;
        }
    }

    buffer[min(pos, bufferSize -1)] = 0;
    return endOfLine;
}

WebSocketHeader AbstractWebSocketTcMenuInitialisation::processHeader(char* buffer, size_t bufferSize) {
    if(readWordUntilTrim(buffer, bufferSize)) return WSH_FINISHED;
    if(strcmp(buffer, "GET") == 0) {
        bool eol = readWordUntilTrim(buffer, bufferSize);
        if(strcmp_P(buffer, expectedPath)!=0) return WSH_ERROR;
        if(!eol) readWordUntilTrim(buffer, bufferSize, true); // and chew up the protocol if needed.
        return WSH_GET;
    } else {
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
            if(!readWordUntilTrim(buffer, bufferSize, true)) return WSH_ERROR;
            return WSH_SEC_WS_KEY;
        } else {
            readWordUntilTrim(buffer, bufferSize, true);
        }
    }
    return WSH_UNPROCESSED;
}

void writePgmStrToTransport(AbstractWebSocketTcMenuTransport* transport, const char* pgmData) {
    char sz[42];
    strcpy_P(sz, pgmData);
    transport->performRawWrite((uint8_t*)sz, strlen(sz));
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
        switch(hdrName) {
        case WSH_ERROR:
            reachedEndOfHeader = true;
            break;
        case WSH_UPGRADE_TO_WEBSOCKET:
            hasUpgraded = true;
            break;
        case WSH_GET:
            getReceivedOK = true;
            break;
        case WSH_FINISHED:
            reachedEndOfHeader = true;
            break;
        case WSH_SEC_WS_KEY: {
            uint8_t shaRaw[20];
            strncat_P(sz, pgmWebSockUuid, sizeof(sz) - strlen(sz) - 1);
            sha1digest(shaRaw, (uint8_t *) sz, strlen(sz));
            tc_b64::base64((const uint8_t *) shaRaw, sizeof(shaRaw), (uint8_t *) sha1Space, sizeof sha1Space);
            break;
        }
        }
    }

    if(transport->connected() && reachedEndOfHeader && getReceivedOK && hasUpgraded) {
        writePgmStrToTransport(transport, pgmWebSockHeaderSwitch);
        writePgmStrToTransport(transport, pgmWebSockHeaderUpgrade);
        writePgmStrToTransport(transport, pgmWsAcceptHeaderStart);
        transport->performRawWrite((uint8_t*)sha1Space, strlen(sha1Space));
        writePgmStrToTransport(transport, pgmWebSockHeaderPostfix);
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
            writtenBytes += 4;
            // It doesn't fit neatly into a 3-byte chunk, so process what's left
            innerBase64(&data[i * 3], dataSize % 3, &buffer[i * 4]);
        }

        if(writtenBytes < bufferSize) {
            buffer[writtenBytes] = 0;
        }
        return writtenBytes;
    }
}

inline WebSocketOpcode getOpcode(uint8_t header) {
    return (WebSocketOpcode)(header & WS_OPCODE_MASK);
}

void AbstractWebSocketTcMenuTransport::close() {
    uint8_t sz[2];
    sendMessageOnWire(OPC_CLOSE, sz, 0);
    bytesLeftInCurrentMsg = 0;
    frameMaskingPosition = 0;
    writePosition = 0;
    readAvail = 0;
    readPosition = 0;
    currentState = WSS_NOT_CONNECTED;
}

bool AbstractWebSocketTcMenuTransport::readAvailable() {
    // short circuit when there's room in the buffer already.
    if(readPosition < readAvail && currentState == WSS_PROCESSING_MSG) return true;

    bool processing = true;
    while(processing) {
        switch (currentState) {
            case WSS_PROCESSING_MSG:
                if(bytesLeftInCurrentMsg > 0) {
                    readAvail = performRawRead(readBuffer, bytesLeftInCurrentMsg);
                    readPosition = 0;
                    if(readAvail > 0) return true;
                }
                setState(WSS_IDLE); // we are now idle and trying to read the two byte frame
                readPosition = 0;
                processing = false;
                break;
            case WSS_IDLE:
            case WSS_LEN_READ: {
                auto actual = performRawRead(&readBuffer[readPosition], readPosition == 0 ? 2 : 1);
                readPosition += actual;
                if (readPosition < 2) return false;
                setState(WSS_LEN_READ);
                int len = readBuffer[1] & 0x7f;
                if (len == WS_FAIL_PAYLOAD_LEN) {
                    close();
                    processing = false;
                } else if (len == WS_EXTENDED_PAYLOAD) {
                    setState(WSS_EXT_LEN_READ);
                    processing = true;
                } else if (len > 0) {
                    bytesLeftInCurrentMsg = len;
                    setState(WSS_MASK_READ);
                }
                if ((readBuffer[1] & 0x80) == 0) return false;
                break;
            }
            case WSS_EXT_LEN_READ: {
                auto actual = performRawRead(&readBuffer[readPosition], readPosition == 2 ? 2 : 1);
                readPosition += actual;
                if (readPosition < 4) return false;
                bytesLeftInCurrentMsg = readBuffer[2] << 8;
                bytesLeftInCurrentMsg |= readBuffer[3];
                setState(WSS_MASK_READ);
                processing = true;
                break;
            }
            case WSS_MASK_READ: {
                int start = (bytesLeftInCurrentMsg > 125) ? 4 : 2;
                auto actual = performRawRead(&readBuffer[readPosition], (start + 4) - (readPosition));
                readPosition += actual;
                frameMask[0] = readBuffer[start];
                frameMask[1] = readBuffer[start + 1];
                frameMask[2] = readBuffer[start + 2];
                frameMask[3] = readBuffer[start + 3];
                frameMaskingPosition = 0;
                setState(WSS_PROCESSING_MSG);
                processing = true;
                break;
            }
            default:
                processing = false;
                break;
        }
    }
}

uint8_t AbstractWebSocketTcMenuTransport::readByte() {
    if(currentState == WSS_UPGRADING) {
        uint8_t sz[1];
        performRawRead(sz, 1);
        return sz[0];
    }
    else if(readPosition < readAvail && currentState == WSS_PROCESSING_MSG) {
        auto data = readBuffer[readPosition] ^ frameMask[frameMaskingPosition];
        readPosition++;
        bytesLeftInCurrentMsg--;
        frameMaskingPosition = (frameMaskingPosition + 1) % 4;
        return data;
    }
    else return 0xff; // fault. called without checking readAvailable
}

int AbstractWebSocketTcMenuTransport::writeChar(char data) {
    if(writePosition >= (bufferSize - 2)) {
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
    buffer[0] = (uint8_t)(WS_FIN | opcode);
    buffer[1] = (uint8_t)size;
    performRawWrite(buffer, size + 2);
}

void AbstractWebSocketTcMenuTransport::endMsg() {
    TagValueTransport::endMsg();
    flush();
}
