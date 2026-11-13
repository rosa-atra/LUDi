#ifndef LUDI_ANDROID_CONTROLS_H
#define LUDI_ANDROID_CONTROLS_H

#include <stdbool.h>
#include <raylib.h>

typedef struct AndroidControls {
    bool available;
    bool touchDown;
    bool touchWasDown;
    bool touchControlDown;
    bool touchControlWasDown;
    bool firePressed;
    bool fireTapped;
    bool speedUp;
    bool boost;
    float moveX;
    Vector2 lastTouchPosition;
    Vector2 touchPrevious;
    Vector2 movementTouchPrevious;
    bool movementTouchWasDown;
} AndroidControls;

void AndroidControlsInit(AndroidControls *controls, int screenWidth, int screenHeight);
void AndroidControlsUpdate(AndroidControls *controls, float dt, bool gamePaused, Rectangle quitButton);
void AndroidControlsDraw(const AndroidControls *controls, bool gamePaused);
bool AndroidControlsBackPressed(void);

#endif
