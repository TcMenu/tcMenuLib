
#include <AUnit.h>
#include <tcMenu.h>
#include <remote/BaseRemoteComponents.h>
#include <RemoteConnector.h>
#include <SimpleCollections.h>
#include <remote/TcMenuWebServer.h>
#include "SimpleTestFixtures.h"
#include "UnitTestDriver.h"
#include "UnitTestDriver.h"

using namespace aunit;
using namespace tcremote;

const char HTTP_WS_REQUEST[] = "GET /chat HTTP/1.1\r\n"
                          "Host: server.example.com\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                          "Origin: http://example.com\r\n"
                          "Sec-WebSocket-Protocol: chat, superchat\r\n"
                          "Sec-WebSocket-Version: 13\r\n\r\n";

const char EXPECTED_HTTP_WS_RESPONSE[] = "HTTP/1.1 101 Switching Protocols\r\n"
                                      "Server: tccWS\r\n"
                                      "Upgrade: websocket\r\n"
                                      "Connection: Upgrade\r\n"
                                      "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n";

const PROGMEM  ConnectorLocalInfo applicationInfo = { "Unit test", "cc66fbc2-6557-42fa-a098-44466b48ac42" };
TcMenuRemoteServer wsRemoteServer(applicationInfo);

class DelegatingWebSocketTransport : public TagValueTransport {
private:
    TagValueTransport* theDelegate;
    WebServerResponse* response;
public:
    DelegatingWebSocketTransport() : TagValueTransport(TVAL_UNBUFFERED), theDelegate(nullptr), response(nullptr) {}

    void flush() override {
        if(theDelegate) theDelegate->flush();
    }

    int writeChar(char data) override {
        if(theDelegate) return theDelegate->writeChar(data);
        else return 0;
    }

    int writeStr(const char *data) override {
        if(theDelegate) return theDelegate->writeStr(data);
        else return 0;
    }

    uint8_t readByte() override {
        if(theDelegate) return theDelegate->readByte();
        else return -1;
    }

    bool readAvailable() override {
        return (theDelegate) != nullptr && theDelegate->readAvailable();
    }

    bool available() override {
        return (theDelegate) != nullptr && theDelegate->available();
    }

    bool connected() override {
        return (theDelegate) != nullptr && theDelegate->connected();
    }

    void endMsg() override {
        if(theDelegate) theDelegate->endMsg();
    }

    void close() override {
        if(theDelegate && response) {
            theDelegate->close();
            response->setMode(tcremote::WebServerResponse::NOT_IN_USE);
        }
        theDelegate = nullptr;
        response = nullptr;
    }

    bool isInUse() {
        return theDelegate != nullptr && response != nullptr;
    }

    void assign(WebServerResponse* resp) {
        this->response = resp;
        this->theDelegate = resp->getTransport();
    }
};

class TcMenuWebSocketConnectionHandler {
private:
    NoInitialisationNeeded noInitialisationNeeded;
    TagValueRemoteServerConnection remoteServerConnection;
    DelegatingWebSocketTransport delegatingTransport;
public:
    TcMenuWebSocketConnectionHandler() : noInitialisationNeeded(false), remoteServerConnection(delegatingTransport, noInitialisationNeeded) {}

    void init(TcMenuRemoteServer& server) {
        server.addConnection(&remoteServerConnection);
    }

    bool hasFreeConnection() { return !delegatingTransport.isInUse();}
    void takeConnection(WebServerResponse* response) { delegatingTransport.assign(response); }
} connectionHandler;

test(testPromoteWebSocket) {
    taskManager.reset();
    TcMenuLightweightWebServer webServer(80, 1);

    webServer.onUrlGet("/chat", [](tcremote::WebServerResponse& response) {
        if(connectionHandler.hasFreeConnection()) {
            response.turnRequestIntoWebSocket();
            connectionHandler.takeConnection(&response);
        } else {
            response.sendError(500);
        }
    });

    serdebugF("Starting websocket promote and protocol test");
    resetUnitLayer();
    webServer.init();
    connectionHandler.init(wsRemoteServer);

    startNetLayerDhcp();
    webServer.exec();
    assertTrue(webServer.isInitialised());

    // Now make a valid request for a simple document.
    simulateAccept();
    assertFalse(driverSocket.isIdle());
    driverSocket.simulateIncomingRaw(HTTP_WS_REQUEST);
    webServer.exec();
    webServer.getWebResponse(0)->exec();
    assertTrue(driverSocket.checkResponseAgainst(EXPECTED_HTTP_WS_RESPONSE));
    assertFalse(driverSocket.didClose());
    assertTrue(webServer.getWebResponse(0)->getMode() == tcremote::WebServerResponse::WEBSOCKET_BUSY);
    driverSocket.setShouldBeInWebSocketMode(true);

    driverSocket.simulateIncomingMsg(MSG_HEARTBEAT, "HI=5000|", true);
    driverSocket.simulateIncomingMsg(MSG_JOIN, "NM=unitTest|VE=103|PF=0|UU=db598308-9e31-451c-8511-25027bcf15fb|", true);

    serdebugF("Start iteration");
    bool commsEnded = false;
    uint32_t iterations = 0;
    while(!commsEnded && iterations++ < 100000) {
        wsRemoteServer.exec();
        auto* msg = driverSocket.getReceivedMessages().getByKey(MSG_BOOTSTRAP);
        commsEnded = msg != nullptr && strncmp(msg->getData(), "BT=END|", 7) == 0;
    }
    serdebugF("Finished iteration");

    assertTrue(checkForMessageOfType(driverSocket.getReceivedMessages(), MSG_BOOT_ANALOG, "PI=0|ID=1|IE=2|RO=0|VI=1|NM=Volume|AU=dB|AM=255|AO=-190|AD=2|VC=0|\002"));
    assertTrue(checkForMessageOfType(driverSocket.getReceivedMessages(), MSG_BOOT_ENUM, "PI=0|ID=2|IE=4|RO=0|VI=1|NM=Channel|VC=0|NC=3|CA=CD Player|CB=Turntable|CC=Computer|\002"));
    assertTrue(checkForMessageOfType(driverSocket.getReceivedMessages(), MSG_BOOT_SUBMENU, "PI=0|ID=3|IE=65535|RO=0|VI=1|NM=Settings|\002"));
    assertTrue(checkForMessageOfType(driverSocket.getReceivedMessages(), MSG_BOOT_BOOL, "PI=3|ID=4|IE=65535|RO=0|VI=1|NM=12V Standby|VC=0|BN=2|\002"));

    serdebugF("Closing remotes");

    resetUnitLayer();
    wsRemoteServer.clearRemotes();

    serdebugF("WS protocol test finished");
}
