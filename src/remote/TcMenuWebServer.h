/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCLIBRARYDEV_TCMENUWEBSERVER_H
#define TCLIBRARYDEV_TCMENUWEBSERVER_H

#include <Arduino.h>
#include "../RemoteConnector.h"
#include "BaseRemoteComponents.h"
#include "BaseBufferedRemoteTransport.h"
#include "TcMenuHttpRequestProcessor.h"
#include <SCCircularBuffer.h>
#include <SimpleCollections.h>
#include "TransportNetworkDriver.h"

#if defined(WS_RTC_INTEGRATED)
void rtcUTCDateInWebForm(const char* buffer, size_t bufferLen);
#endif

#ifndef MAX_WEBSERVER_RESPONSES
#define MAX_WEBSERVER_RESPONSES 4
#endif

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
 * A very cut down and basic web server with webSocket implementation that can act as a web socket endpoint on a given
 * port ONLY for an embedCONTROL connection, be aware that this is not a full websocket implementation. Rather just enough to meet
 * the protocol for this special case. The webserver is reasonably complete for static content purposes and is optimized for
 * serving large files in chunks.
 */

namespace tcremote {

    enum WebSocketOpcode {
        OPC_CONTINUATION, OPC_TEXT, OPC_BINARY, OPC_CLOSE = 8, OPC_PING, OPC_PONG
    };

    enum WebSocketTransportState {
        WSS_NOT_CONNECTED, WSS_HTTP_REQUEST, WSS_IDLE, WSS_LEN_READ, WSS_EXT_LEN_READ, WSS_MASK_READ, WSS_PROCESSING_MSG
    };


    class TcMenuWebServerTransport : public TagValueTransport {
    protected:
        socket_t clientFd;
        size_t bytesLeftInCurrentMsg;
        uint8_t frameMask[4];
        uint8_t* writeBuffer;
        uint8_t* readBuffer;
        WebSocketTransportState currentState;
        const uint8_t bufferSize;
        uint8_t frameMaskingPosition;
        uint8_t readPosition;
        uint8_t readAvail;
        uint8_t writePosition;
        bool consideredOpen;
    public:
        explicit TcMenuWebServerTransport(uint8_t buffSz = 125) : TagValueTransport(TVAL_UNBUFFERED), clientFd(TC_BAD_SOCKET_ID),
                                             bytesLeftInCurrentMsg(0), frameMask{}, writeBuffer(new uint8_t[buffSz]),
                                             readBuffer(new uint8_t[buffSz]), currentState(WSS_NOT_CONNECTED),
                                             bufferSize(buffSz), frameMaskingPosition(0), readPosition(0), readAvail(0),
                                             writePosition(0), consideredOpen(false) {}
        void flush() override;
        void close() override;
        uint8_t readByte() override;
        void endMsg() override;

        void setClient(socket_t client);

        int writeChar(char data) override;
        int writeStr(const char *data) override;

        bool readAvailable() override;
        bool available() override;

        bool connected() override;

        void setState(WebSocketTransportState state) { currentState = state;}
        size_t getReadBufferSize() const { return bufferSize; }
        uint8_t* getReadBuffer() { return readBuffer; }
        size_t getWriteBufferSize() const { return bufferSize; }
        uint8_t* getWriteBuffer() { return writeBuffer; }
        socket_t getClientFd() { return clientFd; }
    private:
        void sendMessageOnWire(WebSocketOpcode opcode, uint8_t* buffer, size_t size);
    };

    typedef void (*WebPageHandler)(WebServerResponse&);

    class UrlWithHandler {
    private:
        uint16_t index;
        WebServerMethod handlerMethod;
        const char* handlerUrl;
        WebPageHandler handlerFn;
    public:
        UrlWithHandler() : index(-1), handlerMethod(GET), handlerUrl(nullptr), handlerFn(nullptr) {}
        UrlWithHandler(uint16_t idx, WebServerMethod method, const char* url, WebPageHandler handler) : index(idx), handlerMethod(method), handlerUrl(url), handlerFn(handler) {}
        UrlWithHandler(const UrlWithHandler& other) = default;
        UrlWithHandler& operator= (const UrlWithHandler& other) = default;
        uint16_t getKey() const { return index; }

        bool isRequestCompatible(const char* url, WebServerMethod method) { return handlerUrl && strcmp(url, handlerUrl) == 0 && method == handlerMethod; }
        void handleUrl(WebServerResponse& response) { handlerFn(response); }
    };

    class TcMenuLightweightWebServer : public BaseEvent {
    protected:
        int numConcurrent;
        WebServerResponse* responses[MAX_WEBSERVER_RESPONSES];
        BtreeList<uint16_t, UrlWithHandler> urlHandlers;
        bool socketInitialised;
        GenericCircularBuffer<socket_t> connectionsWaiting;
        int port;
        taskid_t wsTaskId = TASKMGR_INVALIDID;
    public:
        explicit TcMenuLightweightWebServer(int port, int numConcurrent);
        ~TcMenuLightweightWebServer() override;

        void init();
        void exec() override;
        uint32_t timeOfNextCheck() override;
        void pushClientSocket(socket_t socketIncoming);

        void onUrlGet(const char* url, WebPageHandler pageHandler) { urlHandlers.add(UrlWithHandler(urlHandlers.count(), GET, url, pageHandler));}
        void onUrlPost(const char* url, WebPageHandler pageHandler) { urlHandlers.add(UrlWithHandler(urlHandlers.count(), POST, url, pageHandler)); }

        bool isInitialised() const { return socketInitialised; }
        bool attemptToHandleRequest(WebServerResponse& method, const char* url);
        virtual void sendErrorCode(WebServerResponse* response, int errorCode);

        WebServerResponse *nextAvailableResponse();
        WebServerResponse* getWebResponse(int num) { return responses[num]; }
    };
}

#endif //TCLIBRARYDEV_TCMENUWEBSERVER_H
