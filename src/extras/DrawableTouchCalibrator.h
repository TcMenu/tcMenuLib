/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file DrawableTouchCalibrator.h
 * @brief A simple helper for touch calibration implemented as a TcMenu CustomDrawing instance
 */

#ifndef TCMENU_DRAWABLETOUCHCALIBRATOR_H
#define TCMENU_DRAWABLETOUCHCALIBRATOR_H

#include <PlatformDetermination.h>
#include "../graphics/GraphicsDeviceRenderer.h"
#include "../graphics/MenuTouchScreenEncoder.h"

namespace tcextras {

    /**
     * The calibration manager handles the loading, saving, and presentation if needed of a calibration UI. It is
     * generally created by touch plugins to handle all calibration activities.
     */
    class TouchCalibrationManager {
    public:
        virtual bool loadCalibration() = 0;
        virtual void saveCalibration() = 0;
        virtual void reCalibrateNow() = 0;
    };

    /**
     * This method should be provided to the `IoaTouchScreenCalibrator` class below during initialisation in order to
     * ensure that
     */
    typedef void (*IoaCalibrationScreenPrep)(bool starting);

    /**
     * This class implements custom drawing so that it can take over the display and render a basic touch screen
     * calibration UI. It depends upon the `IoaCalibrationScreenPrep` function that is called during screen take over
     * to adjust the display and touch rotation back to the default non-rotated settings for the panel. It is assumed
     * that with this the range of adjustments needed can be determined. It is assumed that during this call there is
     * no difference between touch and display rotation.
     */
    class IoaTouchScreenCalibrator : public CustomDrawing, TouchCalibrationManager {
    private:
        const uint16_t calibrationMagicIoa = 0xd00d;
        tcgfx::DeviceDrawable *drawable;
        tcgfx::MenuTouchScreenManager *touchScreen;
        tcgfx::GraphicsDeviceRenderer* renderer;
        CustomDrawing* lastDrawing = nullptr;
        uint16_t romPos;
        int oldX = 0, oldY = 0;
        iotouch::CalibrationHandler calibrationHandler {};
        IoaCalibrationScreenPrep screenPrep = nullptr;
        bool needsRedrawing = false;

    public:
        explicit IoaTouchScreenCalibrator(tcgfx::MenuTouchScreenManager *touchScreen, tcgfx::GraphicsDeviceRenderer* renderer, uint16_t romPos)
                : drawable(nullptr), touchScreen(touchScreen), renderer(renderer), romPos(romPos) {
        }

        /**
         * During setup you'd normally call this function to initialise the touch calibrator with the previous settings,
         * and also to provide the calibrator with a function that can remove all rotations while calibrating, and put
         * them back after calibration is finished. This is because it is easier to calibrate with no rotation applied.
         * @param screenPrepFn a function to remove and restore rotations, bool param of true - starting calibration
         * @param presentUiUninitialised true if the the UI should show when EEPROM load fails.
         */
        void initCalibration(IoaCalibrationScreenPrep screenPrepFn, bool presentUiUninitialised) {
            this->screenPrep = screenPrepFn;
            if(!loadCalibration() && presentUiUninitialised) {
                reCalibrateNow();
            }
        }

        /**
         * Loads the calibration data back from EEPROM
         * @return true if loaded, otherwise false
         */
        bool loadCalibration() override;

        /**
         * Saves the calibration data to EEPROM
         */
        void saveCalibration() override;

        /**
         * Forces recalibration immediately.
         */
        void reCalibrateNow() override;
                
        void reset() override {}
        void started(BaseMenuRenderer *currentRenderer) override;
        void renderLoop(unsigned int currentValue, RenderPressMode userClick) override;

        void giveItBack();
    };
} // namespace tcextras

#endif // TCMENU_DRAWABLETOUCHCALIBRATOR_H
