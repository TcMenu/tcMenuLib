
#include <PlatformDetermination.h>
#include "TcMenuHttpRequestProcessor.h"
#include "TcMenuWebServer.h"
#include <TaskManagerIO.h>

using namespace tcremote;

const char pgmWebSockUuid[] PROGMEM = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

namespace tc_b64 {
    int base64(const uint8_t *data, int dataSize, uint8_t *buffer, int bufferSize);
}

// See the associated C file in this directory
extern "C" {
int sha1digest(uint8_t *hexDigest, const uint8_t *data, size_t databytes);
}


inline WebSocketOpcode getOpcode(uint8_t header) {
    return (WebSocketOpcode)(header & WS_OPCODE_MASK);
}

char HttpProcessor::readCharFromTransport() {
    while(transport->connected()) {
        uint8_t sz[1];
        auto actual = rawReadData(transport->getClientFd(), sz, 1);
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
            serlogF(SER_NETWORK_DEBUG, "read loop error");
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
        serlogF(SER_NETWORK_DEBUG, "Socket closed or timeout");
        return REQ_NONE;
    }
    bool getRequest = strcmp(buffer, "GET") == 0;
    bool postRequest = strcmp(buffer, "POST") == 0;
    if(getRequest || postRequest) {
        if(readWordUntilTrim(buffer, bufferSize)) return REQ_ERROR; // missing HTTP/1.1
        char sz[10];
        if(!readWordUntilTrim(sz, sizeof sz, true)) {
            serlogF4(SER_NETWORK_INFO, "Request missing protocol (buffer, proto, isGet)", buffer, sz, getRequest);
            return REQ_ERROR; // protocol eg HTTP*
        }
        if(protocolError || strncmp(sz, "HTTP", 4) != 0) {
            serlogF4(SER_NETWORK_INFO, "Request without HTTP (buffer, proto, isGet)", buffer, sz, getRequest);
            return REQ_ERROR;
        }
        serlogF4(SER_NETWORK_INFO, "Request processed (buffer, proto, isGet)", buffer, sz, getRequest);
        return getRequest ? GET : POST;
    }
    else {
        serlogF(SER_NETWORK_INFO,"Not POST or GET");
        return REQ_ERROR;
    }
}

WebServerHeader HttpProcessor::processHeader(char* buffer, size_t bufferSize) {
    millisStart = millis();
    protocolError = false;

    if(readWordUntilTrim(buffer, bufferSize)) return WSH_FINISHED;
    if(!transport->connected()) return WSH_ERROR;
    if(strcmp(buffer, "Host") == 0) {
        if(!readWordUntilTrim(buffer, bufferSize, true)) {
            serlogF2(SER_NETWORK_INFO, "Host tag not terminated", buffer);
            return WSH_ERROR;
        }
        if(protocolError) return WSH_ERROR;
        serlogF2(SER_NETWORK_DEBUG, "Host ", buffer);
        return WSH_HOST;
    }
    if(strcmp(buffer, "User-Agent") == 0) {
        if(!readWordUntilTrim(buffer, bufferSize, true)) {
            serlogF2(SER_NETWORK_INFO, "User-Agent not terminated", buffer);
            return WSH_ERROR;
        }
        if(protocolError) return WSH_ERROR;
        serlogF2(SER_NETWORK_DEBUG, "User-Agent ", buffer);
        return WSH_USER_AGENT;
    }
    if(strcmp(buffer, "Upgrade")==0) {
        if(!readWordUntilTrim(buffer, bufferSize, true)) {
            serlogF2(SER_NETWORK_INFO, "Upgrade not terminated", buffer);
            return WSH_ERROR;
        }
        if(protocolError) return WSH_ERROR;
        if(strcmp(buffer, "websocket") == 0) {
            serlogF(SER_NETWORK_DEBUG, "Upgrade to websocket OK");
            return WSH_UPGRADE_TO_WEBSOCKET;
        }
    } else if(strcmp(buffer, "Connection") == 0) {
        readWordUntilTrim(buffer, bufferSize, true);
        if(protocolError) return WSH_ERROR;
        if(strcmp(buffer, "Upgrade") == 0) {
            serlogF2(SER_NETWORK_DEBUG, "Connection upgrade OK", buffer);
            return WSH_UPGRADE_TO_WEBSOCKET;
        }
    } else if(strcmp(buffer, "Sec-WebSocket-Key") == 0) {
        if (!readWordUntilTrim(buffer, bufferSize, true)) {
            serlogF2(SER_NETWORK_INFO, "SecKey not terminated", buffer);
            return WSH_ERROR;
        }
        if (protocolError) return WSH_ERROR;
        serlogF2(SER_NETWORK_DEBUG, "SecKey OK", buffer);
        return WSH_SEC_WS_KEY;
    } else if(strcmp(buffer, "Accept-Encoding") == 0) {
        if (!readWordUntilTrim(buffer, bufferSize, true)) {
            serlogF2(SER_NETWORK_INFO, "AcceptEncoding not terminated", buffer);
            return WSH_ERROR;
        }
        if (protocolError) return WSH_ERROR;
        serlogF2(SER_NETWORK_DEBUG,"Accept encoding ", buffer);
        return WSH_ACCEPT_ENCODING;
    } else {
        readWordUntilTrim(buffer, bufferSize, true);
        // you can enable the below if you want to see every single header for debugging.
        serlogF2(SER_NETWORK_DEBUG, "Unprocessed ", buffer);
        serlogF3(SER_NETWORK_DEBUG, "Content: ", buffer, protocolError);
        if(protocolError) return WSH_ERROR;
    }
    return WSH_UNPROCESSED;
}

void tcremote::HttpProcessor::tick() {
    millisStart = millis();
}

void HttpProcessor::reset() {
    millisStart = millis();
    protocolError = false;
}

WebServerResponse::WebServerResponse(TcMenuLightweightWebServer *webServer, TcMenuWebServerTransport *tx,
                                     WebServerResponse::WSRConnectionType conType)
        : webServer(webServer), method(GET), transport(tx), processor(tx), mode(NOT_IN_USE), connectionType(conType), webSocketSha1KeyToRespond{} {}

void WebServerResponse::init() {
    scheduledTaskId = taskManager.scheduleFixedRate(20, this);
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
    serlogF3(SER_NETWORK_INFO, "Start header response", code, textualInfo);
    mode = PREPARING_HEADER;
    if(code == WS_CODE_CHANGING_PROTOCOL) connectionType = WEB_SOCKET; // websockets don't get closed after the request.
    uint8_t* dataArea = transport->getReadBuffer();
    size_t buffSize = transport->getReadBufferSize();
    strcpy((char*)dataArea, "HTTP/1.1 ");
    char sz[32];
    itoa(code, sz, 10);
    strcat((char*)dataArea, sz);
    appendChar((char*)dataArea, ' ', buffSize);
    strcat((char*)dataArea, textualInfo);
    strcat((char*)dataArea, "\r\n");
    rawWriteData(transport->getClientFd(), dataArea, strlen((char*)dataArea), RAM_NEEDS_COPY);
    setHeader(WSH_SERVER, WS_SERVER_NAME);

// if you have an RTC device, you can implement `rtcUTCDateInWebForm` which allows you to give the current date from
// the RTC device for submission in the headers as the DATE header. It is assumed the format is correct.
#if defined(WS_RTC_INTEGRATED)
    rtcUTCDateInWebForm(sz, sizeof sz);
    setHeader(WSH_DATE, sz)
#endif

    // for synchronous single clients (IE one at a time), it's best that we close the connection immediately
    // rather than delay and wait, as otherwise it's probable we will just slow things down waiting around while
    // only one connection at once is allowed
    if(connectionType == CLOSE_AFTER_RESPONSE) {
        setHeader(WSH_CONNECTION, "close");
    }
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

    serlogF3(SER_NETWORK_DEBUG, "Add header ", hdrField, headerValue);

    rawWriteData(transport->getClientFd(), dataArea, strlen((char*)dataArea), RAM_NEEDS_COPY);
}

void WebServerResponse::turnRequestIntoWebSocket() {
    char sz[34];

    serlogF(SER_NETWORK_INFO, "Convert request to websocket");
    startHeader(WS_CODE_CHANGING_PROTOCOL, "Switching Protocols");
    setHeader(WSH_UPGRADE_TO_WEBSOCKET, "websocket");
    setHeader(WSH_CONNECTION, "Upgrade");
    this->transport->setState(WSS_IDLE);
    tc_b64::base64(webSocketSha1KeyToRespond, sizeof(webSocketSha1KeyToRespond), (uint8_t *) sz, sizeof sz);
    setHeader(WSH_SEC_WS_ACCEPT_KEY, sz);

    // at this point the connection is fully established and in web socket mode
    connectionType = WEB_SOCKET;
    setMode(WEBSOCKET_BUSY);
    transport->setState(WSS_IDLE);
    end(); // terminate the header, won't close connection because we've switched to websocket mode.
}

void WebServerResponse::startData() {
    serlogF(SER_NETWORK_DEBUG, "Start data response");
    mode = PREPARING_CONTENT;
    rawWriteData(transport->getClientFd(), (uint8_t*)"\r\n", 2, RAM_NEEDS_COPY);
}

bool WebServerResponse::send(const uint8_t *startingLocation, size_t numBytes, bool memoryIsConst) {
    if(mode != PREPARING_CONTENT) startData();
    MemoryLocationType memType = memoryIsConst ? CONSTANT_NO_COPY : RAM_NEEDS_COPY;
    auto didSend = rawWriteData(transport->getClientFd(), startingLocation, numBytes, memType);
    if(!didSend) {
        closeConnection();
        return false;
    }
    return true;
}

bool WebServerResponse::send_P(const uint8_t *startingLocation, size_t numBytes) {
    if(mode != PREPARING_CONTENT) startData();

    auto err = rawWriteData(transport->getClientFd(), startingLocation, numBytes, IN_PROGRAM_MEM);
    if(err == SOCK_ERR_OK) {
        return true;
    } else if(err == SOCK_ERR_NO_PROGMEM_SUPPORT) {
        size_t bytesSent = 0;
        while (bytesSent < numBytes) {
            size_t toSend = min(transport->getReadBufferSize(), numBytes);
            memcpy_P(transport->getReadBuffer(), &startingLocation[bytesSent], toSend);
            bool didSend = rawWriteData(transport->getClientFd(), transport->getReadBuffer(), toSend, RAM_NEEDS_COPY);
            if (!didSend) {
                closeConnection();
                return false;
            }
            bytesSent += toSend;
        }
        return bytesSent == numBytes;
    }

    return false;
}


bool WebServerResponse::processHeaders() {
    char* buffer = (char*)transport->getReadBuffer();
    size_t bufferSize = transport->getReadBufferSize();
    bool foundEndOfRequest = false;

    serlogF(SER_NETWORK_DEBUG, "Process header");
    while(!foundEndOfRequest) {
        auto hdrType = processor.processHeader(buffer, bufferSize);
        switch (hdrType) {
            case WSH_SEC_WS_KEY: {
                strncat_P(buffer, pgmWebSockUuid, bufferSize - strlen(buffer) - 1);
                sha1digest(webSocketSha1KeyToRespond, (uint8_t *) buffer, strlen(buffer));
                break;
            }
            case WSH_FINISHED:
                serlogF(SER_NETWORK_INFO, "Request processed");
                foundEndOfRequest = true;
                return true;
            case WSH_UPGRADE_TO_WEBSOCKET:
                method = WS_UPGRADE;
                break;
            case WSH_ERROR:
                serlogF(SER_NETWORK_INFO, "Request error");
                foundEndOfRequest = false;
                connectionType = CLOSE_AFTER_RESPONSE; // tell end to close the connection as the request is faulty.
                return false;
            default:
                break;
        }
    }
    return foundEndOfRequest;
}

void WebServerResponse::end() {
    serlogF(SER_NETWORK_INFO, "Finished response");
    if(mode != PREPARING_CONTENT) {
        rawWriteData(transport->getClientFd(), (uint8_t*)"\r\n", 2, RAM_NEEDS_COPY);
    }

    if(connectionType == CLOSE_AFTER_RESPONSE) {
        transport->flush();
        transport->close();
        mode = NOT_IN_USE;
    } else if(connectionType != WEB_SOCKET) {
        mode = TRANSPORT_ASSIGNED;
        processor.tick(); // wait 2 seconds from end of last message.
    }
}

void WebServerResponse::serviceClient(socket_t sock) {
    transport->setClient(sock);
    setMode(TRANSPORT_ASSIGNED);
    connectionType = CLOSE_AFTER_RESPONSE;
}

void WebServerResponse::exec() {
    if(mode == TRANSPORT_ASSIGNED) {
        transport->setState(WSS_HTTP_REQUEST); // regular http request.
        processor.reset();

        bool needAnotherGo = true;
        while (needAnotherGo) {
            method = processor.processRequest(reinterpret_cast<char *>(transport->getReadBuffer()),
                                              transport->getReadBufferSize());
            if (method == POST || method == GET) {
                setMode(WebServerResponse::READING_HEADERS);
                needAnotherGo = webServer->attemptToHandleRequest(*this, (const char *) transport->getReadBuffer());

                // if we upgraded to a websocket, we don't need another go, and we mark the response object busy.
                // It is the responsibility of the websocket handler to close the connection once completed.
                if(connectionType == WEB_SOCKET) {
                    needAnotherGo = false;
                } else if(!needAnotherGo) {
                    closeConnection();
                }
            } else if (method == REQ_ERROR) {
                needAnotherGo = false;
                webServer->sendErrorCode(this, WS_INT_RESPONSE_INT_ERR);
                closeConnection();
            } else {
                needAnotherGo = false;
                closeConnection();
            }
        }
    }
}

void WebServerResponse::closeConnection() {
    serlogF(SER_NETWORK_INFO, "HTTP close");
    mode = NOT_IN_USE;
    if(transport) transport->close();
}

bool WebServerResponse::hasErrorOccurred() {
    return processor.isProtocolError();
}

void WebServerResponse::stop() {
    if(scheduledTaskId != TASKMGR_INVALIDID) {
        taskManager.cancelTask(scheduledTaskId);
    }
}

void WebServerResponse::sendError(int code) {
    this->webServer->sendErrorCode(this, code);
}

