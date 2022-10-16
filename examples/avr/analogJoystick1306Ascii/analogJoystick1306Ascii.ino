/**
 * Space Junk, the game for Uno:)
 *
 * This game shows the use of the SSD1306ASCII library written by greiman, it combines that with the analog joystick
 * support built into tcMenu.
 *
 * Hardware:
 * * I2C OLED display that is suitable for use with SSD1306ASCII
 * * Analog Joystick with X and Y axis (A0-Y axis, A1-X axis, A2-button).
 */

#include "analogJoystick1306Ascii_menu.h"
// default CPP main file for sketch
#include <PlatformDetermination.h>
#include "SpaceJunkGame.h"
#include <TaskManagerIO.h>

void setup() {
    Serial.begin(115200);
    setupMenu();

    // turn off the reset behaviour, no going back to main during the game!
    renderer.turnOffResetLogic();

    menuMgr.load();
}

void loop() {
    taskManager.runLoop();
}

bool instructionsStarting;
void instructionsRenderer(unsigned int value, RenderPressMode mode) {
    if(instructionsStarting) {
        gfx.clear();
        gfx.setFont(System5x7);
        gfx.println("Space Junk");
        gfx.println("Dodge junk!");
        gfx.println("in space");
        gfx.println("Press OK!");
        instructionsStarting = false;
    } else if(mode == RPRESS_PRESSED) {
        renderer.giveBackDisplay();
    }
}

void CALLBACK_FUNCTION onInstructions(int id) {
    instructionsStarting = true;
    renderer.takeOverDisplay(instructionsRenderer);
}

SpaceJunkGame game;
void gameRenderer(unsigned int value, RenderPressMode mode) {
    if(!game.isStarted()) {
        game.start();
    }

    if(mode == RPRESS_PRESSED) {
        game.stop();
        renderer.giveBackDisplay();
    } else {
        game.tick();
    }
}

void CALLBACK_FUNCTION onStartGame(int id) {
    renderer.takeOverDisplay(gameRenderer);
}

const char pgmSaved[] PROGMEM = "Saved to ROM";
void CALLBACK_FUNCTION onSave(int id) {
    menuMgr.save();
    auto dlg = renderer.getDialog();
    if(dlg && !dlg->isInUse()) {
        dlg->setButtons(BTNTYPE_CLOSE, BTNTYPE_NONE);
        dlg->show(pgmSaved, true);
        dlg->copyIntoBuffer("Settings");
    }
}
