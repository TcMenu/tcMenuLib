/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * MessageProcessors.h - standard message processors that decode tcMenu messages.
 */

#ifndef _TCMENU_MESSAGEPROCESSORS_H_
#define _TCMENU_MESSAGEPROCESSORS_H_

#include <Arduino.h>
#include "tcMenu.h"

/**
 * @file MessageProcessors.h
 * 
 * This file contains the default processors that can deal with incoming messages turning
 * them into events on tcMenu.
 */

class TagValueRemoteConnector; // forward reference
class FieldAndValue; // forward reference

/**
 * Message processors need to store some state while they are working through the fields
 * of a message, this union keeps state between a message starting processing and ending
 * It can be added to with additional unions. It is essentially stored globally so size
 * is an issue. If you need to extend the messages that can be processed, you'll probably
 * also need to store some state. This is the ideal place to store such state in the union.
 */
union MessageProcessorInfo {
	struct {
		MenuItem* item;
		int parentId;
		int changeValue;
		ChangeType changeType;
	} value;
	struct {
		uint8_t major, minor;
		ApiPlatform platform;
	} join;
};

/**
 * Each incoming message needs to have a MsgHandler associated with it. It maps the message type
 * to a function that can process the message fields as they arrive.
 */
struct MsgHandler {
	/** A function that will process the message, a field at a time */
	void (*fieldUpdateFn)(TagValueRemoteConnector*, FieldAndValue*, MessageProcessorInfo*);
	/** the type of message the above function can process. */
	uint16_t msgType;
};

/**
 * An array of message handlers, pre constructed that can be passed into the constructor of CombinedMessageProcessor.
 * This provides enough processors for tcMenu to work properly. But can be recreated with additional ones if needed.
 * For example: `CombinedMessageProcessor processor(msgHandlers, MSG_HANDLERS_SIZE);`
 */
extern MsgHandler msgHandlers[];
#define MSG_HANDLERS_SIZE 2

/**
 * If you decide to write your own processor, this method can handle join messages
 */
void fieldUpdateJoinMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info);

/**
 * If you decide to write your own processor, this method can handle value messages.
 */
void fieldUpdateValueMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info);

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

#endif /* _TCMENU_MESSAGEPROCESSORS_H_ */
