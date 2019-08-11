/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * MessageProcessors.cpp - standard message processors that decode tcMenu messages.
 */
#include "RemoteConnector.h"
#include "MessageProcessors.h"
#include "MenuIterator.h"
#include "BaseDialog.h"

/**
 * An array of message handlers, where each one is a function that can process that type of message and a message type.
 * Messages are received a field at a time, so each time the function is called a new field will be available, when the
 * last field is processed the end indicator will be set.
 * @See TagValueRemoteConnector
 */

MsgHandler msgHandlers[] = {
	{ fieldUpdateValueMsg, MSG_CHANGE_INT }, 
	{ fieldUpdateJoinMsg, MSG_JOIN },
	{ fieldUpdatePairingMsg, MSG_PAIR },
	{ fieldUpdateDialogMsg, MSG_DIALOG }
};

void fieldUpdateDialogMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info) {
	if(field->fieldType == FVAL_END_MSG && info->dialog.mode == 'A') {
        BaseDialog* dialog = MenuRenderer::getInstance()->getDialog();
        if(dialog) {
            dialog->remoteAction((ButtonType)info->dialog.button);
            connector->encodeAcknowledgement(info->dialog.correlation, ACK_SUCCESS);
        }
        else {
            connector->encodeAcknowledgement(info->dialog.correlation, ACK_UNKNOWN);
        }
        return;
    }

    switch(field->field) {
    case FIELD_BUTTON1:
        info->dialog.button = field->value[0] - '0';
        break;
    case FIELD_MODE:
        info->dialog.mode = field->value[0];
        break;
    case FIELD_CORRELATION:
        info->dialog.correlation = strtoul(field->value, NULL, 16);
        break;
    }

}

void fieldUpdatePairingMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info) {
	if(field->fieldType == FVAL_END_MSG) return;

    switch(field->field) {
    case FIELD_MSG_NAME:
        strncpy(info->pairing.name, field->value, sizeof(info->pairing.name));
        break;
    case FIELD_UUID:
        serdebugF3("Pairing request: ", info->pairing.name, field->value);
        connector->pairingRequest(info->pairing.name, field->value);
        break;
    }
}

void fieldUpdateJoinMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info) {
	if(field->fieldType == FVAL_END_MSG) {
        serdebugF2("Join from ", info->join.platform);
        serdebugF3("Remote version was ", info->join.major, info->join.minor);

        // ensure that authentication has been done, if not basically fail the connection.
        if(!info->join.authProvided) {
            serdebugF("Connection without authentication - stopping");
            connector->provideAuthentication(NULL);
        }
        else {
            connector->setRemoteConnected(info->join.major, info->join.minor, info->join.platform);
        }
		return;
	}

	switch(field->field) {
	case FIELD_MSG_NAME:
        serdebugF2("Join name ", field->value);
		connector->setRemoteName(field->value);
		break;
	case FIELD_VERSION: {
		int val = atoi(field->value);
		info->join.major = val / 100;
		info->join.minor = val % 100;
		break;
	}
    case FIELD_UUID: {
        serdebugF("Join UUID - start auth");
        connector->provideAuthentication(field->value);
        info->join.authProvided = true;
        break;
    }
	case FIELD_PLATFORM:
		info->join.platform = (ApiPlatform) atoi(field->value);
		break;
	}
}

bool processValueChangeField(FieldAndValue* field, MessageProcessorInfo* info) {
    if(info->value.item->getMenuType() == MENUTYPE_INT_VALUE || info->value.item->getMenuType() == MENUTYPE_ENUM_VALUE) {
        auto valItem = (ValueMenuItem*)info->value.item;
        if(info->value.changeType == CHANGE_ABSOLUTE) {
            uint16_t newValue = atol(field->value);
            valItem->setCurrentValue(newValue); // for absolutes, assume other system did checking.
        }
        else if(info->value.changeType == CHANGE_DELTA) {
            // get the delta and current values.
            int deltaVal = atoi(field->value);
            long existingVal = valItem->getCurrentValue();

            // prevent an underflow or overflow situation.
            if((deltaVal < 0 && existingVal < abs(deltaVal)) || (deltaVal > 0 && (existingVal + deltaVal) > valItem->getMaximumValue()) ) {
                return false; // valid update but outside of range.
            }

            // we must be good to go if we get here, write it..
            valItem->setCurrentValue(existingVal + deltaVal);
        }
        serdebugF2("Int change: ", valItem->getCurrentValue());
    }
    else if(info->value.item->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
        // booleans are always absolute
        BooleanMenuItem* boolItem = reinterpret_cast<BooleanMenuItem*>(info->value.item);
        boolItem->setBoolean(atoi(field->value));
        serdebugF2("Bool change: ", boolItem->getBoolean());
    }
    else if(info->value.item->getMenuType() == MENUTYPE_TEXT_VALUE) {
        // text is always absolute
        TextMenuItem* textItem = reinterpret_cast<TextMenuItem*>(info->value.item);
        textItem->setTextValue(field->value);
        serdebugF2("Text change: ", textItem->getTextValue());
    }
	else if (info->value.item->getMenuType() == MENUTYPE_IPADDRESS) {
		IpAddressMenuItem* ipItem = reinterpret_cast<IpAddressMenuItem*>(info->value.item);
		ipItem->setIpAddress(field->value);
		serdebugF2("Ip Addr change: ", field->value);
	}
    else if(info->value.item->getMenuType() == MENUTYPE_TIME) {
        TimeFormattedMenuItem* timeItem = reinterpret_cast<TimeFormattedMenuItem*>(info->value.item);
        timeItem->setTimeFromString(field->value);
        serdebugF2("Time item change: ", field->value);
    }
    return true;
}

bool processIdChangeField(FieldAndValue* field, MessageProcessorInfo* info) {
    int id = atoi(field->value);

    MenuItem* foundItem = getMenuItemById(id);
    if(foundItem != NULL && !foundItem->isReadOnly()) {
        info->value.item = foundItem;
        serdebugF2("ValChange for ID ", foundItem->getId());
        return true;
    }
    else {
        serdebugF("Bad ID on valchange msg");
        info->value.item = NULL;
        return false;
    }
}

void fieldUpdateValueMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info) {
	if(field->fieldType == FVAL_END_MSG) {
		// if this is an action item, we trigger the callback to occur just before ending.
		if(info->value.item != NULL && info->value.item->getMenuType() == MENUTYPE_ACTION_VALUE) {
			info->value.item->triggerCallback();
		}
		return;
	}
	
    bool ret = false;

	switch(field->field) {
    case FIELD_CORRELATION:
        info->value.correlation = strtoul(field->value, NULL, 16);
        break;
	case FIELD_ID:
        ret = processIdChangeField(field, info);
        if(!ret) connector->encodeAcknowledgement(info->value.correlation, ACK_ID_NOT_FOUND);
		break;
	case FIELD_CURRENT_VAL:
        if(info->value.item != NULL) {
            ret = processValueChangeField(field, info);
            connector->encodeAcknowledgement(info->value.correlation, ret ? ACK_SUCCESS : ACK_VALUE_RANGE);
        }
		break;
	case FIELD_CHANGE_TYPE:
		info->value.changeType = (ChangeType) atoi(field->value);
		break;
	}
}
