/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 *
 * MessageProcessors.cpp - standard message processors that decode tcMenu messages.
 */
#include "RemoteConnector.h"
#include "MessageProcessors.h"

ValueChangeMessageProcessor valueProcessor(NULL);
HeartbeatProcessor heartbeatProcessor(&valueProcessor);
JoinMessageProcessor rootProcessor(&heartbeatProcessor);

void JoinMessageProcessor::initialise() {
	this->major = this->minor = -1;
	this->platform = PLATFORM_ARDUINO_8BIT;
}

void JoinMessageProcessor::fieldRx(FieldAndValue* field) {
	switch(field->field) {
	case FIELD_MSG_NAME:
		//if(TagValueTransport::getListener()) TagValueTransport::getListener()->remoteNameChange(field->value);
		break;
	case FIELD_VERSION: {
		int val = atoi(field->value);
		major = val / 100;
		minor = val % 100;
		break;
	}
	case FIELD_PLATFORM:
		platform = (ApiPlatform) atoi(field->value);
		break;
	}
}

void JoinMessageProcessor::onComplete() {
//	if(TagValueTransport::getListener()) {
		//TagValueTransport::getListener()->newJoiner(major, minor, platform);
//	}
}

void ValueChangeMessageProcessor::initialise() {
	this->parentId = 0;
	this->item = NULL;
	this->changeType = CHANGE_ABSOLUTE;
	this->changeValue = 0;
}

MenuItem* findItem(MenuItem* itm, uint16_t id) {
	while(itm != NULL && itm->getId() != id) {
		itm = itm->getNext();
	}
	return itm;
}

void ValueChangeMessageProcessor::fieldRx(FieldAndValue* field) {
	switch(field->field) {
	case FIELD_PARENT:
		parentId = atoi(field->value);
		break;
	case FIELD_ID: {
		int id = atoi(field->value);
		MenuItem* sub;
		if(parentId != 0) {
			sub = findItem(menuMgr.getRoot(), parentId);
			if(sub == NULL || sub->getMenuType() != MENUTYPE_SUB_VALUE) return;
			sub = ((SubMenuItem*)sub)->getChild();
		}
		else {
			sub = menuMgr.getRoot();
		}
		item = findItem(sub, id);
		break;
	}
	case FIELD_CURRENT_VAL:
		if((item != NULL) && (item->getMenuType() == MENUTYPE_INT_VALUE || item->getMenuType() == MENUTYPE_ENUM_VALUE)) {
			auto valItem = (ValueMenuItem*)item;
			if(changeType == CHANGE_ABSOLUTE) {
				uint16_t newValue = atol(field->value);
				valItem->setCurrentValue(newValue); // for absolutes, assume other system did checking.
			}
			else {
				int deltaVal = atoi(field->value);
				long existingVal = valItem->getCurrentValue();

				if((existingVal < deltaVal) || ((existingVal + deltaVal) > valItem->getMaximumValue()) ) return; // prevent overflow

				valItem->setCurrentValue(existingVal + deltaVal);
			}
		}
		else if(item != NULL && item->getMenuType() == MENUTYPE_BOOLEAN_VALUE) {
			// booleans are always absolute
			((BooleanMenuItem*)item)->setBoolean(atoi(field->value));
		}
		else if(item != NULL && item->getMenuType() == MENUTYPE_TEXT_VALUE) {
			// text is always absolute
			((TextMenuItem*)item)->setTextValue(field->value);
		}
		break;
	case FIELD_CHANGE_TYPE:
		changeType = (ChangeType) atoi(field->value);
		break;
	}
}

void ValueChangeMessageProcessor::onComplete() {
}

void HeartbeatProcessor::onComplete() {
}
