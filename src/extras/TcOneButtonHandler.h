#ifndef TCLIBRARYDEV_TCONEBUTTONHANDLER_H
#define TCLIBRARYDEV_TCONEBUTTONHANDLER_H

#include <PlatformDetermination.h>
#include <SwitchInput.h>
#include <tcMenu.h>

/**
 * Support for a menu with a single button, it will add a button on the required pin, and listen for changes
 * With the following:
 *   Short press: increment by 1
 *   Double press (when not editing): back navigation
 *   Hold down: select
 */
class TcOneButtonHandler : public SwitchListener {
private:
    uint32_t lastPress;
    int doubleClickThreshold;
    pinid_t buttonPin;
public:
    TcOneButtonHandler(pinid_t buttonPin, int doubleClickThreshold)
            : lastPress(0), buttonPin(buttonPin), doubleClickThreshold(doubleClickThreshold) {
    }

    void start() {
        switches.addSwitchListener(buttonPin, this, NO_REPEAT, false);
    }

    void onPressed(pinid_t pin, bool held) override {
    }

    void onReleased(pinid_t pin, bool held) override {
        if(held && pin == buttonPin) {
            menuMgr.onMenuSelect(false);
            lastPress = 0;
        } else {
            if((millis() - lastPress) < doubleClickThreshold) {
                menuMgr.performDirectionMove(true);
                lastPress = 0;
            } else {
                auto enc = switches.getEncoder();
                if(enc->getCurrentReading() == enc->getMaximumValue()) {
                    enc->setCurrentReading(0);
                    enc->runCallback(enc->getCurrentReading());
                } else {
                    enc->increment(1);
                }
                lastPress = millis();
            }
        }
    }
};

#endif //TCLIBRARYDEV_TCONEBUTTONHANDLER_H
