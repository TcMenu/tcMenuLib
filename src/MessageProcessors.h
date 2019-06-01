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
 * 
 * This union structure exists once per remote connection, and given that only one message
 * on a remote connection can be processed at once, it can be a union. It is cleared at
 * the start of each process.
 */
union MessageProcessorInfo {
	struct {
		MenuItem* item;
		int changeValue;
        uint32_t correlation;
		ChangeType changeType;
	} value;
	struct {
		uint8_t major, minor;
		ApiPlatform platform;
        bool authProvided;
	} join;
    struct {
        char name[16];
    } pairing;
    struct {
        char mode;
        uint8_t button;
        uint32_t correlation;
    } dialog;
};

/**
 * Each incoming message needs to have a MsgHandler associated with it. It maps the message type
 * to a function that can process the message fields as they arrive. It will be called every time
 * there is a new field on a message, it should at a minimum be able to process the field updates 
 * end check for the end of the message.
 * @see FieldAndValue
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
#define MSG_HANDLERS_SIZE 4

/**
 * If you decide to write your own processor, this method can handle join messages
 */
void fieldUpdateJoinMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info);

/**
 * If you decide to write your own processor, this method can handle value messages.
 */
void fieldUpdateValueMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info);

/**
 * If you decide to write your own processor, this method can handle pairing messages.
 */
void fieldUpdatePairingMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info);

/**
 * If you decide to write your own processor, this method can handle dialog updates
 */
void fieldUpdateDialogMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info);

/**
 * This message processor is responsible for handling messages coming off the wire and processing them into
 * usable events by the rest of the system. Usually, the message processor actually handles the event in 
 * full.
 * 
 * When a new message arrives, this class attempts to find a suitable processor function (or ignore if we
 * can't process), then each field in the message is passed to the function to processed.
 */
class CombinedMessageProcessor {
private:
	MessageProcessorInfo val;
	MsgHandler* handlers;
	int noOfHandlers;

	MsgHandler* currHandler;
    uint16_t currentMsgType;
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
