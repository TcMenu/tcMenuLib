/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file RuntimeMenuItem.h
 * @brief Contains definitions of menu items that can be fully defined at runtime with no need for prog mem structures.
 */

#ifndef _RUNTIME_MENUITEM_H_
#define _RUNTIME_MENUITEM_H_

#include "MenuItems.h"
#include "tcUtil.h"

/** For items that dont need to have the same id each time (such as back menu items), we just randomly give them an ID */
#define RANDOM_ID_START 50000

/** For items that dont need to have the same id each time (such as back menu items), we just randomly give them an ID */
menuid_t nextRandomId();

/** This is the standard renderering function used for editable text items, for use with TextMenuItem */
int textItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

/** This is the standard rendering function used for editable IP addresses, for use with IpAddressMenuItem */
int ipAddressRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

/** The default rendering function for back menu items */
int backSubItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

/** The default rendering function for time menu items */
int timeItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

/** The default rendering function for time menu items */
int dateItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

/** helper function for text items that finds the position of a char in the allowable set of editable chars */
int findPositionInEditorSet(char ch);

/**
 * Defines the filter that should be applied to values of multi edit menu items on the UI
 */
enum MultiEditWireType : uint8_t {
    /** plain text - zero terminated text data */
	EDITMODE_PLAIN_TEXT = 0,
	/** an ipV4 address */
	EDITMODE_IP_ADDRESS = 1,
	/** a time in the 24 hour clock HH:MM:SS */
	EDITMODE_TIME_24H = 2,
	/** a time in the 12 hour clock HH:MM:SS[AM/PM] */
	EDITMODE_TIME_12H = 3,
	/** a time in the 24 hour clock with hundreds HH:MM:SS.ss */
	EDITMODE_TIME_HUNDREDS_24H = 4,
    /** a date in gregorian format, see DateFormattedMenuItem for global locale formatting */
    EDITMODE_GREGORIAN_DATE = 5,
    /** a duration that optionally shows hours when needed, to the second */
	EDITMODE_TIME_DURATION_SECONDS = 6,
    /** a duration that optionally shows hours when needed, to the hundredth of second */
	EDITMODE_TIME_DURATION_HUNDREDS = 7,
    /** a time in the 24 hour clock HH:MM */
    EDITMODE_TIME_24H_HHMM = 8,
    /** a time in the 12 hour clock HH:MM[AM/PM] */
	EDITMODE_TIME_12H_HHMM = 9,
};

/**
 * A menu item that can be defined at runtime and needs no additional structures. This represents a single value in terms of
 * name and value. The itemPosition will be passed to the rendering function, so you can use the same function for many items.
 * This is the base that is the most configurable, but also most difficult to use, consider one of the more specific sub types 
 * of this for general use.
 */
class RuntimeMenuItem : public MenuItem {
protected:
    menuid_t id;
	uint8_t itemPosition;
	uint8_t noOfParts;
public:
    RuntimeMenuItem(MenuType menuType, menuid_t id, RuntimeRenderingFn renderFn,
				    uint8_t itemPosition, uint8_t numberOfRows, MenuItem* next = nullptr);

    RuntimeMenuItem(const AnyMenuInfo* rtInfo, bool isPgm, MenuType menuType, RuntimeRenderingFn renderFn,
                    uint8_t itemPosition, uint8_t numberOfRows, MenuItem* next = nullptr);

	void copyValue(char* buffer, int bufferSize) const {
		renderFn((RuntimeMenuItem*)this, itemPosition, RENDERFN_VALUE, buffer, bufferSize);
	}

	void runCallback() const { renderFn((RuntimeMenuItem*)this, itemPosition, RENDERFN_INVOKE, nullptr, 0); }
	int getRuntimeId() const { return int(id); }
	int getRuntimeEeprom() const { return renderFn((RuntimeMenuItem*)this, itemPosition, RENDERFN_EEPROM_POS, nullptr, 0); }
	uint8_t getNumberOfParts() const { return noOfParts; }
	void copyRuntimeName(char* buffer, int bufferSize) const { renderFn((RuntimeMenuItem*)this, itemPosition, RENDERFN_NAME, buffer, bufferSize);}

    uint8_t getNumberOfRows() const { return noOfParts; }
    uint8_t getItemPosition() const { return itemPosition; }

    void setNumberOfRows(uint8_t rows) {
		noOfParts = rows;
		setChanged(true); 
		setSendRemoteNeededAll(); 
	}
};

/**
 * Back menu item pairs with an associated SubMenuItem, it only exists in the embedded domain - not the API.
 * This type is always the first item in a series of items for a submenu. It provides the functionality required
 * to get back to root. The default render function is: backSubItemRenderFn
 *
 * For example
 *
 * 		SubMenu.getChild() -> Back Menu Item -> Sub Menu Item 1 ...
 */
class BackMenuItem : public RuntimeMenuItem {
private:
    const char* namePtr;
public:
	/**
	 * Create an instance of the class
	 *
	 * @param nextChild the next menu in the chain if there is one, or NULL.
	 * @param renderFn the callback that provides the runtime information about the menu.
	 */
	BackMenuItem(RuntimeRenderingFn renderFn, MenuItem* next) 
		: RuntimeMenuItem(MENUTYPE_BACK_VALUE, nextRandomId(), renderFn, 0, 1, next), namePtr(nullptr) { }

    /**
     * Create an instance of the class using a SubMenuInfo block, it shares the info block
     * with the Submenu itself to save space.
     *
     * @param info the info block, mainly used for the name
     * @param next the next item in the linked list
     * @param infoInPgm if the info block is const/PROGMEM: true, RAM: false
     */
    BackMenuItem(const SubMenuInfo* info, MenuItem* next, bool infoInPgm);

    /**
     * @return the name pointer or null if not set, could be in progmem.
     */
    const char* getNameUnsafe() const { return namePtr; }
};

/**
 * The implementation of a Menuitem that can contain more menu items as children. The default render function for this
 * menu item is: backSubItemRenderFn
 */
class SubMenuItem : public RuntimeMenuItem {
private:
    MenuItem* child;
public:
    /**
     * Create an instance of SubMenuItem using the traditional SubMenuInfo block, this is no longer used, but we
     * still support it as a means of working with the name.
     * @deprecated use the other constructor, this constructor will be removed in a future version
     * @param info a SubMenuInfo structure
     * @param id the item ID
     * @param child the first child item - (normally a BackMenuItem)
     * @param next the next menu in the chain if there is one, or NULL.
     */
    SubMenuItem(const SubMenuInfo* info, MenuItem* child, MenuItem* next = nullptr, bool infoInPgm = INFO_LOCATION_PGM)
                : RuntimeMenuItem(info, infoInPgm, MENUTYPE_SUB_VALUE, backSubItemRenderFn, 0, 1, next) {
        this->child = child;
    }

    /**
     * Create an instance of SubMenuItem using the runtime method, which can easily be created during system runtime.
     * @param id the ID of the item
     * @param renderFn the callback function for this item
     * @param child the first child item in the sub menu
     * @param next the next menu in the chain if there i one, or NULL.
     */
    SubMenuItem(menuid_t id, RuntimeRenderingFn renderFn, MenuItem* child, MenuItem* next = nullptr)
            : RuntimeMenuItem(MENUTYPE_SUB_VALUE, id, renderFn,
                              0, 1, next) {
        this->child = child;
    }

    /**
     * return the first child item
     */
    MenuItem* getChild() const { return child; }
    void setChild(MenuItem* firstChildItem) { this->child = firstChildItem; }
};

#define LIST_PARENT_ITEM_POS 0xff

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
    enum ListMode: uint8_t { CUSTOM_RENDER, RAM_ARRAY, FLASH_ARRAY };
private:
    const char* const* dataArray;
	uint8_t activeItem;
    ListMode listMode = CUSTOM_RENDER;
public:
    ListRuntimeMenuItem(const AnyMenuInfo* info, int numberOfRows, const char* const* array, ListMode listMode, MenuItem* next = nullptr, bool isPgm = INFO_LOCATION_PGM);
    ListRuntimeMenuItem(const AnyMenuInfo* info, int numberOfRows, RuntimeRenderingFn renderFn, MenuItem* next = nullptr, bool isPgm = INFO_LOCATION_PGM);
    ListRuntimeMenuItem(menuid_t id, int numberOfRows, RuntimeRenderingFn renderFn, MenuItem* next = nullptr);

	RuntimeMenuItem* getChildItem(int pos);
	RuntimeMenuItem* asParent();
	RuntimeMenuItem* asBackMenu();

    ListMode getListMode() const {return listMode;}
    bool isActingAsParent() const { return itemPosition == LIST_PARENT_ITEM_POS; }
    uint8_t getActiveIndex() const { return activeItem; }
    void setActiveIndex(uint8_t idx) {
        activeItem = idx;
        setChanged(true);
    }
    const char* const* getDataArray() { return dataArray; }
};

int defaultRtListCallback(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

/**
 * This menu item allows local editing of anything that can be described as a range of values to be edited with the 
 * spinwheel or emulator. It extends from runtime menu item so has a small footprint in RAM.
 */
class EditableMultiPartMenuItem : public RuntimeMenuItem {
public:
    EditableMultiPartMenuItem(MenuType type, menuid_t id, int numberOfParts, RuntimeRenderingFn renderFn, MenuItem* next = nullptr)
			: RuntimeMenuItem(type, id, renderFn, 0, numberOfParts, next) {
	}
    EditableMultiPartMenuItem(const AnyMenuInfo* rtInfo, bool isPgm, MenuType type, int numberOfParts, RuntimeRenderingFn renderFn, MenuItem* next = nullptr)
            : RuntimeMenuItem(rtInfo, isPgm, type, renderFn, 0, numberOfParts, next) {
    }

	uint8_t beginMultiEdit();

	int changeEditBy(int amt);

	int previousPart();

	int nextPart();

	int getCurrentRange() const {
		return renderFn((RuntimeMenuItem*)this, itemPosition, RENDERFN_GETRANGE, NULL, 0);
	}
	
	void stopMultiEdit();

	int getPartValueAsInt() const {
		return renderFn((RuntimeMenuItem*)this, itemPosition, RENDERFN_GETPART, NULL, 0);
	}

	bool valueChanged(int newVal);
};

// number of characters in the edit set.
#define ALLOWABLE_CHARS_ENCODER_SIZE 94

/**
 * An item that can represent a text value that is held in RAM, and therefore change at runtime. We now manually
 * configure the settings for this menu item in the constructor. This variant gets the name from program memory.
 * The default render function for this type is: `textItemRenderFn`
 */
class TextMenuItem : public EditableMultiPartMenuItem {
private:
    char* data;
    bool passwordField;
public:
    /**
     * Create a text menu item using a RuntimeRenderingFn to handle all elements of the item, including the name and
     * eeprom, this requires you to define a custom callback.
     * @param customRenderFn the custom callback render function
     * @param id the ID of the item
     * @param size the size of the text array
     * @param next optionally, the next item in the linked list
     */
    TextMenuItem(RuntimeRenderingFn customRenderFn, menuid_t id, int size, MenuItem* next = nullptr);
    /**
     * Create a text menu item using a RuntimeRenderingFn to handle all elements of the item, including the name and
     * eeprom, this requires you to define a custom callback.
     * @param customRenderFn the custom callback render function
     * @param initial the initial value or nullptr
     * @param id the ID of the item
     * @param size the size of the text array
     * @param next optionally, the next item in the linked list
     */
    TextMenuItem(RuntimeRenderingFn customRenderFn, const char* initial, menuid_t id, int size, MenuItem* next = nullptr);
    /**
     * Create a text menu item using an info block that holds the name, eeprom and ID values, either in PGM or RAM. This
     * version does not even need the render function to be provided, and uses the default.
     * @param info the info block with the static parameters
     * @param initial the initial value or nullptr
     * @param size the size of the array for text
     * @param next optionally the next item
     */
    TextMenuItem(const AnyMenuInfo* info, const char* initial, int size, MenuItem* next = nullptr, bool isPgm = INFO_LOCATION_PGM);

    /**
     * Create a text menu item using an info block that holds the name, eeprom and ID values, either in PGM or RAM. This
     * version does not even need the render function to be provided, and uses the default.
     * @param info the info block with the static parameters
     * @param customRenderFn the rendering function for cases when you wish to override rendering
     * @param initial the initial value or nullptr
     * @param size the size of the array for text
     * @param next optionally the next item
     */
    TextMenuItem(const AnyMenuInfo* info, RuntimeRenderingFn customRenderFn, const char* initial, int size, MenuItem* next = nullptr, bool isPgm = INFO_LOCATION_PGM);

    void setPasswordField(bool pwd) {
        this->passwordField = pwd;
    }

    /**
     * @return true if the field is being masked for password entry, otherwise false
     */
    bool isPasswordField() const {
        return this->passwordField;
    }

	~TextMenuItem() { delete data; }

	/** @return the max length of the text storage */
	uint8_t textLength() const { return noOfParts; }

	/**
	 * Copies the text into the internal buffer.
  	 * @param text the text to be copied.
  	 * @param silent if the update should be notified via callback
	 */
	void setTextValue(const char* text, bool silent = false);

	/** @return the text value in the internal buffer */
	const char* getTextValue() const { return data; }

	/**
	 * Called after the array has been changed to ensure that it is in a good
	 * state for editing. IE zero's extended to the end.
	 */
	void cleanUpArray();

    /**
     * Set one character at a time of the value, use with care and ensure the string is always zero terminated if
     * not taking the full space.
     * @param location the location in the array
     * @param val the character value
     * @return true if able to set, otherwise false
     */
    bool setCharValue(uint8_t location, char val);

    /**
     * When working with keyboards this allows a value change in the form of a keypress to be decoded if possible
     * by the text control. It triggers a RENDERFN_SET_TEXT_VALUE which is specific to text controls that need to
     * be able to work with keyboard interfaces too.
     * @param keyPress the key on the keyboard that was pressed
     * @return true if successful otherwise false
     */
    bool valueChangedFromKeyboard(char keyPress);
private:
    void initTextItem(const char* initialData);
};

/**
 * finds the position of the character in the editor set
 * @param ch the character to find.
 * @return the location in the allowable chars
 */
int findPositionInEditorSet(char ch);

/**
 * Provides storage for an IP-V4 address using a 4 byte data structure. This does not validate the inputs and purely
 * acts as storage for the 4 parts of the address.
 */
class IpAddressStorage {
private:
    uint8_t data[4];
public:
    explicit IpAddressStorage(const char* address);
    IpAddressStorage(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4);
    IpAddressStorage(const IpAddressStorage& other) = default;
    IpAddressStorage& operator=(const IpAddressStorage& other) = default;

    void setPart(int part, uint8_t newValue) { data[part] = newValue; }
    uint8_t* underlyingArray() { return data; }
};

/**
 * This menu item represents an IP address that can be configured / or just displayed on the device,
 * if it is editable it is edited using the typical 4 byte entries. The default render function for
 * this type is: ipAddressRenderFn
 */
class IpAddressMenuItem : public EditableMultiPartMenuItem {
private:
    IpAddressStorage data;
public:
    /**
     * Create an IP address that initially points to 127.0.0.1, with a given ID and rendering function
     * @param renderFn the rendering function to use.
     * @param id the ID of this item
     * @param next optional pointer to next item
     */
    IpAddressMenuItem(RuntimeRenderingFn renderFn, menuid_t id, MenuItem* next = nullptr)
		: EditableMultiPartMenuItem(MENUTYPE_IPADDRESS, id, 4, renderFn, next), data(127, 0, 0, 1) {}

    /**
     * Create an IP address that has an initial value, with a given ID and rendering function
     * @param renderFn the rendering function to use.
     * @param id the ID of this item
     * @param ipParts a 4 digit IP address as a constant array
     * @param next optional pointer to next item
     */
    IpAddressMenuItem(RuntimeRenderingFn renderFn, const IpAddressStorage& initialIp, menuid_t id, MenuItem* next = nullptr)
		: EditableMultiPartMenuItem(MENUTYPE_IPADDRESS, id, 4, renderFn, next), data(initialIp) {}

    /**
     * Create an IP address that has an initial value, with static data taken from an info block
     * @param info the info block to use for static data
     * @param renderFn the rendering function to use.
     * @param id the ID of this item
     * @param ipParts a 4 digit IP address as a constant array
     * @param next optional pointer to next item
     * @param isPgm optional, if the info block resides in PGM memory or RAM, default PGM.
     */
    IpAddressMenuItem(const AnyMenuInfo* info, RuntimeRenderingFn renderFn, const IpAddressStorage& initialIp, MenuItem* next = nullptr, bool isPgm = INFO_LOCATION_PGM)
            : EditableMultiPartMenuItem(info, isPgm, MENUTYPE_IPADDRESS, 4, renderFn, next), data(initialIp) {}

    /**
     * Create an IP address that has an initial value, with a given ID and
     * @param info the info block to use for static data
     * @param id the ID of this item
     * @param ipParts a 4 digit IP address as a constant array
     * @param next optional pointer to next item
     * @param isPgm optional, if the info block resides in PGM memory or RAM, default PGM.
     */
    IpAddressMenuItem(const AnyMenuInfo* info, const IpAddressStorage& initialIp, MenuItem* next = nullptr, bool isPgm = INFO_LOCATION_PGM)
            : EditableMultiPartMenuItem(info, isPgm, MENUTYPE_IPADDRESS, 4, ipAddressRenderFn, next), data(initialIp) {}

	void setIpAddress(const char* source);

	/** Sets the whole IP address as four parts */
	void setIpAddress(uint8_t p1, uint8_t p2, uint8_t p3, uint8_t p4) {
        data = IpAddressStorage(p1, p2, p3, p4);
    }

	/** gets the IP address as four separate bytes */
	uint8_t* getIpAddress() { return data.underlyingArray(); }

    void setUnderlying(const IpAddressStorage& other);

    IpAddressStorage& getUnderlying() { return data; }
	
	/** sets a single part in the address */
	void setIpPart(uint8_t part, uint8_t newVal);
};

/**
 * The storage for a time field can hold down to hundreds of a second, stored as a series of bytes for hours, minutes,
 * seconds and hundreds.
 */
struct TimeStorage {
    TimeStorage() {
        this->hours = this->minutes = this->seconds = this->hundreds = 0;
    }
    TimeStorage(uint8_t hours, uint8_t minutes, uint8_t seconds = 0, uint8_t hundreds = 0) {
        this->hours = hours;
        this->minutes = minutes;
        this->seconds = seconds;
        this->hundreds = hundreds;
    }
    TimeStorage(const TimeStorage& other) = default;
    TimeStorage& operator=(const TimeStorage& other) = default;

    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint8_t hundreds;
};

/**
 * Storage of a date field where the month and day are represented as a single byte, and the year is a 16 bit value.
 */
struct DateStorage {
    uint8_t day;
    uint8_t month;
    uint16_t year;

    DateStorage() {
        year = day = month = 0;
    }

    DateStorage(int day, int month, int year) {
        this->day = day;
        this->month = month;
        this->year = year;
    }

    DateStorage(const DateStorage& other) = default;
    DateStorage& operator=(const DateStorage& other)=default;
};

/**
 * A runtime menu item that represents time value, either in the 24 hour clock or in the 12 hour clock. Further, the
 * hundreds can be configured to show as well. It is an extension of the multi part editor that supports editing
 * times in parts, hours, then minutes and so on.  Instances of this class should use the `timeItemRenderFn` for the
 * base rendering.
 */
class TimeFormattedMenuItem : public EditableMultiPartMenuItem {
private:
    MultiEditWireType format;
    TimeStorage data;
public:
    TimeFormattedMenuItem(RuntimeRenderingFn renderFn, menuid_t id, MultiEditWireType format, MenuItem* next = nullptr);
    TimeFormattedMenuItem(RuntimeRenderingFn renderFn, const TimeStorage& initial, menuid_t id, MultiEditWireType format, MenuItem* next = nullptr);
    TimeFormattedMenuItem(const AnyMenuInfo* info, RuntimeRenderingFn renderFn, const TimeStorage& initial, MultiEditWireType format, MenuItem* next = nullptr, bool isPgm = INFO_LOCATION_PGM);
    TimeFormattedMenuItem(const AnyMenuInfo* info, const TimeStorage& initial, MultiEditWireType format, MenuItem* next = nullptr, bool isPgm = INFO_LOCATION_PGM);

	/** gets the time as four separate bytes */
	TimeStorage getTime() const { return data; }
    
    /** sets the time */
	void setTime(TimeStorage newTime) { data = newTime; }

    /** sets a time from a string in the form HH:MM:SS[.ss]*/
    void setTimeFromString(const char* time);

    /** gets the formatting currently being used. */
    MultiEditWireType  getFormat() const { return format; }

    TimeStorage* getUnderlyingData() {return &data;}
};

/**
 * A runtime menu item that represents a date value in the gregorian calendar. It is an extension of the multipart editor
 * class that supports editing dates in parts, days, month and finally years. It has some very basic support for leap
 * years. Instances of this class should use the `dateItemRenderFn` for the base rendering.
 */
class DateFormattedMenuItem : public EditableMultiPartMenuItem {
public:
    enum DateFormatOption { DD_MM_YYYY, MM_DD_YYYY, YYYY_MM_DD };
private:
    DateStorage data;
    static char separator;
    static DateFormatOption dateFormatMode;
public:
    DateFormattedMenuItem(RuntimeRenderingFn renderFn, menuid_t id, MenuItem* next = nullptr)
            : EditableMultiPartMenuItem(MENUTYPE_DATE, id, 3, renderFn, next), data(1, 1, 2020) {}

    DateFormattedMenuItem(RuntimeRenderingFn renderFn, const DateStorage& initial, menuid_t id, MenuItem* next = nullptr)
            : EditableMultiPartMenuItem(MENUTYPE_DATE, id, 3, renderFn, next), data(initial) {}

    DateFormattedMenuItem(const AnyMenuInfo* info, RuntimeRenderingFn renderFn, const DateStorage& initial, menuid_t id, MenuItem* next = nullptr, bool isPgm = INFO_LOCATION_PGM)
            : EditableMultiPartMenuItem(info, isPgm, MENUTYPE_DATE, 3, renderFn, next), data(initial) {}

    DateFormattedMenuItem(const AnyMenuInfo* info, const DateStorage& initial, MenuItem* next = nullptr, bool isPgm = INFO_LOCATION_PGM)
            : EditableMultiPartMenuItem(info, isPgm, MENUTYPE_DATE, 3, dateItemRenderFn, next), data(initial) {}

    /**
     * sets the global separator for date rendering.
     * @param sep the new separator character
     */
    static void setDateSeparator(char sep) {
        separator = sep;
    }

    /**
     * @return the global separator for date rendering.
     */
    static char getDateSeparator() {
        return separator;
    }

    /**
     * Sets the global date formatting for dates, one of the enum DateFormatOption.
     * @param fmt the new format to use
     */
    static void setDateFormatStyle(DateFormatOption fmt) {
        dateFormatMode = fmt;
    }

    /**
     * @return the global date format option in use by all date formatters.
     */
    static DateFormatOption getDateFormatStyle() {
        return dateFormatMode;
    }

    DateStorage getDate() const { return data; }

    void setDate(DateStorage newDate) { data = newDate; }

    void setDateFromString(const char *dateText);

    DateStorage* getUnderlyingData() { return &data; }
};

/**
 * Utility function to parse a string from a given offset to obtain an integer
 * @param ptr the pointer to the text
 * @param offset a ref to an integer that starts as the offset and is updated
 * @return the integer that was obtain before a non digit was found
 */
long parseIntUntilSeparator(const char* ptr, int& offset, size_t maxDigits=10);

/**
 * Invokes a menu callback if it is safe to do so
 * @param cb callback to make
 * @param id menuId
 */
inline void invokeIfSafe(MenuCallbackFn cb, MenuItem* pItem) { if(cb && pItem) cb(pItem->getId()); }

/**
 * This macro defines the rendering function for all runtime menu items. It is used to override the name, function
 * callback and eeprom address leaving the rest to go to the default function. You provide the following:
 *
 * 1. name of the function to create
 * 2. the parent pass through function for the type
 * 3. the name a string to go into const memory
 * 4. the position in eeprom
 * 5. an optional invoke method, or nullptr.
 */
#define RENDERING_CALLBACK_NAME_INVOKE(fnName, parent, namepgm, eepromPosition, invoke) \
const char fnName##Pgm[] PROGMEM = namepgm; \
int fnName(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int buffSize) { \
	switch(mode) { \
	case RENDERFN_NAME: \
		safeProgCpy(buffer, fnName##Pgm, buffSize); \
		return true; \
    case RENDERFN_INVOKE: \
		invokeIfSafe(invoke, item); \
		return true; \
	case RENDERFN_EEPROM_POS: \
		return eepromPosition; \
	default: \
		return parent(item, row, mode, buffer, buffSize); \
	} \
}

/**
 * This macro defines the rendering function for all runtime menu items. This version works differently to to INVOKE
 * version, in that for RENDERFN_NAME it first calls the customFn and only if it returns false does it write the default
 * name. For eeprom it always sends back what its configured with, everything else is passed through to your function.
 *
 * 1. name of the function to create
 * 2. the custom function that you'll implement (that should call through to the default one).
 * 3. the name a string to go into const memory
 * 4. the position in eeprom
 */
#define RENDERING_CALLBACK_NAME_OVERRIDDEN(fnName, customFn, namepgm, eepromPosition) \
const char fnName##Pgm[] PROGMEM = namepgm; \
int fnName(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int buffSize) { \
	switch(mode) { \
	case RENDERFN_NAME: \
        if(customFn(item, row, mode, buffer, buffSize) == false) {  \
            safeProgCpy(buffer, fnName##Pgm, buffSize); \
        } \
		return true; \
	case RENDERFN_EEPROM_POS: \
		return eepromPosition; \
	default: \
		return customFn(item, row, mode, buffer, buffSize); \
	} \
}

#endif //_RUNTIME_MENUITEM_H_
