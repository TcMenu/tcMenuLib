/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * Ethernet remote capability plugin. This file is a plugin file and should not be directly edited,
 * it will be replaced each time the project is built. If you want to edit this file in place,
 * make sure to rename it first.
 */

#ifndef _TCMENU_ETHERNETTRANSPORT_H_
#define _TCMENU_ETHERNETTRANSPORT_H_

#include <RemoteConnector.h>
#include <Ethernet.h>

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
	virtual void close();
};

class EthernetTagValServer {
private:
	EthernetTagValTransport transport;
	TagValueRemoteConnector connector;
	EthernetServer *server;
public:
	EthernetTagValServer();
	void begin(EthernetServer* server, const char* namePgm);
	EthernetTagValTransport* getTransport() { return &transport; }
	TagValueRemoteConnector* getRemoteConnector(int num) { return &connector; }
	void runLoop();
};

extern EthernetTagValServer remoteServer;

#endif /* _TCMENU_ETHERNETTRANSPORT_H_ */
