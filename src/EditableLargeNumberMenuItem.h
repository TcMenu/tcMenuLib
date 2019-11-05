/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef _EDITABLE_LARGE_NUMBER_MENU_ITEM_H_
#define _EDITABLE_LARGE_NUMBER_MENU_ITEM_H_

#include "RuntimeMenuItem.h"

/**
 * @file EditableLargeNumberMenuItem.h
 * This file contains the classes needed to edit very large number values that could not be edited with a rotary encoder.
 * These numeric values have 12 digits, and as many decimal places as required.
 */

/**
 * A structure for very large integers that can be edited using the multipart editor.
 * They are represented as binary coded decimal to 12 dp with as many fraction decimal
 * places as needed. The whole can be either negative or positive, while the fraction is 
 * an unsigned number in 32 bit form. This provides numbers in the range of at least 9 digits before
 * the dp, and 9 after.
 */
class LargeFixedNumber {
private:
	uint8_t bcdRepresentation[6];
    bool negative;
    uint8_t fractionDp;
public:
    /**
     * Create a default instance with decimal places set to 4
     */
    LargeFixedNumber() {
		setPrecision(4);
    }

    /**
     * Clears the whole structure before setting to a new value
     */
	void clear();

	/**
	 * @return the number of decimal places needed.
	 */
    int decimalPointIndex() { return fractionDp; }

	/**
	 * Set the number of decimal places and zero out any currently held value.
	 * @param dp the new number of decimal places
	 */
    void setPrecision(uint8_t dp) {
        fractionDp = dp;
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
     * This class can nearly always accurately represent a float value unless it exceeds
     * 12 digits.
     * @param value the float value to convert
     */
    void setFromFloat(float value) {
        bool neg = value < 0.0f;
        uint32_t val = (uint32_t)value;
        uint32_t frc = (value - (float)val) * dpToDivisor(fractionDp);
        setValue(val, frc, neg);
    }

    /**
     * Converts from the BCD packed structure into an integer
     * @param start the index to start in the packed data
     * @param end will stop at one before this point
     * @return the integer value 
     */
	uint32_t fromBcdPacked(int start, int end);

    /**
     * Converts from the BCD packed structure into an integer
     * @param value the value to be encoded
     * @param start the index to start in the packed data
     * @param end will stop at one before this point
     */
	void convertToBcdPacked(int32_t value, int start, int end);

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
    float getAsFloat() {
        float fraction = ((float)getFraction() / (float)dpToDivisor(fractionDp));
		float asFlt = (float)getWhole() + fraction;
		serdebugF3("fract, asFlt ", fraction, asFlt);
		if (negative) asFlt = -asFlt;
		return asFlt;		
    }

    /**
     * @return true if negative otherwise false.
     */
    bool isNegative() { return negative; }

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
        return fromBcdPacked(fractionDp, 12);
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
 * of a rotary encoder or joystick to set their value. This class works by editing each digit in turn.
 */
class EditableLargeNumberMenuItem : public EditableMultiPartMenuItem<LargeFixedNumber> {
public:
	EditableLargeNumberMenuItem(RuntimeRenderingFn renderFn, uint16_t id, int maxDigits, int dps, MenuItem* next = NULL)
		: EditableMultiPartMenuItem(MENUTYPE_LARGENUM_VALUE, id, maxDigits + 1, renderFn, next) {
        data.setPrecision(dps);
	}

    /** gets the large integer value that this class is using */
	LargeFixedNumber* getLargeNumber() { return &data; }  

    /** sets a number from a string in the form whole.fraction */
    void setLargeNumberFromString(const char* largeNum);
};

#endif //_EDITABLE_LARGE_NUMBER_MENU_ITEM_H_