//
// Created by dave on 18/06/2022.
//

#include "tcMenuECWebServer.h"

using namespace tcremote;

bool areWeConnected() {
#ifdef ARDUINO_ARCH_STM32
    // we'll keep checking if the link is up before trying to initialise further
    return (Ethernet.linkStatus() != LinkOFF);
#elif defined(ESP32) || defined(ESP8266)
    return (WiFi.isConnected());
#else
    return true;
#endif
}

bool TcMenuWebSockInitialisation::attemptInitialisation() {
    if(!areWeConnected()) return false;

    serdebugF("Initialising server ");
    this->server->begin();
    initialised = true;
    return initialised;

}

bool TcMenuWebSockInitialisation::attemptNewConnection(tcremote::BaseRemoteServerConnection* remoteConnection) {
    auto client = server->available();
    if(client) {
        serdebugF("Client found");
        auto* tvCon = reinterpret_cast<TagValueRemoteServerConnection*>(remoteConnection);
        auto* wsTransport = reinterpret_cast<TcMenuWebSockTransport*>(tvCon->transport());
        wsTransport->setClient(client);
        if(performUpgradeOnClient(wsTransport)) {
            serdebugF("Transport upgraded");
            return true;
        } else {
            tvCon->transport()->close();
            return false;
        }
    } else return false;
}

AbstractWebSocketTcMenuTransport *TcMenuWebServer::attemptNewConnection() {
    if(transport.connected()) return nullptr; // doing something already, cannot reconnect yet.

    EthernetClient cl = server->available();
    if(cl) {
        transport.setClient(cl);
        return &transport;
    }

    return nullptr;
}

TcMenuWebServer::TcMenuWebServer(EthernetServer *server) : transport(), server(server) {}
