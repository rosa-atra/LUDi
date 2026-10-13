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

int cube(int side,int x, int y ){
    DrawCircle(x-side,y,5,RED);
    DrawCircle(x,y,5,RED);
    DrawCircle(x,y+side,5,RED);
    DrawCircle(x-side,y+side,5,RED);
}

int main(){
    InitWindow(800, 600, "main window");
    SetTargetFPS(60);

    int xpos = GetScreenWidth() / 2 ;
    int ypos = GetScreenHeight() / 2 ;
    int sqside = 100;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawCircle(xpos - sqside, ypos , 5, RED);
        DrawCircle(xpos, ypos , 5, GREEN);
        DrawCircle(xpos-sqside , ypos+sqside , 5, BLUE);
        DrawCircle(xpos, ypos+sqside , 5, YELLOW);
        xpos += 1;
    
        EndDrawing();
    }

    CloseWindow();
    return 0;

}