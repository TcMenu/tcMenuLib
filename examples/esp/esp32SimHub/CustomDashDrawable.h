//#define LED_STATES 10
//    uint16_t ledColors[LED_STATES];
//    bool ledsChanged = true;
//    bool wantLeds;
//
// setup
//        ledsChanged = running = false;
//        wantLeds = drawLeds;
//        for (uint16_t &ledState : ledColors) ledState = 0;
//
//
//    void setLed(int i, uint16_t color) {
//        if (i < 0 || i > LED_STATES) return;
//        ledColors[i] = color;
//        ledsChanged = wantLeds;
//    }
//
//     /**
//     * Redraw the LEDs if they have changed
//     */
//    void drawLeds() {
//        ledsChanged = false;
//        int widthOfOneLed = myGfx->width() / LED_STATES;
//
//        int offsetX = widthOfOneLed / 2;
//        for (uint16_t ledState : ledColors) {
//            myGfx->fillCircle(offsetX, 12, 11, ledState);
//            offsetX += widthOfOneLed;
//        }
//    }