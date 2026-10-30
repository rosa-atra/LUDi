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
    SetExitKey(KEY_NULL);

// Textures and sprites
    Texture2D thruster = LoadTexture(spriteTextures[SPRITE_EFFECT_YELLOW]);
    Texture2D thrusterBoost = LoadTexture(spriteTextures[SPRITE_EFFECT_PURPLE]);
    Texture2D ship = LoadTexture(shipTextures[SHIP_L]);
    Texture2D farAmbience = LoadTexture(spriteTextures[SPRITE_STAR_TINY]);
    Texture2D crossButton = LoadTexture(spriteTextures[SPRITE_ICON_CROSS_SMALL]);
    Texture2D meteorTextures[8] = {
        LoadTexture(spriteTextures[SPRITE_METEOR_DETAILED_LARGE]),
        LoadTexture(spriteTextures[SPRITE_METEOR_DETAILED_SMALL]),
        LoadTexture(spriteTextures[SPRITE_METEOR_LARGE]),
        LoadTexture(spriteTextures[SPRITE_METEOR_SMALL]),
        LoadTexture(spriteTextures[SPRITE_METEOR_SQUARE_DETAILED_LARGE]),
        LoadTexture(spriteTextures[SPRITE_METEOR_SQUARE_DETAILED_SMALL]),
        LoadTexture(spriteTextures[SPRITE_METEOR_SQUARE_LARGE]),
        LoadTexture(spriteTextures[SPRITE_METEOR_SQUARE_SMALL])
    };
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
    bool exitConfirmation = false;
    bool boostActive = false;
    int shipHealth = 7;
    const int maxShipHealth = 7;
    float collisionCooldown = 0.0f;

#if defined(PLATFORM_ANDROID)
    bool touchMoved = false;
    float touchHoldTime = 0.0f;
    float touchWorldOffset = 0.0f;
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
    int meteorTextureIndex[meteorCount];
    float meteorRotation[meteorCount];
    float meteorRotationSpeed[meteorCount];
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
        meteorTextureIndex[i] = GetRandomValue(0, 7);
        meteorRotation[i] = (float)GetRandomValue(-180, 180);
        meteorRotationSpeed[i] = (float)GetRandomValue(-90, 90) / 2.0f;
    }


    while (!quitGame) {

        float dt = GetFrameTime();
        if (WindowShouldClose() ||
            (!exitConfirmation && IsKeyPressed(KEY_ESCAPE))) {
            exitConfirmation = true;
        }
#if defined(PLATFORM_ANDROID)
        if (!exitConfirmation && IsKeyPressed(KEY_BACK)) exitConfirmation = true;
#endif

        if (collisionCooldown > 0.0f) collisionCooldown -= dt;


        Vector2 mouse = GetMousePosition();
        bool hover = CheckCollisionPointRec(mouse, quitButton);

        float panelWidth = (float)screenWidth * 0.70f;
        float panelHeight = (float)screenHeight * 0.30f;
        if (panelWidth > 420.0f) panelWidth = 420.0f;
        if (panelHeight > 220.0f) panelHeight = 220.0f;
        Rectangle exitPanel = {
            ((float)screenWidth - panelWidth) * 0.5f,
            ((float)screenHeight - panelHeight) * 0.5f,
            panelWidth,
            panelHeight
        };
        float buttonWidth = (panelWidth - 48.0f) * 0.5f;
        Rectangle exitButton = {
            exitPanel.x + 16.0f,
            exitPanel.y + panelHeight - 64.0f,
            buttonWidth,
            48.0f
        };
        Rectangle stayButton = {
            exitPanel.x + panelWidth - 16.0f - buttonWidth,
            exitButton.y,
            buttonWidth,
            48.0f
        };

        if (!exitConfirmation && hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            exitConfirmation = true;
        }
        if (exitConfirmation && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse, exitButton)) quitGame = true;
            if (CheckCollisionPointRec(mouse, stayButton)) exitConfirmation = false;
        }

       

#if defined(PLATFORM_ANDROID)
        touchWorldOffset = 0.0f;
        bool touchDown = GetTouchPointCount() > 0;
        Vector2 touchPosition = touchDown ? GetTouchPosition(0) : (Vector2){ -1.0f, -1.0f };
        bool touchHover = CheckCollisionPointRec(touchPosition, quitButton);

        if (touchDown && !touchWasDown) {
            touchPrevious = touchPosition;
            touchHoldTime = 0.0f;
            touchMoved = false;
            if (!exitConfirmation && touchHover) exitConfirmation = true;
        }

        if (touchDown && !touchHover) {
            Vector2 touchDelta = {
                touchPosition.x - touchPrevious.x,
                touchPosition.y - touchPrevious.y
            };

            if (touchMoved || fabsf(touchDelta.x) >= 3.0f * display.scale) {
                touchWorldOffset = -touchDelta.x;
                touchMoved = true;
                touchHoldTime = 0.0f;
            } else {
                touchHoldTime += dt;
            }

            touchPrevious = touchPosition;
        } else if (!touchDown) {
            touchHoldTime = 0.0f;
        }

        if (exitConfirmation && !touchDown && touchWasDown) {
            if (CheckCollisionPointRec(touchPrevious, exitButton)) quitGame = true;
            if (CheckCollisionPointRec(touchPrevious, stayButton)) exitConfirmation = false;
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
#if defined(PLATFORM_ANDROID)
                stars[i].x += touchWorldOffset;
#endif
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
#if defined(PLATFORM_ANDROID)
                stars2[i].x += touchWorldOffset;
#endif
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
#if defined(PLATFORM_ANDROID)
                stars3[i].x += touchWorldOffset;
#endif
                if (boostActive) {
                    stars3[i].y  += dt * speed * 0.6 + 0.5 * accel * dt * dt;
                }
            }

            for (int i = 0; i < meteorCount; i++) {

                Texture2D meteorTexture = meteorTextures[meteorTextureIndex[i]];
                meteorRotation[i] += meteorRotationSpeed[i] * dt;
                DrawTextureEx(meteorTexture, meteor[i], meteorRotation[i], meteorScale[i], GRAY);

                meteor[i].y  += 1.0f;
                if (meteor[i].y > screenHeight + 64) meteor[i].y = -64.0f;
                if (meteor[i].x <= -64) meteor[i].x = screenWidth;
                if (meteor[i].x > screenWidth) meteor[i].x = -64.0f;

                if (boostActive) meteor[i].y  += dt * speed + 0.5 * accel * dt * dt;
                if (IsKeyDown(KEY_A))meteor[i].x += dt * speed;
                if (IsKeyDown(KEY_D))meteor[i].x -= dt * speed;
#if defined(PLATFORM_ANDROID)
                meteor[i].x += touchWorldOffset;
#endif

                Rectangle shipBounds = {
                    pos.x,
                    pos.y,
                    ship.width * display.scale,
                    ship.height * display.scale
                };
                float meteorWidth = meteorTexture.width * meteorScale[i];
                float meteorHeight = meteorTexture.height * meteorScale[i];
                const float meteorCollisionInset = 0.10f;
                Rectangle meteorBounds = {
                    meteor[i].x + meteorWidth * meteorCollisionInset,
                    meteor[i].y + meteorHeight * meteorCollisionInset,
                    meteorWidth * (1.0f - meteorCollisionInset * 2.0f),
                    meteorHeight * (1.0f - meteorCollisionInset * 2.0f)
                };

                if ((shipHealth > 0) && (collisionCooldown <= 0.0f) &&
                    CheckCollisionRecs(shipBounds, meteorBounds)) {
                    shipHealth--;
                    collisionCooldown = 1.0f;
                    meteor[i].y = -64.0f;
                    meteor[i].x = (float)GetRandomValue(0, screenWidth - 1);
                    meteorTextureIndex[i] = GetRandomValue(0, 7);
                    meteorRotation[i] = (float)GetRandomValue(-180, 180);
                    meteorRotationSpeed[i] = (float)GetRandomValue(-90, 90) / 2.0f;
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


            if (exitConfirmation) {
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.65f));
                DrawRectangleRec(exitPanel, DARKGRAY);
                DrawRectangleLinesEx(exitPanel, 2.0f, LIGHTGRAY);

                const char *exitText = "Are you sure you want to exit?";
                int fontSize = 20;
                int textWidth = MeasureText(exitText, fontSize);
                DrawText(exitText,
                    (int)(exitPanel.x + (exitPanel.width - textWidth) * 0.5f),
                    (int)(exitPanel.y + 45.0f), fontSize, WHITE);

                DrawRectangleRec(exitButton, MAROON);
                DrawRectangleRec(stayButton, GREEN);
                DrawRectangleLinesEx(exitButton, 2.0f, LIGHTGRAY);
                DrawRectangleLinesEx(stayButton, 2.0f, LIGHTGRAY);
                DrawText("EXIT", (int)(exitButton.x + (exitButton.width - MeasureText("EXIT", 20)) * 0.5f),
                    (int)(exitButton.y + 14.0f), 20, WHITE);
                DrawText("STAY", (int)(stayButton.x + (stayButton.width - MeasureText("STAY", 20)) * 0.5f),
                    (int)(stayButton.y + 14.0f), 20, WHITE);
            }
        

        EndDrawing();
    }

    UnloadTexture(ship);
    UnloadTexture(thruster);
    UnloadTexture(thrusterBoost);
    UnloadTexture(farAmbience);
    UnloadTexture(crossButton);
    for (int i = 0; i < 8; i++) {
        UnloadTexture(meteorTextures[i]);
    }
    UnloadTexture(health);
    CloseAudioDevice();
    CloseWindow();
    return 0;

}