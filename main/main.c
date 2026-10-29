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

// Textures and sprites
    Texture2D thruster = LoadTexture(spriteTextures[SPRITE_EFFECT_YELLOW]);
    Texture2D thrusterBoost = LoadTexture(spriteTextures[SPRITE_EFFECT_PURPLE]);
    Texture2D ship = LoadTexture(shipTextures[SHIP_L]);
    Texture2D farAmbience = LoadTexture(spriteTextures[SPRITE_STAR_TINY]);
    Texture2D crossButton = LoadTexture(spriteTextures[SPRITE_ICON_CROSS_SMALL]);
    Texture2D meteorText = LoadTexture(spriteTextures[SPRITE_METEOR_DETAILED_LARGE]);
    Texture2D health = LoadTexture(shipTextures[SHIP_B]);

//UI elements
    Rectangle quitButton = {
        (float)(screenWidth - display.quitSize), 0.0f,
        (float)display.quitSize, (float)display.quitSize
    };

    Vector2 pos = {
        (screenWidth * 0.5f) - (ship.width * display.scale * 0.5f),
        (screenHeight * (2.0f / 3.0f)) - (ship.height * display.scale * 0.5f)
    };
    Vector2 posT = { pos.x, pos.y + (60.0f * display.scale) };
    Vector2 icon = { (float)(screenWidth - display.quitSize), 0.0f };
    Vector2 healpos = {0.0f , 0.0f};

    const float speed = 100.0f;
    const float accel = 100.0f;
    bool quitGame = false;
    bool boostActive = false;
    int shipHealth = 7;
    const int maxShipHealth = 7;
    float collisionCooldown = 0.0f;

#if defined(PLATFORM_ANDROID)
    bool touchMoved = false;
    float touchHoldTime = 0.0f;
    Vector2 touchPrevious = { 0.0f, 0.0f };
    bool touchWasDown = false;
#endif

    const int starCount = 100;
    const int meteorCount = 20;

    Vector2 stars[starCount];
    Vector2 stars2[starCount];
    Vector2 stars3[starCount];
    Vector2 meteor[meteorCount];
    float meteorScale[meteorCount];
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

    for (int i = 0; i < meteorCount; i++) {
        meteor[i].x = (float)GetRandomValue(0, screenWidth - 1);
        meteor[i].y = (float)GetRandomValue(0, screenHeight - 1);
        meteorScale[i] = (float)GetRandomValue(90, 100) / 100.0f;
    }


    while (!WindowShouldClose() && !quitGame) {

        float dt = GetFrameTime();
        if (collisionCooldown > 0.0f) collisionCooldown -= dt;


        Vector2 mouse = GetMousePosition();
        bool hover = CheckCollisionPointRec(mouse, quitButton);
        if (hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) quitGame = true;

       

#if defined(PLATFORM_ANDROID)
        bool touchDown = GetTouchPointCount() > 0;
        Vector2 touchPosition = touchDown ? GetTouchPosition(0) : (Vector2){ -1.0f, -1.0f };
        bool touchHover = CheckCollisionPointRec(touchPosition, quitButton);

        if (touchDown && !touchWasDown) {
            touchPrevious = touchPosition;
            touchHoldTime = 0.0f;
            touchMoved = false;
        }

        if (touchDown && !touchHover) {
            Vector2 touchDelta = {
                touchPosition.x - touchPrevious.x,
                touchPosition.y - touchPrevious.y
            };

            if (touchMoved || fabsf(touchDelta.x) >= 3.0f * display.scale) {
                float newX = pos.x + touchDelta.x;
                if ((newX >= 0.0f) && (newX <= screenWidth - (ship.width * display.scale))) {
                    pos.x = newX;
                    posT.x = pos.x;
                }
                touchMoved = true;
                touchHoldTime = 0.0f;
            } else {
                touchHoldTime += dt;
            }

            touchPrevious = touchPosition;
        } else if (!touchDown) {
            touchHoldTime = 0.0f;
        }

        touchWasDown = touchDown;
        boostActive = (touchDown && !touchMoved && (touchHoldTime >= 0.18f));
#else
        boostActive = false;
#endif
        boostActive = boostActive || IsKeyDown(KEY_W) || IsKeyDown(KEY_LEFT_SHIFT);


        BeginDrawing();
            ClearBackground(BLACK);

           


            //1st layer stars
            for (int i = 0; i < starCount; i++) {

                DrawTextureEx(farAmbience, stars[i], 0.0f, starScales[i], Fade(RED , 0.5f));

                stars2[i].y  += 0.01f;

                if (stars[i].y >= screenHeight) stars[i].y = 0.0f;
                if (stars[i].x < 0) stars[i].x = screenWidth;
                if (stars[i].x > screenWidth) stars[i].x = 0.0f;

                if (boostActive) stars[i].y  += dt * speed * 0.2 + 0.5 * accel * dt * dt;
                if (IsKeyDown(KEY_A))stars[i].x += dt * speed * 0.2;
                if (IsKeyDown(KEY_D))stars[i].x -= dt * speed * 0.2;
             }

            // 2nd layer stars
             for (int i = 0; i < starCount; i++) {

                DrawTextureEx(farAmbience, stars2[i], 0.0f, starScales[i], WHITE);

                stars2[i].y  += 0.2f;

                if (stars2[i].y >= screenHeight) stars2[i].y = 0.0f;
                if (stars2[i].x < 0) stars2[i].x = screenWidth;
                if (stars2[i].x > screenWidth) stars2[i].x = 0.0f;

                if (boostActive) stars2[i].y  += dt * speed * 0.4 + 0.5 * accel * dt * dt;
                if (IsKeyDown(KEY_A))stars2[i].x += dt * speed * 0.4;
                if (IsKeyDown(KEY_D))stars2[i].x -= dt * speed * 0.4;
            }
            // 3rd  layer stars
            for (int i = 0; i < starCount; i++) {

                DrawTextureEx(farAmbience, stars3[i], 0.0f, starScales[i], WHITE);

                stars3[i].y  += 0.6f;

                if (stars3[i].y >= screenHeight) stars3[i].y = 0.0f;
                if (stars3[i].x < 0) stars3[i].x = screenWidth;
                if (stars3[i].x > screenWidth) stars3[i].x = 0.0f;

                if (IsKeyDown(KEY_A))stars3[i].x += dt * speed * 0.6;
                if (IsKeyDown(KEY_D))stars3[i].x -= dt * speed * 0.6;
                if (boostActive) {
                    stars3[i].y  += dt * speed * 0.6 + 0.5 * accel * dt * dt;
                }
            }

            for (int i = 0; i < meteorCount; i++) {

                DrawTextureEx(meteorText, meteor[i], 0.0f, meteorScale[i], GRAY);

                meteor[i].y  += 1.0f;
                if (meteor[i].y > screenHeight + 64) meteor[i].y = -64.0f;
                if (meteor[i].x <= -64) meteor[i].x = screenWidth;
                if (meteor[i].x > screenWidth) meteor[i].x = -64.0f;

                if (boostActive) meteor[i].y  += dt * speed + 0.5 * accel * dt * dt;
                if (IsKeyDown(KEY_A))meteor[i].x += dt * speed;
                if (IsKeyDown(KEY_D))meteor[i].x -= dt * speed;

                Rectangle shipBounds = {
                    pos.x,
                    pos.y,
                    ship.width * display.scale,
                    ship.height * display.scale
                };
                Rectangle meteorBounds = {
                    meteor[i].x,
                    meteor[i].y,
                    meteorText.width * meteorScale[i],
                    meteorText.height * meteorScale[i]
                };

                if ((shipHealth > 0) && (collisionCooldown <= 0.0f) &&
                    CheckCollisionRecs(shipBounds, meteorBounds)) {
                    shipHealth--;
                    collisionCooldown = 1.0f;
                    meteor[i].y = -64.0f;
                    meteor[i].x = (float)GetRandomValue(0, screenWidth - 1);
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


            healpos.x = 0.0f;
            for (int i = 0; i < maxShipHealth; i++){
                if (i < shipHealth) {
                    DrawTextureV(health, healpos, WHITE);
                }
                healpos.x = (i + 1) * 36.0f;
            }


             //DrawRectangleRec(quitButton, hover ? Fade(RED, 0.35f) : Fade(GRAY, 0.01f));
            DrawTextureEx(crossButton, icon, 0.0f, display.scale, hover ? RED : WHITE);
        

        EndDrawing();
    }

    UnloadTexture(ship);
    UnloadTexture(thruster);
    UnloadTexture(thrusterBoost);
    UnloadTexture(farAmbience);
    UnloadTexture(crossButton);
    UnloadTexture(meteorText);
    UnloadTexture(health);
    CloseAudioDevice();
    CloseWindow();
    return 0;

}