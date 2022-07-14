/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <PlatformDetermination.h>
#include "TcMenuWebServer.h"
#include "TcMenuHttpRequestProcessor.h"

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


bool AbstractWebSocketTcMenuInitialisation::processRequestLine(AbstractWebSocketTcMenuTransport *t) {
    this->transport = t;
    this->transport->setState(WSS_HTTP_REQUEST);
    this->processor.setTransport(transport);
    this->response.setTransport(transport);

    char* buffer = (char*)transport->getReadBuffer();
    size_t bufferSize = transport->getReadBufferSize();

    WebServerMethod method = processor.processRequest(buffer, bufferSize);
    if(method == REQ_ERROR || method == REQ_NONE) {
        response.startHeader(WS_INT_RESPONSE_INT_ERR, "Bad request");
        response.contentInfo(WebServerResponse::PLAIN_TEXT, 0);
        response.end();
        response.closeConnection();
        return false;
    }

    if(strcmp(buffer, expectedPath) != 0) {
        response.startHeader(WS_INT_RESPONSE_NOT_FOUND, WS_TEXT_RESPONSE_NOT_FOUND);
        response.contentInfo(WebServerResponse::PLAIN_TEXT, 0);
        response.end();
        response.closeConnection();
        return false;
    }

    return true;
}

bool AbstractWebSocketTcMenuInitialisation::performUpgradeOnClient(AbstractWebSocketTcMenuTransport* t) {
    this->transport = t;
    processor.setTransport(t);
    response.setTransport(t);
    transport->setState(WSS_HTTP_REQUEST);
    bool reachedEndOfHeader = false;
    bool hasUpgraded = false;
    char sha1Space[32];
    char* buffer = (char*)transport->getReadBuffer();
    size_t bufferSize = transport->getReadBufferSize();

    while(!reachedEndOfHeader && transport->connected()) {
        auto hdrName = processor.processHeader((char*)transport->getReadBuffer(), transport->getReadBufferSize());
        switch(hdrName) {
        case WSH_ERROR:
            reachedEndOfHeader = true;
            break;
        case WSH_UPGRADE_TO_WEBSOCKET:
            hasUpgraded = true;
            break;
        case WSH_FINISHED:
            reachedEndOfHeader = true;
            break;
        case WSH_SEC_WS_KEY: {
            uint8_t shaRaw[20];
            strncat_P(buffer, pgmWebSockUuid, bufferSize - strlen(buffer) - 1);
            sha1digest(shaRaw, (uint8_t*)buffer, strlen(buffer));
            tc_b64::base64(shaRaw, sizeof(shaRaw), (uint8_t *) sha1Space, sizeof sha1Space);
            break;
        }
        default:
            break;
        }
    }

    if(transport->connected() && reachedEndOfHeader && hasUpgraded) {
        response.setTransport(transport);
        response.startHeader(WS_CODE_CHANGING_PROTOCOL, "Switching Protocols");
        response.setHeader(WSH_UPGRADE_TO_WEBSOCKET, "websocket");
        response.setHeader(WSH_CONNECTION, "Upgrade");
        response.setHeader(WSH_SEC_WS_ACCEPT_KEY, sha1Space);
        response.end();
        this->transport->setState(WSS_IDLE);
        return true;
    } else {
        response.closeConnection();
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
    return false;
}

uint8_t AbstractWebSocketTcMenuTransport::readByte() {
    if(currentState == WSS_HTTP_REQUEST) {
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

// ------------ Web server

AbstractLightweightWebServer::AbstractLightweightWebServer(): socketInitialised(false) {}

void AbstractLightweightWebServer::init() {
    taskManager.registerEvent(this);
}

void AbstractLightweightWebServer::exec() {
    if(!socketInitialised) {
        if(areWeConnected()) {
            initialiseConnection();
            socketInitialised = true;
        }
    } else if(response.getMode() == WebServerResponse::TRANSPORT_ASSIGNED) {
        bool needAnotherGo = true;
        while(needAnotherGo) {
            auto method = response.processRequestLine();
            if(method == POST || method == GET) {
                response.setMode(WebServerResponse::READING_HEADERS);
                needAnotherGo = attemptToHandleRequest(method);
                if(!needAnotherGo) response.closeConnection();
            } else if(method == REQ_ERROR){
                needAnotherGo = false;
                sendErrorCode(WS_INT_RESPONSE_INT_ERR);
                response.closeConnection();
            } else if(method == REQ_NONE && response.hasErrorOccurred()) {
                needAnotherGo = false;
                response.closeConnection();
            } else {
                needAnotherGo = false;
            }
        }
    }
}

bool AbstractLightweightWebServer::attemptToHandleRequest(WebServerMethod &method) {
    for(auto urlWithHandler : urlHandlers) {
        if(urlWithHandler.isRequestCompatible(response.getLastData(), method)) {
            if(response.processHeaders()) {
                urlWithHandler.handleUrl(response);
                if (response.getMode() != WebServerResponse::NOT_IN_USE) {
                    response.end();
                }
                return true;
            } else {
                sendErrorCode(WS_INT_RESPONSE_INT_ERR);
                return false;
            }
        }
    }
    sendErrorCode(WS_INT_RESPONSE_NOT_FOUND);
    return false;
}

uint32_t AbstractLightweightWebServer::timeOfNextCheck() {
    if(!socketInitialised) {
        markTriggeredAndNotify();
        return millisToMicros(10);
    }
    if(response.getMode() != WebServerResponse::NOT_IN_USE) {
        markTriggeredAndNotify();
        return millisToMicros(1);
    } else {
        AbstractWebSocketTcMenuTransport *pTransport = attemptNewConnection();
        if(pTransport) {
            response.setTransport(pTransport);
            markTriggeredAndNotify();
            return millisToMicros(1);
        }
        return millisToMicros(100);
    }
}

void AbstractLightweightWebServer::sendErrorCode(int errorCode) {
    if(errorCode == WS_INT_RESPONSE_NOT_FOUND) {
        response.startHeader(WS_INT_RESPONSE_NOT_FOUND, WS_TEXT_RESPONSE_NOT_FOUND);
        response.contentInfo(WebServerResponse::PLAIN_TEXT, 0);
        response.end();
    } else {
        response.startHeader(WS_INT_RESPONSE_INT_ERR, "Internal error");
        response.contentInfo(WebServerResponse::PLAIN_TEXT, 0);
        response.end();

    }
}
