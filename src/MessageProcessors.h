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
 * Contains the message processors that are used to process incoming messages.
 */

/**
 * Message processors need to store some state while they are working through the fields
 * of a message, this union keeps state between a message starting processing and ending
 * It can be added to with additional unions. It is essentially stored globally so size
 * is an issue.
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

#endif /* _TCMENU_MESSAGEPROCESSORS_H_ */
