#include <stdio.h>
#include <stdbool.h>
#include <raylib.h>


int main(){
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib ring example");

    Vector2 center = { (float)screenWidth / 2.0f, (float)screenHeight / 2.0f };
    float innerRadius = 80.0f;
    float outerRadius = 190.0f;
    float startAngle = 0.0f;
    float endAngle = 360.0f;
    int segments = 0;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Draw a full ring
        DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, segments, MAROON);

        EndDrawing();
    }

    CloseWindow();
    return 0;

}