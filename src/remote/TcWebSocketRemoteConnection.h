#ifndef TC_WEB_SOCKET_REMOTE_CONNECTION
#define TC_WEB_SOCKET_REMOTE_CONNECTION

#include "TcMenuWebServer.h"
#include "TcMenuHttpRequestProcessor.h"
#include "BaseRemoteComponents.h"

namespace tcremote {

    class DelegatingWebSocketTransport : public TagValueTransport {
    private:
        TagValueTransport *theDelegate;
        WebServerResponse *response;
    public:
        DelegatingWebSocketTransport() : TagValueTransport(TVAL_UNBUFFERED), theDelegate(nullptr), response(nullptr) {}

        void flush() override {
            if (theDelegate) theDelegate->flush();
        }

        int writeChar(char data) override {
            if (theDelegate) return theDelegate->writeChar(data);
            else return 0;
        }

        int writeStr(const char *data) override {
            if (theDelegate) return theDelegate->writeStr(data);
            else return 0;
        }

        uint8_t readByte() override {
            if (theDelegate) return theDelegate->readByte();
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
            if (theDelegate) theDelegate->endMsg();
        }

        void close() override {
            if (theDelegate && response) {
                theDelegate->close();
                response->setMode(tcremote::WebServerResponse::NOT_IN_USE);
            }
            theDelegate = nullptr;
            response = nullptr;
        }

        bool isInUse() {
            return theDelegate != nullptr && response != nullptr;
        }

        void assign(WebServerResponse *resp) {
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
        TcMenuWebSocketConnectionHandler() : noInitialisationNeeded(false),
                                             remoteServerConnection(delegatingTransport, noInitialisationNeeded) {}

        void init(TcMenuRemoteServer &server) {
            server.addConnection(&remoteServerConnection);
        }

        bool hasFreeConnection() { return !delegatingTransport.isInUse(); }

        void takeConnection(WebServerResponse *response) { delegatingTransport.assign(response); }
    };

} // tcremote namespace

#endif
