
#include <Arduino.h>
#include <tcMenu.h>
#include <remote/TransportNetworkDriver.h>
#include <TaskManager.h>
#include "IoLogging.h"

using namespace tcremote;

const uint8_t manualIpAddress[] = {192, 168, 0, 202};
const uint8_t manualMacAddress[] = {0xde, 0xad, 0xbe, 0xef, 0xfe, 0xed};
const uint8_t manualIpMask[] = {255, 255, 255, 0};
bool shouldFlush = false;
char testData[] = "hello";

socket_t clientSocketFd = TC_BAD_SOCKET_ID;
bool dataCorrectlySet = false;

void setup() {
    Serial.begin(115200);
    while (!Serial);
}

void fail(const char* reason) {
    serdebugF2("FAIL - ", reason);
    while(1) {
        delay(100);
    }
}

void acceptCallback(tcremote::socket_t fd, void *data) {
    dataCorrectlySet = (testData == data);
    clientSocketFd = fd;
}

void loop() {
    /**
     * This test case allows you to easily test the network stack, it calls most of the methods on a network driver.
     * Before running this sketch start up the JavaAPI DriverSocketClient test class which acts as the other side to this
     */
    uint32_t then = millis();
    char sz[100];

    // step 1, start the network and make sure it is up.
    startNetLayerManual(manualIpAddress, manualMacAddress, manualIpMask);
    serdebugF("Attempting network start");

    while (!isNetworkUp() && (millis() - then) < 10000) {
        taskManager.yieldForMicros(1000);
    }
    if (!isNetworkUp()) {
        fail("Network not up");
    }

    copyIpAddress(TC_LOCALHOST_SOCKET_ID, sz, sizeof sz);
    serdebugF2("Network started as ", sz);

    // step 2, create an accept socket
    auto acceptReturn = initialiseAccept(3333, acceptCallback, testData);

    if(acceptReturn != SOCK_ERR_OK) fail("Accept call failed");

    then = millis();
    while (clientSocketFd == TC_BAD_SOCKET_ID && (millis() - then) < 30000) {
        taskManager.yieldForMicros(1000);
    }
    if(clientSocketFd == TC_BAD_SOCKET_ID) fail("Nothing accepted");

    copyIpAddress(clientSocketFd, sz, sizeof sz);
    serdebugF2("Client IP is ", sz);

    if(rawWriteData(clientSocketFd, "hello world", 11, 1000) != SOCK_ERR_OK) {
        fail("Write returned error code");
    }
    if (shouldFlush) {
        rawFlushAll(clientSocketFd);
    }

    static const char sz1[] = "Message sent from the server back to the client";
    static const char sz2[] = "Longer example message that's  sent from the server back to the client";
    // read and write quite a few times.
    for (int j = 0; j < 500; j++) {

        if(rawWriteData(clientSocketFd, sz1, sizeof(sz1), 1000) != SOCK_ERR_OK) {
            fail("Write sz1 failed");
        }

        if(rawReadAvailable(clientSocketFd)) {
            auto len = rawReadData(clientSocketFd, sz, sizeof(sz));
            sz[len] = 0;
            serdebugF2("Client data = ", sz);
        }

        taskManager.yieldForMicros(millisToMicros(500U));
        if(rawWriteData(clientSocketFd, sz2, sizeof(sz2), 1000) != SOCK_ERR_OK) {
            fail("Write sz2 failed");
        }
        taskManager.yieldForMicros(millisToMicros(100U));
    }

    closeSocket(clientSocketFd);
}

