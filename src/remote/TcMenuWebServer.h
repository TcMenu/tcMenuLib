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

#if defined(WS_RTC_INTEGRATED)
void rtcUTCDateInWebForm(const char* buffer, size_t bufferLen);
#endif

/**
 * A very cut down and basic web server with webSocket implementation that can act as a web socket endpoint on a given
 * port ONLY for an embedCONTROL connection, be aware that this is not a full websocket implementation. Rather just enough to meet
 * the protocol for this special case. The webserver is reasonably complete for static content purposes and is optimized for
 * serving large files in chunks.
 */

namespace tcremote {

    /**
     * Must be implemented for the device being used, works out if we are in a good state to start port connections
     * IE connected to wifi, ethernet enabled etc.
     * @return true if connected, otherwise false.
     */
    bool areWeConnected();

    enum WebSocketOpcode {
        OPC_CONTINUATION, OPC_TEXT, OPC_BINARY, OPC_CLOSE = 8, OPC_PING, OPC_PONG
    };

    enum WebSocketTransportState {
        WSS_NOT_CONNECTED, WSS_HTTP_REQUEST, WSS_IDLE, WSS_LEN_READ, WSS_EXT_LEN_READ, WSS_MASK_READ, WSS_PROCESSING_MSG
    };


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


    class AbstractWebSocketTcMenuInitialisation : public DeviceInitialisation {
    private:
        const char* expectedPath;
        AbstractWebSocketTcMenuTransport* transport;
        HttpProcessor processor;
        WebServerResponse response;
    public:
        explicit AbstractWebSocketTcMenuInitialisation(const char *expectedPath) : expectedPath(expectedPath), transport(nullptr),
                                                                                   processor() {}
        bool processRequestLine(AbstractWebSocketTcMenuTransport* t);
        bool performUpgradeOnClient(AbstractWebSocketTcMenuTransport* t);
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
    protected:
        BtreeList<uint16_t, UrlWithHandler> urlHandlers;
        WebServerResponse response;
        bool socketInitialised;
    public:
        AbstractLightweightWebServer();
        void init();
        void exec() override;
        uint32_t timeOfNextCheck() override;
        virtual AbstractWebSocketTcMenuTransport* attemptNewConnection()=0;
        virtual void initialiseConnection()=0;
        virtual void sendErrorCode(int errorCode);

        void onUrlGet(const char* url, WebPageHandler pageHandler) { urlHandlers.add(UrlWithHandler(urlHandlers.count(), GET, url, pageHandler));}
        void onUrlPost(const char* url, WebPageHandler pageHandler) { urlHandlers.add(UrlWithHandler(urlHandlers.count(), POST, url, pageHandler)); }

        bool isInitialised() const { return socketInitialised; }
    private:
        bool attemptToHandleRequest(WebServerMethod &method);
    };
}

#endif //TCLIBRARYDEV_TCMENUWEBSERVER_H
