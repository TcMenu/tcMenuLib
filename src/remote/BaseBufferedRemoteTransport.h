/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_BASEBUFFEREDREMOTETRANSPORT_H
#define TCMENU_BASEBUFFEREDREMOTETRANSPORT_H

#include <RemoteConnector.h>

namespace tcremote {

    enum BufferingMode : uint8_t { BUFFER_ONE_MESSAGE, BUFFER_MESSAGES_TILL_FULL };

    /**
     * Many transports need buffering of messages, for example the regular Ethernet2 library will send
     * a character at a time with no buffering. The BLE driver requires buffering because it is only legal
     * to send single full messages at a time. Some other drivers can benefit from some level of buffering.
     * To avoid writing this code many times in different contexts it is available in this core package.
     */
    class BaseBufferedRemoteTransport : public TagValueTransport {
    protected:
        const int writeBufferSize;
        uint8_t* readBuffer;
        uint8_t* writeBuffer;
        int writeBufferPos;
        const int8_t readBufferSize;
        int8_t readBufferPos;
        int8_t readBufferAvail;
        BufferingMode mode;
    public:
        BaseBufferedRemoteTransport(BufferingMode bufferMode, int8_t readBufferSize, int writeBufferSize);
        ~BaseBufferedRemoteTransport() override;

        void endMsg() override;

        int writeChar(char data) override ;
        int writeStr(const char* data) override;
        uint8_t readByte() override;
        bool readAvailable() override;
        void close() override;

        virtual int fillReadBuffer(uint8_t* dataBuffer, int maxSize)=0;
    };

}

#endif //TCMENU_BASEBUFFEREDREMOTETRANSPORT_H
