#ifndef TCMENU_ESPAMPLIFIER_AMPLIFIERCONTROLLER_H
#define TCMENU_ESPAMPLIFIER_AMPLIFIERCONTROLLER_H

#include <Arduino.h>
#include <tcMenu.h>
#include <IoLogging.h>
#include "esp32AdaTftTouch_menu.h"

#ifndef MENU_MAGIC_KEY
#define MENU_MAGIC_KEY 0xfade
#endif // MENU_MAGIC_KEY

#ifndef NUM_CHANNELS
#define NUM_CHANNELS 4
#endif // NUM_CHANNELS

//                                            1234567890123456 1234567890123456 1234567890123456 1234567890123456
const char pgmDefaultChannelNames[] PROGMEM = "Turntable\0     Auxiliary\0      USB Audio\0      Storage\0";


class AmplifierController  {
public:
    enum AmplifierStatus {
        WARMING_UP,
        VALVE_WARM_UP,
        RUNNING,
        DC_PROTECTION,
        OVERLOADED,
        OVERHEATED
    };
private:
    AnalogMenuItem* levelTrims[NUM_CHANNELS]{};
    bool audioDirect;

public:
    explicit AmplifierController(AnalogMenuItem* trims[NUM_CHANNELS]) {
        for(int i=0; i<NUM_CHANNELS; i++) {
            levelTrims[i] = trims[i];
        }
    }

    uint8_t getChannelInt() {
        uint8_t ch = menuChannels.getCurrentValue();
        if(ch > NUM_CHANNELS) return 0;
        return ch;
    }

    void setAmpStatus(AmplifierStatus status) {
        menuStatusAmpStatus.setCurrentValue(status);
    }

    AmplifierStatus getAmpStatus() {
        return static_cast<AmplifierStatus>(menuStatusAmpStatus.getCurrentValue());
    }

    void onVolumeChanged() {
        auto trim = levelTrims[getChannelInt()]->getCurrentValue();
        auto vol = menuVolume.getCurrentValue() + trim;
        auto volToWrite = menuMute.getBoolean() ? 0 : vol;
        serdebugF2("write volume to bus", volToWrite);
        onChannelChanged(); // mute going on turns off all channels
    }

    void onChannelChanged() {
        auto ch = (menuMute.getBoolean()) ? 0 : (1 << getChannelInt());
        onVolumeChanged(); // apply the new trim
        serdebugF2("write channel to bus", ch);
    }

    void onAudioDirect(bool direct) {
        audioDirect = direct;
    }
};

#endif //TCMENU_ESPAMPLIFIER_AMPLIFIERCONTROLLER_H
