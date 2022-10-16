#ifndef TCLIBRARYDEV_SPACEJUNKGAME_H
#define TCLIBRARYDEV_SPACEJUNKGAME_H

#include <Arduino.h>
#include "analogJoystick1306Ascii_menu.h"
#include <TaskManagerIO.h>

#define DISPLAY_HEIGHT_ROWS 8
#define DISPLAY_WIDTH_MINUS_ONE 110
#define ANALOG_PIN A0

enum SpaceJunkType: uint8_t {
    ROCK = '*', SATELLITE = '%', SPACE_STATION = '@', OFF_SCREEN = 0
};

class SpaceJunkItem {
    uint8_t xPosition = 0;
    uint8_t yPosition = 0;
    uint8_t speed = 1;
    SpaceJunkType junkType = ROCK;

public:
    void init(uint8_t xpos, uint8_t ypos, SpaceJunkType type, uint8_t sp = 1) {
        Serial.print("junk ");Serial.print(xpos);Serial.print(' ');Serial.print(ypos); Serial.print(' '); Serial.println(type);
        xPosition = xpos;
        yPosition = ypos;
        junkType = type;
        speed = sp;
    }

    bool isShipInRange(uint8_t row) {
        return !isOffscreen() && (xPosition < (speed + 20) && yPosition == row);
    }

    void tick() {
        xPosition = xPosition - speed;
        if(xPosition > DISPLAY_WIDTH_MINUS_ONE) {
            junkType = OFF_SCREEN;
        } else {
            gfx.setCursor(xPosition, yPosition);
            gfx.print((char)junkType);
        }
    }

    bool isOffscreen() { return junkType == OFF_SCREEN; }
};

class SpaceJunkGame {
public:
    enum GameMode { GAME_STARTED, GAME_STOPPED, GAME_OVER };
private:
    SpaceJunkItem items[5];
    uint8_t counter=0;
    GameMode gameStarted = GAME_STARTED;
    int score = 0;
public:
    void start() {
        for(SpaceJunkItem& item : items) {
            item.init(0, 0, OFF_SCREEN);
        }
        score = 0;
        gameStarted = GAME_STARTED;
    }

    void stop() {
        gameStarted = GAME_STOPPED;
        if(score > menuHighScore.getCurrentValue()) {
            menuHighScore.setCurrentValue(score);
        }
    }

    bool isStarted() { return gameStarted == GAME_STARTED || gameStarted == GAME_OVER; }

    SpaceJunkType nextSpaceJunkType() {
        counter = rand()%3;
        switch(counter) {
            case 0: return SPACE_STATION;
            case 1: return SATELLITE;
            default:
                counter = 0;
                return ROCK;
        }
    }
    void tick() {
        if(gameStarted == GAME_STOPPED || gameStarted == GAME_OVER) return;

        gfx.setFont(System5x7);

        Serial.println("Game");
        score = score + 1;
        int level = (score / 50) + 1;
        gfx.clear();

        int row = internalAnalogIo()->getCurrentFloat(ANALOG_PIN) * DISPLAY_HEIGHT_ROWS;
        row = min(row, DISPLAY_HEIGHT_ROWS - 1);
        gfx.setCursor(0, row);
        gfx.print("=>");

        for(SpaceJunkItem& item : items) {
            if(item.isShipInRange(row)) {
                gameStarted = GAME_OVER;
                gfx.setCursor(10, 1);
                gfx.print("Game Over");
                return;
            }
            item.tick();
            if(item.isOffscreen()) {
                item.init(DISPLAY_WIDTH_MINUS_ONE, rand() % DISPLAY_HEIGHT_ROWS, nextSpaceJunkType(), (rand() % 20) + level);
            }
        }
    }
};

#endif //TCLIBRARYDEV_SPACEJUNKGAME_H
