#ifndef LUDI_ANDROID_DISPLAY_H
#define LUDI_ANDROID_DISPLAY_H

#include <raylib.h>

#define LUDI_DESIGN_WIDTH 800
#define LUDI_DESIGN_HEIGHT 600

typedef struct {
    int width;
    int height;
    float scaleX;
    float scaleY;
    float scale;
    float offsetX;
    float offsetY;
    int quitSize;
} AndroidDisplayConfig;

static inline AndroidDisplayConfig GetGameDisplayConfig(void)
{
    AndroidDisplayConfig config;

    /* Raylib reports the current NativeActivity/monitor resolution.
       Do not cache a device-specific width or height here. */
    config.width = GetScreenWidth();
    config.height = GetScreenHeight();

#if !defined(PLATFORM_ANDROID)
    if (config.width <= 0) config.width = LUDI_DESIGN_WIDTH;
    if (config.height <= 0) config.height = LUDI_DESIGN_HEIGHT;
#else
    if ((config.width <= 0) || (config.height <= 0)) {
        config.width = GetMonitorWidth(GetCurrentMonitor());
        config.height = GetMonitorHeight(GetCurrentMonitor());
    }
#endif

    if (config.width <= 0) config.width = 1;
    if (config.height <= 0) config.height = 1;

    config.scaleX = (float)config.width / LUDI_DESIGN_WIDTH;
    config.scaleY = (float)config.height / LUDI_DESIGN_HEIGHT;
    config.scale = (config.scaleX < config.scaleY) ? config.scaleX : config.scaleY;
    config.offsetX = (config.width - (LUDI_DESIGN_WIDTH * config.scale))*0.5f;
    config.offsetY = (config.height - (LUDI_DESIGN_HEIGHT * config.scale))*0.5f;
    config.quitSize = (int)(64.0f * config.scale);
    if (config.quitSize < 48) config.quitSize = 48;

    return config;
}

static inline float ScaleGameX(float x)
{
    AndroidDisplayConfig config = GetGameDisplayConfig();
    return config.offsetX + (x * config.scale);
}

static inline float ScaleGameY(float y)
{
    AndroidDisplayConfig config = GetGameDisplayConfig();
    return config.offsetY + (y * config.scale);
}

static inline void InitGameDisplay(const char *title)
{
#if defined(PLATFORM_ANDROID)
    SetConfigFlags(FLAG_FULLSCREEN_MODE | FLAG_VSYNC_HINT);
    InitWindow(0, 0, title); /* raylib uses the NativeActivity display size. */
#else
    InitWindow(LUDI_DESIGN_WIDTH, LUDI_DESIGN_HEIGHT, title);
#endif
}

#endif
