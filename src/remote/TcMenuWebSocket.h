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

#define WS_BUFFER_SIZE 126

/**
 * A very cut down and basic web socket implementation that can act as a web socket endpoint on a given port ONLY for
 * a embedCONTROL connection, be aware that this is not a full websocket implementation. Rather just enough to meet
 * the protocol for this special case.
 */

namespace tcremote {

    enum WebSocketOpcode {
        OPC_CONTINUATION, OPC_TEXT, OPC_BINARY, OPC_CLOSE = 8, OPC_PING, OPC_PONG
    };

    enum WebSocketHeader {
        WSH_UNPROCESSED, WSH_GET, WSH_FINISHED, WSH_UPGRADE_TO_WEBSOCKET, WSH_SEC_WS_KEY, WSH_ERROR
    };

    enum WebSocketTransportState: uint8_t {
        WSS_NOT_CONNECTED, WSS_UPGRADING, WSS_IDLE, WSS_FLAGS_READ, WSS_LEN_READ, WSS_MASK_READ, WSS_PROCESSING_MSG
    };

    class AbstractWebSocketTcMenuTransport : public TagValueTransport {
    protected:
        size_t bytesLeftInCurrentMsg;
        uint8_t frameMask[4];
        uint8_t writeBuffer[WS_BUFFER_SIZE];
        uint8_t readBuffer[WS_BUFFER_SIZE];
        WebSocketTransportState currentState;
        uint8_t frameMaskingPosition;
        uint8_t readPosition;
        uint8_t readAvailable;
        uint8_t writePosition;
    public:
        AbstractWebSocketTcMenuTransport() : TagValueTransport(TVAL_UNBUFFERED),
                                             bytesLeftInCurrentMsg(0), frameMask{}, writeBuffer{}, readBuffer{}, currentState(WSS_NOT_CONNECTED),
                                             frameMaskingPosition(0), readPosition(0), readAvailable(0), writePosition(0) {}
        void flush() override;
        void close() override;
        uint8_t readByte() override;
        void endMsg() override;

        int writeChar(char data) override;
        int writeStr(const char *data) override;

        // these will be used to write web socket data directly, not put things in the buffer.
        virtual int performRawRead(uint8_t* buffer, size_t bufferSize)=0;
        virtual int performRawWrite(uint8_t* data, size_t dataSize)=0;

        void setState(WebSocketTransportState state) { currentState = state;}
    private:
        void sendMessageOnWire(WebSocketOpcode opcode, uint8_t* buffer, size_t size);
    };

    class AbstractWebSocketTcMenuInitialisation : public DeviceInitialisation {
    private:
        const char* expectedPath;
        AbstractWebSocketTcMenuTransport* transport;
    public:
        explicit AbstractWebSocketTcMenuInitialisation(const char *expectedPath) : expectedPath(expectedPath), transport(nullptr) {}

        bool performUpgradeOnClient(AbstractWebSocketTcMenuTransport* t);
    private:
        bool readWordUntilTrim(char* buffer, size_t bufferSize);
        char readCharFromTransport();
        WebSocketHeader processHeader(char* buffer, size_t bufferSize);
    };

}

#endif //TCLIBRARYDEV_TCMENUWEBSOCKET_H
