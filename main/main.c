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
    int ammo;
    float xPos;
    float yPos;
    bool hasFuel;

}starship;



int main(){
    printf("Hello World!\n");
    return 0;

}