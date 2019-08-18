/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * RemoteConnector.cpp - contains the base functionality for communication between the menu library
 * and remote APIs.
 */

#include <Arduino.h>
#include "RemoteConnector.h"
#include "RemoteAuthentication.h"
#include "MenuItems.h"
#include "RemoteMenuItem.h"
#include "TaskManager.h"
#include "tcMenu.h"
#include "MessageProcessors.h"
#include "tcUtil.h"
#include <IoLogging.h>
#include "BaseDialog.h"
#include "BaseRenderers.h"

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

void stopPairing();

TagValueRemoteConnector::TagValueRemoteConnector(uint8_t remoteNo) : bootPredicate(MENUTYPE_BACK_VALUE, TM_INVERTED_LOCAL_ONLY), remotePredicate(remoteNo) {
	this->transport = NULL;
	this->processor = NULL;
    this->remoteName[0] = 0;
	this->localInfoPgm = NULL;
	this->remoteNo = remoteNo;
	this->ticksLastRead = this->ticksLastSend = 0xffff;
	this->flags = 0;
    this->commsCallback = NULL;
    this->authManager = NULL;
}

void TagValueRemoteConnector::initialise(TagValueTransport* transport, CombinedMessageProcessor* processor, const ConnectorLocalInfo* localInfoPgm) {
    this->processor = processor;
    this->transport = transport;
    this->localInfoPgm = localInfoPgm;
    
    // we must always have a mode of authentication, if nothing has been set then create the NoAuthentication manager.
    if(this->authManager == NULL) authManager = new NoAuthenticationManager();
}

void TagValueRemoteConnector::setRemoteName(const char* name) {
	strncpy(remoteName, name, sizeof remoteName);
	remoteName[sizeof(remoteName)-1]=0;
}

void TagValueRemoteConnector::setRemoteConnected(uint8_t major, uint8_t minor, ApiPlatform platform) {
    if(isAuthenticated()) {
        remoteMajorVer = major;
        remoteMinorVer = minor;
        remotePlatform = platform;
        initiateBootstrap();
    }
    else {
        serdebugF("Not authenticated, dropping");
        transport->close();
    }
}

void TagValueRemoteConnector::provideAuthentication(const char* auth) {
    if(auth == NULL || !authManager->isAuthenticated(remoteName, auth)) {
        serdebugF2("Authentication failed for ", remoteName);
        // wait before returning the state to prevent denial of service.
        encodeAcknowledgement(0, ACK_CREDENTIALS_INVALID);
        taskManager.yieldForMicros(15000); 
        setAuthenticated(false);
        transport->close();
        commsNotify(COMMSERR_DISCONNECTED);
    }
    else {
        encodeAcknowledgement(0, ACK_SUCCESS);
        setAuthenticated(true);
        serdebugF2("Authenticated device ", remoteName);
        commsNotify(COMMSERR_CONNECTED);
    }
}

const char headerPairingText[] PROGMEM = "Pairing waiting";
const char headerPairingDone[] PROGMEM = "Pairing complete";
const char* lastUuid;

void onPairingFinished(ButtonType ty, void* voidConnector) {
    TagValueRemoteConnector* connector = reinterpret_cast<TagValueRemoteConnector*>(voidConnector);
    if(ty==BTNTYPE_ACCEPT) {
        bool added = connector->getAuthManager()->addAdditionalUUIDKey(connector->getRemoteName(), lastUuid);
        connector->encodeAcknowledgement(0, added ? ACK_SUCCESS : ACK_CREDENTIALS_INVALID);
    }
    else {
        connector->encodeAcknowledgement(0, ACK_CREDENTIALS_INVALID);
    }
} 

void stopPairing() {
    BaseDialog* dialog = MenuRenderer::getInstance()->getDialog();
    if(dialog->isInUse()) dialog->hide();
}

void TagValueRemoteConnector::pairingRequest(const char* name, const char* uuid) {
    BaseDialog* dlg = MenuRenderer::getInstance()->getDialog();
    if(!dlg) {
        // TODO, special handling for where there's no display locally.
        return;
    }
    // store the fields
    lastUuid = uuid;
    setRemoteName(name);

    if(!isPairing()) {
        // mark this connection as paring only until disconnected
        // this prevents any other use of the connection basically.
        setPairing(true);

        // show the dialog.
        dlg->setButtons(BTNTYPE_ACCEPT, BTNTYPE_CANCEL, 1);
        dlg->setUserData(this);
        dlg->show(headerPairingText, false, onPairingFinished);
    }
    dlg->copyIntoBuffer(name);
}

void TagValueRemoteConnector::commsNotify(uint16_t err) {
    if(commsCallback != NULL) {
        CommunicationInfo info;
        info.remoteNo = remoteNo;
        info.connected = isAuthenticated();
        info.errorMode = err;
        commsCallback(info);
    }
}

void TagValueRemoteConnector::tick() {
    dealWithHeartbeating();

	if(isConnected() && transport->connected() && isAuthenticated()) {
		performAnyWrites();
	}

    if(isPairing()) return; // never read anything else when in pairing mode.

    // field if available is kind of like a state machine. Due to limited memory
    // on some AVR's and problems with the size of virtual tables, it cannot be
    // implemented as a series of classes that implement an interface.
    //
    // Every tick we call the method and it processes whatever data is available
    // on the socket turning it into a message a field at a time.
	FieldAndValue* field = transport->fieldIfAvailable();
	switch(field->fieldType) {
	case FVAL_NEW_MSG:
        serdebugMsgHdr("Msg In S: ", remoteNo, field->msgType);
		processor->newMsg(field->msgType);
		break;
	case FVAL_FIELD:
		processor->fieldUpdate(this, field);
        break;
	case FVAL_END_MSG:
        serdebugMsgHdr("Msg In E: ", remoteNo, field->msgType);
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

void TagValueRemoteConnector::setConnected(bool conn) {
    if(!conn) {
        this->ticksLastRead = this->ticksLastSend = 0xffff;
        flags = 0; // clear all flags on disconnect.
    }
    else {
        bitWrite(flags, FLAG_CURRENTLY_CONNECTED, true);
    } 

    commsNotify(conn ? COMMSERR_CONNECTED : COMMSERR_DISCONNECTED);
}

void TagValueRemoteConnector::dealWithHeartbeating() {
	++ticksLastRead;
	++ticksLastSend;

    // pairing will not send heartbeats, so we wait about 10 seconds before closing out
    unsigned int interval = isPairing() ? (PAIRING_TIMEOUT_TICKS) : (HEARTBEAT_INTERVAL_TICKS * 3);

	if(ticksLastRead > interval) {
		if(isConnected()) {
         	serdebugF3("Remote disconnected (rNo, ticks): ", remoteNo, ticksLastSend);
            if(isPairing()) stopPairing();
			setConnected(false);
			transport->close();
		}
	} else if(!isConnected() && transport->connected()) {
       	serdebugF2("Remote connected: ", remoteNo);
        encodeHeartbeat();
		encodeJoin();
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
	else if(isBootstrapComplete()) {
        MenuItem* item = iterator.nextItem();
        if(item) {
            item->setSendRemoteNeeded(remoteNo, false);
            encodeChangeValue(item);
        }

        BaseDialog* dlg = MenuRenderer::getInstance()->getDialog();
        if(dlg!=NULL && dlg->isRemoteUpdateNeeded(remoteNo)) {
            dlg->encodeMessage(this);
            dlg->setRemoteUpdateNeeded(remoteNo, false);
        }
    }
}

void TagValueRemoteConnector::initiateBootstrap() {
	if(isBootstrapMode()) return; // already booting.

    serdebugF2("Starting bootstrap mode", remoteNo);
    iterator.reset();
    iterator.setPredicate(&bootPredicate);
	encodeBootstrap(false);
	setBootstrapMode(true);
    setBootstrapComplete(false);
}

void TagValueRemoteConnector::nextBootstrap() {
	if(!transport->available()) return; // skip a turn, no write available.

    MenuItem* bootItem = iterator.nextItem();
	MenuItem* parent = iterator.currentParent() ;
    int parentId = parent == NULL ? 0 : parent->getId();
	if(!bootItem) {
        serdebugF2("Finishing bootstrap mode", remoteNo);
		setBootstrapMode(false);
        setBootstrapComplete(true);
		encodeBootstrap(true);
        iterator.reset();
        iterator.setPredicate(&remotePredicate);
		return;
	}

	bootItem->setSendRemoteNeeded(remoteNo, false);
	switch(bootItem->getMenuType()) {
	case MENUTYPE_SUB_VALUE:
		encodeSubMenu(parentId, (SubMenuItem*)bootItem);
		break;
	case MENUTYPE_BOOLEAN_VALUE:
		encodeBooleanMenu(parentId, (BooleanMenuItem*)bootItem);
		break;
	case MENUTYPE_ENUM_VALUE:
		encodeEnumMenu(parentId, (EnumMenuItem*)bootItem);
		break;
	case MENUTYPE_INT_VALUE:
		encodeAnalogItem(parentId, (AnalogMenuItem*)bootItem);
		break;
	case MENUTYPE_IPADDRESS:
	case MENUTYPE_TEXT_VALUE:
    case MENUTYPE_TIME:
		encodeMultiEditMenu(parentId, reinterpret_cast<RuntimeMenuItem*>(bootItem));
		break;
	case MENUTYPE_RUNTIME_LIST:
	case MENUTYPE_RUNTIME_VALUE:
		encodeRuntimeMenuItem(parentId, reinterpret_cast<RuntimeMenuItem*>(bootItem));
		break;
	case MENUTYPE_FLOAT_VALUE:
		encodeFloatMenu(parentId, (FloatMenuItem*)bootItem);
		break;
	case MENUTYPE_ACTION_VALUE:
		encodeActionMenu(parentId, (ActionMenuItem*)bootItem);
		break;
	default:
		break;
	}
}

void TagValueRemoteConnector::encodeDialogMsg(uint8_t mode, uint8_t btn1, uint8_t btn2, const char* hdrPgm, const char* b1) {
	if(!prepareWriteMsg(MSG_DIALOG)) return;
    
    char buffer[20];
    buffer[0]=mode;
    buffer[1]=0;
    transport->writeField(FIELD_MODE, buffer);

    transport->writeFieldInt(FIELD_BUTTON1, btn1);
    transport->writeFieldInt(FIELD_BUTTON2, btn2);
    
    if(mode == 'S') {
        safeProgCpy(buffer, hdrPgm, sizeof(buffer));
        transport->writeField(FIELD_HEADER, buffer);    
        transport->writeField(FIELD_BUFFER, b1);
    }
    transport->endMsg();
}

void TagValueRemoteConnector::encodeJoin() {
	if(!prepareWriteMsg(MSG_JOIN)) return;
    char szName[40];
    safeProgCpy(szName, localInfoPgm->uuid, sizeof(szName));
    transport->writeField(FIELD_UUID, szName);
    safeProgCpy(szName, localInfoPgm->name, sizeof(szName));
    transport->writeField(FIELD_MSG_NAME, szName);
    transport->writeFieldInt(FIELD_VERSION, API_VERSION);
    transport->writeFieldInt(FIELD_PLATFORM, TCMENU_DEFINED_PLATFORM);
    transport->endMsg();
    serdebugF2("Join sent ", szName);
}

void TagValueRemoteConnector::encodeBootstrap(bool isComplete) {
	if(!prepareWriteMsg(MSG_BOOTSTRAP)) return;
    transport->writeField(FIELD_BOOT_TYPE, potentialProgramMemory(isComplete ?  pmemBootEndText : pmemBootStartText));
    transport->endMsg();
}

void TagValueRemoteConnector::encodeHeartbeat() {
	if(!prepareWriteMsg(MSG_HEARTBEAT)) return;
    transport->writeFieldInt(FIELD_HB_INTERVAL, HEARTBEAT_INTERVAL);
    transport->writeFieldLong(FIELD_HB_MILLISEC, millis());
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
    transport->writeFieldInt(FIELD_ID, item->getId());
    transport->writeFieldInt(FIELD_EEPROM, item->getEepromPosition());
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

void TagValueRemoteConnector::encodeMultiEditMenu(int parentId, RuntimeMenuItem* item) {
	if(!prepareWriteMsg(MSG_BOOT_TEXT)) return;
    encodeBaseMenuFields(parentId, item);

	if (item->getMenuType() == MENUTYPE_TEXT_VALUE) {
		transport->writeFieldInt(FIELD_EDIT_MODE, EDITMODE_PLAIN_TEXT);
	}
	else if (item->getMenuType() == MENUTYPE_IPADDRESS) {
		transport->writeFieldInt(FIELD_EDIT_MODE, EDITMODE_IP_ADDRESS);
	}
    else if(item->getMenuType() == MENUTYPE_TIME) {
        TimeFormattedMenuItem* editable = reinterpret_cast<TimeFormattedMenuItem*>(item);
        transport->writeFieldInt(FIELD_EDIT_MODE, editable->getFormat());
    }

    EditableMultiPartMenuItem<byte[4]>* multipart = reinterpret_cast<EditableMultiPartMenuItem<byte[4]>*>(item);
    char sz[20];
    multipart->copyValue(sz, sizeof(sz));
    transport->writeField(FIELD_CURRENT_VAL, sz);
    transport->writeFieldInt(FIELD_MAX_LEN, multipart->getNumberOfParts());

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

void runtimeSendList(ListRuntimeMenuItem* item, TagValueTransport* transport) {
	char sz[25];
	for (int i = 0; i < item->getNumberOfParts(); i++) {
		item->getChildItem(i);
		item->copyValue(sz, sizeof(sz));
		transport->writeField(msgFieldToWord(FIELD_PREPEND_CHOICE, 'A' + i), sz);
		item->copyNameToBuffer(sz, sizeof(sz));
		transport->writeField(msgFieldToWord(FIELD_PREPEND_NAMECHOICE, 'A' + i), sz);
	}
	item->asParent();
}

void TagValueRemoteConnector::encodeRuntimeMenuItem(int parentId, RuntimeMenuItem * item) {
	if (!prepareWriteMsg(MSG_BOOT_LIST)) return;
	transport->writeFieldInt(FIELD_NO_CHOICES, item->getNumberOfParts());
	if (item->getMenuType() == MENUTYPE_RUNTIME_LIST) {
		runtimeSendList(reinterpret_cast<ListRuntimeMenuItem*>(item), transport);
	}
	else {
		char sz[25];
		item->copyValue(sz, sizeof(sz));
		transport->writeField(FIELD_PREPEND_CHOICE | 'A', sz);
	}
	encodeBaseMenuFields(parentId, item);
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

void TagValueRemoteConnector::encodeAcknowledgement(uint32_t correlation, AckResponseStatus status) {
    if(!prepareWriteMsg(MSG_ACKNOWLEDGEMENT)) return;
    transport->writeFieldInt(FIELD_ACK_STATUS, status);
    char sz[10];
    ltoa(correlation, sz, 16);
    transport->writeField(FIELD_CORRELATION, sz);
    transport->endMsg();

    serdebugF3("Ack send: ", correlation, status);
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

void TagValueRemoteConnector::encodeChangeValue(MenuItem* theItem) {
	if(!prepareWriteMsg(MSG_CHANGE_INT)) return;
    transport->writeFieldInt(FIELD_ID, theItem->getId());
    transport->writeFieldInt(FIELD_CHANGE_TYPE, CHANGE_ABSOLUTE); // menu host always sends absolute!
    switch(theItem->getMenuType()) {
    case MENUTYPE_ENUM_VALUE:
    case MENUTYPE_INT_VALUE:
    case MENUTYPE_BOOLEAN_VALUE:
        transport->writeFieldInt(FIELD_CURRENT_VAL, ((ValueMenuItem*)theItem)->getCurrentValue());
        break;
	case MENUTYPE_IPADDRESS:
    case MENUTYPE_TIME:
	case MENUTYPE_TEXT_VALUE: {
		char sz[20];
		((RuntimeMenuItem*)theItem)->copyValue(sz, sizeof(sz));
		transport->writeField(FIELD_CURRENT_VAL, sz);
		break;
	}
	case MENUTYPE_RUNTIME_LIST:
		runtimeSendList(reinterpret_cast<ListRuntimeMenuItem*>(theItem), transport);
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

	// protocol byte
	writeChar(TAG_VAL_PROTOCOL);

    // message type high then low
	writeChar(msgType >> 8);
	writeChar(msgType & 0xff);
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

void TagValueTransport::writeFieldLong(uint16_t field, long value) {
	char sz[12];
	sz[0] = field >> 8;
	sz[1] = field & 0xff;
	sz[2] = '=';
	sz[3] = 0;
	writeStr(sz);
	ltoa(value, sz, 10);
	writeStr(sz);
	writeChar('|');
}

void TagValueTransport::endMsg() {
	writeChar(0x02);
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
		if(r == 0x02) {
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
         currentField.fieldType = FVAL_FIELD;
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

		case FVAL_PROCESSING_PROTOCOL: // we need to make sure the protocol is valid
			if(readAvailable()) {
                currentField.fieldType = (readByte() == TAG_VAL_PROTOCOL) ? FVAL_PROCESSING_MSGTYPE_HI : FVAL_ERROR_PROTO;
                serdebugF("Protocol");
            }
			break;

        case FVAL_PROCESSING_MSGTYPE_HI:
            if(readAvailable()) {
                currentField.msgType = readByte() << 8;
                currentField.fieldType = FVAL_PROCESSING_MSGTYPE_LO;
            } 
            break;

        case FVAL_PROCESSING_MSGTYPE_LO:
            if(readAvailable()) {
                currentField.msgType |= readByte() & 0xff;
                currentField.fieldType = FVAL_NEW_MSG;
            }
            return &currentField;

        case FVAL_NEW_MSG:
		case FVAL_FIELD: // the field finished last time around, now reset it.
			currentField.fieldType = FVAL_PROCESSING;
			currentField.field = UNKNOWN_FIELD_PART;
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
    this->currentMsgType = 0;
}

void CombinedMessageProcessor::newMsg(uint16_t msgType) {
	currHandler = NULL;
	for(int i=0;i<noOfHandlers;++i) {
		if(handlers[i].msgType == msgType) {
			currHandler = &handlers[i];
		}
	}

	if(currHandler != NULL) {
        currentMsgType = msgType;
		memset(&val, 0, sizeof val);
	}
}

void CombinedMessageProcessor::fieldUpdate(TagValueRemoteConnector* connector, FieldAndValue* field) {
    uint16_t mt = field->msgType;
	if(currHandler != NULL && (connector->isAuthenticated() || mt == MSG_JOIN || mt == MSG_PAIR || mt == MSG_HEARTBEAT)) {
		currHandler->fieldUpdateFn(connector, field, &val);
	}
    else if(mt != MSG_HEARTBEAT) {
        serdebugF3("Did not proccess(hdlr,auth)", (unsigned int)currHandler, connector->isAuthenticated());
    }
}
