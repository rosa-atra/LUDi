#include "android_controls.h"
#include <math.h>

#if defined(PLATFORM_ANDROID)
#include <android/looper.h>
#include <android/sensor.h>

static ALooper *accelerometerLooper = NULL;
static ASensorEventQueue *accelerometerQueue = NULL;
static volatile float accelerometerX = 0.0f;
static float controlRadius = 0.0f;
static Vector2 fireButtonCenter = { 0.0f, 0.0f };
static bool fireWasPressed = false;
static bool speedHold = false;
static bool boostHold = false;
static bool doubleTapCandidate = false;
static float touchHoldDuration = 0.0f;
static float lastTapReleaseTime = 0.0f;
static float touchControlTime = 0.0f;

static void InitAccelerometer(void)
{
    ASensorManager *sensorManager = ASensorManager_getInstance();
    if (sensorManager == NULL) return;

    const ASensor *accelerometer = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    if (accelerometer == NULL) return;

    accelerometerLooper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    accelerometerQueue = ASensorManager_createEventQueue(sensorManager, accelerometerLooper, 1, NULL, NULL);
    if (accelerometerQueue == NULL) return;

    ASensorEventQueue_setEventRate(accelerometerQueue, accelerometer, 10000);
    ASensorEventQueue_enableSensor(accelerometerQueue, accelerometer);
}

static void UpdateAccelerometer(void)
{
    if (accelerometerLooper == NULL || accelerometerQueue == NULL) return;

    int ident = 0;
    while (ALooper_pollOnce(0, &ident, NULL, NULL) >= 0) {
        if (ident != 1) continue;
        ASensorEvent events[8];
        ssize_t count = ASensorEventQueue_getEvents(accelerometerQueue, events, 8);
        for (ssize_t i = 0; i < count; i++) {
            if (events[i].type == ASENSOR_TYPE_ACCELEROMETER) accelerometerX = events[i].data[0];
        }
    }
}
#endif

void AndroidControlsInit(AndroidControls *controls, int screenWidth, int screenHeight)
{
#if defined(PLATFORM_ANDROID)
    controls->available = true;
    controls->lastTouchPosition = (Vector2){ -1.0f, -1.0f };
    controls->touchPrevious = (Vector2){ 0.0f, 0.0f };
    controlRadius = fminf((float)screenWidth, (float)screenHeight) * 0.15f;
    if (controlRadius < 80.0f) controlRadius = 80.0f;
    if (controlRadius > 120.0f) controlRadius = 120.0f;
    fireButtonCenter = (Vector2){ screenWidth - controlRadius * 1.65f, screenHeight * 0.67f };
    InitAccelerometer();
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
    UpdateAccelerometer();
    controls->touchDown = false;
    controls->touchControlDown = false;
    controls->firePressed = false;

    int touchCount = GetTouchPointCount();
    controls->touchDown = touchCount > 0;
    for (int i = 0; i < touchCount; i++) {
        Vector2 position = GetTouchPosition(i);
        controls->lastTouchPosition = position;
        float dx = position.x - fireButtonCenter.x;
        float dy = position.y - fireButtonCenter.y;
        if (dx * dx + dy * dy <= controlRadius * controlRadius) controls->firePressed = true;
        else controls->touchControlDown = true;
    }

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

float AndroidControlsTilt(void)
{
#if defined(PLATFORM_ANDROID)
    return accelerometerX / 9.81f;
#else
    return 0.0f;
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
