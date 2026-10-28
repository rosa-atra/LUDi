#include <stdio.h>
#include <stdbool.h>
#include <raylib.h>



typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;



void rainfall(float x,float height, u32 drops){
    for (u32 i = 0; i < drops; i++)
    {
        DrawCircle(x,height,1,WHITE);
        x -= GetScreenWidth()/drops;
   
    }
}


int main(){
    const u32 screenWidth = 800;
    const u32 screenHeight = 600;


    InitWindow(screenWidth, screenHeight, "LUDi");


    SetTargetFPS(60);


    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);


        if (IsKeyDown(KEY_A)){
           
        }
        if (IsKeyDown(KEY_D)){
            if (
        }
        if (IsKeyDown(KEY_W)){
            
        }
        if (IsKeyDown(KEY_S)){
            
        }

        EndDrawing();
    }


    CloseWindow();
    return 0;

}