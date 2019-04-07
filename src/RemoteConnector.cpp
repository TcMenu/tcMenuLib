/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * RemoteConnector.cpp - contains the base functionality for communication between the menu library
 * and remote APIs.
 */

#include <Arduino.h>
#include "RemoteConnector.h"
#include "MenuItems.h"
#include "RemoteMenuItem.h"
#include "TaskManager.h"
#include "tcMenu.h"
#include "MessageProcessors.h"
#include "tcUtil.h"
#include <IoLogging.h>

const char PGM_TCM EMPTYNAME[] = "Device";
const char PGM_TCM pmemBootStartText[] = "START";
const char PGM_TCM pmemBootEndText[] = "END";

#ifdef IO_LOGGING_DEBUG
// utility function to write out a debug line with the message type attached.
inline void serdebugMsgHdr(const char* tx, int remoteNo, uint16_t msgType) {
    char sz[3];
    sz[0] = msgType>>8;
    sz[1] = msgType & 0xff;
    sz[2] = 0;
    serdebug3(tx, remoteNo, sz);
}
#else
#define serdebugMsgHdr(x, y, z)
#endif

TagValueRemoteConnector::TagValueRemoteConnector() {
	this->transport = NULL;
	this->processor = NULL;
	this->bootMenuPtr = preSubMenuBootPtr = NULL;
    this->remoteName[0] = 0;
	this->localNamePgm = EMPTYNAME;
	this->remoteNo = 0;
	this->ticksLastRead = this->ticksLastSend = 0xffff;
	this->flags = 0;
    this->commsCallback = NULL;
}

void TagValueRemoteConnector::setRemoteName(const char* name) {
	strncpy(remoteName, name, sizeof remoteName);
	remoteName[sizeof(remoteName)-1]=0;
}

void TagValueRemoteConnector::setRemoteConnected(uint8_t major, uint8_t minor, ApiPlatform platform) {
	remoteMajorVer = major;
	remoteMinorVer = minor;
	remotePlatform = platform;
	initiateBootstrap(menuMgr.getRoot());
}

void TagValueRemoteConnector::commsNotify(uint16_t err) {
    if(commsCallback != NULL) {
        CommunicationInfo info;
        info.remoteNo = remoteNo;
        info.connected = isConnected();
        info.errorMode = err;
        commsCallback(info);
    }
}

void TagValueRemoteConnector::tick() {
	dealWithHeartbeating();

	if(isConnected() && transport->connected()) {
		performAnyWrites();
	}

	FieldAndValue* field = transport->fieldIfAvailable();
	switch(field->fieldType) {
	case FVAL_NEW_MSG:
		processor->newMsg(field->msgType);
		break;
	case FVAL_FIELD:
	case FVAL_END_MSG:
        serdebugMsgHdr("Msg In: ", remoteNo, field->msgType);
		processor->fieldUpdate(this, field);
		ticksLastRead = 0;
		break;
	case FVAL_ERROR_PROTO:
		commsNotify(COMMSERR_PROTOCOL_ERROR);
		break;
	default: // not ready for processing yet.
		break;
	}
}

void TagValueRemoteConnector::dealWithHeartbeating() {
	++ticksLastRead;
	++ticksLastSend;

	if(ticksLastRead > (HEARTBEAT_INTERVAL_TICKS * 3)) {
		if(isConnected()) {
         	serdebugF3("Remote disconnected (rNo, ticks): ", remoteNo, ticksLastSend);
			setConnected(false);
			transport->close();
		}
	} else if(!isConnected() && transport->connected()) {
       	serdebugF2("Remote connected: ", remoteNo);
		encodeJoin(localNamePgm);
		setConnected(true);
	}

	if(ticksLastSend > HEARTBEAT_INTERVAL_TICKS) {
		if(isConnected() && transport->available()) {
         	serdebugF3("Sending HB (rNo, ticks) : ", remoteNo, ticksLastSend);
            encodeHeartbeat();
        }
	}
}

void TagValueRemoteConnector::performAnyWrites() {
	if(isBootstrapMode()) {
		nextBootstrap();
	}
	else {
		if(bootMenuPtr == NULL) bootMenuPtr = menuMgr.getRoot();

		// we loop here until either we've gone through the structure or something has changed
		while(bootMenuPtr) {
			int parentId = (preSubMenuBootPtr != NULL) ? preSubMenuBootPtr->getId() : 0;
			if(bootMenuPtr->getMenuType() == MENUTYPE_SUB_VALUE) {
				preSubMenuBootPtr = bootMenuPtr;
				SubMenuItem* sub = (SubMenuItem*) bootMenuPtr;
				bootMenuPtr = sub->getChild();
			}
			else if(bootMenuPtr->isSendRemoteNeeded(remoteNo)) {
				bootMenuPtr->setSendRemoteNeeded(remoteNo, false);
				encodeChangeValue(parentId, bootMenuPtr);
				return; // exit once something is written
			}

			// see if there's more to do, including moving between submenu / root.
			bootMenuPtr = bootMenuPtr->getNext();
			if(bootMenuPtr == NULL && preSubMenuBootPtr != NULL) {
				bootMenuPtr = preSubMenuBootPtr->getNext();
				preSubMenuBootPtr = NULL;
			}
		}
	}
}

void TagValueRemoteConnector::initiateBootstrap(MenuItem* firstItem) {
	if(isBootstrapMode()) return; // already booting.

    serdebugF2("Starting bootstrap mode", remoteNo);
	bootMenuPtr = firstItem;
	preSubMenuBootPtr = NULL;
	encodeBootstrap(false);
	setBootstrapMode(true);
}

void TagValueRemoteConnector::nextBootstrap() {
	if(!bootMenuPtr) {
        serdebugF2("Finishing bootstrap mode", remoteNo);
		setBootstrapMode(false);
		encodeBootstrap(true);
		preSubMenuBootPtr = NULL;
		return;
	}

	if(!transport->available()) return; // skip a turn, no write available.

	int parentId = (preSubMenuBootPtr != NULL) ? preSubMenuBootPtr->getId() : 0;
	bootMenuPtr->setSendRemoteNeeded(remoteNo, false);
	switch(bootMenuPtr->getMenuType()) {
	case MENUTYPE_SUB_VALUE:
		encodeSubMenu(parentId, (SubMenuItem*)bootMenuPtr);
		preSubMenuBootPtr = bootMenuPtr;
		bootMenuPtr = ((SubMenuItem*)bootMenuPtr)->getChild();
		break;
	case MENUTYPE_BOOLEAN_VALUE:
		encodeBooleanMenu(parentId, (BooleanMenuItem*)bootMenuPtr);
		break;
	case MENUTYPE_ENUM_VALUE:
		encodeEnumMenu(parentId, (EnumMenuItem*)bootMenuPtr);
		break;
	case MENUTYPE_INT_VALUE:
		encodeAnalogItem(parentId, (AnalogMenuItem*)bootMenuPtr);
		break;
	case MENUTYPE_TEXT_VALUE:
		encodeTextMenu(parentId, (TextMenuItem*)bootMenuPtr);
		break;
	case MENUTYPE_REMOTE_VALUE:
		encodeRemoteMenu(parentId, (RemoteMenuItem*)bootMenuPtr);
		break;
	case MENUTYPE_FLOAT_VALUE:
		encodeFloatMenu(parentId, (FloatMenuItem*)bootMenuPtr);
		break;
	case MENUTYPE_ACTION_VALUE:
		encodeActionMenu(parentId, (ActionMenuItem*)bootMenuPtr);
		break;
	default:
		break;
	}

	// see if there's more to do, including moving between submenu / root.
	bootMenuPtr = bootMenuPtr->getNext();
	if(bootMenuPtr == NULL && preSubMenuBootPtr != NULL) {
		bootMenuPtr = preSubMenuBootPtr->getNext();
		preSubMenuBootPtr = NULL;
	}
}

void TagValueRemoteConnector::encodeJoin(const char* localName) {
	if(!prepareWriteMsg(MSG_JOIN)) return;
    char szName[20];
    safeProgCpy(szName, localName, sizeof(szName));
    transport->writeField(FIELD_MSG_NAME, szName);
    transport->writeFieldInt(FIELD_VERSION, API_VERSION);
    transport->writeFieldInt(FIELD_PLATFORM, TCMENU_DEFINED_PLATFORM);
    transport->endMsg();
    serdebugF3("Join message send: ", szName, TCMENU_DEFINED_PLATFORM);
}

void TagValueRemoteConnector::encodeBootstrap(bool isComplete) {
	if(!prepareWriteMsg(MSG_BOOTSTRAP)) return;
    transport->writeField(FIELD_BOOT_TYPE, potentialProgramMemory(isComplete ?  pmemBootEndText : pmemBootStartText));
    transport->endMsg();
}

void TagValueRemoteConnector::encodeHeartbeat() {
	if(!prepareWriteMsg(MSG_HEARTBEAT)) return;
    transport->endMsg();
}

bool TagValueRemoteConnector::prepareWriteMsg(uint16_t msgType) {
    if(!transport->connected()) {
        serdebugMsgHdr("Wr Err ", remoteNo, msgType);
        commsNotify(COMMSERR_WRITE_NOT_CONNECTED);
        setConnected(false); // we are immediately not connected in this case.
        return false;
    }
    transport->startMsg(msgType);
    ticksLastSend = 0;
    serdebugMsgHdr("Msg Out ", remoteNo, msgType);
    return true;
}

void TagValueRemoteConnector::encodeBaseMenuFields(int parentId, MenuItem* item) {
    transport->writeFieldInt(FIELD_PARENT, parentId);
    transport->writeFieldInt(FIELD_ID,item->getId());
    transport->writeFieldInt(FIELD_READONLY, item->isReadOnly());
    char sz[20];
    item->copyNameToBuffer(sz, sizeof(sz));
    transport->writeField(FIELD_MSG_NAME, sz);
}

void TagValueRemoteConnector::encodeAnalogItem(int parentId, AnalogMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_ANALOG)) return;
    encodeBaseMenuFields(parentId, item);
    char sz[10];
    item->copyUnitToBuffer(sz);
    transport->writeField(FIELD_ANALOG_UNIT, sz);
    transport->writeFieldInt(FIELD_ANALOG_MAX, item->getMaximumValue());
    transport->writeFieldInt(FIELD_ANALOG_OFF, item->getOffset());
    transport->writeFieldInt(FIELD_ANALOG_DIV, item->getDivisor());
    transport->writeFieldInt(FIELD_CURRENT_VAL, item->getCurrentValue());
    transport->endMsg();
}

void TagValueRemoteConnector::encodeTextMenu(int parentId, TextMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_TEXT)) return;
    encodeBaseMenuFields(parentId, item);
    transport->writeFieldInt(FIELD_MAX_LEN, item->textLength());
    transport->writeField(FIELD_CURRENT_VAL, item->getTextValue());
    transport->endMsg();
}

void TagValueRemoteConnector::encodeRemoteMenu(int parentId, RemoteMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_REMOTE)) return;
    encodeBaseMenuFields(parentId, item);
    transport->writeFieldInt(FIELD_REMOTE_NO, item->getRemoteNum());
    char sz[20];
    item->getCurrentState(sz, sizeof sz);
    transport->writeField(FIELD_CURRENT_VAL, sz);
    transport->endMsg();
}

void writeFloatValueToTransport(TagValueTransport* transport, FloatMenuItem* item) {
	char sz[20];
	sz[0]=0;
	ltoa((long)item->getFloatValue(), sz, 10);
	appendChar(sz, '.', sizeof sz);
	
	long whole = item->getFloatValue();
	long fract = abs((item->getFloatValue() - whole) * 1000000L);
	fastltoa_mv(sz, fract, 1000000L, '0', sizeof sz);
	transport->writeField(FIELD_CURRENT_VAL, sz);
}

void TagValueRemoteConnector::encodeFloatMenu(int parentId, FloatMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_FLOAT)) return;
    encodeBaseMenuFields(parentId, item);
    transport->writeFieldInt(FIELD_FLOAT_DP, item->getDecimalPlaces());
    writeFloatValueToTransport(transport, item);
    transport->endMsg();
}

void TagValueRemoteConnector::encodeEnumMenu(int parentId, EnumMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_ENUM)) return;
    encodeBaseMenuFields(parentId, item);
    transport->writeFieldInt(FIELD_CURRENT_VAL, item->getCurrentValue());
    uint8_t noChoices = item->getMaximumValue() + 1;
    transport->writeFieldInt(FIELD_NO_CHOICES, noChoices);
    for(uint8_t i=0;i<noChoices;++i) {
        uint16_t choiceKey = msgFieldToWord(FIELD_PREPEND_CHOICE, 'A' + i);
        char szChoice[20];
        item->copyEnumStrToBuffer(szChoice, sizeof(szChoice), i);
        transport->writeField(choiceKey, szChoice);
    }
    transport->endMsg();
}

void TagValueRemoteConnector::encodeBooleanMenu(int parentId, BooleanMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_BOOL)) return;
    encodeBaseMenuFields(parentId, item);
    transport->writeFieldInt(FIELD_CURRENT_VAL, item->getCurrentValue());
    transport->writeFieldInt(FIELD_BOOL_NAMING, item->getBooleanNaming());
    transport->endMsg();
}

void TagValueRemoteConnector::encodeSubMenu(int parentId, SubMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_SUBMENU)) return;
    encodeBaseMenuFields(parentId, item);
    transport->endMsg();
}

void TagValueRemoteConnector::encodeActionMenu(int parentId, ActionMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_ACTION)) return;
    encodeBaseMenuFields(parentId, item);
    transport->endMsg();
}

void writeRemoteValueToTransport(TagValueTransport* transport, RemoteMenuItem* item) {
	char sz[20];
	item->getCurrentState(sz, sizeof sz);
	transport->writeField(FIELD_CURRENT_VAL, sz);
}

void TagValueRemoteConnector::encodeChangeValue(int parentId, MenuItem* theItem) {
	if(!prepareWriteMsg(MSG_CHANGE_INT)) return;
    transport->writeFieldInt(FIELD_PARENT, parentId);
    transport->writeFieldInt(FIELD_ID, theItem->getId());
    transport->writeFieldInt(FIELD_CHANGE_TYPE, CHANGE_ABSOLUTE); // menu host always sends absolute!
    switch(theItem->getMenuType()) {
    case MENUTYPE_ENUM_VALUE:
    case MENUTYPE_INT_VALUE:
    case MENUTYPE_BOOLEAN_VALUE:
        transport->writeFieldInt(FIELD_CURRENT_VAL, ((ValueMenuItem*)theItem)->getCurrentValue());
        break;
    case MENUTYPE_TEXT_VALUE:
        transport->writeField(FIELD_CURRENT_VAL, ((TextMenuItem*)theItem)->getTextValue());
        break;
    case MENUTYPE_REMOTE_VALUE: 
        writeRemoteValueToTransport(transport, (RemoteMenuItem*)theItem);
        break;
    case MENUTYPE_FLOAT_VALUE:
        writeFloatValueToTransport(transport, (FloatMenuItem*)theItem);
        break;
    default:
        break;
    }
    transport->endMsg();
}

//
// Base transport capabilities
//

TagValueTransport::TagValueTransport() {
	this->currentField.field = UNKNOWN_FIELD_PART;
	this->currentField.fieldType = FVAL_PROCESSING_AWAITINGMSG;
	this->currentField.msgType = UNKNOWN_MSG_TYPE;
	this->currentField.len = 0;
}

void TagValueTransport::startMsg(uint16_t msgType) {
	// start of message
	writeChar(START_OF_MESSAGE);

	// protocol low and high byte
	writeChar(TAG_VAL_PROTOCOL);

	char sz[3];
	sz[0] = msgType >> 8;
	sz[1] = msgType & 0xff;
	sz[2] = 0;
	writeField(FIELD_MSG_TYPE, sz);
}

void TagValueTransport::writeField(uint16_t field, const char* value) {
	char sz[4];
	sz[0] = field >> 8;
	sz[1] = field & 0xff;
	sz[2] = '=';
	sz[3] = 0;
	writeStr(sz);
	writeStr(value);
	writeChar('|');
}

void TagValueTransport::writeFieldInt(uint16_t field, int value) {
	char sz[10];
	sz[0] = field >> 8;
	sz[1] = field & 0xff;
	sz[2] = '=';
	sz[3] = 0;
	writeStr(sz);
	itoa(value, sz, 10);
	writeStr(sz);
	writeChar('|');
}

void TagValueTransport::endMsg() {
	writeStr("~\n");
}

void TagValueTransport::clearFieldStatus(FieldValueType ty) {
	currentField.fieldType = ty;
	currentField.field = UNKNOWN_FIELD_PART;
	currentField.msgType = UNKNOWN_MSG_TYPE;
}

bool TagValueTransport::findNextMessageStart() {
	char read = 0;
	while(readAvailable() && read != START_OF_MESSAGE) {
		read = readByte();
	}
	return (read == START_OF_MESSAGE);
}

bool TagValueTransport::processMsgKey() {
	if(highByte(currentField.field) == UNKNOWN_FIELD_PART && readAvailable()) {
		char r = readByte();
		if(r == '~') {
			currentField.fieldType = FVAL_END_MSG;
			return false;
		}
		else {
			currentField.field = ((uint16_t)r) << 8;
		}
	}

	// if we are PROCESSING the key and we've already filled in the top half, then now we need the lower part.
	if(highByte(currentField.field) != UNKNOWN_FIELD_PART && lowByte(currentField.field) == UNKNOWN_FIELD_PART && readAvailable()) {
		currentField.field |= ((readByte()) & 0xff);
		currentField.fieldType = FVAL_PROCESSING_WAITEQ;
	}

	return true;
}

bool TagValueTransport::processValuePart() {
	char current = 0;
	while(readAvailable() && current != '|') {
		current = readByte();
		if(current != '|') {
			currentField.value[currentField.len] = current;
			// safety check for too much data!
			if(++currentField.len > (sizeof(currentField.value)-1)) {
				return false;
			}
		}
	}

	// reached end of field?
	if(current == '|') {
		currentField.value[currentField.len] = 0;

		// if this is a new message and the first field is not the type, that's an error
		if(currentField.msgType == UNKNOWN_MSG_TYPE && currentField.field != FIELD_MSG_TYPE) {
			return false;
		}

		// if its the message type field, populate it and report new message, otherwise report regular field
		if(currentField.field == FIELD_MSG_TYPE) {
			currentField.msgType = msgFieldToWord(currentField.value[0], currentField.value[1]);
			currentField.fieldType = FVAL_NEW_MSG;
		}
		else currentField.fieldType = FVAL_FIELD;
	}
	return true;
}

FieldAndValue* TagValueTransport::fieldIfAvailable() {
    // don't start processing below when not connected or available
    if(!connected()) {
        clearFieldStatus(FVAL_PROCESSING_AWAITINGMSG);
        return &currentField;
    } 

	bool contProcessing = true;
	while(contProcessing) {
		switch(currentField.fieldType) {
		case FVAL_END_MSG:
			clearFieldStatus(FVAL_PROCESSING_AWAITINGMSG);
			break;
		case FVAL_ERROR_PROTO:
		case FVAL_PROCESSING_AWAITINGMSG: // in these states we need to find the next message
			if(findNextMessageStart()) {
				clearFieldStatus(FVAL_PROCESSING_PROTOCOL);
			}
			else {
				currentField.fieldType = FVAL_PROCESSING_AWAITINGMSG;
				return &currentField;
			}
			break;

		case FVAL_NEW_MSG:
		case FVAL_FIELD: // the field finished last time around, now reset it.
			currentField.fieldType = FVAL_PROCESSING;
			currentField.field = UNKNOWN_FIELD_PART;
			break;

		case FVAL_PROCESSING_PROTOCOL: // we need to make sure the protocol is valid
			if(!readAvailable()) break;
			currentField.fieldType = (readByte() == TAG_VAL_PROTOCOL) ? FVAL_PROCESSING : FVAL_ERROR_PROTO;
			break;

		case FVAL_PROCESSING: // we are looking for the field key
			contProcessing = processMsgKey();
			break;

		case FVAL_PROCESSING_WAITEQ: // we expect an = following the key
			if(!readAvailable()) break;
			if(readByte() != '=') {
				clearFieldStatus(FVAL_ERROR_PROTO);
				return &currentField;
			}
			currentField.len = 0;
			currentField.fieldType = FVAL_PROCESSING_VALUE;
			break;

		case FVAL_PROCESSING_VALUE: // and lastly a value followed by pipe.
			if(!processValuePart()) {
				clearFieldStatus(FVAL_ERROR_PROTO);
			}
			if(currentField.fieldType != FVAL_PROCESSING_VALUE) return &currentField;
			break;
		}
		contProcessing = contProcessing && readAvailable();
	}
	return &currentField;
}

CombinedMessageProcessor::CombinedMessageProcessor(MsgHandler handlers[], int noOfHandlers) {
	this->handlers = handlers;
	this->noOfHandlers = noOfHandlers;
	this->currHandler = NULL;
}

void CombinedMessageProcessor::newMsg(uint16_t msgType) {
	currHandler = NULL;
	for(int i=0;i<noOfHandlers;++i) {
		if(handlers[i].msgType == msgType) {
			currHandler = &handlers[i];
		}
	}

	if(currHandler != NULL) {
		memset(&val, 0, sizeof val);
	}
}

void CombinedMessageProcessor::fieldUpdate(TagValueRemoteConnector* connector, FieldAndValue* field) {
	if(currHandler != NULL) {
		currHandler->fieldUpdateFn(connector, field, &val);
	}
}
