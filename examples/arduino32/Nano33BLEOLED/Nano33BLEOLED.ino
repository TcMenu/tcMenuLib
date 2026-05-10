/**
* @file Nano33BLEOLED.ino
 * An example of using tcMenu with the Nano33BLE Sense. This example uses some of the sensors on-board the device
 * along with an I2C LCD backpack based display and a rotary encoder. Although this is pre-generated for you, you
 * can load the example's emf file into TcMenu Designer and take a look around or re-build it.
 *
 * You can get or adjust the pin configurations by loading the emf file into the designer
 * Web designer: https://designer.thecoderscorner.com
 */

// include the menu project file, it contains all the glue code to get things working
#include "Nano33BLEOLED_menu.h"
// and this example uses the inbuilt sensors on the board, there are two classes that access them.
#include "SensorManager.h"
#include "MotionDetection.h"
// and the BLE library
#include <ArduinoBLE.h>
// and stock icons that include things like Wi-Fi and connectivity icons.
#include "stockIcons/wifiAndConnectionIcons16x12.h"

// on the analog menu, we both have an analog input and an analog output (PWM). Change as appropriate.
constexpr int analogInputPin = A0;
constexpr int pwmOutputPin = 2;

// Title widgets
TitleWidget bleRssiWidget(iconsWifi, 5, 16, 12, nullptr);

// We create a class extending Executable for the temperature, humidity, and pressure sensors that are built in
SensorManager sensorManager;

// We create an event class extending BaseEvent to manage the motion detection
MotionDetection motionDetection;

// This is the menu structure, you can adjust this as needed yourself, no need to round-trip to designer,
// unless of course you prefer editing menus there.
void buildMenu(TcMenuBuilder& builder) {
    builder.usingDynamicEEPROMStorage()
        .analogBuilder(MENU_TEMP_ID, "Temp", DONT_SAVE, MenuFlags().readOnly(), 0, nullptr)
            .offset(0).divisor(10).step(1).maxValue(2000).unit("C").endItem()
        .analogBuilder(MENU_HUMIDITY_ID, "Humidity", DONT_SAVE, MenuFlags().readOnly(), 0, nullptr)
            .offset(0).divisor(10).step(1).maxValue(1000).unit("%").endItem()
        .analogBuilder(MENU_B_PRESSURE_ID, "B. Pressure", DONT_SAVE, MenuFlags().readOnly(), 0, nullptr)
            .offset(0).divisor(10).step(1).maxValue(32000).unit("KPa").endItem()
        .subMenu(MENU_ACCELEROMETER_ID, "Accelerometer", NoMenuFlags, nullptr)
            .floatItem(MENU_MAG_X_ID, "MagX", DONT_SAVE, 1, NoMenuFlags, 0.0, nullptr)
            .floatItem(MENU_MAG_Y_ID, "MagY", DONT_SAVE, 1, NoMenuFlags, 0.0, nullptr)
            .floatItem(MENU_MAG_Z_ID, "MagZ", DONT_SAVE, 1, NoMenuFlags, 0.0, nullptr)
            .floatItem(MENU_ACCEL_X_ID, "AccelX", DONT_SAVE, 1, NoMenuFlags, 0.0, nullptr)
            .floatItem(MENU_ACCEL_Y_ID, "AccelY", DONT_SAVE, 1, NoMenuFlags, 0.0, nullptr)
            .floatItem(MENU_ACCEL_Z_ID, "AccelZ", DONT_SAVE, 1, NoMenuFlags, 0.0, nullptr)
            .endSub()
        .subMenu(MENU_ANALOG_READINGS_ID, "Analog Readings", NoMenuFlags, nullptr)
            .floatItem(MENU_IN_A0_ID, "In A0", DONT_SAVE, 1, NoMenuFlags, 0.0, nullptr)
            .analogBuilder(MENU_OUTPUT_P_W_M_ID, "Output PWM", DONT_SAVE, NoMenuFlags, 0, onPWMChanged)
                .offset(0).divisor(0).step(1).maxValue(100).unit("%").endItem()
            .endSub();
}

void setup() {
    // start up serial and wait for it to actually begin, needed on this board.
    Serial.begin(115200);
    while(!Serial && millis() < 10000) {}

    // start up wire as well.
    Wire.begin();
    Wire.setClock(400000);

    setupMenu();

    // First we set up the analog pins, we configure a PWM output pin onto which should be connected
    // an LED, and we take readings from the analogInputPin. These are configured further up.
    internalAnalogDevice().initPin(pwmOutputPin, DIR_OUT);
    internalAnalogDevice().initPin(analogInputPin, DIR_IN);

    // Here we tell the encoder not to wrap (we don't technically need to do this as false is the default.
    // Wrap means go from maxValue back to 0, or from 0 back to maxValue. On is true, Off (default) is false.
    menuMgr.setUseWrapAroundEncoder(false);
    // We can also define overrides for a particular menu item
    menuMgr.addEncoderWrapOverride(getMenuOutputPWM(), true);

    // add a title widget that represents the ble signal strength / connection
    // and create a task that updates its status each second.
    renderer.setFirstWidget(&bleRssiWidget);
    taskManager.schedule(repeatSeconds(1), [] {
        if(!BLE.connected()) bleRssiWidget.setCurrentState(0);
        else if(BLE.rssi() > 80) bleRssiWidget.setCurrentState(1);
        else if(BLE.rssi() > 65) bleRssiWidget.setCurrentState(2);
        else if(BLE.rssi() > 55) bleRssiWidget.setCurrentState(3);
        else bleRssiWidget.setCurrentState(4);
    });

    // then we initialise our sensor and motion detection and register with task manager.
    sensorManager.initialise();
    motionDetection.initialise();

    // see the task manager documentation: https://www.thecoderscorner.com/products/arduino-libraries/taskmanager-io/
    taskManager.registerEvent(&motionDetection);
    taskManager.scheduleFixedRate(1, &sensorManager, TIME_SECONDS);

    // we can register call back that is called when the main menu title is pressed, in this case we just show
    // the tcMenu version number in a dialog.
    // https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/rendering-with-tcmenu-lcd-tft-oled/#presenting-a-dialog-to-the-user
    setTitlePressedCallback([](int id) {
        showVersionDialog(&applicationInfo);
    });

    // lastly we set up something simple to read from analog in
    taskManager.scheduleFixedRate(100, [] {
       getMenuInA0().setFloatValue(internalAnalogDevice().getCurrentFloat(analogInputPin));
    });
}

void loop() {
    taskManager.runLoop();

}


void CALLBACK_FUNCTION onPWMChanged(int id) {
    // here we are notified of changes in the PWM menu item and we convert that change to a value between 0 and 1.
    auto newPwm = getMenuOutputPWM().getCurrentValue() / 100.0F;
    // then we can apply that to the output pin, analogDevice does all the conversion work for us.
    internalAnalogDevice().setCurrentFloat(pwmOutputPin, newPwm);
}
