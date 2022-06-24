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

char HttpProcessor::readCharFromTransport() {
    while(transport->connected()) {
        if(transport->available()) {
            return transport->readByte();
        } else {
#ifndef TC_DEBUG_SOCKET_LAYER
            if((millis() - millisStart) > 2000) {
                protocolError = true;
                return -1;
            }
#endif
            taskManager.yieldForMicros(1000);
        }
    }
    return 0;
}

bool HttpProcessor::readWordUntilTrim(char* buffer, size_t bufferSize, bool skipSeparator) {
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

WebServerHeader HttpProcessor::processHeader(char* buffer, size_t bufferSize) {
    if(readWordUntilTrim(buffer, bufferSize)) return WSH_FINISHED;
    bool getRequest = strcmp(buffer, "GET") == 0;
    bool postRequest = strcmp(buffer, "POST") == 0;
    if(getRequest || postRequest) {
        if(readWordUntilTrim(buffer, bufferSize)) return WSH_ERROR; // missing HTTP/1.1
        char sz[10];
        if(!readWordUntilTrim(sz, sizeof sz, true)) return WSH_ERROR; // protocol eg HTTP*
        if(protocolError || strncmp(sz, "HTTP", 4) != 0) return WSH_ERROR;
        return getRequest ? WSH_GET : WSH_POST;
    } else {
        if(strcmp(buffer, "Host") == 0) {
            if(!readWordUntilTrim(buffer, bufferSize)) return WSH_ERROR;
            if(protocolError) return WSH_ERROR;
            return WSH_HOST;
        }
        if(strcmp(buffer, "User-Agent") == 0) {
            if(!readWordUntilTrim(buffer, bufferSize)) return WSH_ERROR;
            if(protocolError) return WSH_ERROR;
            return WSH_USER_AGENT;
        }
        if(strcmp(buffer, "Upgrade")==0) {
            if(!readWordUntilTrim(buffer, bufferSize)) return WSH_ERROR;
            if(protocolError) return WSH_ERROR;
            if(strcmp(buffer, "websocket") == 0) {
                return WSH_UPGRADE_TO_WEBSOCKET;
            }
        } else if(strcmp(buffer, "Connection") == 0) {
            readWordUntilTrim(buffer, bufferSize);
            if(protocolError) return WSH_ERROR;
            if(strcmp(buffer, "Upgrade") == 0) {
                return WSH_UPGRADE_TO_WEBSOCKET;
            }
        } else if(strcmp(buffer, "Sec-WebSocket-Key") == 0) {
            if(!readWordUntilTrim(buffer, bufferSize, true)) return WSH_ERROR;
            if(protocolError) return WSH_ERROR;
            return WSH_SEC_WS_KEY;
        } else {
            readWordUntilTrim(buffer, bufferSize, true);
            if(protocolError) return WSH_ERROR;
        }
    }
    return WSH_UNPROCESSED;
}

void HttpProcessor::setTransport(AbstractWebSocketTcMenuTransport *tx) {
    transport = tx;
    millisStart = millis();
    protocolError = false;
}

void writePgmStrToTransport(AbstractWebSocketTcMenuTransport* transport, const char* pgmData) {
    char sz[42];
    strcpy_P(sz, pgmData);
    transport->performRawWrite((uint8_t*)sz, strlen(sz));
}

bool AbstractWebSocketTcMenuInitialisation::performUpgradeOnClient(AbstractWebSocketTcMenuTransport* t) {
    this->transport = t;
    processor.setTransport(t);
    this->transport->setState(WSS_HTTP_REQUEST);
    bool reachedEndOfHeader = false;
    bool getReceivedOK = false;
    bool hasUpgraded = false;
    char sz[64];
    char sha1Space[32];
    while(!reachedEndOfHeader && transport->connected()) {
        auto hdrName = processor.processHeader(sz, sizeof sz);

        switch(hdrName) {
        case WSH_ERROR:
            reachedEndOfHeader = true;
            break;
        case WSH_UPGRADE_TO_WEBSOCKET:
            hasUpgraded = true;
            break;
        case WSH_GET:
            getReceivedOK = strcmp_P(sz, expectedPath)==0;
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

AbstractLightweightWebServer::AbstractLightweightWebServer() {}

void AbstractLightweightWebServer::init() {
    taskManager.registerEvent(this);
}

void AbstractLightweightWebServer::exec() {
    if(response.getMode() == WebServerResponse::TRANSPORT_ASSIGNED) {
        response.setMode(WebServerResponse::READING_HEADERS);
        if (!response.processHeaders()) return;
        findAndAssociateUrl(response.getPath(), response.getMethod());
    }
}

uint32_t AbstractLightweightWebServer::timeOfNextCheck() {
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

void AbstractLightweightWebServer::findAndAssociateUrl(const char* path, WebServerMethod method) {
    for(auto urlWithHandler : urlHandlers) {
        if(urlWithHandler.isRequestCompatible(path, method)) {
            urlWithHandler.handleUrl(response);
            if(response.getMode() != WebServerResponse::NOT_IN_USE) {
                response.end();
            }
            return;
        }
    }

    // we didn't find a handler...

    response.startHeader(WS_INT_RESPONSE_NOT_FOUND, WS_TEXT_RESPONSE_NOT_FOUND);
    response.contentInfo(WebServerResponse::PLAIN_TEXT, 0);
    response.end();

}

void WebServerResponse::contentInfo(WSRContentType contentType, size_t len) {
    const char *headerText;
    switch(contentType) {
        case WebServerResponse::HTML_TEXT:
            headerText = "text/html";
            break;
        case WebServerResponse::PNG_IMAGE:
            headerText = "image/png";
            break;
        case WebServerResponse::JPG_IMAGE:
            headerText = "image/jpeg";
            break;
        case WebServerResponse::JSON_TEXT:
            headerText = "application/json";
            break;
        default:
            headerText = "text/plain";
            break;
    }
    setHeader(WSH_CONTENT_TYPE, headerText);

    char sz[10];
    itoa((int)len, sz, 10);
    setHeader(WSH_CONTENT_LENGTH, sz);
}

void WebServerResponse::startHeader(int code, const char* textualInfo) {
    mode = PREPARING_HEADER;
    uint8_t* dataArea = transport->getReadBuffer();
    size_t buffSize = transport->getReadBufferSize();
    strcpy((char*)dataArea, "HTTP/1.1 ");
    char sz[32];
    itoa(code, sz, 10);
    strcat((char*)dataArea, sz);
    appendChar((char*)dataArea, ' ', buffSize);
    strcat((char*)dataArea, textualInfo);
    strcat((char*)dataArea, "\r\n");
    transport->performRawWrite(dataArea, strlen((char*)dataArea));
    setHeader(WSH_SERVER, WS_SERVER_NAME);
#if defined(WS_RTC_INTEGRATED)
    rtcUTCDateInWebForm(sz, sizeof sz);
    setHeader(WSH_DATE, sz)
#endif
}

const char* getHeaderAsText(WebServerHeader header) {
    switch(header) {
        case WebServerHeader::WSH_SERVER: return "Server: ";
        case WebServerHeader::WSH_CONTENT_TYPE: return "Content-Type: ";
        case WebServerHeader::WSH_CONTENT_LENGTH: return "Content-Length: ";
        case WebServerHeader::WSH_DATE: return "Date: ";
        case WebServerHeader::WSH_LAST_MODIFIED: return "Last-Modified: ";
        case WebServerHeader::WSH_CACHE_CONTROL: return "Cache-Control: ";
        case WebServerHeader::WSH_CONTENT_ENCODING: return "Content-Encoding: ";
        default: return nullptr; // shouldn't be sent
    }
}

void WebServerResponse::setHeader(WebServerHeader header, const char *headerValue) {
    auto hdrField = getHeaderAsText(header);
    if(!hdrField || !headerValue) return; // can't be encoded safely

    uint8_t* dataArea = transport->getReadBuffer();
    strcpy((char*)dataArea, hdrField);
    strcat((char*)dataArea, headerValue);
    strcat((char*)dataArea, "\r\n");
    transport->performRawWrite(dataArea, strlen((char*)dataArea));
}

void WebServerResponse::startData() {
    mode = PREPARING_CONTENT;
    transport->performRawWrite((uint8_t*)"\r\n", 2);
}

bool WebServerResponse::send(const uint8_t *startingLocation, size_t numBytes) {
    if(mode != PREPARING_CONTENT) startData();
    size_t bytesSent = 0;
    while(bytesSent < numBytes) {
        size_t toSend = min(500U, numBytes);
        int iterations = 0;
        while(!transport->available() && transport->connected() && ++iterations < 100) {
            taskManager.yieldForMicros(1000);
        }
        auto actual = transport->performRawWrite(&startingLocation[bytesSent], toSend);
        if(actual == 0) {
            transport->close();
            return false;
        }
        bytesSent += actual;
    }
    return bytesSent == numBytes;
}

bool WebServerResponse::send_P(const uint8_t *startingLocation, size_t numBytes) {
    if(mode != PREPARING_CONTENT) startData();
    size_t bytesSent = 0;
    while(bytesSent < numBytes) {
        size_t toSend = min(transport->getReadBufferSize(), numBytes);
        memcpy_P(transport->getReadBuffer(), &startingLocation[bytesSent], toSend);
        int iterations = 0;
        while(!transport->available() && transport->connected() && ++iterations < 100) {
            taskManager.yieldForMicros(1000);
        }
        auto actual = transport->performRawWrite(transport->getReadBuffer(), toSend);
        if(actual == 0) {
            transport->close();
            return false;
        }
        bytesSent += actual;
    }
    return bytesSent == numBytes;
}

bool WebServerResponse::processHeaders() {
    HttpProcessor processor;
    processor.setTransport(transport);
    bool foundEndOfRequest = false;
    bool validGetReceived = false;
    while(!foundEndOfRequest) {
        auto hdrType = processor.processHeader((char*)transport->getReadBuffer(), transport->getReadBufferSize());
        switch (hdrType) {
            case WSH_GET:
            case WSH_POST:
                strncpy(pathText, (char*)transport->getReadBuffer(), sizeof pathText);
                method = hdrType == WSH_GET ? GET : POST;
                validGetReceived = true;
                break;
            case WSH_FINISHED:
                foundEndOfRequest = true;
                break;
            case WSH_ERROR:
                validGetReceived = false;
                foundEndOfRequest = true;
                break;
            default:
                break;
        }
    }
    if(!validGetReceived) {
        startHeader(WS_INT_RESPONSE_INT_ERR, "Parse error");
        contentInfo(PLAIN_TEXT, 0);
        end();
    }
    return validGetReceived;
}

void WebServerResponse::end() {
    if(mode != PREPARING_CONTENT) {
        transport->performRawWrite((uint8_t*)"\r\n", 2);
    }
    transport->close();
    mode = NOT_IN_USE;
}

void WebServerResponse::setTransport(AbstractWebSocketTcMenuTransport *tx) {
    transport = tx;
    transport->setState(WSS_HTTP_REQUEST); // regular http request.
    mode = TRANSPORT_ASSIGNED;
}
