#include <stdio.h>
#include <stdbool.h>
#include <raylib.h>
#include <time.h>
#include <math.h>
#include "obj.h"
#include "display_config.h"


int main(){
    InitGameDisplay("SPACE ADVENTURE");
    AndroidDisplayConfig display = GetGameDisplayConfig();
    const int screenWidth = display.width;
    const int screenHeight = display.height;

    SetRandomSeed((dword)time(NULL));
    InitAudioDevice();
    SetTargetFPS(60);



    Texture2D thruster = LoadTexture(spriteTextures[SPRITE_EFFECT_YELLOW]);
    Texture2D thrusterBoost = LoadTexture(spriteTextures[SPRITE_EFFECT_PURPLE]);
    Texture2D ship = LoadTexture(shipTextures[SHIP_L]);
    Texture2D farAmbience = LoadTexture(spriteTextures[SPRITE_STAR_TINY]);
    Texture2D crossButton = LoadTexture(spriteTextures[SPRITE_ICON_CROSS_SMALL]);

    Rectangle quitButton = {
        (float)(screenWidth - display.quitSize), 0.0f,
        (float)display.quitSize, (float)display.quitSize
    };

    Vector2 pos = { ScaleGameX(390.0f), ScaleGameY(450.0f) };
    Vector2 posT = { pos.x, pos.y + (60.0f * display.scale) };
    Vector2 icon = { (float)(screenWidth - display.quitSize), 0.0f };

    const float speed = 100.0f;
    const float accel = 100.0f;
    bool quitGame = false;
    bool boostActive = false;
#if defined(PLATFORM_ANDROID)
    bool touchMoved = false;
    float touchHoldTime = 0.0f;
    Vector2 touchPrevious = { 0.0f, 0.0f };
#endif

    const int starCount = 100;
    Vector2 stars[starCount];
    Vector2 stars2[starCount];
    Vector2 stars3[starCount];
    float starScales[starCount];

    for (int i = 0; i < starCount; i++) {
        stars[i].x = (float)GetRandomValue(0, screenWidth - 1);
        stars[i].y = (float)GetRandomValue(0, screenHeight - 1);
        starScales[i] = (float)GetRandomValue(2, 8) / 100.0f;
    }
      for (int i = 0; i < starCount; i++) {
        stars2[i].x = (float)GetRandomValue(0, screenWidth - 1);
        stars2[i].y = (float)GetRandomValue(0, screenHeight - 1);
        starScales[i] = (float)GetRandomValue(8, 15) / 100.0f;
    }

    for (int i = 0; i < starCount; i++) {
        stars3[i].x = (float)GetRandomValue(0, screenWidth - 1);
        stars3[i].y = (float)GetRandomValue(0, screenHeight - 1);
        starScales[i] = (float)GetRandomValue(15, 20) / 100.0f;
    }

    


    while (!WindowShouldClose() && !quitGame) {

        float dt = GetFrameTime();

        Vector2 mouse = GetMousePosition();
        bool hover = CheckCollisionPointRec(mouse, quitButton);
        if (hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) quitGame = true;

#if defined(PLATFORM_ANDROID)
        bool touchDown = (GetTouchPointCount() > 0) || IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        Vector2 touchPosition = GetTouchPosition(0);

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            touchPrevious = touchPosition;
            touchHoldTime = 0.0f;
            touchMoved = false;
        }

        if (touchDown && !hover) {
            Vector2 touchDelta = {
                touchPosition.x - touchPrevious.x,
                touchPosition.y - touchPrevious.y
            };

            if (fabsf(touchDelta.x) >= 3.0f * display.scale || touchMoved) {
                float newX = pos.x + touchDelta.x;
                if ((newX >= 0.0f) && (newX <= screenWidth - (ship.width * display.scale))) pos.x = newX;
                touchMoved = true;
                touchHoldTime = 0.0f;
            } else {
                touchHoldTime += dt;
            }

            touchPrevious = touchPosition;
        } else {
            touchHoldTime = 0.0f;
        }

        boostActive = (touchDown && !touchMoved && (touchHoldTime >= 0.18f));
#else
        boostActive = false;
#endif
        boostActive = boostActive || IsKeyDown(KEY_W) || IsKeyDown(KEY_LEFT_SHIFT);


        if (IsKeyDown(KEY_D)) {
            if (pos.x >= screenWidth-64) pos.x = screenWidth-64;
            if (posT.x >= screenWidth-64) posT.x = screenWidth-64;
            pos.x += speed * dt;
            posT.x += speed * dt;}

        if (IsKeyDown(KEY_A))  {
            if (pos.x <= 0) pos.x = 0;
            if (posT.x <= 0) posT.x = 0;
            pos.x -= speed * dt;
            posT.x -= speed * dt;}

        posT.x = pos.x;
        posT.y = pos.y + (60.0f * display.scale);


        BeginDrawing();
            ClearBackground(BLACK);

            //DrawRectangleRec(quitButton, hover ? Fade(RED, 0.35f) : Fade(GRAY, 0.01f));
            DrawTextureEx(crossButton, icon, 0.0f, display.scale, hover ? RED : WHITE);


            //1st layer stars
            for (int i = 0; i < starCount; i++) {

                DrawTextureEx(farAmbience, stars[i], 0.0f, starScales[i], Fade(RED , 0.5f));
                stars2[i].y  += 0.01f;
                if (stars[i].y >= screenHeight) stars[i].y = 0.0f;
                if (boostActive) stars[i].y  += dt * speed * 0.2 + 0.5 * accel * dt * dt;
            }

            // 2nd layer stars
             for (int i = 0; i < starCount; i++) {

                DrawTextureEx(farAmbience, stars2[i], 0.0f, starScales[i], WHITE);

                stars2[i].y  += 0.2f;
                if (stars2[i].y >= screenHeight) stars2[i].y = 0.0f;

                if (boostActive) stars2[i].y  += dt * speed * 0.5 + 0.5 * accel * dt * dt;
            }
            // 3rd  layer stars
            for (int i = 0; i < starCount; i++) {

                DrawTextureEx(farAmbience, stars3[i], 0.0f, starScales[i], WHITE);

                stars3[i].y  += 0.6f;
                if (stars3[i].y >= screenHeight) stars3[i].y = 0.0f;
                if (boostActive) {
                    stars3[i].y  += dt * speed * 0.6 + 0.5 * accel * dt * dt;
                }
                else{stars3[i].y  += dt * speed * 0.6 - 0.5 * accel * dt * dt;
                }
            }


            // star ship
            DrawTextureEx(ship, pos, 0.0f, display.scale, WHITE);
            // ship thruster
            if (boostActive) {
                DrawTextureEx(thrusterBoost, posT, 0.0f, display.scale, WHITE);
            }
            else {
                DrawTextureEx(thruster, posT, 0.0f, display.scale, WHITE);
            }

            DrawFPS(10, 10);



        EndDrawing();
    }

    UnloadTexture(ship);
    UnloadTexture(thruster);
    UnloadTexture(thrusterBoost);
    UnloadTexture(farAmbience);
    UnloadTexture(crossButton);
    CloseAudioDevice();
    CloseWindow();
    return 0;

}