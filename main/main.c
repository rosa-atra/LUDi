#include <stdio.h>
#include <stdbool.h>
#include <raylib.h>


int main(){
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "raylib ring example");

    Vector2 center = { (float)screenWidth / 2.0f, (float)screenHeight / 2.0f };
    float innerRadius = 89.0f;
    float outerRadius = 90.0f;
    float startAngle = 0.0f;
    float endAngle = 360.0f;
    int segments = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        
        // Draw a full ring
        DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, segments, MAROON);
        //DrawLine(200,600,300,300,WHITE);
        //DrawLine(600,600,500,300,WHITE);
        //innerRadius += 1;
        //outerRadius += 1;

        if (IsKeyDown(KEY_A)){
            center.x -= 5.0f;
        }
        if (IsKeyDown(KEY_D)){
            center.x += 5.0f;
        }
        if (IsKeyDown(KEY_W)){
            innerRadius -= 1.0f;
            outerRadius -= 1.0f;
        }
        if (IsKeyDown(KEY_S)){
            innerRadius += 1.0f;
            outerRadius += 1.0f;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;

}