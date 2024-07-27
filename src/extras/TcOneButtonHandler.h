#ifndef TCLIBRARYDEV_TCONEBUTTONHANDLER_H
#define TCLIBRARYDEV_TCONEBUTTONHANDLER_H

#include <PlatformDetermination.h>
#include <SwitchInput.h>
#include <tcMenu.h>

/**
 * Support for a menu with a single button, it will add a button on the required pin, and listen for changes
 * With the following:
 *   Short press: increment by 1
 *   Double press (when not editing): select item
 *   Hold down: go back navigation
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
            menuMgr.performDirectionMove(true);
            lastPress = 0;
        } else {
            if((millis() - lastPress) < doubleClickThreshold) {
                menuMgr.onMenuSelect(held);
                lastPress = 0;
            } else {
                switches.getEncoder()->increment(1);
                lastPress = millis();
            }
        }
    }
};

#endif //TCLIBRARYDEV_TCONEBUTTONHANDLER_H
