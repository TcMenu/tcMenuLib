
#include <PlatformDetermination.h>
#include "TcMenuHttpRequestProcessor.h"
#include "TcMenuWebServer.h"
#include <TaskManager.h>

using namespace tcremote;

char HttpProcessor::readCharFromTransport() {
    while(transport->connected()) {
        uint8_t sz[1];
        auto actual = transport->performRawRead(sz, 1);
        if(actual) {
            return (char)sz[0];
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

    while(!wordRead) {
        auto ch = readCharFromTransport();
        if(protocolError || !transport->connected()) {
            protocolError = true;
            serdebugF("read loop error");
            buffer[0] = 0;
            return false;
        }
        if(trimming && ch == ' ') continue;
        if(!skipSeparator && (ch == ':' || ch == ' ')) wordRead = true;
        else if(ch == '\n') {
            wordRead = true;
            endOfLine = true;
        } else if(ch == '\r') {
            // ignore the \r, we are supposed to read \r\n
        } else if(pos < bufferSize) {
            trimming = false;
            buffer[pos] = ch;
            pos++;
        }
    }
    buffer[min(pos, bufferSize -1)] = 0;
    return endOfLine;
}

WebServerMethod HttpProcessor::processRequest(char* buffer, size_t bufferSize) {
    if(readWordUntilTrim(buffer, bufferSize) || !transport->connected()) {
        return REQ_NONE;
    }
    if(protocolError) {
        serdebugF("Socket closed or timeout");
        return REQ_NONE;
    }
    bool getRequest = strcmp(buffer, "GET") == 0;
    bool postRequest = strcmp(buffer, "POST") == 0;
    if(getRequest || postRequest) {
        if(readWordUntilTrim(buffer, bufferSize)) return REQ_ERROR; // missing HTTP/1.1
        char sz[10];
        if(!readWordUntilTrim(sz, sizeof sz, true)) {
            serdebugF4("Request missing protocol (buffer, proto, isGet)", buffer, sz, getRequest);
            return REQ_ERROR; // protocol eg HTTP*
        }
        if(protocolError || strncmp(sz, "HTTP", 4) != 0) {
            serdebugF4("Request without HTTP (buffer, proto, isGet)", buffer, sz, getRequest);
            return REQ_ERROR;
        }
        serdebugF4("Request processed (buffer, proto, isGet)", buffer, sz, getRequest);
        return getRequest ? GET : POST;
    }
    else {
        serdebugF("Not POST or GET");
        return REQ_ERROR;
    }
}

WebServerHeader HttpProcessor::processHeader(char* buffer, size_t bufferSize) {
    if(readWordUntilTrim(buffer, bufferSize)) return WSH_FINISHED;
    if(!transport->connected()) return WSH_ERROR;
    if(strcmp(buffer, "Host") == 0) {
        if(!readWordUntilTrim(buffer, bufferSize, true)) {
            serdebugF2("Host tag not terminated", buffer);
            return WSH_ERROR;
        }
        if(protocolError) return WSH_ERROR;
        serdebugF2("Host ", buffer);
        return WSH_HOST;
    }
    if(strcmp(buffer, "User-Agent") == 0) {
        if(!readWordUntilTrim(buffer, bufferSize, true)) {
            serdebugF2("User-Agent not terminated", buffer);
            return WSH_ERROR;
        }
        if(protocolError) return WSH_ERROR;
        serdebugF2("User-Agent ", buffer);
        return WSH_USER_AGENT;
    }
    if(strcmp(buffer, "Upgrade")==0) {
        if(!readWordUntilTrim(buffer, bufferSize, true)) {
            serdebugF2("Upgrade not terminated", buffer);
            return WSH_ERROR;
        }
        if(protocolError) return WSH_ERROR;
        if(strcmp(buffer, "websocket") == 0) {
            serdebugF("Upgrade to websocket OK");
            return WSH_UPGRADE_TO_WEBSOCKET;
        }
    } else if(strcmp(buffer, "Connection") == 0) {
        readWordUntilTrim(buffer, bufferSize, true);
        if(protocolError) return WSH_ERROR;
        if(strcmp(buffer, "Upgrade") == 0) {
            serdebugF2("Connection upgrade OK", buffer);
            return WSH_UPGRADE_TO_WEBSOCKET;
        }
    } else if(strcmp(buffer, "Sec-WebSocket-Key") == 0) {
        if (!readWordUntilTrim(buffer, bufferSize, true)) {
            serdebugF2("SecKey not terminated", buffer);
            return WSH_ERROR;
        }
        if (protocolError) return WSH_ERROR;
        serdebugF2("SecKey OK", buffer);
        return WSH_SEC_WS_KEY;
    } else if(strcmp(buffer, "Accept-Encoding") == 0) {
        if (!readWordUntilTrim(buffer, bufferSize, true)) {
            serdebugF2("AcceptEncoding not terminated", buffer);
            return WSH_ERROR;
        }
        if (protocolError) return WSH_ERROR;
        serdebugF2("Accept encoding ", buffer);
        return WSH_ACCEPT_ENCODING;
    } else {
        serdebugF2("Unprocessed ", buffer);
        readWordUntilTrim(buffer, bufferSize, true);
        serdebugF3("Content: ", buffer, protocolError);
        if(protocolError) return WSH_ERROR;
    }
    return WSH_UNPROCESSED;
}

void HttpProcessor::setTransport(AbstractWebSocketTcMenuTransport *tx) {
    transport = tx;
    millisStart = millis();
    protocolError = false;
}

void tcremote::HttpProcessor::tick() {
    millisStart = millis();
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
        case WebServerResponse::WEBP_IMAGE:
            headerText = "image/webp";
            break;
        case WebServerResponse::JSON_TEXT:
            headerText = "application/json";
            break;
        case WebServerResponse::TEXT_CSS:
            headerText = "text/css";
            break;
        case WebServerResponse::JAVASCRIPT:
            headerText = "text/javascript";
            break;
        case WebServerResponse::IMG_ICON:
            headerText = "image/vnd.microsoft.icon";
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
    serdebugF3("Start header response", code, textualInfo);
    mode = PREPARING_HEADER;
    if(code == WS_CODE_CHANGING_PROTOCOL) singleRequestMode = false; // websockets don't get closed after the request.
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
        case WebServerHeader::WSH_UPGRADE_TO_WEBSOCKET: return "Upgrade: ";
        case WebServerHeader::WSH_CONNECTION: return "Connection: ";
        case WebServerHeader::WSH_LAST_MODIFIED: return "Last-Modified: ";
        case WebServerHeader::WSH_CACHE_CONTROL: return "Cache-Control: ";
        case WebServerHeader::WSH_CONTENT_ENCODING: return "Content-Encoding: ";
        case WebServerHeader::WSH_SEC_WS_ACCEPT_KEY: return "Sec-WebSocket-Accept: ";
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

    serdebugF3("Add header ", hdrField, headerValue);

    transport->performRawWrite(dataArea, strlen((char*)dataArea));
}

void WebServerResponse::startData() {
    serdebugF("Start data response");
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
            closeConnection();
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
    processor.setTransport(transport);
    bool foundEndOfRequest = false;
    serdebugF("Process header");
    while(!foundEndOfRequest) {
        auto hdrType = processor.processHeader((char*)transport->getReadBuffer(), transport->getReadBufferSize());
        switch (hdrType) {
            case WSH_FINISHED:
                serdebugF("Request processed");
                foundEndOfRequest = true;
                return true;
            case WSH_ERROR:
                serdebugF("Request error");
                foundEndOfRequest = false;
                singleRequestMode = true; // tell end to close the connection as the request is faulty.
                return false;
            default:
                break;
        }
    }
    return foundEndOfRequest;
}

void WebServerResponse::end() {
    serdebugF("Finished response");
    if(mode != PREPARING_CONTENT) {
        transport->performRawWrite((uint8_t*)"\r\n", 2);
    }
    if(singleRequestMode) {
        transport->close();
        mode = NOT_IN_USE;
    } else {
        mode = TRANSPORT_ASSIGNED;
        processor.tick(); // wait 2 seconds from end of last message.
    }
}

void WebServerResponse::setTransport(AbstractWebSocketTcMenuTransport *tx) {
    transport = tx;
    transport->setState(WSS_HTTP_REQUEST); // regular http request.
    mode = TRANSPORT_ASSIGNED;
}

WebServerMethod WebServerResponse::processRequestLine() {
    processor.setTransport(transport);
    return processor.processRequest(reinterpret_cast<char *>(transport->getReadBuffer()), transport->getReadBufferSize());
}

const char *WebServerResponse::getLastData() {
    return reinterpret_cast<const char *>(transport->getReadBuffer());
}

void WebServerResponse::closeConnection() {
    serdebugF("HTTP close");
    mode = NOT_IN_USE;
    if(transport) transport->close();
}

bool WebServerResponse::hasErrorOccurred() {
    return processor.isProtocolError();
}
