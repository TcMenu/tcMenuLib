
#ifndef TCLIBRARYDEV_TCMENUECWEBSERVER_H
#define TCLIBRARYDEV_TCMENUECWEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <remote/TcMenuWebServer.h>
#include <remote/BaseRemoteComponents.h>

#define AVAILABLE_FOR_WRITE_WORKS false

/**
 * This global method absolutely must be provided when you are not using the pre-packaged version of embedCONTROL-JS
 * @param server The server reference to provide the static hosting and configuration details to.
 */

namespace tcremote {

    class TcMenuWebSockTransport : public tcremote::AbstractWebSocketTcMenuTransport {
    private:
        WiFiClient client;
    public:
        TcMenuWebSockTransport() = default;

        void setClient(WiFiClient newClient) {
            client = newClient;
            setState(tcremote::WSS_HTTP_REQUEST);
        }

        bool available() override {
#if AVAILABLE_FOR_WRITE_WORKS == true
            return client.availableForWrite();
#else
            return connected();
#endif
        }

        bool connected() override {
            return client.connected();
        }

        int performRawRead(uint8_t *buffer, size_t bufferSize) override {
            return client.read(buffer, bufferSize);
        }

        int performRawWrite(const uint8_t *data, size_t dataSize) override {
            return (int) client.write(data, dataSize);
        }
    };

    class TcMenuWebSockInitialisation : public tcremote::AbstractWebSocketTcMenuInitialisation {
    private:
        WiFiServer *server;
    public:
        TcMenuWebSockInitialisation(WiFiServer *server, const char *expectedPath)
                : AbstractWebSocketTcMenuInitialisation(expectedPath), server(server) {}

        bool attemptInitialisation() override;

        bool attemptNewConnection(tcremote::BaseRemoteServerConnection *remoteConnection) override;
    };

    class TcMenuWebServer : public tcremote::AbstractLightweightWebServer {
    private:
        TcMenuWebSockTransport transport;
        WiFiServer *server;
        unsigned long timeStart;
    public:
        explicit TcMenuWebServer(WiFiServer *server);

        tcremote::AbstractWebSocketTcMenuTransport *attemptNewConnection() override;

        void initialiseConnection() override;
    };

    int fromWiFiRSSITo4StateIndicator(int strength);
}

void prepareWebServer(tcremote::TcMenuWebServer& webServer);

#endif //TCLIBRARYDEV_TCMENUECWEBSERVER_H
