/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _EDITABLE_LARGE_NUMBER_MENU_ITEM_H_
#define _EDITABLE_LARGE_NUMBER_MENU_ITEM_H_

#include "RuntimeMenuItem.h"
#include <IoLogging.h>

/**
 * @file EditableLargeNumberMenuItem.h
 * @brief This file contains the classes needed to edit very large number values that could not be edited with a rotary encoder.
 * These numeric values have 12 digits, and as many decimal places as required.
 */

#define LARGE_NUM_MAX_DIGITS 12
#define LARGE_NUM_ALLOC_SIZE (LARGE_NUM_MAX_DIGITS / 2)

/**
 * A structure for very large numbers that can be edited using the multipart editor. They are represented as binary
 * coded decimal to 12 dp with between 0 and 9 fraction decimal places if needed. The whole can be either negative or
 * positive between 1 and 9 decimal places. Both fraction and whole can be extracted as either a floating point, a
 * per digit value, or as integers containing whole, fraction and negative flag.
 */
class LargeFixedNumber {
private:
	uint8_t bcdRepresentation[LARGE_NUM_ALLOC_SIZE] = {};
    bool negative = false;
    uint8_t totalSize = 12;
    uint8_t fractionDp = 0;
public:
    /**
     * Create a default instance which needs to be configured before use
     */
    LargeFixedNumber() = default;

    /**
     * Create a fully populated LargeFixedNumber giving the number of digits and the initial value
     * @param totalDigits the maximum digits to use
     * @param decimalPointIndex the number of decimal places
     * @param whole the whole value expressed as an unsigned integer
     * @param fraction the fractional part expressed as a unsigned integer
     * @param negative if the value is positive or negative
     */
    LargeFixedNumber(int totalDigits, int decimalPointIndex, uint32_t whole, uint32_t fraction, bool negative);

    LargeFixedNumber(const LargeFixedNumber& other) = default;
    LargeFixedNumber& operator=(const LargeFixedNumber& other) = default;

    /**
     * Clears the whole structure before setting to a new value
     */
	void clear();

	/**
	 * @return the number of decimal places this represents.
	 */
    int decimalPointIndex() const { return fractionDp; }

    /**
     * @return the total number of digits it can represent
     */
    int getTotalDigits() const { return totalSize; }

	/**
	 * Set the number of decimal places and optionally the total size then zero out any currently held value. When
	 * setting this value you should ensure that: fractionDp is not larger than 9, the difference between fractionDp
	 * and maxDigits is not greater than 9.
	 * @param dp the new number of decimal places
	 * @param maxDigits the total number of digits needed.
	 */
    void setPrecision(uint8_t dp, uint8_t maxDigits = 12) {
        fractionDp = dp;
        totalSize = maxDigits;
        clear();
    }

    /**
     * Sets the value of this instance without changing the decimal places.
     * @param whole the whole part of the value
     * @param fraction the fractional part of the value
	 * @param negative if the value to hold is negative.
     */
	void setValue(uint32_t whole, uint32_t fraction, bool negative);

    /**
     * Takes a floating point value and converts it into the internal representation.
     * This will represent the float within the bounds of the current total digits and decimal precision.
     * @param value the float value to convert
     */
    void setFromFloat(float value);

    /**
     * Converts from the BCD packed structure into an integer
     * @param start the index to start in the packed data
     * @param end will stop at one before this point
     * @return the integer value 
     */
	uint32_t fromBcdPacked(int start, int end);

    /**
     * Converts from an integer into BCD packed.
     * @param value the value to be encoded
     * @param start the index to start in the packed data
     * @param end will stop at one before this point
     */
	void convertToBcdPacked(uint32_t value, int start, int end);

    /**
     * get a particular digit from the bit packed structure.
     * @param digit the number of the digit zero based.
     * @return the number at this location
     */
	int getDigit(int digit);

    /**
     * set a particular digit in the packed structure to a value
     * @param digit the number of the digit zero based
     * @param value the new value for that digit
     */
	void setDigit(int digit, int val);

    /**
     * Gets the value converted to a float, note that floats cannot represent all values accurately
     * and as such this will be the nearest float that represents the value. With numbers that get
     * close to the 12 digit range it is highly possible that the float will be inaccurate.
     * 
     * @return the current represented value as a floating point number.
     */
    float getAsFloat();

    /**
     * @return true if negative otherwise false.
     */
    bool isNegative() const { return negative; }

	/**
	 * Sets the negative flag, if true the number becomes a negative value.
	 * @param neg the new negative flag value
	 */
	void setNegative(bool neg) {
		negative = neg;
	}

    /**
     * @return the whole part of the value
     */
    uint32_t getWhole() {
        return fromBcdPacked(fractionDp, totalSize);
    }

    /**
     * @return the fractional part of the value
     */
    uint32_t getFraction() {
        return fromBcdPacked(0, fractionDp);
    }

	/**
	 * Gets the underlying buffer to enable storage / loading from EEPROM
	 * @return the underlying bcd buffer
	 */
	uint8_t* getNumberBuffer() {
		return bcdRepresentation;
	}
};

/**
 * this is the runtime function that should be used with EditableLargeNumberMenuItem
 */
int largeNumItemRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

/**
 * A multipart editor for very large numbers, either integer or fixed point decimal that exceed the usable range 
 * of a rotary encoder or joystick to set their value. This class works by editing each digit in turn. This is based
 * on LargeFixedNumber, see that for more details on the capabilities
 * RenderFn - largeNumItemRenderFn
 * @see LargeFixedNumber
 */
class EditableLargeNumberMenuItem : public EditableMultiPartMenuItem {
private:
    LargeFixedNumber data;
    bool negativeAllowed;
public:
    EditableLargeNumberMenuItem(RuntimeRenderingFn renderFn, const LargeFixedNumber& initial, uint16_t id, bool allowNeg, MenuItem* next = nullptr)
            : EditableMultiPartMenuItem(MENUTYPE_LARGENUM_VALUE, id, initial.getTotalDigits() + (allowNeg ? 1 : 0), renderFn, next) {
        data = initial;
        negativeAllowed = allowNeg;
    }

    EditableLargeNumberMenuItem(RuntimeRenderingFn renderFn, uint16_t id, int maxDigits, int dps, bool allowNeg, MenuItem* next = nullptr)
            : EditableMultiPartMenuItem(MENUTYPE_LARGENUM_VALUE, id, maxDigits + (allowNeg ? 1 : 0), renderFn, next) {
        data.setPrecision(dps, maxDigits);
        negativeAllowed = allowNeg;
    }

	EditableLargeNumberMenuItem(RuntimeRenderingFn renderFn, uint16_t id, int maxDigits, int dps, MenuItem* next = nullptr)
		: EditableMultiPartMenuItem(MENUTYPE_LARGENUM_VALUE, id, maxDigits + 1, renderFn, next) {
        data.setPrecision(dps, maxDigits);
        negativeAllowed = true;
	}

    EditableLargeNumberMenuItem(const AnyMenuInfo* info, const LargeFixedNumber& initial, bool allowNeg, MenuItem* next = nullptr, bool isPgm = INFO_LOCATION_PGM)
		: EditableMultiPartMenuItem(info, isPgm, MENUTYPE_LARGENUM_VALUE, initial.getTotalDigits() + (allowNeg ? 1 : 0), largeNumItemRenderFn, next) {
        data = initial;
        negativeAllowed = allowNeg;
	}

    /** gets the large integer value that this class is using */
	LargeFixedNumber* getLargeNumber() { return &data; }  

    /** sets a number from a string in the form whole.fraction */
    void setLargeNumberFromString(const char* largeNum);

    bool isNegativeAllowed() const { return negativeAllowed; }
};

#endif //_EDITABLE_LARGE_NUMBER_MENU_ITEM_H_