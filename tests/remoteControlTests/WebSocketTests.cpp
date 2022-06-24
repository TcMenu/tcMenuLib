
#include <AUnit.h>
#include <tcMenu.h>
#include <remote/BaseRemoteComponents.h>
#include <SimpleCollections.h>
#include <remote/TcMenuWebSocket.h>
#include "SimpleTestFixtures.h"
#include "UnitTestTransport.h"

using namespace aunit;
using namespace tcremote;
extern TcMenuRemoteServer remoteServer;

const uint8_t serverMask[] = {0xaa, 0xee, 0xdd, 0xa0};

const char HTTP_REQUEST[] = "GET /chat HTTP/1.1\r\n"
                          "Host: server.example.com\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                          "Origin: http://example.com\r\n"
                          "Sec-WebSocket-Protocol: chat, superchat\r\n"
                          "Sec-WebSocket-Version: 13\r\n\r\n";

const char EXPECTED_HTTP_RESPONSE[] = "HTTP/1.1 101 Switching Protocols\r\n"
                                    "Upgrade: websocket\r\n"
                                    "Connection: Upgrade\r\n"
                                    "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n";

class UnitTestWebSockTransport : public AbstractWebSocketTcMenuTransport {
private:
    bool isConnected;
    bool hasClosed;
    SCCircularBuffer readScBuffer;
    SCCircularBuffer writeScBuffer;
    BtreeList<uint16_t, ReceivedMessage> receivedMessages;
public:
    UnitTestWebSockTransport(): isConnected(false), hasClosed(false), readScBuffer(512), writeScBuffer(512) {}

    int performRawRead(uint8_t *buffer, size_t bufferSize) override {
        int pos = 0;
        while(readScBuffer.available() && pos < bufferSize) {
            buffer[pos] = readScBuffer.get();
            pos++;
        }
        return pos;
    }

    void simulateRxFromClient(const char* data) {
        while(*data) {
            readScBuffer.put(*data);
            data++;
        }
    }

    int performRawWrite(const uint8_t *data, size_t dataSize) override {
        size_t pos = 0;
        while(pos < dataSize) {
            writeScBuffer.put(data[pos]);
            pos++;
        }
        return pos;
    }

    bool available() override { return true;}

    int getClientTxBytesRaw(char *data, int size) {
        int pos=0;
        while(writeScBuffer.available() && pos < size ) {
            data[pos] = (char)(writeScBuffer.get());
            pos++;
        }
        return pos;
    }

    void reset(bool connectionState = false) {
        // clear out both buffers and reset to not connected.
        while(readScBuffer.available()) readScBuffer.get();
        while(writeScBuffer.available()) writeScBuffer.get();
        isConnected = connectionState;
        bytesLeftInCurrentMsg = 0;
        frameMaskingPosition = 0;
        writePosition = 0;
        readAvail = 0;
        readPosition = 0;
        currentState = WSS_NOT_CONNECTED;
    }

    void close() override {
        hasClosed = true;
    }

    bool connected() override {
        return isConnected;
    }

    WebSocketTransportState getState() { return currentState; }

    void simulateIncomingMsg(uint16_t msgType, const char *data, bool masked) {

        readScBuffer.put(0x81);
        auto dataLen = strlen(data) + 5;
        uint8_t maskData = masked ? 0x80 : 0;
        if(dataLen > 125) {
            readScBuffer.put(maskData | 126);
            readScBuffer.put(dataLen >> 8);
            readScBuffer.put(dataLen & 0xff);
        } else {
            readScBuffer.put(maskData | dataLen);
        }

        if(masked) {
            readScBuffer.put(serverMask[0]); // mask 4 bytes
            readScBuffer.put(serverMask[1]);
            readScBuffer.put(serverMask[2]);
            readScBuffer.put(serverMask[3]);
        }
        else {
            readScBuffer.put(dataLen);
        }
        readScBuffer.put(0x01 ^ serverMask[0]); // start
        readScBuffer.put(0x01 ^ serverMask[1]); // tag val
        readScBuffer.put((msgType >> 8) ^ serverMask[2]);
        readScBuffer.put((msgType & 0xff) ^ serverMask[3]);
        int maskPosition = 0;
        while(*data) { // data
            readScBuffer.put(*data ^ serverMask[maskPosition%4]);
            maskPosition++;
            data++;
        }
        readScBuffer.put(0x02 ^ serverMask[maskPosition%4]); // end
    }

    BtreeList<uint16_t, ReceivedMessage>& getReceivedMessages() {
        return receivedMessages;
    }

    void flush() override {
        AbstractWebSocketTcMenuTransport::flush();
        int fl = writeScBuffer.get();
        if(fl != 0x81) return; // Final message, text
        int len = writeScBuffer.get();
        if(len > 125) {
            return; // don't handle extended case
        }
        char sz[128];
        int i;
        for(i=0;i<len;i++) {
            sz[i] = writeScBuffer.get();
        }
        sz[i]=0;
        int pos = 0;
        if(sz[pos++] != 0x01 || sz[pos++] != 0x01) return;
        uint16_t msgTy = sz[pos++] << 8U;
        msgTy |= sz[pos++];

        receivedMessages.add(ReceivedMessage(receivedMessages.count(), msgTy, &sz[pos]));

    }
};

class UnitTestWebSockInitialisation : public AbstractWebSocketTcMenuInitialisation {
private:
    bool connectionMade = false;
public:
    UnitTestWebSockInitialisation(const char *expectedPath) : AbstractWebSocketTcMenuInitialisation(expectedPath) {}

    bool attemptInitialisation() override {
        initialised = true;
        return initialised;
    }

    bool attemptNewConnection(BaseRemoteServerConnection *connection) override {
        auto* tagValConnector = reinterpret_cast<TagValueRemoteServerConnection*>(connection);
        auto* testTransport = reinterpret_cast<UnitTestWebSockTransport*>(tagValConnector->transport());
        testTransport->reset(true);
        connectionMade = true;
        testTransport->simulateRxFromClient(HTTP_REQUEST);
        return performUpgradeOnClient(testTransport);
    }

    bool isConnectionMade() { return connectionMade; }
};

test(testPromoteWebSocket) {
    remoteServer.clearRemotes();
    UnitTestWebSockTransport wsTransport;
    UnitTestWebSockInitialisation wsInitialisation("/chat");
    TagValueRemoteServerConnection rsc(wsTransport, wsInitialisation);
    remoteServer.addConnection(&rsc);

    rsc.runLoop();
    rsc.runLoop();

    assertTrue(wsInitialisation.isConnectionMade());
    assertTrue(wsTransport.connected());
    assertTrue(wsTransport.getState() == tcremote::WSS_IDLE);
    char header[192];
    int actual = wsTransport.getClientTxBytesRaw(header, sizeof header);
    header[actual] = 0;
    assertTrue(strcmp(EXPECTED_HTTP_RESPONSE, header)==0);

    wsTransport.simulateIncomingMsg(MSG_HEARTBEAT, "HI=5000|", true);
    wsTransport.simulateIncomingMsg(MSG_JOIN, "NM=unitTest|VE=103|PF=0|UU=db598308-9e31-451c-8511-25027bcf15fb|", true);

    bool commsEnded = false;
    uint32_t iterations = 0;
    while(!commsEnded && iterations++ < 100000) {
        rsc.runLoop();
        auto* msg = wsTransport.getReceivedMessages().getByKey(MSG_BOOTSTRAP);
        commsEnded = msg != nullptr && strncmp(msg->getData(), "BT=END|", 7) == 0;
    }

    assertTrue(checkForMessageOfType(wsTransport.getReceivedMessages(), MSG_BOOT_ANALOG, "PI=0|ID=1|IE=2|RO=0|VI=1|NM=Volume|AU=dB|AM=255|AO=-190|AD=2|VC=0|\002"));
    assertTrue(checkForMessageOfType(wsTransport.getReceivedMessages(), MSG_BOOT_ENUM, "PI=0|ID=2|IE=4|RO=0|VI=1|NM=Channel|VC=0|NC=3|CA=CD Player|CB=Turntable|CC=Computer|\002"));
    assertTrue(checkForMessageOfType(wsTransport.getReceivedMessages(), MSG_BOOT_SUBMENU, "PI=0|ID=3|IE=65535|RO=0|VI=1|NM=Settings|\002"));
    assertTrue(checkForMessageOfType(wsTransport.getReceivedMessages(), MSG_BOOT_BOOL, "PI=3|ID=4|IE=65535|RO=0|VI=1|NM=12V Standby|VC=0|BN=2|\002"));
    assertTrue(commsEnded);

    wsTransport.reset();
    remoteServer.clearRemotes();
}
