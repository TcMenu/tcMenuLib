
#include "TwoButtonSwitchEncoder.h"

TwoButtonSwitchEncoder::TwoButtonSwitchEncoder(pinid_t up, pinid_t down, EncoderCallbackFn callbackFn)
            : RotaryEncoder(callbackFn), upPin(up), downPin(down) {
    switches.addSwitchListener(up, this, NO_REPEAT);
    switches.addSwitchListener(down, this, NO_REPEAT);
}

void TwoButtonSwitchEncoder::onReleased(pinid_t pin, bool held) {
    auto invert = intent == SCROLL_THROUGH_ITEMS;
    if(pin == upPin) {
        if(held) {
            menuMgr.performDirectionMove(true);
        } else {
            int8_t dir = invert ? -stepSize : stepSize;
            increment(dir);
        }
    } else if(pin == downPin) {
        if(held) {
            menuMgr.onMenuSelect(false);
        } else {
            int8_t dir = invert ? stepSize : -stepSize;
            increment(dir);
        }
    }
}

void TwoButtonSwitchEncoder::onPressed(pinid_t pin, bool held) {
    // ignored for two button case
}
