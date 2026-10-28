#include <stdio.h>
#include <stdbool.h>
#include <raylib.h>
#include "obj.h"



int main(){
    const dword screenWidth = 800;
    const dword screenHeight = 600;


    InitWindow(screenWidth, screenHeight, "LUDi");
    InitAudioDevice();
    SetTargetFPS(60);



    Texture2D ship = LoadTexture(shipTextures[SHIP_L]);



    Vector2 pos = { 400, 225 };
    const float speed = 200.0f;


    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        

        if (IsKeyDown(KEY_D)) pos.x += speed * dt;
        if (IsKeyDown(KEY_A))  pos.x -= speed * dt;
        if (IsKeyDown(KEY_S))  pos.y += speed * dt;
        if (IsKeyDown(KEY_W))    pos.y -= speed * dt;

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTextureV(ship, pos, WHITE);
            DrawText("LUDi", 200, 50, 30, WHITE);
            DrawFPS(10, 10);


        EndDrawing();
    }

    UnloadTexture(ship);
    CloseAudioDevice();
    CloseWindow();
    return 0;

}