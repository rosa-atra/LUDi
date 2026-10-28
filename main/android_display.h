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
    int quitSize;
} AndroidDisplayConfig;

static inline AndroidDisplayConfig GetGameDisplayConfig(void)
{
    AndroidDisplayConfig config;

    config.width = GetScreenWidth();
    config.height = GetScreenHeight();
    if (config.width <= 0) config.width = LUDI_DESIGN_WIDTH;
    if (config.height <= 0) config.height = LUDI_DESIGN_HEIGHT;

    config.scaleX = (float)config.width / LUDI_DESIGN_WIDTH;
    config.scaleY = (float)config.height / LUDI_DESIGN_HEIGHT;
    config.scale = (config.scaleX < config.scaleY) ? config.scaleX : config.scaleY;
    config.quitSize = (int)(64.0f * config.scale);
    if (config.quitSize < 48) config.quitSize = 48;

    return config;
}

static inline float ScaleGameX(float x)
{
    return x * (float)GetScreenWidth() / LUDI_DESIGN_WIDTH;
}

static inline float ScaleGameY(float y)
{
    return y * (float)GetScreenHeight() / LUDI_DESIGN_HEIGHT;
}

static inline void InitGameDisplay(const char *title)
{
#if defined(PLATFORM_ANDROID)
    InitWindow(0, 0, title); /* raylib uses the NativeActivity display size. */
#else
    InitWindow(LUDI_DESIGN_WIDTH, LUDI_DESIGN_HEIGHT, title);
#endif
}

#endif
