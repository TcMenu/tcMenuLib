
#include <AUnit.h>
#include <tcMenu.h>
#include <remote/BaseRemoteComponents.h>
#include <SimpleCollections.h>

using namespace aunit;
using namespace tcremote;

const ConnectorLocalInfo localInfo PROGMEM = { "unitTest", "699cb00d-200f-47f5-8d2e-471acbe4adb3"};
TagValueRemoteConnector connector0(0);
TcMenuRemoteServer remoteServer(localInfo);

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
    uint16_t rawDataOutPosition = 0;
    char rawDataIn[100];
    char rawDataOut[100];
    bool flushed = false;
    bool closed = false;
public:

    void reset() {
        flushed = false;
        closed = false;
        rawDataOutPosition = 0;
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
        while(data[written] != 0) {
            if(writeChar(data[written]) == 0) return written;
            written++;
        }
    }

    uint8_t readByte() override {
        if(!readAvailable()) return -1;
        return rawDataOut[rawDataOutPosition++];
    }

    bool readAvailable() override {
        return (rawDataOutPosition < sizeof(rawDataOut));
    }

    bool available() override {
        return !closed;
    }

    bool connected() override {
        return !closed;
    }

    void close() override {
        closed = true;
    }

    void endMsg() override {
        TagValueTransport::endMsg();
        rawDataIn[rawDataInPosition] = 0;
        uint16_t pos = 0;
        while(rawDataIn[pos] != START_OF_MESSAGE && pos < sizeof(rawDataIn)) pos++;
        if(rawDataIn[rawDataInPosition++] == START_OF_MESSAGE && rawDataIn[pos++] == TAG_VAL_PROTOCOL) {
            uint16_t msgTy = rawDataIn[pos++] << 8U;
            msgTy |= rawDataIn[pos++];
            receivedMessages.add(ReceivedMessage(receivedMessages.count(), msgTy, &rawDataIn[pos]));
        }
        rawDataInPosition = 0;
    }

    void pushByte(uint8_t data) {
        if(rawDataOutPosition < sizeof(rawDataOut)) {
            rawDataOut[rawDataOutPosition++] = (char)data;
        }
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

class TestTransportInitialisation : public DeviceInitialisation {
private:
    bool connectionMade = false;
    int initCount = 0;
public:
    bool attemptInitialisation() override {
        connectionMade = false;
        initCount++;
        return initialised;
    }

    void setInialisationDone(bool done) {
        initialised = done;
    }

    int getInitialisationAttempts() const {
        return initCount;
    }

    bool isConnectionMade() const {
        return connectionMade;
    }

    bool attemptNewConnection(TagValueTransport *transport) override {
        if(initialised) {
            auto* testTransport = reinterpret_cast<TestTagValTransport*>(transport);
            testTransport->reset();
            connectionMade = true;
            initCount = 0;
        }
        return false;
    }
};

bool checkForMessageOfType(BtreeList<uint16_t, ReceivedMessage> msgs, uint16_t msgType, const char* expected) {
    auto* rxMsg = msgs.getByKey(msgType);
    if(rxMsg == nullptr) {
        serdebugF2("message not found ", msgType);
    }

    char *actualData = rxMsg->getData();
    if(strcmp(actualData, expected) != 0) {
        serdebugF4("string mismatch: ", msgType, actualData, expected);
        return false;
    }
    
    return true;
}

test(testTcMenuRemoteServer) {
    TestTagValTransport testTransport;
    TestTransportInitialisation testInitialisation;
    RemoteServerConnection rsc(testTransport, testInitialisation);
    remoteServer.addConnection(&rsc);

    assertEqual(&connector0, remoteServer.getRemoteConnector(0));
    assertEqual(&testTransport, remoteServer.getTransport(0));
    assertEqual(1, remoteServer.remoteCount());
    assertEqual(nullptr, remoteServer.getRemoteConnector(1));
    assertEqual(nullptr, remoteServer.getTransport(1));

    taskManager.runLoop();
    assertEqual(1, testInitialisation.getInitialisationAttempts());
    testInitialisation.setInialisationDone(true);
    taskManager.runLoop();
    assertTrue(testInitialisation.isConnectionMade());

    testTransport.simulateIncomingMsg(MSG_HEARTBEAT, "HI=5000|");
    testTransport.simulateIncomingMsg(MSG_JOIN, "NM=unitTest|VE=103|PF=0|UU=db598308-9e31-451c-8511-25027bcf15fb|");

    bool commsEnded = false;
    int iterations = 0;
    while(!commsEnded || iterations++ > 1000) {
        taskManager.yieldForMicros(500);
        auto* msg = testTransport.getReceivedMessages().getByKey(MSG_BOOTSTRAP);
        commsEnded = msg != nullptr && strncmp(msg->getData(), "BT=END|", 7) == 0;
    }

    checkForMessageOfType(testTransport.getReceivedMessages(), MSG_BOOT_ANALOG, "");
    remoteServer.clearRemotes();
    testTransport.reset();
}
