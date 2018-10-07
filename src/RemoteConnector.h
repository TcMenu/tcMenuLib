/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * RemoteConnector.h - contains the base functionality for communication between the menu library
 * and remote APIs.
 */
#ifndef _TCMENU_REMOTECONNECTOR_H_
#define _TCMENU_REMOTECONNECTOR_H_

#include <Arduino.h>
#include "RemoteTypes.h"
#include "MenuItems.h"
#include "MessageProcessors.h"

#define TAG_VAL_PROTOCOL 0x01
#define START_OF_MESSAGE 0x01
#define TICK_INTERVAL 20
#define HEARTBEAT_INTERVAL_TICKS (10000 / TICK_INTERVAL)

enum FieldValueType : byte {
	FVAL_NEW_MSG, FVAL_END_MSG, FVAL_FIELD, FVAL_ERROR_PROTO,
	// below are internal only states, and should not be acted upon.
	FVAL_PROCESSING, FVAL_PROCESSING_WAITEQ, FVAL_PROCESSING_VALUE, FVAL_PROCESSING_AWAITINGMSG, FVAL_PROCESSING_PROTOCOL
};

struct FieldAndValue {
	FieldValueType fieldType;
	uint16_t msgType;
	uint16_t field;
	char value[MAX_VALUE_LEN];
	uint8_t len;
};

enum CommsNotificationType : byte {
	COMMS_CONNECTED1 = 1,
	COMMS_CONNECTED2,
	COMMS_CONNECTED3,
	COMMS_DISCONNECTED1 = 50,
	COMMS_DISCONNECTED2,
	COMMS_DISCONNECTED3,
	COMMS_ERR_WRITE_NOT_CONNECTED = 100,
	COMMS_ERR_WRONG_PROTOCOL,

};

typedef void (*CommsCallbackFn)(CommsNotificationType);

class TagValueTransport {
protected:
	FieldAndValue currentField;
	static CommsCallbackFn notificationFn;
public:
	TagValueTransport();
	virtual ~TagValueTransport() {}
	static void commsNotify(CommsNotificationType notifyType) { if(notificationFn) notificationFn(notifyType);}
	static void setNoificationFn(CommsCallbackFn l) { notificationFn = l; }

	void startMsg(uint16_t msgType);
	void writeField(uint16_t field, const char* value);
	void writeFieldP(uint16_t field, const char* value);
	void writeFieldInt(uint16_t field, int value);
	void endMsg();
	FieldAndValue* fieldIfAvailable();
	void clearFieldStatus(FieldValueType ty = FVAL_PROCESSING);

	virtual void flush() = 0;
	virtual int writeChar(char data) = 0;
	virtual int writeStr(const char* data) = 0;
	virtual uint8_t readByte()=0;
	virtual bool readAvailable()=0;

	virtual bool available() = 0;
	virtual bool connected() = 0;
	virtual void close() = 0;

private:
	bool findNextMessageStart();
	bool processMsgKey();
	bool processValuePart();
};

class TagValueRemoteConnector; // forward reference

struct MsgHandler {
	void (*fieldUpdateFn)(TagValueRemoteConnector*, FieldAndValue*, MessageProcessorInfo*);
	uint16_t msgType;
};

class CombinedMessageProcessor {
private:
	MessageProcessorInfo val;
	MsgHandler* handlers;
	int noOfHandlers;

	MsgHandler* currHandler;
public:
	CombinedMessageProcessor(MsgHandler handlers[], int noOfHandlers); 
	void newMsg(uint16_t msgType);
	void fieldUpdate(TagValueRemoteConnector* connector, FieldAndValue* field);	
};

extern CombinedMessageProcessor defaultMsgProcessor;


#define FLAG_CURRENTLY_CONNECTED 0
#define FLAG_BOOTSTRAP_MODE 1
#define FLAG_WRITING_MSGS 2

class TagValueRemoteConnector {
private:
	const char* localNamePgm;
	uint16_t ticksLastSend;
	uint16_t ticksLastRead;
	CombinedMessageProcessor* processor;
	TagValueTransport* transport;
	uint8_t flags;
	uint8_t remoteNo;
	
	// the remote connection details take 16 bytes
	char remoteName[8];
	uint8_t remoteMajorVer, remoteMinorVer;
	ApiPlatform remotePlatform;

	// for bootstrapping
	MenuItem* bootMenuPtr;
	MenuItem* preSubMenuBootPtr;
public:
	TagValueRemoteConnector(TagValueTransport* transport, uint8_t remoteNo);
	void setName(const char* namePgm) {localNamePgm = namePgm;}

	bool isTransportAvailable() { return transport->available(); }
	bool isTransportConnected() { return transport->connected(); }

	void encodeJoinP(const char* localName);
	void encodeBootstrap(bool isComplete);
	void encodeHeartbeat();
	void encodeAnalogItem(int parentId, AnalogMenuItem* item);
	void encodeSubMenu(int parentId, SubMenuItem* item);
	void encodeBooleanMenu(int parentId, BooleanMenuItem* item);
	void encodeTextMenu(int parentId, TextMenuItem* item);
	void encodeEnumMenu(int parentId, EnumMenuItem* item);
	void encodeChangeValue(int parentId, MenuItem* theItem);

	void tick();

	void initiateBootstrap(MenuItem* firstItem);

	uint8_t getRemoteNo() {return remoteNo;}
	const char* getRemoteName() {return remoteName;}
	uint8_t getRemoteMajorVer() {return remoteMajorVer;}
	uint8_t getRemoteMinorVer() {return remoteMinorVer;}
	ApiPlatform getRemotePlatform() {return remotePlatform;}
	bool isConnected() { return bitRead(flags, FLAG_CURRENTLY_CONNECTED); }
	void setRemoteName(const char* name);
	void setRemoteConnected(uint8_t major, uint8_t minor, ApiPlatform platform);
private:
	void encodeBaseMenuFields(int parentId, MenuItem* item);
	void nextBootstrap();
	void performAnyWrites();
	void dealWithHeartbeating();
	void setBootstrapMode(bool mode) { bitWrite(flags, FLAG_BOOTSTRAP_MODE, mode); }
	void setConnected(bool mode) { bitWrite(flags, FLAG_CURRENTLY_CONNECTED, mode); }
	bool isBootstrapMode() { return bitRead(flags, FLAG_BOOTSTRAP_MODE); }
};

#define serdebug(x) //
#define serdebug2(x, y) //

//#define serdebug(x) Serial.println(x);
//#define serdebug2(x1, x2) Serial.print(x1); Serial.println(x2);

#endif /* _TCMENU_REMOTECONNECTOR_H_ */
