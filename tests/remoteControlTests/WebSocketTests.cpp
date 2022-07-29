
#include <AUnit.h>
#include <tcMenu.h>
#include <remote/BaseRemoteComponents.h>
#include <SimpleCollections.h>
#include <remote/TcMenuWebServer.h>
#include "SimpleTestFixtures.h"
#include "UnitTestDriver.h"

using namespace aunit;
using namespace tcremote;

const char HTTP_REQUEST[] = "GET /chat HTTP/1.1\r\n"
                          "Host: server.example.com\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
                          "Origin: http://example.com\r\n"
                          "Sec-WebSocket-Protocol: chat, superchat\r\n"
                          "Sec-WebSocket-Version: 13\r\n\r\n";

const char EXPECTED_HTTP_RESPONSE[] = "HTTP/1.1 101 Switching Protocols\r\n"
                                      "Server: tccWS\r\n"
                                      "Upgrade: websocket\r\n"
                                      "Connection: Upgrade\r\n"
                                      "Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n";

TcMenuLightweightWebServer webServer(80, 1);

test(testPromoteWebSocket) {
    serdebugF("Starting websocket promote and protocol test");
    webServer.init();
    remoteServer.clearRemotes();
    remoteServer.addConnection(&rsc);

    rsc.runLoop();
    rsc.runLoop();

    assertTrue(wsInitialisation.isConnectionMade());
    assertTrue(wsTransport.connected());
    assertTrue(wsTransport.getState() == tcremote::WSS_IDLE);
    char header[192];
    int actual = wsTransport.getClientTxBytesRaw(header, sizeof header);
    header[actual] = 0;
    serdebugF2("Header sent: ", header);
    assertTrue(strcmp(EXPECTED_HTTP_RESPONSE, header)==0);

    wsTransport.simulateIncomingMsg(MSG_HEARTBEAT, "HI=5000|", true);
    wsTransport.simulateIncomingMsg(MSG_JOIN, "NM=unitTest|VE=103|PF=0|UU=db598308-9e31-451c-8511-25027bcf15fb|", true);

    serdebugF("Start iteration");
    bool commsEnded = false;
    uint32_t iterations = 0;
    while(!commsEnded && iterations++ < 100000) {
        rsc.runLoop();
        auto* msg = wsTransport.getReceivedMessages().getByKey(MSG_BOOTSTRAP);
        commsEnded = msg != nullptr && strncmp(msg->getData(), "BT=END|", 7) == 0;
    }
    serdebugF("Finished iteration");


    assertTrue(checkForMessageOfType(wsTransport.getReceivedMessages(), MSG_BOOT_ANALOG, "PI=0|ID=1|IE=2|RO=0|VI=1|NM=Volume|AU=dB|AM=255|AO=-190|AD=2|VC=0|\002"));
    assertTrue(checkForMessageOfType(wsTransport.getReceivedMessages(), MSG_BOOT_ENUM, "PI=0|ID=2|IE=4|RO=0|VI=1|NM=Channel|VC=0|NC=3|CA=CD Player|CB=Turntable|CC=Computer|\002"));
    assertTrue(checkForMessageOfType(wsTransport.getReceivedMessages(), MSG_BOOT_SUBMENU, "PI=0|ID=3|IE=65535|RO=0|VI=1|NM=Settings|\002"));
    assertTrue(checkForMessageOfType(wsTransport.getReceivedMessages(), MSG_BOOT_BOOL, "PI=3|ID=4|IE=65535|RO=0|VI=1|NM=12V Standby|VC=0|BN=2|\002"));
    assertTrue(commsEnded);

    serdebugF("Closing remotes");

    wsTransport.reset();
    remoteServer.clearRemotes();
    serdebugF("WS protocol test finished");
}
