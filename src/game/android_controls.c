#include "android_controls.h"
#include <math.h>

#if defined(PLATFORM_ANDROID)
static float controlRadius = 0.0f;
static Vector2 fireButtonCenter = { 0.0f, 0.0f };
static bool fireWasPressed = false;
static bool speedHold = false;
static bool boostHold = false;
static bool doubleTapCandidate = false;
static float touchHoldDuration = 0.0f;
static float lastTapReleaseTime = 0.0f;
static float touchControlTime = 0.0f;
#endif

void AndroidControlsInit(AndroidControls *controls, int screenWidth, int screenHeight)
{
#if defined(PLATFORM_ANDROID)
    controls->available = true;
    controls->lastTouchPosition = (Vector2){ -1.0f, -1.0f };
    controls->touchPrevious = (Vector2){ 0.0f, 0.0f };
    controls->movementTouchPrevious = (Vector2){ 0.0f, 0.0f };
    controlRadius = fminf((float)screenWidth, (float)screenHeight) * 0.15f;
    if (controlRadius < 80.0f) controlRadius = 80.0f;
    if (controlRadius > 120.0f) controlRadius = 120.0f;
    fireButtonCenter = (Vector2){ screenWidth - controlRadius * 1.65f, screenHeight * 0.67f };
#else
    (void)screenWidth;
    (void)screenHeight;
    controls->available = false;
#endif
}

void AndroidControlsUpdate(AndroidControls *controls, float dt, bool gamePaused, Rectangle quitButton)
{
#if defined(PLATFORM_ANDROID)
    (void)quitButton;
    controls->touchDown = false;
    controls->touchControlDown = false;
    controls->firePressed = false;
    controls->moveX = 0.0f;
    Vector2 movementTouchPosition = { 0.0f, 0.0f };
    bool movementTouchDown = false;

    int touchCount = GetTouchPointCount();
    controls->touchDown = touchCount > 0;
    for (int i = 0; i < touchCount; i++) {
        Vector2 position = GetTouchPosition(i);
        controls->lastTouchPosition = position;
        float dx = position.x - fireButtonCenter.x;
        float dy = position.y - fireButtonCenter.y;
        if (dx * dx + dy * dy <= controlRadius * controlRadius) controls->firePressed = true;
        else {
            controls->touchControlDown = true;
            if (!movementTouchDown) {
                movementTouchPosition = position;
                movementTouchDown = true;
            }
        }
    }

    if (!gamePaused && movementTouchDown && controls->movementTouchWasDown) {
        controls->moveX = movementTouchPosition.x - controls->movementTouchPrevious.x;
        if (controls->moveX > 80.0f) controls->moveX = 80.0f;
        if (controls->moveX < -80.0f) controls->moveX = -80.0f;
    }
    if (movementTouchDown) controls->movementTouchPrevious = movementTouchPosition;
    controls->movementTouchWasDown = movementTouchDown;

    controls->fireTapped = controls->firePressed && !fireWasPressed;
    fireWasPressed = controls->firePressed;

    touchControlTime += dt;
    if (controls->touchControlDown && !controls->touchControlWasDown) {
        touchHoldDuration = 0.0f;
        if (doubleTapCandidate && touchControlTime - lastTapReleaseTime <= 0.35f) {
            boostHold = true;
            doubleTapCandidate = false;
        }
    } else if (controls->touchControlDown) {
        touchHoldDuration += dt;
    }

    if (controls->touchControlDown && boostHold) {
        controls->boost = !gamePaused;
        controls->speedUp = false;
        speedHold = false;
    } else if (controls->touchControlDown && touchHoldDuration >= 0.16f) {
        speedHold = true;
        controls->speedUp = !gamePaused;
        controls->boost = false;
    } else {
        speedHold = false;
        controls->speedUp = false;
        controls->boost = false;
        boostHold = false;
    }

    if (!controls->touchControlDown && controls->touchControlWasDown) {
        if (!boostHold && !speedHold && touchHoldDuration < 0.16f) {
            doubleTapCandidate = true;
            lastTapReleaseTime = touchControlTime;
        }
        boostHold = false;
    }
    if (doubleTapCandidate && touchControlTime - lastTapReleaseTime > 0.35f) doubleTapCandidate = false;

    if (controls->touchDown) controls->touchPrevious = controls->lastTouchPosition;
    controls->touchControlWasDown = controls->touchControlDown;
    controls->touchWasDown = controls->touchDown;
#else
    (void)controls;
    (void)dt;
    (void)gamePaused;
    (void)quitButton;
    controls->moveX = 0.0f;
#endif
}

void AndroidControlsDraw(const AndroidControls *controls, bool gamePaused)
{
#if defined(PLATFORM_ANDROID)
    if (controls->available && !gamePaused) {
        DrawCircleV(fireButtonCenter, controlRadius,
            Fade(controls->firePressed ? ORANGE : MAROON, 0.75f));
        DrawCircleLines((int)fireButtonCenter.x, (int)fireButtonCenter.y,
            controlRadius, Fade(WHITE, 0.85f));
        const char *fireLabel = "FIRE";
        int labelWidth = MeasureText(fireLabel, 24);
        DrawText(fireLabel, (int)(fireButtonCenter.x - labelWidth * 0.5f),
            (int)(fireButtonCenter.y - 12), 24, WHITE);
    }
#else
    (void)controls;
    (void)gamePaused;
#endif
}

bool AndroidControlsBackPressed(void)
{
#if defined(PLATFORM_ANDROID)
    return IsKeyPressed(KEY_BACK);
#else
    return false;
#endif
}
