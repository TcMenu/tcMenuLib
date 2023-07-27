/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_TWOBUTTONSWITCHCONTROL_H
#define TCMENU_TWOBUTTONSWITCHCONTROL_H

/**
 * @file TwoButtonSwitchControl.h
 * @brief contains a class that can be used to set up basic two button control of a menu
 */

#include <tcMenu.h>
#include <IoAbstraction.h>

/**
 * Use this class to control a menu application with only two buttons. The up/down buttons double up as back and
 * select when held. This means that repeat operation is not possible, keep integer and choice value ranges small
 * so as to avoid needing many presses.
 */
class TwoButtonSwitchEncoder : public RotaryEncoder, public SwitchListener {
private:
    pinid_t upPin;
    pinid_t downPin;

public:
    /**
     * Create a two button switch given an up and down button along with the callback function for the encoder.
     * @param up the up button (doubles as back when held)
     * @param down the down button (doubles as OK when held)
     * @param callbackFn the encoder change callback.
     */
    TwoButtonSwitchEncoder(pinid_t up, pinid_t down, EncoderCallbackFn callbackFn);

    void onPressed(pinid_t pin, bool held) override;

    void onReleased(pinid_t pin, bool held) override;

};

#endif //TCMENU_TWOBUTTONSWITCHCONTROL_H
