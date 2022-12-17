/**
 * This example shows how to use the simhub connector to produce a realistic steering wheel simulation.
 * In this case we use a SH1106 display and a Jostick encoder to control the menu, but using the tcMenu
 * Designer you could change the display and input to whatever was more suitable for your design.
 *
 * We provide an example of how to make a custom dashboard view similiar to those found on custom
 * racing steering wheels. The view comes into play when the menu has been left inactive for more
 * than around 30 seconds. You can change this by configuring the renderer's reset interval.
 *
 * Quick instructions (see the link below for more detailed instructions).
 *
 * * Enable the custom serial protocol from SimHub settings
 * * In Custom Serial, configure the serial port and baud to match your settings.
 * * In the "after connection established" enter exactly: 'simhubStart\n'
 * * In the "before disconnect" enter exactly: 'simhubEnd\n'
 *
 * We now need to map each menu item ID to a SimHub statistic:
 *
 * * Add a new Update entry and enter: '1=' + [RPM] + '\n'
 * * Add a new Update entry and enter: '2=' + [MPH] + '\n'
 * * Add a new Update entry and enter: '4=' + [Gear] + '\n'
 *
 * See the Simhub guide for full instructions:
 * https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-plugins/simhub-connector/
 *
 * Connections by default:
 * Encoder A=36, B=37, Sel=21
 * Touch Up=5, Down=7, Sel=6
 *
 * Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/
 */

#include <Adafruit_ILI9341.h>
#include "esp32SimHub_menu.h"
#include <Wire.h>
#include <IoLogging.h>
#include <AnalogDeviceAbstraction.h>
#include <extras/DrawableDashboard.h>
#include <tcMenuVersion.h>
#include "dashboardSetup.h"


// here we pulsate an LED using the ESP32's DAC
const int dacPin = 32;
float ledLevel = 0.0;
float ledAdjustment = 0.01;

const int lcdBacklightPin = 5;

// we need access to the analog device of the ESP32
HardwareRotaryEncoder* secondEncoder;

void setup() {
    SPI.begin(18, 19, 23);
    SPI.setFrequency(20000000);

    // we start the serial bus at 115200 so it's ready for our connector to use
    Serial.begin(115200);


    // initialise the menu library, the following line was automatically added to setup by tcMenu.
    setupMenu();

    // Here we register a custom drawing class, it will handle the reset events and also any
    // events to do with taking over the display.
    setupDashboard();
    renderer.takeOverDisplay();

    menuGear.setTextValue("N");

    //menuSettingsNewLargeNumber.setLargeNumberFromString("12.1");
    auto largeNum = menuSettingsNewLargeNumber.getLargeNumber();
    //largeNum->setFromFloat(12.1f);
    largeNum->setValue(12, 1, false);
    //menuSettingsNewLargeNumber.setLargeNumberFromString("12.1");
    menuSettingsTestItem1.setFromFloatingPointValue(21.5F);

    internalAnalogDevice().initPin(dacPin, DIR_OUT);
    taskManager.scheduleFixedRate(10, [] {
        ledLevel += ledAdjustment;
        if(ledLevel > 0.98) ledAdjustment = -0.01;
        if(ledLevel < 0.01) ledAdjustment = 0.01;
        internalAnalogDevice().setCurrentFloat(dacPin, ledLevel);
    });

    setTitlePressedCallback([](int id) {
        withMenuDialogIfAvailable([](MenuBasedDialog* dlg) {
            dlg->setButtons(BTNTYPE_OK, BTNTYPE_NONE);
            dlg->show(applicationInfo.name, true);
            char szVersion[10];
            tccore::copyTcMenuVersion(szVersion, sizeof szVersion);
            dlg->copyIntoBuffer(szVersion);
        });
    });

    taskManager.scheduleFixedRate(250, [] {
        for(int i=0;i<LED_STATES;i++) {
            dashboardDelegate.setLed(i, rand());
        }
    });

    //
    // if you want to test your rendering without simhub connected, uncomment the below code, it will update all the
    // values a few times a second.
    //
    /*taskManager.scheduleFixedRate(3000, [] {
        menuTyreTemp.setCurrentValue(random(50) + 40);
        auto gear = random(10);
        if(gear == 0) menuGear.setTextValue("N");
        else if(gear > 8) menuGear.setTextValue("R");
        else {
            char sz[2];
            sz[0] = gear + '0';
            sz[1] = 0;
            menuGear.setTextValue(sz);
        }
    });

    taskManager.scheduleFixedRate(250, [] {
        int rpm = menuRPM.getCurrentValue();
        int speed = menuSpeed.getCurrentValue();

        speed = min(250, max(5, speed + (rand() % 4) - 1));
        rpm = min(16000, max(1500, rpm + (rand() % 100) - 25));
        menuRPM.setCurrentValue(rpm);
        menuSpeed.setCurrentValue(speed);

        int rpm = menuRPM.getCurrentValue();
        uint16_t color;
        if(rpm < 6500) {
            color = (rpm < 5000) ? ILI9341_BLACK : (rpm < 6000) ? ILI9341_GREEN : ILI9341_RED;
        }
        for (int i = 0; i < LED_STATES; i++) {
            dashCustomDrawing->setLed(i, color);
        }
    });*/
    // end manually adjusting values
}

// All IoAbstraction and TcMenu sketches need the runLoop to be called very frequently, preferably in the loop
// method with nothing else in the loop that can cause delays.
void loop() {
    taskManager.runLoop();
}

void CALLBACK_FUNCTION onConnectionChange(int id) {
    // we registered a call back on the simhub link menu item
    // this is the menu item that gets set/cleared by simhub.
    // you can take custom action on it here if needed.
    serdebugF2("Simhub connection", menuSimHubLink.getBoolean());
}

void CALLBACK_FUNCTION onDashChanged(int id) {
    // if you had more than one dash, here you could switch between them, in a similar way to setupDashboard.
}

void CALLBACK_FUNCTION onShowDash(int id) {
    renderer.takeOverDisplay();
}

const char dlgHeaderPgm[] PROGMEM = "Dialog Header Pgm";

class MyDialogController : public BaseDialogController, public Executable {
private:
    MenuBasedDialog* dialog;
    taskid_t repeatingTaskId;
public:
    void starting(MenuBasedDialog* dlg) {
        dialog = dlg;
        repeatingTaskId = taskManager.scheduleFixedRate(200, this);
    }

    void exec() override {
        char sz[20];
        strcpy(sz,"rand=");
        fastltoa(sz, rand() % 1000, 3, '0', sizeof sz);
        dialog->copyIntoBuffer(sz);
    }

    void initialiseAndGetHeader(BaseDialog* dlg, char* buffer, size_t bufferSize) override {
        strcpy(buffer, "OO header");
    }

    void dialogDismissed(ButtonType buttonType) override {
        taskManager.cancelTask(repeatingTaskId);
        if(buttonType == BTNTYPE_OK) {
            dialog->setButtons(BTNTYPE_NONE, BTNTYPE_CANCEL);
            dialog->show(dlgHeaderPgm, false, nullptr);
            dialog->copyIntoBuffer("hello world");
        }
    }

    bool dialogButtonPressed(int buttonNum) override {
        serdebugF2("dialog button press ", buttonNum);
        return true;
    }

    void copyCustomButtonText(int buttonNumber, char *buffer, size_t bufferSize) override {
        switch (buttonNumber) {
            case SECOND_DEFAULT_BUTTON:
                strncpy(buffer, "Next", bufferSize);
                break;
            default:
                buffer[0]=0;
                break;
        }
    }
} dialogController;

void CALLBACK_FUNCTION onShowDialogs(int id) {
    auto* dlg = renderer.getDialog();
    if(dlg && !dlg->isInUse()) {
        dlg->setButtons(BTNTYPE_OK, BTNTYPE_CUSTOM0);
        dialogController.starting((MenuBasedDialog*)dlg);
        dlg->showController(false, &dialogController);
        dlg->copyIntoBuffer("text 1");
    }
}
