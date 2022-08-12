/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include <PlatformDetermination.h>
#include "TcMenuWebServer.h"
#include "TcMenuHttpRequestProcessor.h"

using namespace tcremote;

void TcMenuWebServerTransport::close() {
    consideredOpen = false;
    if(currentState != WSS_HTTP_REQUEST && currentState != WSS_NOT_CONNECTED) {
        // don't send a ws close event unless we are in web socket mode.
        uint8_t sz[2];
        sendMessageOnWire(OPC_CLOSE, sz, 0);
    }
    closeSocket(clientFd);
    bytesLeftInCurrentMsg = 0;
    frameMaskingPosition = 0;
    writePosition = 0;
    readAvail = 0;
    readPosition = 0;
    currentState = WSS_NOT_CONNECTED;
}

bool TcMenuWebServerTransport::available() {
    if(!consideredOpen) return false;
    return rawWriteAvailable(clientFd);
}

bool TcMenuWebServerTransport::readAvailable() {
    if(!consideredOpen) return false;
    // short circuit when there's room in the buffer already.
    if(readPosition < readAvail && currentState == WSS_PROCESSING_MSG) return true;

    bool processing = true;
    while(processing) {
        switch (currentState) {
            case WSS_PROCESSING_MSG:
                if(bytesLeftInCurrentMsg > 0) {
                    readAvail = performRawRead(readBuffer, min(bytesLeftInCurrentMsg, (size_t)bufferSize));
                    bytesLeftInCurrentMsg = bytesLeftInCurrentMsg - readAvail;
                    readPosition = 0;
                    return readAvail > 0;
                }
                setState(WSS_IDLE); // we are now idle and trying to read the two byte frame
                readPosition = 0;
                processing = false;
                break;
            case WSS_IDLE:
            case WSS_LEN_READ: {
                auto actual = performRawRead(&readBuffer[readPosition], readPosition == 0 ? 2 : 1);
                if(actual < 0) {
                    return false;
                }
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

uint8_t TcMenuWebServerTransport::readByte() {
    if(currentState == WSS_HTTP_REQUEST) {
        uint8_t sz[1];
        performRawRead(sz, 1);
        return sz[0];
    }
    else if(readPosition < readAvail && currentState == WSS_PROCESSING_MSG) {
        auto data = readBuffer[readPosition] ^ frameMask[frameMaskingPosition];
        readPosition++;
        frameMaskingPosition = (frameMaskingPosition + 1) % 4;
        return data;
    }
    else return 0xff; // fault. called without checking readAvailable
}

int TcMenuWebServerTransport::writeChar(char data) {
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

int TcMenuWebServerTransport::writeStr(const char *data) {
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

bool TcMenuWebServerTransport::connected() {
    return consideredOpen;
}

void TcMenuWebServerTransport::flush() {
    if(!consideredOpen) return;

    if(currentState == WSS_HTTP_REQUEST) {
        rawFlushAll(clientFd);
    } else if(writePosition != 0) {
        sendMessageOnWire(OPC_TEXT, writeBuffer, writePosition);
        serdebugF2("Buffer written ", writePosition);
        writePosition = 0;
        rawFlushAll(clientFd);
    }
}

void TcMenuWebServerTransport::sendMessageOnWire(WebSocketOpcode opcode, uint8_t* buffer, size_t size) {
    buffer[0] = (uint8_t)(WS_FIN | opcode);
    buffer[1] = (uint8_t)size;
    performRawWrite(buffer, size + 2);
}

void TcMenuWebServerTransport::endMsg() {
    TagValueTransport::endMsg();
    flush();
}

void TcMenuWebServerTransport::setClient(socket_t client) {
    clientFd = client;
    consideredOpen = true;
    readPosition = readAvail = writePosition = frameMaskingPosition = 0;
    setState(tcremote::WSS_HTTP_REQUEST);
}

// ------------ Web server

TcMenuLightweightWebServer::TcMenuLightweightWebServer(int port, int numConcurrent): numConcurrent(numConcurrent), responses {}, socketInitialised(false),
                                                                                     connectionsWaiting(5), port(port) {
    if(numConcurrent > MAX_WEBSERVER_RESPONSES) numConcurrent = MAX_WEBSERVER_RESPONSES;

    for(int i=0; i<numConcurrent; i++){
        responses[i] = new WebServerResponse(this, new TcMenuWebServerTransport(), WebServerResponse::CLOSE_AFTER_RESPONSE);
    }
}

TcMenuLightweightWebServer::~TcMenuLightweightWebServer() {
    if(wsTaskId != TASKMGR_INVALIDID) {
        taskManager.cancelTask(wsTaskId);
    }

    for(int i=0; i<numConcurrent; i++) {
        responses[i]->stop();
        delete responses[i];
    }
}

void TcMenuLightweightWebServer::init() {
    wsTaskId = taskManager.registerEvent(this);
    initialiseAccept(port, [](socket_t fd, void* d) { reinterpret_cast<TcMenuLightweightWebServer*>(d)->pushClientSocket(fd); }, this);
    for(int i=0; i<numConcurrent; i++) {
        responses[i]->init();
    }
}

void TcMenuLightweightWebServer::exec() {
    if(!socketInitialised) {
        socketInitialised = isNetworkUp();
    } else {
        while(connectionsWaiting.available()) {
            socket_t sockFd = connectionsWaiting.get();
            WebServerResponse* response = nextAvailableResponse();
            if(response) {
                response->serviceClient(sockFd);
            } else {
                serdebugF("Too many connections");
                return;
            }
        }
    }
}

void TcMenuLightweightWebServer::pushClientSocket(socket_t socketIncoming) {
    connectionsWaiting.put(socketIncoming);
    markTriggeredAndNotify();
}

uint32_t TcMenuLightweightWebServer::timeOfNextCheck() {
    if(connectionsWaiting.available() || !socketInitialised) {
        markTriggeredAndNotify();
    }
    return millisToMicros(100);
}

bool TcMenuLightweightWebServer::attemptToHandleRequest(WebServerResponse& response, const char* url) {
    for(auto urlWithHandler : urlHandlers) {
        if(urlWithHandler.isRequestCompatible(url, response.getMethod())) {
            if(response.processHeaders()) {
                urlWithHandler.handleUrl(response);
                if (response.getMode() != WebServerResponse::NOT_IN_USE && response.getMode() != WebServerResponse::WEBSOCKET_BUSY) {
                    response.end();
                }
                // we return if it is likley that more request will be on the same connection, if in single shot mode
                // then this should be false. Otherwise true, to keep the connection open.
                return !response.isInSingleShotMode();
            } else {
                sendErrorCode(&response, WS_INT_RESPONSE_INT_ERR);
                return false;
            }
        }
    }
    sendErrorCode(&response, WS_INT_RESPONSE_NOT_FOUND);
    return false;
}

void TcMenuLightweightWebServer::sendErrorCode(WebServerResponse* response, int errorCode) {
    if(errorCode == WS_INT_RESPONSE_NOT_FOUND) {
        response->startHeader(WS_INT_RESPONSE_NOT_FOUND, WS_TEXT_RESPONSE_NOT_FOUND);
        response->contentInfo(WebServerResponse::PLAIN_TEXT, 0);
        response->end();
    } else {
        response->startHeader(WS_INT_RESPONSE_INT_ERR, "Internal error");
        response->contentInfo(WebServerResponse::PLAIN_TEXT, 0);
        response->end();

    }
}

WebServerResponse *TcMenuLightweightWebServer::nextAvailableResponse() {
    for(int i=0;i<numConcurrent;i++) {
        if(responses[i] != nullptr && responses[i]->getMode() == WebServerResponse::NOT_IN_USE) return responses[i];
    }
    return nullptr;
}
