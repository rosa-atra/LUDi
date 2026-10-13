#include <stdio.h>
#include <stdbool.h>
#include <raylib.h>



typedef struct {
    char id;
    int health;
    int stamina;
    float xPos;
    float yPos;
    bool isAlive;
}player;

typedef struct {
    char id;
    int durability;
    int firepower;
    float xPos;
    float yPos;
    bool hasFuel;

}starship;



int main(){
    InitWindow(800, 600, "main window");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 50, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;

}