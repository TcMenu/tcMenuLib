/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file EthernetTransport.h
 * 
 * Ethernet remote capability plugin. This file is a plugin file and should not be directly edited,
 * it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 */

#ifndef _TCMENU_ETHERNETTRANSPORT_H_
#define _TCMENU_ETHERNETTRANSPORT_H_

#include <RemoteConnector.h>
#include <TaskManager.h>
#include <Ethernet.h>
#include <tcUtil.h>
#include <remote/BaseRemoteComponents.h>

#ifndef ETHERNET_BUFFER_SIZE
#define ETHERNET_BUFFER_SIZE 64
#endif

#if ETHERNET_BUFFER_SIZE > 0
#include <remote/BaseBufferedRemoteTransport.h>
#endif

namespace tcremote {

#if ETHERNET_BUFFER_SIZE > 0

/**
 * An implementation of TagValueTransport that is able to read and write via a buffer to sockets.
 */
class EthernetTagValTransport : public tcremote::BaseBufferedRemoteTransport {
private:
	EthernetClient client;
public:
    EthernetTagValTransport() : BaseBufferedRemoteTransport(BUFFER_MESSAGES_TILL_FULL, ETHERNET_BUFFER_SIZE, MAX_VALUE_LEN) { }
    ~EthernetTagValTransport() override = default;
	void setClient(EthernetClient cl) { this->client = cl; }

	int fillReadBuffer(uint8_t* data, int maxSize) override;
	void flush() override;
	bool available() override;
	bool connected() override;
    void close() override;
};

#else // ethernet buffering not needed

/**
 * An implementation of TagValueTransport that is able to read and write using sockets.
 */
class EthernetTagValTransport : public TagValueTransport {
private:
	EthernetClient client;
public:
	EthernetTagValTransport();
	virtual ~EthernetTagValTransport();
	void setClient(EthernetClient client) { this->client = client; }

	virtual int writeChar(char data);
	virtual int writeStr(const char* data);
	virtual void flush();
	virtual bool available();
	virtual bool connected();
	virtual uint8_t readByte();
	virtual bool readAvailable();
    void close() override {
        currentField.msgType = UNKNOWN_MSG_TYPE;
        currentField.fieldType = FVAL_PROCESSING_AWAITINGMSG;
        client.stop();
    }
};

#endif // ethernet buffering check

/**
 * This class provides the initialisation and connection generation logic for ethernet connections.
 */
class EthernetInitialisation : public DeviceInitialisation {
private:
	EthernetServer *server;
public:
    explicit EthernetInitialisation(EthernetServer* server) : server(server) {}

    bool attemptInitialisation() override;

    bool attemptNewConnection(TagValueTransport *transport) override;
};

/**
 * This function converts from a RSSI (Radio Strength indicator)
 * measurement into a series of icons (of the ones we have defined
 * in the stock icons. The input is the RSSI figure in dB as an
 * integer.
 * @param strength the signal strength (usually negative) as an int
 * @return a state that can be used with the standard wifi TitleWidget
 */
int fromWiFiRSSITo4StateIndicator(int strength);

}  // namespace tcremote

using namespace tcremote;

#endif /* _TCMENU_ETHERNETTRANSPORT_H_ */
