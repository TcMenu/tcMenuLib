/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file BaseBufferedRemoteTransport.h
 * @brief helper classes for dealing with buffering remote transports
 */

#ifndef TCMENU_BASEBUFFEREDREMOTETRANSPORT_H
#define TCMENU_BASEBUFFEREDREMOTETRANSPORT_H

#include <RemoteConnector.h>
#define TICKS_TO_FLUSH_WRITE 140

namespace tcremote {

    enum BufferingMode : uint8_t {
        BUFFER_ONE_MESSAGE, BUFFER_MESSAGES_TILL_FULL
    };

    /**
     * An implementation of this class can both encrypt and decrypt data on behalf of a BaseBufferedTagValTransport
     * instance.
     */
    class EncryptionHandler {
    public:
        /**
         * Encrypt plain text data into encrypted format into the buffer
         * @param plainText the plain bytes to encrypt
         * @param bytesIn the number of bytes to encrypt
         * @param buffer the output encrypted message
         * @param buffLen the buffer maximum length
         * @return the number of bytes encrypted or 0 if it fails.
         */
        virtual int encryptData(const uint8_t *plainText, int bytesIn, const uint8_t *buffer, size_t buffLen) = 0;
        /**
         * Decrypt data from the wire into plain text and store the output into the buffer
         * @param encoded the encoded data to decrypt
         * @param bytesIn the number of encoded bytes to decrypt
         * @param buffer the buffer to output plaintext to
         * @param buffLen the size of the buffer
         * @return the number of bytes returned, or 0 if it fails.
         */
        virtual int decryptData(const uint8_t *encoded, int bytesIn, const uint8_t *buffer, size_t buffLen) = 0;
    };


    /**
     * Many transports need buffering of messages, for example the regular Ethernet2 library will send
     * a character at a time with no buffering. The BLE driver requires buffering because it is only legal
     * to send single full messages at a time. Some other drivers can benefit from some level of buffering.
     * To avoid writing this code many times in different contexts it is available in this core package.
     */
    class BaseBufferedRemoteTransport : public TagValueTransport {
    protected:
        const uint16_t writeBufferSize;
        const uint16_t readBufferSize;
        uint8_t *readBuffer;
        uint8_t *writeBuffer;
        uint8_t *encryptionBuffer;
        uint16_t writeBufferPos;
        uint16_t readBufferPos;
        uint16_t encryptionBufferPos;
        uint16_t readBufferAvail;
        EncryptionHandler* encryptionHandler;
        BufferingMode mode;
        uint8_t ticksSinceWrite;
    public:
        BaseBufferedRemoteTransport(BufferingMode bufferMode, uint8_t readBufferSize, uint8_t writeBufferSize,
                                    EncryptionHandler* encHandler = nullptr);

        ~BaseBufferedRemoteTransport() override;

        void endMsg() override;

        int writeChar(char data) override;

        int writeStr(const char *data) override;

        uint8_t readByte() override;

        bool readAvailable() override;

        void close() override;

        void flushIfRequired();

        void flushInternal();

        virtual int fillReadBuffer(uint8_t *dataBuffer, int maxSize) = 0;
    };
}

#endif //TCMENU_BASEBUFFEREDREMOTETRANSPORT_H
