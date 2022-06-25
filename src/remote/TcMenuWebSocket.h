/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCLIBRARYDEV_TCMENUWEBSOCKET_H
#define TCLIBRARYDEV_TCMENUWEBSOCKET_H

#include <Arduino.h>
#include "../RemoteConnector.h"
#include "BaseRemoteComponents.h"
#include "BaseBufferedRemoteTransport.h"

#define WS_SERVER_NAME "tccWS"
#define WS_TEXT_RESPONSE_NOT_FOUND "Not found"
#define WS_INT_RESPONSE_NOT_FOUND 404
#define WS_INT_RESPONSE_OK 200
#define WS_TEXT_RESPONSE_OK "OK"
#define WS_INT_RESPONSE_INT_ERR 500

#if defined(WS_RTC_INTEGRATED)
void rtcUTCDateInWebForm(const char* buffer, size_t bufferLen);
#endif

/**
 * A very cut down and basic web socket implementation that can act as a web socket endpoint on a given port ONLY for
 * a embedCONTROL connection, be aware that this is not a full websocket implementation. Rather just enough to meet
 * the protocol for this special case.
 */

namespace tcremote {

    enum WebSocketOpcode {
        OPC_CONTINUATION, OPC_TEXT, OPC_BINARY, OPC_CLOSE = 8, OPC_PING, OPC_PONG
    };

    enum WebServerHeader {
        WSH_UNPROCESSED, WSH_GET, WSH_POST, WSH_FINISHED, WSH_UPGRADE_TO_WEBSOCKET, WSH_SEC_WS_KEY, WSH_DATE, WSH_SERVER, WSH_LAST_MODIFIED, WSH_CONTENT_TYPE,
        WSH_CONTENT_LENGTH, WSH_CONTENT_ENCODING, WSH_CACHE_CONTROL, WSH_HOST, WSH_USER_AGENT, WSH_ERROR
    };

    enum WebSocketTransportState {
        WSS_NOT_CONNECTED, WSS_HTTP_REQUEST, WSS_IDLE, WSS_LEN_READ, WSS_EXT_LEN_READ, WSS_MASK_READ, WSS_PROCESSING_MSG
    };

    enum WebServerMethod { GET, POST };

    class AbstractWebSocketTcMenuTransport : public TagValueTransport {
    protected:
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
    public:
        explicit AbstractWebSocketTcMenuTransport(uint8_t buffSz = 125) : TagValueTransport(TVAL_UNBUFFERED),
                                             bytesLeftInCurrentMsg(0), frameMask{}, writeBuffer(new uint8_t[buffSz]),
                                             readBuffer(new uint8_t[buffSz]), currentState(WSS_NOT_CONNECTED),
                                             bufferSize(buffSz), frameMaskingPosition(0), readPosition(0), readAvail(0), writePosition(0) {}
        void flush() override;
        void close() override;
        uint8_t readByte() override;
        void endMsg() override;

        int writeChar(char data) override;
        int writeStr(const char *data) override;

        bool readAvailable() override;

        // these will be used to write web socket data directly, not put things in the buffer.
        virtual int performRawRead(uint8_t* buffer, size_t bufferSize)=0;
        virtual int performRawWrite(const uint8_t* data, size_t dataSize)=0;

        void setState(WebSocketTransportState state) { currentState = state;}
        size_t getReadBufferSize() const { return bufferSize; }
        uint8_t* getReadBuffer() { return readBuffer; }
        size_t getWriteBufferSize() const { return bufferSize; }
        uint8_t* getWriteBuffer() { return writeBuffer; }
    private:
        void sendMessageOnWire(WebSocketOpcode opcode, uint8_t* buffer, size_t size);
    };

    class HttpProcessor {
    private:
        AbstractWebSocketTcMenuTransport* transport;
        unsigned long millisStart;
        bool protocolError = false;
    public:
        HttpProcessor() : transport(nullptr), millisStart(0) {}
        void setTransport(AbstractWebSocketTcMenuTransport* tx);
        bool readWordUntilTrim(char* buffer, size_t bufferSize, bool skipSeparator = false);
        char readCharFromTransport();
        WebServerHeader processHeader(char* buffer, size_t bufferSize);
    };

    class AbstractWebSocketTcMenuInitialisation : public DeviceInitialisation {
    private:
        const char* expectedPath;
        AbstractWebSocketTcMenuTransport* transport;
        HttpProcessor processor;
    public:
        explicit AbstractWebSocketTcMenuInitialisation(const char *expectedPath) : expectedPath(expectedPath), transport(nullptr),
                                                                                   processor() {}

        bool performUpgradeOnClient(AbstractWebSocketTcMenuTransport* t);
    };

    class WebServerResponse {
    public:
        enum WSRMode { NOT_IN_USE, TRANSPORT_ASSIGNED, READING_HEADERS, PREPARING_HEADER, PREPARING_CONTENT };
        enum WSRContentType { PLAIN_TEXT, HTML_TEXT, PNG_IMAGE, JPG_IMAGE, JSON_TEXT };
    private:
        WebServerMethod method;
        char pathText[100];
        AbstractWebSocketTcMenuTransport* transport;
        enum WSRMode mode;
    public:
        WebServerResponse() : method(GET), transport(nullptr), mode(NOT_IN_USE) {}
        void setTransport(AbstractWebSocketTcMenuTransport* tx);
        bool processHeaders();

        WSRMode getMode() { return mode; }
        void setMode(WSRMode newMode) { mode = newMode; }

        void startHeader() { startHeader(WS_INT_RESPONSE_OK, WS_TEXT_RESPONSE_OK); }
        void startHeader(int code, const char* textualInfo);
        void setHeader(WebServerHeader header, const char* headerValue);
        void contentInfo(WSRContentType contentType, size_t len);
        void startData();
        bool send_P(const char* startingLocation, size_t numBytes) { return send_P((uint8_t*) startingLocation, numBytes);}
        bool send(const char* startingLocation, size_t numBytes) { return send((uint8_t*)startingLocation, numBytes);}
        bool send_P(const uint8_t* startingLocation, size_t numBytes);
        bool send(const uint8_t* startingLocation, size_t numBytes);
        void end();

        const char* getPath() { return pathText; }
        WebServerMethod getMethod() { return method; }
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

    class AbstractLightweightWebServer : public BaseEvent {
    private:
        BtreeList<uint16_t, UrlWithHandler> urlHandlers;
        WebServerResponse response;
    public:
        AbstractLightweightWebServer();
        void init();
        void exec() override;
        uint32_t timeOfNextCheck() override;
        virtual AbstractWebSocketTcMenuTransport* attemptNewConnection()=0;

        void onUrlGet(const char* url, WebPageHandler pageHandler) { urlHandlers.add(UrlWithHandler(urlHandlers.count(), GET, url, pageHandler));}
        void onUrlPost(const char* url, WebPageHandler pageHandler) { urlHandlers.add(UrlWithHandler(urlHandlers.count(), POST, url, pageHandler)); }
    private:

        void findAndAssociateUrl(const char* path, WebServerMethod method);
    };
}

#endif //TCLIBRARYDEV_TCMENUWEBSOCKET_H
