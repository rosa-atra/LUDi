#include <stdio.h>
#include <stdbool.h>
#include <raylib.h>
#include <time.h>
#include <math.h>
#include "obj.h"
#include "display_config.h"
#include "game.h"
#include "android_controls.h"

static int LoadHighScore(void)
{
    const char *workingDirectory = GetWorkingDirectory();
    if (workingDirectory == NULL || workingDirectory[0] == '\0') return 0;

    char path[512];
    snprintf(path, sizeof(path), "%s/space_high_score.txt", workingDirectory);
    FILE *file = fopen(path, "r");
    if (file == NULL) return 0;

    int highScore = 0;
    if (fscanf(file, "%d", &highScore) != 1 || highScore < 0) {
        TraceLog(LOG_WARNING, "Invalid high score file: %s", path);
        highScore = 0;
    }
    fclose(file);
    return highScore;
}

static void SaveHighScore(int highScore)
{
    const char *workingDirectory = GetWorkingDirectory();
    if (workingDirectory == NULL || workingDirectory[0] == '\0') {
        TraceLog(LOG_WARNING, "Could not determine where to save the high score");
        return;
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/space_high_score.txt", workingDirectory);
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        TraceLog(LOG_WARNING, "Could not save high score to %s", path);
        return;
    }

    if (fprintf(file, "%d\n", highScore) < 0) {
        TraceLog(LOG_WARNING, "Could not write high score to %s", path);
    }
    fclose(file);
}

static void DrawMenuButton(Rectangle button, const char *label, bool selected)
{
    Color fill = selected ? (Color){ 35, 115, 190, 255 } : (Color){ 25, 45, 70, 245 };
    DrawRectangleRounded(button, 0.18f, 8, fill);
    DrawRectangleRoundedLines(button, 0.18f, 8, selected ? SKYBLUE : LIGHTGRAY);
    int fontSize = (int)(button.height * 0.42f);
    int labelWidth = MeasureText(label, fontSize);
    DrawText(label, (int)(button.x + (button.width - labelWidth) * 0.5f),
        (int)(button.y + (button.height - fontSize) * 0.5f), fontSize, WHITE);
}

static void DrawMenuScreen(int screenWidth, int screenHeight, bool highScoreScreen,
    int highScore, int selectedMenuOption, const Rectangle menuButtons[4],
    Rectangle highScoreBackButton)
{
    BeginDrawing();
    ClearBackground((Color){ 3, 8, 24, 255 });

    const char *title = highScoreScreen ? "HIGH SCORES" : "SPACE ADVENTURE";
    int titleFontSize = (int)((float)screenWidth * 0.09f);
    if (titleFontSize > 48) titleFontSize = 48;
    if (titleFontSize < 24) titleFontSize = 24;
    int titleWidth = MeasureText(title, titleFontSize);
    DrawText(title, (screenWidth - titleWidth) / 2, (int)(screenHeight * 0.14f),
        titleFontSize, SKYBLUE);

    if (highScoreScreen) {
        const char *recordText = TextFormat("BEST SCORE: %d", highScore);
        int recordFontSize = 28;
        int recordWidth = MeasureText(recordText, recordFontSize);
        DrawText(recordText, (screenWidth - recordWidth) / 2,
            (int)(screenHeight * 0.43f), recordFontSize, GOLD);
        DrawMenuButton(highScoreBackButton, "BACK",
            CheckCollisionPointRec(GetMousePosition(), highScoreBackButton));
    } else {
        const char *labels[4] = { "CONTINUE", "NEW GAME", "HIGH SCORES", "EXIT" };
        for (int i = 0; i < 4; i++) {
            bool selected = i == selectedMenuOption ||
                CheckCollisionPointRec(GetMousePosition(), menuButtons[i]);
            DrawMenuButton(menuButtons[i], labels[i], selected);
        }
        const char *recordText = TextFormat("HIGH SCORE: %d", highScore);
        int recordWidth = MeasureText(recordText, 18);
        DrawText(recordText, (screenWidth - recordWidth) / 2,
            (int)(screenHeight * 0.91f), 18, LIGHTGRAY);
    }

    EndDrawing();
}

int RunGame(void){
    InitGameDisplay("SPACE ADVENTURE");
    SetGesturesEnabled(GESTURE_TAP);
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
    Texture2D health = LoadTexture(shipTextures[SHIP_L]);
    Texture2D enemyTextures[5] = {
        LoadTexture(spriteTextures[SPRITE_ENEMY_A]),
        LoadTexture(spriteTextures[SPRITE_ENEMY_B]),
        LoadTexture(spriteTextures[SPRITE_ENEMY_C]),
        LoadTexture(spriteTextures[SPRITE_ENEMY_D]),
        LoadTexture(spriteTextures[SPRITE_ENEMY_E])
    };
    Texture2D stationTextures[3] = {
        LoadTexture(spriteTextures[SPRITE_STATION_A_RETINA]),
        LoadTexture(spriteTextures[SPRITE_STATION_B_RETINA]),
        LoadTexture(spriteTextures[SPRITE_STATION_C_RETINA])
    };
    Sound plasmaSound = LoadSound("LUDi/assets/sfx/laserRetro_000.ogg");
    Sound thrusterSound = LoadSound("LUDi/assets/sfx/thrusterFire_000.ogg");
    Sound boostThrusterSound = LoadSound("LUDi/assets/sfx/thrusterFire_002.ogg");
    Sound meteorExplosionSound = LoadSound("LUDi/assets/sfx/explosionCrunch_000.ogg");
    SetSoundVolume(plasmaSound, 0.05f);
    SetSoundVolume(thrusterSound, 0.10f);
    SetSoundVolume(boostThrusterSound, 0.10f);
    SetSoundVolume(meteorExplosionSound, 0.12f);

//UI elements
    Rectangle quitButton = {
        (float)(screenWidth - display.quitSize), 0.0f,
        (float)display.quitSize, (float)display.quitSize
    };
    AndroidControls androidControls = { 0 };
    AndroidControlsInit(&androidControls, screenWidth, screenHeight);

    Vector2 shipPositionOffset = {
        (screenWidth * 0.5f) - (ship.width * display.scale * 0.5f),
        (screenHeight * (2.0f / 3.0f)) - (ship.height * display.scale * 0.5f)
    };
    Vector2 thrusterPositionOffset = { shipPositionOffset.x, shipPositionOffset.y + (60.0f * display.scale) };
    const float speed = 100.0f;
    const float accel = 100.0f;
    bool quitGame = false;
    bool exitConfirmation = false;
    bool gameOver = false;
    bool mainMenu = true;
    bool highScoreScreen = false;
    bool menuHasBeenShown = false;
    bool menuTouchWasDown = false;
    Vector2 menuTouchPosition = { -1.0f, -1.0f };
    bool resetGameRequested = false;
    int selectedMenuOption = 0;
    int score = 0;
    int highScore = LoadHighScore();
    bool yellowThrusterActive = false;
    bool blueThrusterActive = false;
    int shipHealth = 6;
    const int maxShipHealth = 6;
    float shipHealthBar = 100.0f;
    const float maxShipHealthBar = 100.0f;
    float shipFuel = 200.0f;
    const float maxShipFuel = 200.0f;
    float collisionCooldown = 0.0f;
    float shipVelocity = 10.0f;


    const int starCount = 100;
    const int maxMeteorCount = 30;
    const int maxEnemyCount = 8;
    const int maxEnemyBolts = 32;
    int meteorCount = 0;
    int enemyCount = 4;

    typedef struct {
        Vector2 position;
        Vector2 velocity;
        float life;
    } PlasmaBolt;

    const int maxPlasmaBolts = 64;
    PlasmaBolt plasmaBolts[maxPlasmaBolts];
    float plasmaFireCooldown = 0.0f;

    Vector2 starsClusterLayer[starCount];
    Vector2 starsClusterLayer2[starCount];
    Vector2 starsClusterLayer3[starCount];
    Vector2 meteor[maxMeteorCount];
    float meteorScale[maxMeteorCount];
    int meteorTextureIndex[maxMeteorCount];
    float meteorRotation[maxMeteorCount];
    float meteorRotationSpeed[maxMeteorCount];
    bool meteorActive[maxMeteorCount];

    Vector2 enemies[maxEnemyCount];
    float enemyScale[maxEnemyCount];
    int enemyTextureIndex[maxEnemyCount];
    float enemyShootTimer[maxEnemyCount];
    float enemyHorizontalSpeed[maxEnemyCount];
    bool enemyActive[maxEnemyCount];
    Vector2 enemyBolts[maxEnemyBolts];
    Vector2 enemyBoltVelocity[maxEnemyBolts];
    float enemyBoltLife[maxEnemyBolts];

    float activeGameplayTime = 0.0f;
    const float stationInterval = 60.0f;
    const float stationScaleFactor = 0.45f;
    int stationNumber = 1;
    int stationTextureIndex = GetRandomValue(0, 2);
    Vector2 stationPosition = {
        (screenWidth - stationTextures[stationTextureIndex].width * display.scale * stationScaleFactor) * 0.5f,
        screenHeight * 0.24f
    };
    bool stationActive = true;
    bool stationHealthRefilled = false;
    float starScales[starCount];

    for (int i = 0; i < starCount; i++) {
        starsClusterLayer[i].x = (float)GetRandomValue(0, screenWidth - 1);
        starsClusterLayer[i].y = (float)GetRandomValue(0, screenHeight - 1);
        starScales[i] = (float)GetRandomValue(2, 8) / 100.0f;
    }

      for (int i = 0; i < starCount; i++) {
        starsClusterLayer2[i].x = (float)GetRandomValue(0, screenWidth - 1);
        starsClusterLayer2[i].y = (float)GetRandomValue(0, screenHeight - 1);
        starScales[i] = (float)GetRandomValue(8, 15) / 100.0f;
    }

    for (int i = 0; i < starCount; i++) {
        starsClusterLayer3[i].x = (float)GetRandomValue(0, screenWidth - 1);
        starsClusterLayer3[i].y = (float)GetRandomValue(0, screenHeight - 1);
        starScales[i] = (float)GetRandomValue(15, 20) / 100.0f;
    }

    for (int i = 0; i < maxMeteorCount; i++) {
        meteor[i].x = (float)GetRandomValue(0, screenWidth - 1);
        meteor[i].y = (float)GetRandomValue(0, screenHeight - 1);
        meteorScale[i] = (float)GetRandomValue(90, 100) / 100.0f;
        meteorTextureIndex[i] = GetRandomValue(0, 7);
        meteorRotation[i] = (float)GetRandomValue(-180, 180);
        meteorRotationSpeed[i] = (float)GetRandomValue(-90, 90) / 2.0f;
        meteorActive[i] = false;
    }
    for (int i = 0; i < maxEnemyCount; i++) {
        enemies[i] = (Vector2){
            (float)GetRandomValue(40, screenWidth - 40),
            (float)GetRandomValue(-400, -80)
        };
        enemyScale[i] = (float)GetRandomValue(55, 85) / 100.0f;
        enemyTextureIndex[i] = GetRandomValue(0, 4);
        enemyShootTimer[i] = (float)GetRandomValue(8, 25) / 10.0f;
        enemyHorizontalSpeed[i] = (float)GetRandomValue(-55, 55);
        enemyActive[i] = i < enemyCount;
    }
    for (int i = 0; i < maxEnemyBolts; i++) {
        enemyBolts[i] = (Vector2){ 0.0f, 0.0f };
        enemyBoltVelocity[i] = (Vector2){ 0.0f, 720.0f * display.scale };
        enemyBoltLife[i] = 0.0f;
    }
    for (int i = 0; i < maxPlasmaBolts; i++) plasmaBolts[i].life = 0.0f;


    while (!quitGame) {

        float dt = GetFrameTime();
        if (WindowShouldClose()) {
            exitConfirmation = true;
        }
        if (!exitConfirmation && IsKeyPressed(KEY_ESCAPE)) {
            if (highScoreScreen) {
                highScoreScreen = false;
                mainMenu = true;
            } else if (mainMenu) {
                exitConfirmation = true;
            } else if (!gameOver) {
                mainMenu = true;
            }
        }
        if (!exitConfirmation && AndroidControlsBackPressed()) {
            if (highScoreScreen) {
                highScoreScreen = false;
                mainMenu = true;
            } else if (mainMenu) {
                exitConfirmation = true;
            } else if (!gameOver) {
                mainMenu = true;
            }
        }

        const int menuOptionCount = 4;
#if defined(PLATFORM_ANDROID)
        float menuButtonWidth = (float)screenWidth * 0.78f;
        if (menuButtonWidth > 960.0f) menuButtonWidth = 960.0f;
        if (menuButtonWidth < 220.0f) menuButtonWidth = (float)screenWidth - 24.0f;
        float menuButtonHeight = (float)screenHeight * 0.095f;
        if (menuButtonHeight > 120.0f) menuButtonHeight = 120.0f;
        if (menuButtonHeight < 72.0f) menuButtonHeight = 72.0f;
        float menuButtonGap = 18.0f;
#else
        float menuButtonWidth = (float)screenWidth * 0.68f;
        if (menuButtonWidth > 380.0f) menuButtonWidth = 380.0f;
        if (menuButtonWidth < 220.0f) menuButtonWidth = (float)screenWidth - 24.0f;
        float menuButtonHeight = (float)screenHeight * 0.075f;
        if (menuButtonHeight > 54.0f) menuButtonHeight = 54.0f;
        if (menuButtonHeight < 34.0f) menuButtonHeight = 34.0f;
        float menuButtonGap = 10.0f;
#endif
        float menuFirstButtonY = (float)screenHeight * 0.40f;
#if defined(PLATFORM_ANDROID)
        menuFirstButtonY = (float)screenHeight * 0.30f;
#endif
        if (menuFirstButtonY + (menuButtonHeight + menuButtonGap) * menuOptionCount > screenHeight - 8.0f) {
            menuFirstButtonY = (float)screenHeight -
                (menuButtonHeight + menuButtonGap) * menuOptionCount - 8.0f;
        }
        Rectangle menuButtons[menuOptionCount];
        for (int i = 0; i < menuOptionCount; i++) {
            menuButtons[i] = (Rectangle){
                ((float)screenWidth - menuButtonWidth) * 0.5f,
                menuFirstButtonY + (menuButtonHeight + menuButtonGap) * (float)i,
                menuButtonWidth,
                menuButtonHeight
            };
        }
        Rectangle highScoreBackButton = {
            ((float)screenWidth - menuButtonWidth) * 0.5f,
            (float)screenHeight * 0.68f,
            menuButtonWidth,
            menuButtonHeight
        };
        bool touchDown = GetTouchPointCount() > 0;
        Vector2 touchPosition = touchDown ? GetTouchPosition(0) : (Vector2){ -1.0f, -1.0f };
        bool touchPressed = touchDown && !menuTouchWasDown;
        bool touchReleased = !touchDown && menuTouchWasDown;
        if (touchDown) menuTouchPosition = touchPosition;
        menuTouchWasDown = touchDown;
        bool menuTap = touchPressed || IsMouseButtonPressed(MOUSE_BUTTON_LEFT) ||
            IsGestureDetected(GESTURE_TAP);
        bool menuClick = menuTap || IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || touchReleased;
        Vector2 menuPointer = touchPressed ? touchPosition :
            (touchReleased ? menuTouchPosition : GetMousePosition());
#if defined(PLATFORM_ANDROID)
        if (menuClick) {
            Vector2 lastTouchPosition = GetTouchPosition(0);
            if (lastTouchPosition.x >= 0.0f && lastTouchPosition.x < (float)screenWidth &&
                lastTouchPosition.y >= 0.0f && lastTouchPosition.y < (float)screenHeight) {
                menuPointer = lastTouchPosition;
            }
        }
#endif

        if (!exitConfirmation && mainMenu && !highScoreScreen && menuHasBeenShown) {
            if (IsKeyPressed(KEY_DOWN)) selectedMenuOption = (selectedMenuOption + 1) % menuOptionCount;
            if (IsKeyPressed(KEY_UP)) selectedMenuOption =
                (selectedMenuOption + menuOptionCount - 1) % menuOptionCount;

            int activatedOption = -1;
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
                activatedOption = selectedMenuOption;
            } else if (menuClick) {
                for (int i = 0; i < menuOptionCount; i++) {
                    Rectangle hitArea = menuButtons[i];
#if defined(PLATFORM_ANDROID)
                    hitArea.x = 0.0f;
                    hitArea.width = (float)screenWidth;
                    hitArea.y -= menuButtonGap * 0.5f;
                    hitArea.height += menuButtonGap;
#endif
                    if (CheckCollisionPointRec(menuPointer, hitArea)) {
                        activatedOption = i;
                        selectedMenuOption = i;
                        break;
                    }
                }
            }

            if (activatedOption == 0) {
                mainMenu = false;
            } else if (activatedOption == 1) {
                resetGameRequested = true;
                mainMenu = false;
                gameOver = false;
            } else if (activatedOption == 2) {
                highScoreScreen = true;
                mainMenu = false;
            } else if (activatedOption == 3) {
                quitGame = true;
            }
        } else if (!exitConfirmation && highScoreScreen && menuHasBeenShown &&
            (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) ||
             menuClick)) {
            Rectangle backHitArea = highScoreBackButton;
#if defined(PLATFORM_ANDROID)
            backHitArea.x = 0.0f;
            backHitArea.width = (float)screenWidth;
            backHitArea.y -= menuButtonGap * 0.5f;
            backHitArea.height += menuButtonGap;
#endif
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER) ||
                CheckCollisionPointRec(menuPointer, backHitArea)) {
                highScoreScreen = false;
                mainMenu = true;
            }
        }

        if (resetGameRequested) {
            score = 0;
            shipHealth = maxShipHealth;
            shipHealthBar = maxShipHealthBar;
            shipFuel = maxShipFuel;
            shipPositionOffset.x =
                (screenWidth - ship.width * display.scale) * 0.5f;
            thrusterPositionOffset.x = shipPositionOffset.x;
            collisionCooldown = 0.0f;
            plasmaFireCooldown = 0.0f;
            shipVelocity = 10.0f;
            activeGameplayTime = 0.0f;
            stationNumber = 1;
            stationActive = true;
            stationHealthRefilled = false;
            stationTextureIndex = GetRandomValue(0, 2);
            stationPosition = (Vector2){
                (screenWidth - stationTextures[stationTextureIndex].width * display.scale * stationScaleFactor) * 0.5f,
                screenHeight * 0.24f
            };
            meteorCount = 0;
            enemyCount = 4;
            yellowThrusterActive = false;
            blueThrusterActive = false;
            for (int i = 0; i < maxMeteorCount; i++) {
                meteor[i] = (Vector2){
                    (float)GetRandomValue(0, screenWidth - 1),
                    (float)GetRandomValue(0, screenHeight - 1)
                };
                meteorScale[i] = (float)GetRandomValue(90, 100) / 100.0f;
                meteorTextureIndex[i] = GetRandomValue(0, 7);
                meteorRotation[i] = (float)GetRandomValue(-180, 180);
                meteorRotationSpeed[i] = (float)GetRandomValue(-90, 90) / 2.0f;
                meteorActive[i] = false;
            }
            for (int i = 0; i < maxEnemyCount; i++) {
                enemies[i] = (Vector2){
                    (float)GetRandomValue(40, screenWidth - 40),
                    (float)GetRandomValue(-400, -80)
                };
                enemyScale[i] = (float)GetRandomValue(55, 85) / 100.0f;
                enemyTextureIndex[i] = GetRandomValue(0, 4);
                enemyShootTimer[i] = (float)GetRandomValue(8, 25) / 10.0f;
                enemyHorizontalSpeed[i] = (float)GetRandomValue(-55, 55);
                enemyActive[i] = i < enemyCount;
            }
            for (int i = 0; i < maxEnemyBolts; i++) enemyBoltLife[i] = 0.0f;
            for (int i = 0; i < maxPlasmaBolts; i++) plasmaBolts[i].life = 0.0f;
            gameOver = false;
            resetGameRequested = false;
        }

        if ((mainMenu || highScoreScreen) && !exitConfirmation) {
            DrawMenuScreen(screenWidth, screenHeight, highScoreScreen, highScore,
                selectedMenuOption, menuButtons, highScoreBackButton);
            menuHasBeenShown = true;
            continue;
        }

        bool gamePaused = exitConfirmation || gameOver || mainMenu || highScoreScreen;
        if (gamePaused) dt = 0.0f;

        if (collisionCooldown > 0.0f) collisionCooldown -= dt;
        if (plasmaFireCooldown > 0.0f) plasmaFireCooldown -= dt;
        if (!gamePaused) activeGameplayTime += dt;

        // The opening station remains fixed during a short approach period.
        // Afterwards it moves naturally with the surrounding environment.
        if (!gamePaused && stationActive && stationNumber == 1 && stationPosition.y > screenHeight + 80.0f) {
            stationActive = false;
            stationNumber++;
            meteorCount = (stationNumber - 1 > 5) ? 30 : (stationNumber - 1) * 6;
            enemyCount = (stationNumber < maxEnemyCount) ? stationNumber : maxEnemyCount;
        }
        if (!gamePaused && !stationActive && activeGameplayTime >= (float)((stationNumber - 1) * stationInterval)) {
            stationActive = true;
            stationTextureIndex = GetRandomValue(0, 2);
            stationPosition = (Vector2){
                (screenWidth - stationTextures[stationTextureIndex].width * display.scale * stationScaleFactor) * 0.5f,
                -stationTextures[stationTextureIndex].height * display.scale * stationScaleFactor
            };
            stationHealthRefilled = false;
        }
        if (!gamePaused && stationActive && stationNumber > 1) {
            if (stationPosition.y > screenHeight + 80.0f) {
                stationActive = false;
                stationNumber++;
                int completedStations = stationNumber - 1;
                meteorCount = (completedStations * 6 > 30) ? 30 : completedStations * 6;
                enemyCount = (completedStations < maxEnemyCount) ? completedStations : maxEnemyCount;
            }
        }

        Vector2 mouse = GetMousePosition();
        bool hover = CheckCollisionPointRec(mouse, quitButton);

        float panelWidth = (float)screenWidth * 0.82f;
        float panelHeight = (float)screenHeight * 0.38f;
        if (panelWidth > 620.0f) panelWidth = 620.0f;
        if (panelHeight > 320.0f) panelHeight = 320.0f;
        if (panelWidth < 330.0f && screenWidth > 330.0f) panelWidth = 330.0f;
        if (panelHeight < 210.0f) panelHeight = 210.0f;
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
        if (gameOver && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse, exitButton)) quitGame = true;
            if (CheckCollisionPointRec(mouse, stayButton)) {
                resetGameRequested = true;
                gameOver = false;
            }
        }

       

        AndroidControlsUpdate(&androidControls, dt, gamePaused, quitButton);
        float shipHorizontalInput = 0.0f;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) shipHorizontalInput -= 1.0f;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) shipHorizontalInput += 1.0f;
        if (!gamePaused) {
            shipPositionOffset.x += shipHorizontalInput * 420.0f * display.scale * dt;
            shipPositionOffset.x += androidControls.moveX * 1.35f;
            float shipMaxX = screenWidth - ship.width * display.scale;
            if (shipPositionOffset.x < 0.0f) shipPositionOffset.x = 0.0f;
            if (shipPositionOffset.x > shipMaxX) shipPositionOffset.x = shipMaxX;
            thrusterPositionOffset.x = shipPositionOffset.x;
        }
        yellowThrusterActive = !gamePaused &&
            (androidControls.speedUp || IsKeyDown(KEY_UP));
        blueThrusterActive = !gamePaused &&
            (androidControls.boost || IsKeyDown(KEY_LEFT_SHIFT));

        if (androidControls.available && androidControls.touchDown &&
            !androidControls.touchWasDown &&
            CheckCollisionPointRec(androidControls.lastTouchPosition, quitButton)) {
            exitConfirmation = true;
        }
        if (androidControls.available && exitConfirmation &&
            !androidControls.touchDown && androidControls.touchWasDown) {
            if (CheckCollisionPointRec(androidControls.touchPrevious, exitButton)) quitGame = true;
            if (CheckCollisionPointRec(androidControls.touchPrevious, stayButton)) exitConfirmation = false;
        }
        if (androidControls.available && gameOver &&
            !androidControls.touchDown && androidControls.touchWasDown) {
            if (CheckCollisionPointRec(androidControls.touchPrevious, exitButton)) quitGame = true;
            if (CheckCollisionPointRec(androidControls.touchPrevious, stayButton)) {
                resetGameRequested = true;
                gameOver = false;
            }
        }

        bool yellowThrusterRunning = !gamePaused && shipFuel > 0.0f &&
                                    yellowThrusterActive && !blueThrusterActive;
        if (yellowThrusterRunning && !IsSoundPlaying(thrusterSound)) PlaySound(thrusterSound);
        if (!yellowThrusterRunning && IsSoundPlaying(thrusterSound)) StopSound(thrusterSound);

        bool blueThrusterRunning = !gamePaused && blueThrusterActive && shipFuel > 0.0f;
        if (blueThrusterRunning && !IsSoundPlaying(boostThrusterSound)) PlaySound(boostThrusterSound);
        if (!blueThrusterRunning && IsSoundPlaying(boostThrusterSound)) StopSound(boostThrusterSound);

        if (!gamePaused) {
            if (blueThrusterActive && shipFuel > 0.0f) {
                shipFuel -= 2.0f * dt;
                shipVelocity += accel * dt * 2.0f;
            }
            else if (yellowThrusterActive && shipFuel > 0.0f) {
                shipFuel -= 1.0f * dt;
                shipVelocity += accel * dt;
            }
            if (shipFuel < 0.0f) shipFuel = 0.0f;

            if (IsKeyDown(KEY_SPACE)) {
                shipVelocity -= accel * 1.5f * dt;
                if (shipVelocity < 0.0f) shipVelocity = 0.0f;
            }

        }

        if (!gamePaused && plasmaFireCooldown <= 0.0f &&
            (IsKeyDown(KEY_X) || IsKeyDown(KEY_W) || androidControls.fireTapped ||
             (!androidControls.available && IsMouseButtonDown(MOUSE_BUTTON_LEFT)))) {
            for (int bolt = 0; bolt < maxPlasmaBolts; bolt++) {
                if (plasmaBolts[bolt].life <= 0.0f) {
                    float shipPixelScale = display.scale;
                    float muzzleX = shipPositionOffset.x + ship.width * shipPixelScale * 0.5f;
                    float muzzleY = shipPositionOffset.y + 2.0f * shipPixelScale;
                    plasmaBolts[bolt].position = (Vector2){ muzzleX, muzzleY };
                    plasmaBolts[bolt].velocity = (Vector2){ 0.0f, -720.0f * display.scale };
                    plasmaBolts[bolt].life = 1.6f;
                    PlaySound(plasmaSound);
                    break;
                }
            }
            plasmaFireCooldown = 0.10f;
        }
        for (int i = 0; i < maxPlasmaBolts; i++) {
            if (plasmaBolts[i].life > 0.0f) {
                plasmaBolts[i].position.x += plasmaBolts[i].velocity.x * dt;
                plasmaBolts[i].position.y += plasmaBolts[i].velocity.y * dt;
                plasmaBolts[i].life -= dt;
                if (plasmaBolts[i].position.y < -20.0f || plasmaBolts[i].life <= 0.0f)
                    plasmaBolts[i].life = 0.0f;
            }
        }

        for (int i = 0; i < maxMeteorCount; i++) meteorActive[i] = (i < meteorCount);

        float environmentDistance = gamePaused ? 0.0f : shipVelocity * dt;

        if (!gamePaused) {
            for (int i = 0; i < maxEnemyCount; i++) {
                enemyActive[i] = i < enemyCount && !stationActive;
                if (!enemyActive[i]) continue;

                enemies[i].y += (environmentDistance * 0.55f + 35.0f * dt);

                enemies[i].x += enemyHorizontalSpeed[i] * dt;
                if (fabsf(enemyHorizontalSpeed[i]) < 20.0f) {
                    enemyHorizontalSpeed[i] = (enemyHorizontalSpeed[i] < 0.0f ? -1.0f : 1.0f) * 35.0f;
                }
                if (enemies[i].x < 24.0f) {
                    enemies[i].x = 24.0f;
                    enemyHorizontalSpeed[i] = fabsf(enemyHorizontalSpeed[i]);
                }
                if (enemies[i].x > screenWidth - 24.0f) {
                    enemies[i].x = screenWidth - 24.0f;
                    enemyHorizontalSpeed[i] = -fabsf(enemyHorizontalSpeed[i]);
                }
                if (enemies[i].y > screenHeight + 80.0f) {
                    enemies[i].y = (float)GetRandomValue(-300, -60);
                    enemies[i].x = (float)GetRandomValue(40, screenWidth - 40);
                }

                enemyShootTimer[i] -= dt;
                if (enemyShootTimer[i] <= 0.0f) {
                    for (int bolt = 0; bolt < maxEnemyBolts; bolt++) {
                        if (enemyBoltLife[bolt] <= 0.0f) {
                            // Fire vertically from the enemy's current fixed lane.
                            // Velocity is assigned once and never follows the player.
                            enemyBolts[bolt] = (Vector2){ enemies[i].x, enemies[i].y + 35.0f * display.scale };
                            enemyBoltVelocity[bolt] = (Vector2){ 0.0f, 720.0f * display.scale };
                            enemyBoltLife[bolt] = 4.0f;
                            break;
                        }
                    }
                    enemyShootTimer[i] = (float)GetRandomValue(12, 28) / 10.0f;
                }
            }

            for (int i = 0; i < maxEnemyBolts; i++) {
                if (enemyBoltLife[i] <= 0.0f) continue;
                // Enemy bullets never acquire or update horizontal velocity after launch.
                enemyBolts[i].x += enemyBoltVelocity[i].x * dt;
                enemyBolts[i].y += enemyBoltVelocity[i].y * dt;
                enemyBoltLife[i] -= dt;
                if (enemyBolts[i].y > screenHeight + 20.0f) enemyBoltLife[i] = 0.0f;

                float shipWidth = ship.width * display.scale;
                float shipHeight = ship.height * display.scale;
                Rectangle playerBounds = {
                    shipPositionOffset.x + shipWidth * 0.1f,
                    shipPositionOffset.y + shipHeight * 0.1f,
                    shipWidth * 0.8f,
                    shipHeight * 0.8f
                };
                Rectangle boltBounds = {
                    enemyBolts[i].x - 3.0f * display.scale,
                    enemyBolts[i].y - 6.0f * display.scale,
                    6.0f * display.scale,
                    12.0f * display.scale
                };
                if (collisionCooldown <= 0.0f && CheckCollisionRecs(playerBounds, boltBounds)) {
                    enemyBoltLife[i] = 0.0f;
                    shipHealthBar -= 25.0f;
                    if (shipHealthBar <= 0.0f) {
                        shipHealthBar = 0.0f;
                        shipHealth--;
                        if (shipHealth > 0) shipHealthBar = maxShipHealthBar;
                        else gameOver = true;
                    }
                    collisionCooldown = 1.0f;
                }
            }
        }

        if (!gamePaused && stationActive && !stationHealthRefilled) {
            float stationScale = display.scale * stationScaleFactor;
            float stationWidth = stationTextures[stationTextureIndex].width * stationScale;
            float stationHeight = stationTextures[stationTextureIndex].height * stationScale;
            float stationCenterX = stationPosition.x + stationWidth * 0.5f;
            float stationCenterY = stationPosition.y + stationHeight * 0.5f;
            float shipCenterX = shipPositionOffset.x + ship.width * display.scale * 0.5f;
            float shipCenterY = shipPositionOffset.y + ship.height * display.scale * 0.5f;
            float horizontalDistance = fabsf(stationCenterX - shipCenterX);
            float verticalDistance = fabsf(stationCenterY - shipCenterY);
            if (horizontalDistance < (stationWidth + ship.width * display.scale) * 0.5f &&
                verticalDistance < (stationHeight + ship.height * display.scale) * 0.5f) {
                shipHealth = maxShipHealth;
                shipHealthBar = maxShipHealthBar;
                stationHealthRefilled = true;
            }
        }

        BeginDrawing();
            ClearBackground(BLACK);

           


            /////////////////////////////////////////////////////////////////////////////1st layer starsClusterLayer
            for (int i = 0; i < starCount; i++) {

                DrawTextureEx(farAmbience, starsClusterLayer[i], 0.0f, starScales[i], Fade(RED , 0.5f));

                if (!gamePaused) starsClusterLayer[i].y += environmentDistance * 0.2f;

                if (!gamePaused && starsClusterLayer[i].y >= screenHeight) starsClusterLayer[i].y = 0.0f;
                if (!gamePaused && starsClusterLayer[i].x < 0) starsClusterLayer[i].x = screenWidth;
                if (!gamePaused && starsClusterLayer[i].x > screenWidth) starsClusterLayer[i].x = 0.0f;

                if (!gamePaused && shipVelocity > 0.0f && IsKeyDown(KEY_A))starsClusterLayer[i].x += dt * speed * 0.2;
                if (!gamePaused && shipVelocity > 0.0f && IsKeyDown(KEY_D))starsClusterLayer[i].x -= dt * speed * 0.2;
             }

            ////////////////////////////////////////////////////////////////////////////////// 2nd layer starsClusterLayer
             for (int i = 0; i < starCount; i++) {

                DrawTextureEx(farAmbience, starsClusterLayer2[i], 0.0f, starScales[i], WHITE);

                if (!gamePaused) starsClusterLayer2[i].y += environmentDistance * 0.4f;

                if (!gamePaused && starsClusterLayer2[i].y >= screenHeight) starsClusterLayer2[i].y = 0.0f;
                if (!gamePaused && starsClusterLayer2[i].x < 0) starsClusterLayer2[i].x = screenWidth;
                if (!gamePaused && starsClusterLayer2[i].x > screenWidth) starsClusterLayer2[i].x = 0.0f;

                if (!gamePaused && shipVelocity > 0.0f && IsKeyDown(KEY_A))starsClusterLayer2[i].x += dt * speed * 0.4;
                if (!gamePaused && shipVelocity > 0.0f && IsKeyDown(KEY_D))starsClusterLayer2[i].x -= dt * speed * 0.4;
            }
            /////////////////////////////////////////////////////////////////////////////////////// 3rd  layer starsClusterLayer
            for (int i = 0; i < starCount; i++) {

                DrawTextureEx(farAmbience, starsClusterLayer3[i], 0.0f, starScales[i], WHITE);

                if (!gamePaused) starsClusterLayer3[i].y += environmentDistance * 0.6f;

                if (!gamePaused && starsClusterLayer3[i].y >= screenHeight) starsClusterLayer3[i].y = 0.0f;
                if (!gamePaused && starsClusterLayer3[i].x < 0) starsClusterLayer3[i].x = screenWidth;
                if (!gamePaused && starsClusterLayer3[i].x > screenWidth) starsClusterLayer3[i].x = 0.0f;

                if (!gamePaused && shipVelocity > 0.0f && IsKeyDown(KEY_A))starsClusterLayer3[i].x += dt * speed * 0.6;
                if (!gamePaused && shipVelocity > 0.0f && IsKeyDown(KEY_D))starsClusterLayer3[i].x -= dt * speed * 0.6;
            }

            ///////////////////////////////////////////////////////////////////////////////////////// Meteor layer
            for (int i = 0; i < meteorCount; i++) {
                if (!meteorActive[i]) continue;

                Texture2D meteorTexture = meteorTextures[meteorTextureIndex[i]];
                if (!gamePaused) meteorRotation[i] += meteorRotationSpeed[i] * dt;
                DrawTextureEx(meteorTexture, meteor[i], meteorRotation[i], meteorScale[i], GRAY);

                if (!gamePaused) meteor[i].y += environmentDistance;
                if (!gamePaused && meteor[i].y > screenHeight + 64) {meteor[i].y = -64.0f; meteor[i].x = (float)GetRandomValue(0, screenWidth - 1);}
                if (!gamePaused && meteor[i].x <= -64) {meteor[i].x = screenWidth; meteor[i].y = (float)GetRandomValue(0, screenHeight - 1);}
                if (!gamePaused && meteor[i].x > screenWidth) meteor[i].x = -64.0f;

                const float shipCollisionInset = 0.10f;
                float shipWidth = ship.width * display.scale;
                float shipHeight = ship.height * display.scale;
                Rectangle shipBounds = {
                    shipPositionOffset.x + shipWidth * shipCollisionInset,
                    shipPositionOffset.y + shipHeight * shipCollisionInset,
                    shipWidth * (1.0f - shipCollisionInset * 2.0f),
                    shipHeight * (1.0f - shipCollisionInset * 2.0f)
                };

                // DrawTextureEx rotates around the meteor's top-left corner.
                // Rotate the center point by the same angle so collision stays
                // attached to the visible sprite at every angle.
                float meteorWidth = meteorTexture.width * meteorScale[i];
                float meteorHeight = meteorTexture.height * meteorScale[i];
                float meteorAngleRadians = meteorRotation[i] * DEG2RAD;
                float meteorCenterOffsetX = meteorWidth * 0.5f;
                float meteorCenterOffsetY = meteorHeight * 0.5f;
                float cosAngle = cosf(meteorAngleRadians);
                float sinAngle = sinf(meteorAngleRadians);
                Vector2 meteorCenter = {
                    meteor[i].x + meteorCenterOffsetX * cosAngle - meteorCenterOffsetY * sinAngle,
                    meteor[i].y + meteorCenterOffsetX * sinAngle + meteorCenterOffsetY * cosAngle
                };
                float meteorCollisionRadius = meteorWidth * 0.30f;
                if (meteorHeight < meteorWidth) meteorCollisionRadius = meteorHeight * 0.30f;

                for (int bolt = 0; bolt < maxPlasmaBolts; bolt++) {
                    if (plasmaBolts[bolt].life > 0.0f &&
                        CheckCollisionCircleRec(meteorCenter, meteorCollisionRadius,
                            (Rectangle){ plasmaBolts[bolt].position.x - 3.0f * display.scale,
                                         plasmaBolts[bolt].position.y - 6.0f * display.scale,
                                         6.0f * display.scale, 12.0f * display.scale })) {
                        plasmaBolts[bolt].life = 0.0f;
                        PlaySound(meteorExplosionSound);
                        shipFuel += 2.0f;
                        if (shipFuel > maxShipFuel) shipFuel = maxShipFuel;
                        meteor[i].y = -64.0f;
                        meteor[i].x = (float)GetRandomValue(0, screenWidth - 1);
                        meteorTextureIndex[i] = GetRandomValue(0, 7);
                        meteorRotation[i] = (float)GetRandomValue(-180, 180);
                        meteorRotationSpeed[i] = (float)GetRandomValue(-90, 90) / 2.0f;
                        break;
                    }
                }

                if (!gamePaused && (shipHealth > 0) && (collisionCooldown <= 0.0f) &&
                    CheckCollisionCircleRec(meteorCenter, meteorCollisionRadius, shipBounds)) {
                    int meteorHitsRequired;
                    if (meteorScale[i] >= 0.98f) meteorHitsRequired = 2;
                    else if (meteorScale[i] >= 0.96f) meteorHitsRequired = 3;
                    else if (meteorScale[i] >= 0.93f) meteorHitsRequired = 4;
                    else meteorHitsRequired = 5;
                    shipHealthBar -= maxShipHealthBar / (float)meteorHitsRequired;
                    if (shipHealthBar <= 0.0f) {
                        shipHealthBar = 0.0f;
                        shipHealth--;
                        if (shipHealth > 0) shipHealthBar = maxShipHealthBar;
                        else gameOver = true;
                    }
                    collisionCooldown = 1.0f;
                    meteor[i].y = -64.0f;
                    meteor[i].x = (float)GetRandomValue(0, screenWidth - 1);
                    meteorTextureIndex[i] = GetRandomValue(0, 7);
                    meteorRotation[i] = (float)GetRandomValue(-180, 180);
                    meteorRotationSpeed[i] = (float)GetRandomValue(-90, 90) / 2.0f;
                }
             }

            for (int i = 0; i < maxEnemyCount; i++) {
                if (!enemyActive[i]) continue;
                Texture2D enemyTexture = enemyTextures[enemyTextureIndex[i]];
                float enemyWidth = enemyTexture.width * enemyScale[i] * display.scale;
                float enemyHeight = enemyTexture.height * enemyScale[i] * display.scale;
                DrawTextureEx(enemyTexture,
                    (Vector2){ enemies[i].x - enemyWidth * 0.5f, enemies[i].y },
                    180.0f, enemyScale[i] * display.scale, WHITE);

                Rectangle enemyBounds = {
                    enemies[i].x - enemyWidth * 0.5f,
                    enemies[i].y,
                    enemyWidth,
                    enemyHeight
                };
                for (int bolt = 0; bolt < maxPlasmaBolts; bolt++) {
                    if (plasmaBolts[bolt].life <= 0.0f) continue;
                    Rectangle boltBounds = {
                        plasmaBolts[bolt].position.x - 3.0f * display.scale,
                        plasmaBolts[bolt].position.y - 8.0f * display.scale,
                        6.0f * display.scale,
                        16.0f * display.scale
                    };
                    if (!gamePaused && CheckCollisionRecs(enemyBounds, boltBounds)) {
                        plasmaBolts[bolt].life = 0.0f;
                        score += 100;
                        if (score > highScore) {
                            highScore = score;
                            SaveHighScore(highScore);
                        }
                        enemies[i].y = (float)GetRandomValue(-300, -60);
                        enemies[i].x = (float)GetRandomValue(40, screenWidth - 40);
                        enemyTextureIndex[i] = GetRandomValue(0, 4);
                        PlaySound(meteorExplosionSound);
                        break;
                    }
                }
            }

            for (int i = 0; i < maxEnemyBolts; i++) {
                if (enemyBoltLife[i] > 0.0f) {
                    float pixel = (display.scale < 1.0f) ? 1.0f : display.scale;
                    DrawRectangle((int)(enemyBolts[i].x - pixel), (int)(enemyBolts[i].y - 6.0f * pixel),
                        (int)(3.0f * pixel), (int)(12.0f * pixel), RED);
                }
            }

            if (stationActive) {
                float stationScale = display.scale * stationScaleFactor;
                if (!gamePaused) {
                    stationPosition.y += environmentDistance;
                    float stationWidth = stationTextures[stationTextureIndex].width * stationScale;
                    if (stationPosition.x < -stationWidth) stationPosition.x = screenWidth;
                    if (stationPosition.x > screenWidth) stationPosition.x = -stationWidth;
                }
                DrawTextureEx(stationTextures[stationTextureIndex], stationPosition, 0.0f, stationScale, WHITE);
            }

            for (int i = 0; i < maxPlasmaBolts; i++) {
                if (plasmaBolts[i].life <= 0.0f) continue;
                float pixel = (display.scale < 1.0f) ? 1.0f : display.scale;
                DrawRectangle((int)(plasmaBolts[i].position.x - pixel),
                              (int)(plasmaBolts[i].position.y - 6.0f * pixel),
                              (int)(2.0f * pixel), (int)(12.0f * pixel), SKYBLUE);
                DrawRectangle((int)(plasmaBolts[i].position.x),
                              (int)(plasmaBolts[i].position.y - 8.0f * pixel),
                              (int)(pixel), (int)(16.0f * pixel), WHITE);
            }

            // star ship
            DrawTextureEx(ship, shipPositionOffset, 0.0f, display.scale, WHITE);
            // ship thruster
            if (blueThrusterActive) {
                DrawTextureEx(thrusterBoost, thrusterPositionOffset, 0.0f, display.scale, WHITE);
            }
            else if (shipVelocity > 0.0f && shipFuel > 0.0f) {
                DrawTextureEx(thruster, thrusterPositionOffset, 0.0f, display.scale, WHITE);
            }

            /////////////////////////////////////////////////////////////////////////////////// Telemetry
            const int hudX = 10;
            const float healthBarWidth = 180.0f;
            const float healthBarHeight = 16.0f;
            const char *scoreText = TextFormat("SCORE: %d", score);
            DrawText(scoreText,
                screenWidth - MeasureText(scoreText, 22) - 16, display.quitSize + 8, 22, GOLD);
            DrawRectangle(hudX, 10, (int)healthBarWidth, (int)healthBarHeight, DARKGRAY);
            DrawRectangle(hudX, 10, (int)(healthBarWidth * (shipHealthBar / maxShipHealthBar)), (int)healthBarHeight,
                shipHealthBar > 50.0f ? GREEN : (shipHealthBar > 25.0f ? ORANGE : RED));
            DrawRectangleLines(hudX, 10, (int)healthBarWidth, (int)healthBarHeight, WHITE);
            DrawText("HEALTH", hudX + 184, 10, 16, WHITE);

            int fuelY = 30;
            DrawRectangle(hudX, fuelY, (int)healthBarWidth, (int)healthBarHeight, DARKGRAY);
            DrawRectangle(hudX, fuelY, (int)(healthBarWidth * shipFuel / maxShipFuel), (int)healthBarHeight, SKYBLUE);
            DrawRectangleLines(hudX, fuelY, (int)healthBarWidth, (int)healthBarHeight, WHITE);
            DrawText("FUEL", hudX + 184, fuelY, 16, WHITE);

            for (int i = 0; i < maxShipHealth; i++){
                Vector2 healthPosition = { (float)(i * 32 + 2), 70.0f };
                if (i < shipHealth) {
                    DrawTextureEx(health, healthPosition, 0.0f, 0.5f, WHITE);
                }
            }

            AndroidControlsDraw(&androidControls, gamePaused);

            if (exitConfirmation || (gameOver && !mainMenu)) {
                DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.65f));
                DrawRectangleRec(exitPanel, DARKGRAY);
                DrawRectangleLinesEx(exitPanel, 2.0f, LIGHTGRAY);

                const char *exitText = gameOver ? "Game over! Play again?" : "Are you sure you want to exit?";
                const char *secondaryButtonText = gameOver ? "RESTART" : "STAY";
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
                DrawText(secondaryButtonText, (int)(stayButton.x + (stayButton.width - MeasureText(secondaryButtonText, 20)) * 0.5f),
                    (int)(stayButton.y + 14.0f), 20, WHITE);
            }
        

        EndDrawing();
    }

    UnloadTexture(ship);
    UnloadTexture(thruster);
    UnloadTexture(thrusterBoost);
    UnloadTexture(farAmbience);
    UnloadSound(plasmaSound);
    UnloadSound(thrusterSound);
    UnloadSound(boostThrusterSound);
    UnloadSound(meteorExplosionSound);
    for (int i = 0; i < 8; i++) {
        UnloadTexture(meteorTextures[i]);
    }
    UnloadTexture(health);
    for (int i = 0; i < 5; i++) UnloadTexture(enemyTextures[i]);
    for (int i = 0; i < 3; i++) UnloadTexture(stationTextures[i]);
    CloseAudioDevice();
    CloseWindow();
    return 0;

}