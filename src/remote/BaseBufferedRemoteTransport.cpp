/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "BaseBufferedRemoteTransport.h"

namespace tcremote {

    BaseBufferedRemoteTransport::BaseBufferedRemoteTransport(BufferingMode bufferMode, uint8_t readBufferSize,
                                                             uint8_t writeBufferSize, EncryptionHandler* encHandler)
            : TagValueTransport(TVAL_BUFFERED), writeBufferSize(writeBufferSize),
              readBufferSize(readBufferSize), writeBufferPos(0), readBufferPos(0), encryptionBufferPos(0), readBufferAvail(0),
              encryptionHandler(encHandler), mode(bufferMode),
              ticksSinceWrite(0) {
        if(mode != BUFFER_ONE_MESSAGE && encHandler != nullptr) {
            serlogF(SER_ERROR, "EncHandler requires mode=BUFFER_ONE_MESSAGE");
            encHandler = nullptr; // turn off encryption, will not work in any other mode
        }
        readBuffer = new uint8_t[readBufferSize];
        writeBuffer = new uint8_t[writeBufferSize];
        encryptionBuffer = new uint8_t[readBufferSize];
    }

    BaseBufferedRemoteTransport::~BaseBufferedRemoteTransport() {
        delete[] readBuffer;
        delete[] writeBuffer;
    }

    void BaseBufferedRemoteTransport::endMsg() {
        TagValueTransport::endMsg();
        if (mode == BUFFER_ONE_MESSAGE) flushInternal();
    }

    uint8_t BaseBufferedRemoteTransport::readByte() {
        if (!readAvailable()) return -1;
        auto ch = readBuffer[readBufferPos];
        readBufferPos += 1;
        // only uncomment the below for worst case debugging.
        //serlogF2(SER_DEBUG, "readByte ", ch);
        return ch;
    }

    bool BaseBufferedRemoteTransport::readAvailable() {
        if (readBufferAvail && readBufferPos < readBufferAvail) {
            return true;
        }

        if(encryptionHandler != nullptr && encryptionBuffer != nullptr) {
            readBufferAvail = readBufferPos = 0;
            if(encryptionBufferPos < 2) {
                encryptionBufferPos = (uint16_t) fillReadBuffer(encryptionBuffer, readBufferSize);
            }
            if(encryptionBufferPos >= 2) {
                int encryptionSize = (encryptionBuffer[0] << 8) + encryptionBuffer[1];
                if(encryptionBufferPos >= encryptionSize) {
                    int len = encryptionHandler->decryptData(&encryptionBuffer[2], encryptionBufferPos - 2, readBuffer, readBufferSize);
                    readBufferAvail = len;
                    readBufferPos = 0;
                    encryptionBufferPos = 0;
                    return len > 0;
                }
            }
        } else {
            readBufferAvail = (uint16_t) fillReadBuffer(readBuffer, readBufferSize);
            readBufferPos = 0;
        }
        return readBufferPos < readBufferAvail;
    }

    int BaseBufferedRemoteTransport::writeChar(char data) {
        if (writeBufferPos >= writeBufferSize) {
            // we've exceeded the buffer size so we must flush, and then ensure
            // that flush actually did something and there is now capacity.
            flushInternal();
            if (writeBufferPos >= writeBufferSize) return 0;// we did not write so return an error condition.
        }
        writeBuffer[writeBufferPos++] = data;
        ticksSinceWrite = 0;
        return 1;
    }

    int BaseBufferedRemoteTransport::writeStr(const char *data) {
        // only uncomment below for worst case debugging..
        //	serlogF2(SER_NETWORK_DEBUG, "writing ", data);

        size_t len = strlen(data);
        for (size_t i = 0; i < len; ++i) {
            if (writeChar(data[i]) == 0) {
                return 0;
            }
        }
        return (int) len;
    }

    void BaseBufferedRemoteTransport::flushIfRequired() {
        if (!connected() || writeBufferPos == 0 || mode == BUFFER_ONE_MESSAGE) return;

        if (ticksSinceWrite < TICKS_TO_FLUSH_WRITE) ++ticksSinceWrite;
        if (ticksSinceWrite == TICKS_TO_FLUSH_WRITE) {
            ticksSinceWrite = 0xff;
            flush();
        }
    }

    void BaseBufferedRemoteTransport::close() {
        writeBufferPos = 0;
        readBufferPos = 0;
        readBufferAvail = 0;
        currentField.msgType = UNKNOWN_MSG_TYPE;
        currentField.fieldType = FVAL_PROCESSING_AWAITINGMSG;
    }

    void BaseBufferedRemoteTransport::flushInternal() {
        if(encryptionHandler != nullptr && encryptionBuffer != nullptr) {
            int written = encryptionHandler->encryptData(writeBuffer, writeBufferPos, encryptionBuffer, writeBufferSize);
            if(written == 0) {
                serlogF(SER_ERROR, "Net encrypt fail");
                close();
            }  else {
                memcpy(writeBuffer, encryptionBuffer, written);
                writeBufferPos = written;
                flush();
            }
        } else {
            flush();
        }
    }
}