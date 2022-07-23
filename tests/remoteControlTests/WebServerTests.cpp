
#include <AUnit.h>
#include <remote/BaseRemoteComponents.h>
#include <SimpleCollections.h>
#include <remote/TcMenuWebServer.h>
#include "SimpleTestFixtures.h"
#include "UnitTestDriver.h"

using namespace aunit;
using namespace tcremote;
extern TcMenuRemoteServer remoteServer;

const char HTTP_REQ_GET1[]= "GET /index.html HTTP/1.1\r\n"
                            "Host: server.example.com\r\n"
                            "User-Agent: TurboBanger\r\n"
                            "Connection: close\r\n"
                            "Accept-Language: en, de\r\n\r\n";
const char HTTP_REQ_404[]= "GET /notFound.html HTTP/1.1\r\n"
                            "Host: server.example.com\r\n"
                            "User-Agent: TurboBanger\r\n"
                            "Connection: close\r\n"
                            "Accept-Language: en, de\r\n\r\n";
const char HTTP_REQ_POST[]= "POST /my/post.do HTTP/1.1\r\n"
                            "Host: server.example.com\r\n"
                            "User-Agent: TurboBanger\r\n"
                            "Connection: close\r\n"
                            "Accept-Language: en, de\r\n\r\n";
const char HTTP_REQ_GET2[]= "GET /data1.txt HTTP/1.1\r\n"
                            "Host: server.example.com\r\n"
                            "User-Agent: TurboBanger\r\n"
                            "Accept-Language: en, de\r\n\r\n"
                            "GET /data2.txt HTTP/1.1\r\n"
                            "Host: server.example.com\r\n"
                            "User-Agent: TurboBanger\r\n"
                            "Accept-Language: en, de\r\n\r\n";
const char HTTP_REQ_ERROR1[]= "Not a http request sent to wrong port\r\n\r\n";
const char HTTP_REQ_ERROR2[]= "GET wrong\r\n\r\n";

const char EXPECTED_RESP1[] = "HTTP/1.1 200 OK\r\n"
                              "Server: tccWS\r\n"
                              "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
                              "Content-Type: text/html\r\n"
                              "Content-Length: 73\r\n"
                              "\r\n"
                              "<html><head><title>hello</title></head><body><h1>index</h1></body></html>";
const char EXPECTED_RESP2[] = "HTTP/1.1 404 Not found\r\n"
                              "Server: tccWS\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: 0\r\n"
                              "\r\n";
const char EXPECTED_RESP3[] = "HTTP/1.1 200 OK\r\n"
                              "Server: tccWS\r\n"
                              "Cache-Control: never\r\n"
                              "Content-Type: text/html\r\n"
                              "Content-Length: 72\r\n"
                              "\r\n"
                              "<html><head><title>hello</title></head><body><h1>post</h1></body></html>";
const char EXPECTED_RESP4[] = "HTTP/1.1 500 Internal error\r\n"
                              "Server: tccWS\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: 0\r\n"
                              "\r\n";
const char EXPECTED_RESP5[] = "HTTP/1.1 200 OK\r\n"
                              "Server: tccWS\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: 11\r\n"
                              "\r\n"
                              "Hello WorldHTTP/1.1 200 OK\r\n"
                              "Server: tccWS\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: 5\r\n"
                              "\r\n"
                              "Aloha";

void copyHtmlWithTitleAndText(const char* title, const char* heading, char* buffer) {
    strcpy(buffer, "<html><head><title>");
    strcat(buffer, title);
    strcat(buffer, "</title></head><body><h1>");
    strcat(buffer, heading);
    strcat(buffer, "</h1></body></html>");
}

class TestTcMenuWebServer : public TcMenuLightweightWebServer {
private:
    UnitTestWebSockTransport transport;
    const char* requestUrl{};
public:
    explicit TestTcMenuWebServer(bsize_t sz) : transport(sz) {}

    void setRequestUrl(const char* r) { requestUrl = r; }

    AbstractWebSocketTcMenuTransport* attemptNewConnection() override {
        if(requestUrl) {
            transport.reset(true);
            transport.simulateRxFromClient(requestUrl);
            return &transport;
        } else return nullptr;
    }

    bool checkResponseAgainst(const char* expected) {
        transport.getClientTxBytesRaw((char*)transport.getReadBuffer(), strlen(expected));
        char* actual = (char*)transport.getReadBuffer();
        actual[strlen(expected)] = 0;
        bool cmp = strncmp(expected, actual, strlen(expected)) == 0;
        if(!cmp) {
            serdebugF2("Mismatch actual was ", actual);
        }
        return cmp;
    }

    bool isTransportClosed() {
        return transport.didClose();
    }
};

test(testAbstractTcMenuWebServer) {
    TestTcMenuWebServer webServer(200);

    webServer.onUrlGet("/index.html", [](tcremote::WebServerResponse& response) {
        response.startHeader();
        response.setHeader(WSH_LAST_MODIFIED, "Wed, 22 Jul 2009 19:15:56 GMT");
        char sz[96];
        copyHtmlWithTitleAndText("hello", "index", sz);
        response.contentInfo(tcremote::WebServerResponse::HTML_TEXT, strlen(sz));
        response.send((uint8_t*)sz, sizeof sz);
    });

    webServer.onUrlGet("/data1.txt", [](tcremote::WebServerResponse& response) {
        response.startHeader();
        response.contentInfo(tcremote::WebServerResponse::PLAIN_TEXT, 11);
        response.send("Hello World", 11);
    });

    webServer.onUrlGet("/data2.txt", [](tcremote::WebServerResponse& response) {
        response.startHeader();
        response.contentInfo(tcremote::WebServerResponse::PLAIN_TEXT, 5);
        response.startData();
        response.send("Aloha", 5);
        response.end();
    });

    webServer.onUrlPost("/my/post.do", [](tcremote::WebServerResponse& response) {
        response.startHeader();
        response.setHeader(WSH_CACHE_CONTROL, "never");
        char sz[96];
        copyHtmlWithTitleAndText("hello", "post", sz);
        response.contentInfo(tcremote::WebServerResponse::HTML_TEXT, strlen(sz));
        response.startData();
        response.send((uint8_t*)sz, sizeof sz);
        response.end();
    });

    webServer.exec();
    assertTrue(webServer.isInitialised());

    webServer.setRequestUrl(HTTP_REQ_GET1);
    webServer.timeOfNextCheck();
    assertTrue(webServer.isTriggered());
    webServer.exec();
    assertTrue(webServer.checkResponseAgainst(EXPECTED_RESP1));
    assertTrue(webServer.isTransportClosed());

    webServer.setRequestUrl(HTTP_REQ_404);
    webServer.timeOfNextCheck();
    assertTrue(webServer.isTriggered());
    webServer.exec();
    assertTrue(webServer.checkResponseAgainst(EXPECTED_RESP2));
    assertTrue(webServer.isTransportClosed());

    webServer.setTriggered(false);
    webServer.setRequestUrl(nullptr);
    webServer.timeOfNextCheck();
    webServer.timeOfNextCheck();
    assertFalse(webServer.isTriggered());

    webServer.setRequestUrl(HTTP_REQ_POST);
    webServer.timeOfNextCheck();
    assertTrue(webServer.isTriggered());
    webServer.exec();
    assertTrue(webServer.checkResponseAgainst(EXPECTED_RESP3));

    webServer.setRequestUrl(HTTP_REQ_ERROR1);
    webServer.timeOfNextCheck();
    assertTrue(webServer.isTriggered());
    webServer.exec();
    assertTrue(webServer.checkResponseAgainst(EXPECTED_RESP4));
    assertTrue(webServer.isTransportClosed());

    webServer.setRequestUrl(HTTP_REQ_ERROR2);
    webServer.timeOfNextCheck();
    assertTrue(webServer.isTriggered());
    webServer.exec();
    assertTrue(webServer.checkResponseAgainst(EXPECTED_RESP4));
    assertTrue(webServer.isTransportClosed());

    webServer.setRequestUrl(HTTP_REQ_GET2); // get 2 items in same connection
    webServer.timeOfNextCheck();
    assertTrue(webServer.isTriggered());
    webServer.exec();
    assertTrue(webServer.checkResponseAgainst(EXPECTED_RESP5));
    delay(3000); // make sure we exceed the delay for closing the socket when idle.
    webServer.exec();
    assertTrue(webServer.isTransportClosed());
}
