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

/**
 * @file RemoteConnector.h
 * 
 * This class contains the majority of the code for dealing with remote connections using the TagVal protocol.
 */

/**
 * Internal class used by the message processors, that shows the current state of the input parser.
 */
enum FieldValueType : byte {
	FVAL_NEW_MSG, FVAL_END_MSG, FVAL_FIELD, FVAL_ERROR_PROTO,
	// below are internal only states, and should not be acted upon.
	FVAL_PROCESSING, FVAL_PROCESSING_WAITEQ, FVAL_PROCESSING_VALUE, FVAL_PROCESSING_AWAITINGMSG, FVAL_PROCESSING_PROTOCOL
};

/** 
 * Internal class used by the message processors, that shows the current state of the input parser.
 */
struct FieldAndValue {
	FieldValueType fieldType;
	uint16_t msgType;
	uint16_t field;
	char value[MAX_VALUE_LEN];
	uint8_t len;
};

/**
 * This is used by the notification callback to indicate the latest state of a connection.
 */
enum CommsNotificationType : byte {
	/** connection 1 is now connected */
	COMMS_CONNECTED1 = 1,
	/** connection 2 is now connected */
	COMMS_CONNECTED2,
	/** connection 3 is now connected */
	COMMS_CONNECTED3,
	/** connection 1 is now disconnected */
	COMMS_DISCONNECTED1 = 50,
	/** connection 2 is now disconnected */
	COMMS_DISCONNECTED2,
	/** connection 3 is now disconnected */
	COMMS_DISCONNECTED3,
	/** one of the connections had a write error */
	COMMS_ERR_WRITE_NOT_CONNECTED = 100,
	/** one of the connections have a protocol problem */
	COMMS_ERR_WRONG_PROTOCOL,

};

/**
 * A callback function that will receive information about comms channels.
 */
typedef void (*CommsCallbackFn)(CommsNotificationType);

/**
 * The definition of a transport that can send and receive information remotely using the TagVal protocol.
 * Implementations include SerialTransport and EthernetTransport located in the remotes directory.
 */
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

/**
 * The definition of a message handler, used to extend the capabilities of TagVal to support more messages.
 */
struct MsgHandler {
	/** A function that will process the message, a field at a time */
	void (*fieldUpdateFn)(TagValueRemoteConnector*, FieldAndValue*, MessageProcessorInfo*);
	/** the type of message the above function can process. */
	uint16_t msgType;
};

/**
 * This message processor is responsible for handling messages coming off the wire and processing them into
 * usable events by the rest of the system.
 */
class CombinedMessageProcessor {
private:
	MessageProcessorInfo val;
	MsgHandler* handlers;
	int noOfHandlers;

	MsgHandler* currHandler;
public:
	/**
	 * Consructor takes an array of processors and the number of processors in the array.
	 */
	CombinedMessageProcessor(MsgHandler handlers[], int noOfHandlers); 
	/**
	 * Whenever there is a new message, this will be called, to re-initialise the internal state
	 */
	void newMsg(uint16_t msgType);
	/**
	 * Called whenever a field has been processed in the current message, after a call to newMsg
	 */
	void fieldUpdate(TagValueRemoteConnector* connector, FieldAndValue* field);	
};

/**
 * This is the default message processor for tcMenu
 */
extern CombinedMessageProcessor defaultMsgProcessor;


#define FLAG_CURRENTLY_CONNECTED 0
#define FLAG_BOOTSTRAP_MODE 1
#define FLAG_WRITING_MSGS 2

/**
 * The remote connector is what we would normally interact with when dealing with a remote. It provides functionality
 * at the message processing level, for sending messages and processing incoming ones.
 */
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
	/**
	 * Construct an instance/
	 * @param transport the actual underlying transport
	 * @param remoteNo the index of this connector, 0 based.
	 */
	TagValueRemoteConnector(TagValueTransport* transport, uint8_t remoteNo);

	/**
	 * Sets the name of this connector, string must be in PROGMEM
	 * @param namePgm string containing name in progmem
	 */
	void setName(const char* namePgm) {localNamePgm = namePgm;}

	/**
	 * Indicates if the underlying transport is functionality
	 */
	bool isTransportAvailable() { return transport->available(); }

	/**
	 *  Indicates if the underlying transport is connected.
	 */
	bool isTransportConnected() { return transport->connected(); }

	/**
	 * Encode a join message onto the wire, giving local name
	 * @param localName the name to send in the join message
	 */
	void encodeJoinP(const char* localName);

	/**
	 * Encode a bootstrap message indicating we are sending state
	 * @param isComplete true - end of boot sequence.
	 */
	void encodeBootstrap(bool isComplete);

	/**
	 * Encodes a heartbeat message onto the transport
	 */
	void encodeHeartbeat();

	/**
	 * Encodes a bootstrap for an Analog menu item, this gives all needed state
	 * to the remote. 
	 * @param parentId the parent menu
	 * @param item the item to be bootstrapped.
	 */
	void encodeAnalogItem(int parentId, AnalogMenuItem* item);

	/**
	 * Encodes a bootstrap for a Sub menu item, this gives all needed state
	 * to the remote. 
	 * @param parentId the parent menu
	 * @param item the item to be bootstrapped.
	 */
	void encodeSubMenu(int parentId, SubMenuItem* item);

	/**
	 * Encodes a bootstrap for a boolean menu item, this gives all needed state
	 * to the remote. 
	 * @param parentId the parent menu
	 * @param item the item to be bootstrapped.
	 */
	void encodeBooleanMenu(int parentId, BooleanMenuItem* item);

	/**
	 * Encodes a bootstrap for a text menu item, this gives all needed state
	 * to the remote. 
	 * @param parentId the parent menu
	 * @param item the item to be bootstrapped.
	 */
	void encodeTextMenu(int parentId, TextMenuItem* item);

	/**
	 * Encodes a bootstrap for an enum menu item, this gives all needed state
	 * to the remote. 
	 * @param parentId the parent menu
	 * @param item the item to be bootstrapped.
	 */
	void encodeEnumMenu(int parentId, EnumMenuItem* item);

	/**
	 * Encodes a bootstrap for a remote menu item, this gives all needed state
	 * to the remote. 
	 * @param parentId the parent menu
	 * @param item the item to be bootstrapped.
	 */
	void encodeRemoteMenu(int parentId, RemoteMenuItem* item);

	/**
	 * Encodes a bootstrap for an action menu item, this gives all needed state
	 * to the remote. 
	 * @param parentId the parent menu
	 * @param item the item to be bootstrapped.
	 */
	void encodeActionMenu(int parentId, ActionMenuItem* item);

	/**
	 * Encodes a bootstrap for a float menu item, this gives all needed state
	 * to the remote. 
	 * @param parentId the parent menu
	 * @param item the item to be bootstrapped.
	 */
	void encodeFloatMenu(int parentId, FloatMenuItem* item);

	/**
	 * Encodes a value change message to be sent to the remote. The embedded device
	 * always sends absolute changes out. 
	 * @param parentId the parent menu
	 * @param theItem the item to be bootstrapped.
	 */
	void encodeChangeValue(int parentId, MenuItem* theItem);

	/**
	 * Called frequently to perform all functions, this is arranged interally by
	 * registering a taskManager task.
	 */
	void tick();

	/**
	 * Called internally to start a bootstrap on new connections
	 */
	void initiateBootstrap(MenuItem* firstItem);

	/**
	 * Returns the remoteNo (or remote number) for this remote
	 */
	uint8_t getRemoteNo() {return remoteNo;}

	/**
	 * Returns the remote name for this connection. The name of the other side
	 */
	const char* getRemoteName() {return remoteName;}

	/**
	 * returns the major version of the other party API
	 */
	uint8_t getRemoteMajorVer() {return remoteMajorVer;}

	/**
	 * Returns the minor version of the other party API
	 */
	uint8_t getRemoteMinorVer() {return remoteMinorVer;}
	
	/**
	 * Returns the platform of the other party.
	 */
	ApiPlatform getRemotePlatform() {return remotePlatform;}

	/**
	 * Indicates if we are currently connected. Different to transport level connection as
	 * this includes checking for heartbeats.
	 */
	bool isConnected() { return bitRead(flags, FLAG_CURRENTLY_CONNECTED); }

	/**
	 * Sets the remote name, usually only used by the message processor.
	 */
	void setRemoteName(const char* name);

	/**
	 * Sets the remote connection state, again only used by message processor.
	 */
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
