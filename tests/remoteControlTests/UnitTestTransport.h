
#ifndef UNITTEST_TRANSPORT_H
#define UNITTEST_TRANSPORT_H

#include <SCCircularBuffer.h>
#include <RemoteConnector.h>
#include <inttypes.h>

class ReceivedMessage {
private:
    uint16_t messageType;
    char data[100];

public:
    ReceivedMessage() : messageType(0), data {} { }
    ReceivedMessage(const ReceivedMessage& other) = default;
    ReceivedMessage(uint8_t order, uint16_t msgType, const char* dat) : messageType(msgType) {
        strncpy(data, dat, sizeof(data));
    }
    ReceivedMessage& operator= (const ReceivedMessage& other) = default;
    uint16_t getKey() const {
        return messageType;
    }
    char* getData() { return data; }
};

class TestTagValTransport : public TagValueTransport {
private:
    BtreeList<uint16_t, ReceivedMessage> receivedMessages;
    uint16_t rawDataInPosition = 0;
    char rawDataIn[100];
    SCCircularBuffer simOutBuffer;
    bool flushed = false;
    bool closed = true;
    bool closeCalled = false;
public:
    TestTagValTransport() : TagValueTransport(TVAL_UNBUFFERED), simOutBuffer(128) {}

    void reset() {
        flushed = false;
        closed = true;
        closeCalled = false;
        rawDataInPosition = 0;
        receivedMessages.clear();
    }

    void flush() override {
        flushed = true;
    }

    int writeChar(char data) override {
        if(rawDataInPosition >= sizeof(rawDataIn)) return 0;
        rawDataIn[rawDataInPosition++] = data;
        return 1;
    }

    int writeStr(const char *data) override {
        int written = 0;
        while(*data) {
            if(!writeChar(*data)) break;
            data++;
            written++;
        }
        return written;
    }

    uint8_t readByte() override {
        if(!readAvailable()) return -1;
        return simOutBuffer.get();
    }

    bool readAvailable() override {
        return (simOutBuffer.available());
    }

    bool available() override {
        return !closed;
    }

    bool connected() override {
        return !closed;
    }

    void simulateConnection() {
        closed = false;
    }

    void close() override {
        closeCalled = true;
    }

    void startMsg(uint16_t msgType) override {
        memset(rawDataIn, 0, sizeof(rawDataIn));
        rawDataInPosition = 0;
        TagValueTransport::startMsg(msgType);
    }

    void endMsg() override {
        TagValueTransport::endMsg();
        rawDataIn[rawDataInPosition] = 0;
        uint16_t pos = 0;
        while(rawDataIn[pos] != START_OF_MESSAGE && pos < sizeof(rawDataIn)) pos++;
        if(rawDataIn[pos++] == START_OF_MESSAGE && rawDataIn[pos++] == TAG_VAL_PROTOCOL) {
            uint16_t msgTy = rawDataIn[pos++] << 8U;
            msgTy |= rawDataIn[pos++];
            receivedMessages.add(ReceivedMessage(receivedMessages.count(), msgTy, &rawDataIn[pos]));
        }
    }

    void pushByte(uint8_t data) {
        simOutBuffer.put(data);
    }

    void simulateIncomingMsg(uint16_t msgType, const char* data) {
        pushByte(START_OF_MESSAGE);
        pushByte(TAG_VAL_PROTOCOL);
        pushByte(msgType >> 8U);
        pushByte(msgType & 0xffU);
        while(*data) {
            pushByte(*data);
            data++;
        }
        pushByte(0x02);
    }

    BtreeList<uint16_t, ReceivedMessage>& getReceivedMessages() {
        return receivedMessages;
    }
};

bool checkForMessageOfType(BtreeList<uint16_t, ReceivedMessage> msgs, uint16_t msgType, const char* expected);

#endif // UNITTEST_TRANSPORT_H
