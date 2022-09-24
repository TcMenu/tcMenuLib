
#include <AUnit.h>
#include <remote/BaseRemoteComponents.h>
#include <SimpleCollections.h>
#include "remote/TcMenuWebServer.h"
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
                              "Connection: close\r\n"
                              "Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n"
                              "Content-Type: text/html\r\n"
                              "Content-Length: 73\r\n"
                              "\r\n"
                              "<html><head><title>hello</title></head><body><h1>index</h1></body></html>";
const char EXPECTED_RESP2[] = "HTTP/1.1 404 Not found\r\n"
                              "Server: tccWS\r\n"
                              "Connection: close\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: 0\r\n"
                              "\r\n";
const char EXPECTED_RESP3[] = "HTTP/1.1 200 OK\r\n"
                              "Server: tccWS\r\n"
                              "Connection: close\r\n"
                              "Cache-Control: never\r\n"
                              "Content-Type: text/html\r\n"
                              "Content-Length: 72\r\n"
                              "\r\n"
                              "<html><head><title>hello</title></head><body><h1>post</h1></body></html>";
const char EXPECTED_RESP4[] = "HTTP/1.1 500 Internal error\r\n"
                              "Server: tccWS\r\n"
                              "Connection: close\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: 0\r\n"
                              "\r\n";

// for the first version we'll not support more than one request on a connection.
/*const char EXPECTED_RESP5[] = "HTTP/1.1 200 OK\r\n"
                              "Server: tccWS\r\n"
                              "Connection: close\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: 11\r\n"
                              "\r\n"
                              "Hello WorldHTTP/1.1 200 OK\r\n"
                              "Server: tccWS\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: 5\r\n"
                              "\r\n"
                              "Aloha";*/

void copyHtmlWithTitleAndText(const char* title, const char* heading, char* buffer) {
    strcpy(buffer, "<html><head><title>");
    strcat(buffer, title);
    strcat(buffer, "</title></head><body><h1>");
    strcat(buffer, heading);
    strcat(buffer, "</h1></body></html>");
}

test(testAbstractTcMenuWebServer) {
    taskManager.reset();
    resetUnitLayer();
    TcMenuLightweightWebServer webServer(80, 1);
    webServer.init();

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

    // first make sure the server waits gracefully for the network to start, here it is not started
    webServer.timeOfNextCheck();
    assertTrue(webServer.isTriggered());
    webServer.exec();
    assertFalse(webServer.isInitialised());

    // now we start the network layer up and try again, it should now start
    startNetLayerDhcp();
    webServer.timeOfNextCheck();
    assertTrue(webServer.isTriggered());
    webServer.exec();
    assertTrue(webServer.isInitialised());

    // Now make a valid request for a simple document.
    simulateAccept();
    assertFalse(driverSocket.isIdle());
    driverSocket.simulateIncomingRaw(HTTP_REQ_GET1);
    webServer.timeOfNextCheck();
    assertTrue(webServer.isTriggered());
    webServer.exec();
    webServer.getWebResponse(0)->exec();
    assertTrue(driverSocket.checkResponseAgainst(EXPECTED_RESP1));
    assertTrue(driverSocket.didClose());

    // Now make a request that results in a 404.
    driverSocket.reset(false);
    simulateAccept();
    assertFalse(driverSocket.isIdle());
    driverSocket.simulateIncomingRaw(HTTP_REQ_404);
    webServer.exec();
    webServer.getWebResponse(0)->exec();
    assertTrue(driverSocket.checkResponseAgainst(EXPECTED_RESP2));
    assertTrue(driverSocket.didClose());

    driverSocket.reset(false);
    simulateAccept();
    assertFalse(driverSocket.isIdle());
    driverSocket.simulateIncomingRaw(HTTP_REQ_POST);
    webServer.exec();
    webServer.getWebResponse(0)->exec();
    assertTrue(driverSocket.checkResponseAgainst(EXPECTED_RESP3));
    assertTrue(driverSocket.didClose());

    driverSocket.reset(false);
    simulateAccept();
    assertFalse(driverSocket.isIdle());
    driverSocket.simulateIncomingRaw(HTTP_REQ_ERROR1);
    webServer.exec();
    webServer.getWebResponse(0)->exec();
    assertTrue(driverSocket.checkResponseAgainst(EXPECTED_RESP4));
    assertTrue(driverSocket.didClose());

    driverSocket.reset(false);
    simulateAccept();
    driverSocket.simulateIncomingRaw(HTTP_REQ_ERROR2);
    webServer.exec();
    webServer.getWebResponse(0)->exec();
    assertTrue(driverSocket.checkResponseAgainst(EXPECTED_RESP4));
    assertTrue(driverSocket.didClose());
}
