/**
 * An example menu that works on mbed, we explicitly tested this on a STM32F439 device with ethernet capability. However,
 * there should be general applicability to any mbed device with similar features. This uses our OLED library to present
 * the menu, we defaulted to SPI, but you could change easily to i2c.
 *
 * Getting started: https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/tcmenu-overview-quick-start/
 */

#include <mbed.h>
#include "generated/stm32OledEncoder_menu.h"
#include "NTPTimeEvent.h"
#include "ScreenSaverCustomDrawing.h"
#include "mbed/HalStm32EepromAbstraction.h"
#include <stockIcons/wifiAndConnectionIcons16x12.h>
#include <RemoteMenuItem.h>
#include <tcMenuVersion.h>
#include <Adafruit_SSD1306.h>

BufferedSerial serPort(USBTX, USBRX);
MBedLogger LoggingPort(serPort);

// indicates that the process is still active and the run loop should continue.
bool isRunning = true;

// When we created the choices item in the designer, we said create choices in ram, and provided a variable name.
// So we must declare the variable here of type const char*
const char * choicesItems = "xmas\0     easter\0   whitsun\0  mayday\0    ";

// We said the OLED display was on SPI, we must set it up here.
SPI spi(PB_5, PB_4, PB_3);

// This is the actual title widget declaration - see setup
TitleWidget deviceConnectedWidget(iconsConnection, 2, 16, 12, nullptr);
TitleWidget ethernetConnectionWidget(iconsEthernetConnection, 2, 16, 12, &deviceConnectedWidget);
ScreenSaverCustomDrawing screenSaver;

// forward declaration
void prepareRealtimeClock();

void setup() {
    serPort.set_baud(115200);

    // we add a simple widget that appears on the top right of the menu, do this before calling setup to ensure that
    // it draws immediately. There's three things to do here
    // 1. include the stock icons - see includes
    // 2. create one more title widgets (see tcMenu docs for how)
    // 3. add them to the renderer
    renderer.setFirstWidget(&ethernetConnectionWidget);

    // now we have a task that every few seconds checks if we are still connected to the network
    taskManager.scheduleFixedRate(2, [] {
        auto pConnection = remoteServer.getRemoteServerConnection(0);
        if(pConnection) ethernetConnectionWidget.setCurrentState(pConnection->getDeviceInitialisation().isInitialised() ? 1 : 0);
    }, TIME_SECONDS);

    // and register to receive updates from the connector when connectivity changes. When connected is true
    // we have a user connected to our remote endpoint.
    menuIoTMonitor.registerCommsNotification([](CommunicationInfo info) {
        deviceConnectedWidget.setCurrentState(info.connected ? 1 : 0);
    });

    // this was added by designer, it sets up the input, display and remote.
    setupMenu();

    menuMgr.load();

    // and now lets try and acquire time using our quick ntp time class
    prepareRealtimeClock();

    // here we register our screen saver class with the renderer, see ScreenSaverCustomDrawing.h
    renderer.setCustomDrawingHandler(&screenSaver);

    // we add another switch on the user button that just clears the screen saver.
    switches.addSwitch(USER_BUTTON, [](pinid_t /*pin*/, bool /*held*/) {
        screenSaver.removeScreenSaver();
    });

    // When the main title is pressed or touched, we can register a callback to be executed.
    // Here we just present a simple dialog.
    setTitlePressedCallback([](int id) {
        showVersionDialog(&applicationInfo);
    });

    // We set the number of items in our runtime list
    menuCountingList.setNumberOfRows(10);
}

int main() {
    setup();
    while(isRunning) {
        taskManager.runLoop();

        // If there's a large enough gap between tasks, you can add a low power manager as follows below, just ensure
        // that it still wakes up on all the events.
        //auto microsToWait = taskManager.microsToNextTask();
        //myLowPowerLib.sleepWithInterruptCapability(microsToWait);
    }
}

void CALLBACK_FUNCTION onTenthsChaned(int /*id*/) {
    char sz[20];
    ltoaClrBuff(sz, menuTenths.getCurrentValue(), 5, '0', sizeof sz);
    menuEdit.setTextValue(sz);
}

void CALLBACK_FUNCTION onFoodChange(int /*id*/) {
    char sz[20];
    menuFoods.copyEnumStrToBuffer(sz, sizeof  sz, menuFoods.getCurrentValue());
    menuEdit.setTextValue(sz);
}

void CALLBACK_FUNCTION onFrequencyChanged(int /*id*/) {
    char sz[20];
    menuFrequency.copyValue(sz, sizeof sz);
    menuEdit.setTextValue(sz);
}

class NTPTimeMenuSetupEvent : public NTPTimeEvent {
public:

    NTPTimeMenuSetupEvent(NetworkInterface *interface, const char *timeServer, int timePort)
            : NTPTimeEvent(interface, timeServer, timePort) {
    }

    void exec() override {
        serdebugF("Time event has triggered");
        set_time(_presentValue);

        taskManager.scheduleFixedRate(1, [] {
            auto timeNow = time(nullptr);
            auto tm = gmtime(&timeNow);
            menuRTCDate.getUnderlyingData()->year = (tm->tm_year + 1900);
            menuRTCDate.getUnderlyingData()->month = tm->tm_mon + 1;
            menuRTCDate.getUnderlyingData()->day = tm->tm_mday;
            menuRTCTime.getUnderlyingData()->hours = tm->tm_hour;
            menuRTCTime.getUnderlyingData()->minutes = tm->tm_min;
            menuRTCTime.getUnderlyingData()->seconds = tm->tm_sec;
            menuRTCTime.setSendRemoteNeededAll();
            menuRTCDate.setSendRemoteNeededAll();
            menuRTCTime.setChanged(true);
            menuRTCDate.setChanged(true);

        }, TIME_SECONDS);
        setCompleted(true);
    }
};

void prepareRealtimeClock() {

    // make sure a connection was registered before trying to use it
    BaseRemoteServerConnection *pConnection = remoteServer.getRemoteServerConnection(0);

    // and then we
    auto& devInit = reinterpret_cast<const MbedEthernetInitialiser&>(pConnection->getDeviceInitialisation());
    if(pConnection != nullptr && devInit.isInitialised()) {
        SocketAddress addr;
        if(devInit.getInterface()->get_ip_address(&addr) == NSAPI_ERROR_OK) {
            menuIP.setIpAddress(addr.get_ip_address());
            taskManager.registerEvent(new NTPTimeMenuSetupEvent(
                    devInit.getInterface(),
                    "2.pool.ntp.org", 123),  true);
            return;
        }
    }
    taskManager.scheduleOnce(5, prepareRealtimeClock, TIME_SECONDS);
}

void CALLBACK_FUNCTION onSaveAll(int id) {
    menuMgr.save();
    reinterpret_cast<HalStm32EepromAbstraction*>(menuMgr.getEepromAbstraction())->commit();
    auto* dlg = renderer.getDialog();
    if(dlg && !dlg->isInUse()) {
        dlg->setButtons(BTNTYPE_NONE, BTNTYPE_OK);
        dlg->showRam("Saved to ROM", false);
        dlg->copyIntoBuffer("");
    }
}

// This callback needs to be implemented by you, see the below docs:
//  1. List Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
//  2. ScrollChoice Docs - https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
int CALLBACK_FUNCTION fnCountingListRtCall(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize) {
    if(mode == RENDERFN_VALUE && row != LIST_PARENT_ITEM_POS) {
        strncpy(buffer, "Val", bufferSize);
        fastltoa(buffer, row, 3, NOT_PADDED, bufferSize);
        return true;
    }
    return defaultRtListCallback(item, row, mode, buffer, bufferSize);
}
