/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * MessageProcessors.cpp - standard message processors that decode tcMenu messages.
 */
#include "RemoteConnector.h"
#include "MessageProcessors.h"

/**
 * An array of message handlers, where each one is a function that can process that type of message and a message type.
 * Messages are received a field at a time, so each time the function is called a new field will be available, when the
 * last field is processed the end indicator will be set.
 * @See TagValueRemoteConnector
 */

MsgHandler msgHandlers[] = {
	{ fieldUpdateValueMsg, MSG_CHANGE_INT }, 
	{ fieldUpdateJoinMsg, MSG_JOIN }
};

void fieldUpdateJoinMsg(TagValueRemoteConnector* connector, FieldAndValue* field, MessageProcessorInfo* info) {
	if(field->fieldType == FVAL_END_MSG) {
		connector->setRemoteConnected(info->join.major, info->join.minor, info->join.platform);
		return;
	}

	switch(field->field) {
	case FIELD_MSG_NAME:
		connector->setRemoteName(field->value);
		break;
	case FIELD_VERSION: {
		int val = atoi(field->value);
		info->join.major = val / 100;
		info->join.minor = val % 100;
		break;
	}
	case FIELD_PLATFORM:
		info->join.platform = (ApiPlatform) atoi(field->value);
		break;
	}
}

MenuItem* findItem(MenuItem* itm, uint16_t id) {
	while(itm != NULL && itm->getId() != id) {
		itm = itm->getNext();
	}
	return itm;
}

void fieldUpdateValueMsg(TagValueRemoteConnector* /*unused*/, FieldAndValue* field, MessageProcessorInfo* info) {
	if(field->fieldType == FVAL_END_MSG) {
		// if this is an action item, we trigger the callback to occur just before ending.
		if(info->value.item != NULL && info->value.item->getMenuType() == MENUTYPE_ACTION_VALUE) {
			info->value.item->triggerCallback();
		}
		return;
	}
	
	switch(field->field) {
	case FIELD_PARENT:
		info->value.parentId = atoi(field->value);
		break;
	case FIELD_ID: {
		int id = atoi(field->value);
		MenuItem* sub;
		if(info->value.parentId != 0) {
			sub = findItem(menuMgr.getRoot(), info->value.parentId);
			if(sub == NULL || sub->getMenuType() != MENUTYPE_SUB_VALUE) return;
			sub = ((SubMenuItem*)sub)->getChild();
		}
		else {
			sub = menuMgr.getRoot();
		}
        
		MenuItem* foundItem = findItem(sub, id);
        if(foundItem != NULL && !foundItem->isReadOnly()) {
            info->value.item = foundItem;
        }
		break;
	}
	case FIELD_CURRENT_VAL:
		if((info->value.item != NULL) && (info->value.item->getMenuType() == MENUTYPE_INT_VALUE || info->value.item->getMenuType() == MENUTYPE_ENUM_VALUE)) {
			auto valItem = (ValueMenuItem*)info->value.item;
			if(info->value.changeType == CHANGE_ABSOLUTE) {
				uint16_t newValue = atol(field->value);
				valItem->setCurrentValue(newValue); // for absolutes, assume other system did checking.
			}
			else {
				// get the delta and current values.
				int deltaVal = atoi(field->value);
				long existingVal = valItem->getCurrentValue();

				// prevent an underflow or overflow situation.
				if((deltaVal < 0 && existingVal < abs(deltaVal)) || (deltaVal > 0 && (existingVal + deltaVal) > valItem->getMaximumValue()) ) return;

				// we must be good to go if we get here, write it..
				valItem->setCurrentValue(existingVal + deltaVal);
			}
		}
		else if(info->value.item != NULL && info->value.item->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
			// booleans are always absolute
			((BooleanMenuItem*)info->value.item)->setBoolean(atoi(field->value));
		}
		else if(info->value.item != NULL && info->value.item->getMenuType() == MENUTYPE_TEXT_VALUE) {
			// text is always absolute
			((TextMenuItem*)info->value.item)->setTextValue(field->value);
		}
		break;
	case FIELD_CHANGE_TYPE:
		info->value.changeType = (ChangeType) atoi(field->value);
		break;
	}
}
