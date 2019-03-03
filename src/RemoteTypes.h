/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */
/**
 * @file RemoteTypes.h
 * 
 * contains the definitions of each message and field.
 */

#ifndef _TCMENU_REMOTETYPES_H_
#define _TCMENU_REMOTETYPES_H_

/**
 * This defines the maximum size of any field value that can be received by this library.
 * If you need longer fields, change this value to a higher one.
 */
#define MAX_VALUE_LEN 24

/**
 * A helper to generate the major minor version numbers used in the protocol
 */
#define majorminor(maj, min) ((maj * 100) + min)

/**
 * Definition of the current API version
 */
#define API_VERSION majorminor(1, 0)

/**
 * Converts a message field as two separate entities into a single word.
 */
#define msgFieldToWord(a,b)  ( (((uint16_t)a)<<8) | ((uint16_t)b) )

/*
 * Definitions for an unknown field key or part thereof.
 */
#define UNKNOWN_FIELD_PART 0x00

/**
 * Definition for an unknown message key
 */
#define UNKNOWN_MSG_TYPE 0x0000

/** Message type definition for Join message */
#define MSG_JOIN msgFieldToWord('N','J')
/** Message type definition for heartbeat message */
#define MSG_HEARTBEAT msgFieldToWord('H','B')
/** Message type definition for Bootstrap message */
#define MSG_BOOTSTRAP msgFieldToWord('B','S')
/** Message type definition for Analog item bootstrap message */
#define MSG_BOOT_ANALOG msgFieldToWord('B','A')
/** Message type definition for ACtion item bootstap message */
#define MSG_BOOT_ACTION msgFieldToWord('B', 'C')
/** Message type definition for sub menu bootstrap message */
#define MSG_BOOT_SUBMENU msgFieldToWord('B', 'M')
/** Message type definition for enum bootstrap message */
#define MSG_BOOT_ENUM msgFieldToWord('B', 'E')
/** Message type definition for boolean bootstrap message */
#define MSG_BOOT_BOOL msgFieldToWord('B', 'B')
/** Message type definition for text boostrap message */
#define MSG_BOOT_TEXT msgFieldToWord('B','T')
/** Message type definition for floating point bootstrap message */
#define MSG_BOOT_FLOAT msgFieldToWord('B','F')
/** Message type definition for remote status bootstrap message */
#define MSG_BOOT_REMOTE msgFieldToWord('B','R')
/** Message type definition for value change message */
#define MSG_CHANGE_INT msgFieldToWord('V', 'C')

#define FIELD_MSG_TYPE    msgFieldToWord('M', 'T')
#define FIELD_MSG_NAME    msgFieldToWord('N', 'M')
#define FIELD_VERSION     msgFieldToWord('V', 'E')
#define FIELD_PLATFORM    msgFieldToWord('P', 'F')
#define FIELD_BOOT_TYPE   msgFieldToWord('B', 'T')
#define FIELD_ID          msgFieldToWord('I', 'D')
#define FIELD_READONLY    msgFieldToWord('R', 'O')
#define FIELD_PARENT      msgFieldToWord('P', 'I')
#define FIELD_ANALOG_MAX  msgFieldToWord('A', 'M')
#define FIELD_ANALOG_OFF  msgFieldToWord('A', 'O')
#define FIELD_ANALOG_DIV  msgFieldToWord('A', 'D')
#define FIELD_ANALOG_UNIT msgFieldToWord('A', 'U')
#define FIELD_CURRENT_VAL msgFieldToWord('V', 'C')
#define FIELD_BOOL_NAMING msgFieldToWord('B', 'N')
#define FIELD_NO_CHOICES  msgFieldToWord('N', 'C')
#define FIELD_CHANGE_TYPE msgFieldToWord('T', 'C')
#define FIELD_MAX_LEN     msgFieldToWord('M', 'L')
#define FIELD_REMOTE_NO   msgFieldToWord('R', 'N')
#define FIELD_FLOAT_DP    msgFieldToWord('F', 'D')

#define FIELD_PREPEND_CHOICE 'C'

/**
 * Defines the types of change that can be received / sent in changes messages, either
 * delta or incremental (for example menuVolume + 3) or absolulte (channel is now 2)
 */
enum ChangeType: byte {
	CHANGE_DELTA = 0, CHANGE_ABSOLUTE = 1
};

/**
 * Defines the API platforms that are supported at the moment
 */
enum ApiPlatform : byte {
	PLATFORM_ARDUINO_8BIT = 0,
	PLATFORM_JAVA_API = 1,
    PLATFORM_ARDUINO_32BIT = 2
};

#endif /* _TCMENU_REMOTETYPES_H_ */
