/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

 /**
  * @file RuntimeMenuItem.h
  * Contains definitions of menu items that can be fully defined at runtime with no need for prog mem structures.
  */

#ifndef _RUNTIME_MENUITEM_H_
#define _RUNTIME_MENUITEM_H_

#include "MenuItems.h"

/**
 * this is used in the redering function to indicate what needs to be done in this call.
 */
enum RenderFnMode : byte {
	/** render the current value for the item provided */
	RENDERFN_VALUE,
	/** render the name part into the buffer */
	RENDERFN_NAME,
	/** the callback has been triggered */
	RENDERFN_INVOKE,
	/** A new value for a position in the list, provided in buffer, it's length is in size. - used only in editable mode */
	RENDERFN_SET_VALUE,
	/** Gets the range zero based, for this part - used only in editable mode */
	RENDERFN_GETRANGE,
	/** Gets the integer value of the current part that is being edited */
	RENDERFN_GETPART
};

/**
 * When implementing runtime menu items, they need to have a render function that is called for
 * to obtain the name and value for each row, and should populate the buffer with appropriate 
 * text.
 *
 * @param item the menu item that is being drawn
 * @param row the row in the list of items that is being drawn
 * @param mode one of the RenderFnMode enumerations indicating what you need to do.
 * @param buffer the buffer to copy into
 * @param bufferSize the size of the buffer available.
 */
typedef int (*RuntimeRenderingFn)(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

/** This is the standard renderering function used for editable text items, for use with TextMenuItem */
int textItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

/** This is the standard rendering function used for editable IP addresses, for use with IpAddressMenuItem */
int ipAddressRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

/**
 * A menu item that can be defined at runtime and needs no additional structures. This represents a single value in terms of
 * name and value. The itemPosition will be passed to the rendering function, so you can use the same function for many items.
 * This is the base that is the most configurable, but also most difficult to use, consider one of the more specific sub types 
 * of this for general use.
 */
class RuntimeMenuItem : public MenuItem {
protected:
	uint16_t id;
	uint16_t eeprom;
	RuntimeRenderingFn renderFn;
	uint8_t itemPosition;
	uint8_t noOfParts;
public:
	RuntimeMenuItem(MenuType menuType, uint16_t id, uint16_t eeprom, RuntimeRenderingFn renderFn, 
				    uint8_t itemPosition, uint8_t numberOfRows, MenuItem* next = NULL);
	
	void copyValue(char* buffer, int bufferSize) {
		renderFn(this, itemPosition, RENDERFN_VALUE, buffer, bufferSize);
	}

	void runCallback() { renderFn(this, itemPosition, RENDERFN_INVOKE, NULL, 0); }
	int getRuntimeId() { return id; }
	int getRuntimeEeprom() { return eeprom; }
	uint8_t getNumberOfParts() { return noOfParts; }
	void copyRuntimeName(char* buffer, int bufferSize) { renderFn(this, itemPosition, RENDERFN_NAME, buffer, bufferSize); }
};

/**
 * A menu item that represents a list of items and can be defined at runtime. This takes an ID that 
 * will act as a range, this is important, as this will use from ID through to ID + numberOfItems so 
 * always allocate enough ID range space. 
 * It can either represent a single item or a range of items, for a single item set the initial
 * rows to 0. The value and name of each item is obtained from the callback function.
 * Note that setting one item change sets all items in the list changed and similar with any other flag.
 * These are the only menu items that can presently be created dynamically at runtime.
 */
class ListRuntimeMenuItem : public RuntimeMenuItem {
public:
	ListRuntimeMenuItem(uint16_t id, int numberOfRows, RuntimeRenderingFn renderFn, MenuItem* next = NULL);

	RuntimeMenuItem* getChildItem(int pos) {
		itemPosition = (pos < noOfParts) ? pos : 0xff;
		return this;
	}

	RuntimeMenuItem* asParent() { 
		itemPosition = 0xff;
		return this;
	}

	void setNumberOfRows(uint8_t rows) { 
		noOfParts = rows; 
		setChanged(true); 
		setSendRemoteNeededAll(); 
	}
};

/**
 * This menu item allows local editing of anything that can be described as a range of values to be edited with the 
 * spinwheel or emulator. It extends from runtime menu item so has a small footprint in RAM.
 */
template<class V> class EditableMultiPartMenuItem : public RuntimeMenuItem {
protected:
	V data;
public:
	EditableMultiPartMenuItem(MenuType type, uint16_t id, uint16_t eeprom, int numberOfParts, RuntimeRenderingFn renderFn, MenuItem* next = NULL) 
			: RuntimeMenuItem(type, id, eeprom, renderFn, 0, numberOfParts, next) {
	}

	uint8_t beginMultiEdit() {
		setEditing(true);
		itemPosition = 0;
		return noOfParts;
	}

	int nextPart() {
		itemPosition++;
		if (itemPosition > noOfParts) {
			stopMultiEdit();
			return 0;
		}

		setChanged(true);
		setSendRemoteNeededAll();
		return renderFn(this, itemPosition, RENDERFN_GETRANGE, NULL, 0);
	}
	
	void stopMultiEdit() {
		itemPosition = 0xff;
		setEditing(false);
		setChanged(true);
		setSendRemoteNeededAll();
		runCallback();
	}

	int getPartValueAsInt() {
		return renderFn(this, itemPosition, RENDERFN_GETPART, NULL, 0);
	}

	bool valueChanged(int newVal) {
		setChanged(true);
		setSendRemoteNeededAll();

		byte sz[2];
		sz[0] = lowByte(newVal);
		sz[1] = highByte(newVal);
		return renderFn(this, itemPosition, RENDERFN_SET_VALUE, sz, sizeof(sz));
	}
};

/**
 * An item that can represent a text value that is held in RAM, and therefore change at runtime. We now manually
 * configure the settings for this menu item in the constructor. This variant gets the name from program memory
 */
class TextMenuItem : public EditableMultiPartMenuItem<char*> {
public:
	TextMenuItem(RuntimeRenderingFn customRenderFn, uint16_t id, uint16_t eeprom, int size, MenuItem* next = NULL)
		: EditableMultiPartMenuItem(MENUTYPE_TEXT_VALUE, id, eeprom, size, customRenderFn, next) {
		data = new char[size];
		memset(data, 0, size);
	}

	~TextMenuItem() { delete data; }

	/** get the max length of the text storage */
	uint8_t textLength() { return noOfParts; }

	/**
	 * Copies the text into the internal buffer.
  	 * @param text the text to be copied.
	 */
	void setTextValue(const char* text, bool silent = false);

	/** returns the text value in the internal buffer */
	const char* getTextValue() { return data; }

private:
	bool setCharValue(uint8_t location, char val);
	friend int textItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);
};

/**
 * This menu item represents an IP address that can be configured / or just displayed on the device,
 * if it is editable it is edited 
 */
class IpAddressMenuItem : public EditableMultiPartMenuItem<byte[4]> {
public:
	IpAddressMenuItem(RuntimeRenderingFn renderFn, uint16_t id, uint16_t eeprom, MenuItem* next = NULL)
		: EditableMultiPartMenuItem(MENUTYPE_IPADDRESS, id, eeprom, 4, renderFn, next) {
		setIpAddress(127, 0, 0, 1);
	}

	/** Sets the whole IP address as four parts */
	void setIpAddress(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4) {
		data[0] = p1;
		data[1] = p2;
		data[2] = p3;
		data[3] = p4;
		setChanged(true);
		setSendRemoteNeededAll();
	}

	/** gets the IP address as four separate bytes */
	byte* getIpAddress() { return data; }
	
	/** sets a single part in the address */
	void setIpPart(uint8_t part, byte newVal) {
		if (part > 3) return;
		data[part] = newVal;
		setChanged(true);
		setSendRemoteNeededAll();
	}
};

/**
 * This macro defines a rendering callback that will be often used with remote types, it takes as it's parameters
 * a parent function for the type in question, the variable containing the progmem name and the call back method
 * or NULL if there's no callback.
 */
#define RENDERING_CALLBACK_NAME_INVOKE(fnName, parent, namepgm, invoke) \
int fnName(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int buffSize) { \
	switch(mode) { \
	case RENDERFN_NAME: \
		safeProgCpy(buffer, namepgm, buffSize); \
		return true; \
	case RENDERFN_INVOKE: \
		if(invoke) (reinterpret_cast<MenuCallbackFn>(invoke))(int(item->getId())); \
		return true; \
	default: \
		return parent(item, row, mode, buffer, buffSize); \
	} \
}

#endif //_RUNTIME_MENUITEM_H_

