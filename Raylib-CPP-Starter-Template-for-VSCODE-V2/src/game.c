#include "game.h"
#include <math.h>

/* ============================================================
 *  Background asset
 * ============================================================ */

static Texture2D g_backgroundTex = { 0 };
static bool      g_backgroundLoaded = false;

static void LoadBackgroundAsset(void) {
    if (g_backgroundLoaded) return;

    g_backgroundTex = LoadTexture("assets/background.png");

    if (g_backgroundTex.id == 0) {
        Image img = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, MAGENTA);
        g_backgroundTex = LoadTextureFromImage(img);
        UnloadImage(img);
    }

    g_backgroundLoaded = true;
}

static void UnloadBackgroundAsset(void) {
    if (g_backgroundLoaded && g_backgroundTex.id != 0) {
        UnloadTexture(g_backgroundTex);
        g_backgroundTex.id = 0;
        g_backgroundLoaded = false;
    }
}

/* ============================================================
 *  Animated overlay
 * ============================================================ */

static void DrawAnimatedOverlay(void) {
    float t = (float)GetTime();

    for (int i = 0; i < 90; i++) {
        int   x     = (i * 137) % SCREEN_WIDTH;
        int   y     = (i * 79)  % (SCREEN_HEIGHT / 2);
        float phase = (float)(i * 13);
        float tw    = 0.5f + 0.5f * sinf(t * 2.0f + phase);

        int alpha = 80 + (int)(tw * 175);
        float r   = 0.8f + (i % 3) * 0.5f;

        DrawCircle(x, y, r, (Color){ 255, 255, 255, (unsigned char)alpha });
    }

    for (int i = 0; i < 40; i++) {
        float sx = (float)((i * 97)  % SCREEN_WIDTH);
        float sy = fmodf((float)(i * 53) + t * 18.0f, (float)SCREEN_HEIGHT);
        DrawCircle(sx, sy, 1.2f, (Color){ 255, 255, 255, 100 });
    }

    for (int i = 0; i < 25; i++) {
        float sx = (float)((i * 173) % SCREEN_WIDTH);
        float sy = fmodf((float)(i * 91) + t * 42.0f, (float)SCREEN_HEIGHT);
        DrawCircle(sx, sy, 2.2f, (Color){ 255, 255, 255, 190 });
    }

    DrawRectangleGradientV(0, 0, SCREEN_WIDTH, 90,
                           (Color){ 0, 0, 0, 80 }, (Color){ 0, 0, 0, 0 });
    DrawRectangleGradientV(0, SCREEN_HEIGHT - 120, SCREEN_WIDTH, 120,
                           (Color){ 0, 0, 0, 0 }, (Color){ 0, 0, 0, 120 });
}

static void DrawBackground(void) {
    Rectangle src = { 0, 0,
                      (float)g_backgroundTex.width,
                      (float)g_backgroundTex.height };
    Rectangle dst = { 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };

    DrawTexturePro(g_backgroundTex, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
    DrawAnimatedOverlay();
}

/* ============================================================
 *  HUD
 * ============================================================ */

static void DrawHUD(const Game *game) {
    DrawText(TextFormat("SCORE: %d", game->score),
             20, 20, 30, (Color){ 232, 245, 255, 255 });

    if (!game->active) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                      (Color){ 0, 0, 0, 150 });

        const char *over    = "GAME OVER";
        const char *restart = "Press R to Restart";
        const char *menu    = "Press M for Menu";

        DrawText(over,
                 SCREEN_WIDTH / 2 - MeasureText(over, 60) / 2,
                 SCREEN_HEIGHT / 2 - 60, 60,
                 (Color){ 255, 255, 255, 255 });

        DrawText(restart,
                 SCREEN_WIDTH / 2 - MeasureText(restart, 30) / 2,
                 SCREEN_HEIGHT / 2 + 20, 30,
                 (Color){ 200, 220, 240, 255 });

        DrawText(menu,
                 SCREEN_WIDTH / 2 - MeasureText(menu, 30) / 2,
                 SCREEN_HEIGHT / 2 + 60, 30,
                 (Color){ 200, 220, 240, 255 });
    }
}

/* ============================================================
 *  Menu
 * ============================================================ */

typedef enum MenuItem {
    MENU_PLAY = 0,
    MENU_CONTROLS,
    MENU_QUIT,
    MENU_COUNT
} MenuItem;

static const char *MENU_LABELS[MENU_COUNT] = { "PLAY", "CONTROLS", "QUIT" };

#define MENU_ITEM_W       240
#define MENU_ITEM_H       44
#define MENU_ITEM_SPACING 58
#define MENU_FIRST_Y      250

static Rectangle MenuItemRect(int i) {
    return (Rectangle){
        (float)(SCREEN_WIDTH / 2 - MENU_ITEM_W / 2),
        (float)(MENU_FIRST_Y + i * MENU_ITEM_SPACING),
        (float)MENU_ITEM_W,
        (float)MENU_ITEM_H
    };
}

static void UpdateMenu(Game *game) {
    /* Keyboard navigation */
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        game->menuSelection = (game->menuSelection + 1) % MENU_COUNT;
    }
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        game->menuSelection = (game->menuSelection + MENU_COUNT - 1) % MENU_COUNT;
    }

    /* Mouse hover (only when the mouse actually moves, so it doesn't
       fight with keyboard navigation) */
    Vector2 mouse   = GetMousePosition();
    Vector2 delta   = GetMouseDelta();
    bool    moved   = (delta.x != 0.0f || delta.y != 0.0f);
    bool    hovered = false;

    for (int i = 0; i < MENU_COUNT; i++) {
        if (CheckCollisionPointRec(mouse, MenuItemRect(i))) {
            if (moved) game->menuSelection = i;
            if (game->menuSelection == i) hovered = true;
        }
    }

    bool activate = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                    (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT));

    if (!activate) return;

    switch (game->menuSelection) {
        case MENU_PLAY:
            Game_Init(game);                 /* fresh run, sets state to PLAYING */
            break;
        case MENU_CONTROLS:
            game->state = GAME_STATE_CONTROLS;
            break;
        case MENU_QUIT:
            game->quitRequested = true;
            break;
    }
}

static void DrawMenu(const Game *game) {
    float t = (float)GetTime();

    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 90 });

    /* Title with a gentle bob */
    const char *title    = "JACK FROST";
    const char *subtitle = "Freeze them all";
    int   titleSize = 72;
    float bob       = sinf(t * 2.0f) * 4.0f;
    int   titleX    = SCREEN_WIDTH / 2 - MeasureText(title, titleSize) / 2;
    int   titleY    = 90 + (int)bob;

    DrawText(title, titleX + 3, titleY + 3, titleSize, (Color){ 20, 50, 90, 200 });
    DrawText(title, titleX,     titleY,     titleSize, (Color){ 232, 245, 255, 255 });

    DrawText(subtitle,
             SCREEN_WIDTH / 2 - MeasureText(subtitle, 22) / 2,
             titleY + titleSize + 6, 22, (Color){ 170, 210, 240, 255 });

    /* Buttons */
    for (int i = 0; i < MENU_COUNT; i++) {
        Rectangle r        = MenuItemRect(i);
        bool      selected = (game->menuSelection == i);

        Color fill   = selected ? (Color){ 120, 190, 240, 230 }
                                : (Color){ 20, 40, 70, 170 };
        Color border = selected ? (Color){ 255, 255, 255, 255 }
                                : (Color){ 130, 170, 210, 200 };
        Color text   = selected ? (Color){ 10, 30, 60, 255 }
                                : (Color){ 232, 245, 255, 255 };

        DrawRectangleRec(r, fill);
        DrawRectangleLinesEx(r, 2.0f, border);

        int fontSize = 26;
        DrawText(MENU_LABELS[i],
                 (int)(r.x + r.width / 2) - MeasureText(MENU_LABELS[i], fontSize) / 2,
                 (int)(r.y + (r.height - fontSize) / 2),
                 fontSize, text);
    }

    const char *hint = "Arrows / W,S to move  -  Enter to select";
    DrawText(hint,
             SCREEN_WIDTH / 2 - MeasureText(hint, 16) / 2,
             SCREEN_HEIGHT - 30, 16, (Color){ 200, 220, 240, 200 });
}

/* ============================================================
 *  Controls screen
 * ============================================================ */

static void UpdateControls(Game *game) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) ||
        IsKeyPressed(KEY_SPACE)  || IsKeyPressed(KEY_BACKSPACE) ||
        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game->state = GAME_STATE_MENU;
    }
}

static void DrawControls(void) {
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 150 });

    const char *title = "CONTROLS";
    DrawText(title,
             SCREEN_WIDTH / 2 - MeasureText(title, 48) / 2,
             70, 48, (Color){ 232, 245, 255, 255 });

    /* TODO: adjust these to match the keys used in Jack_Update() */
    const char *lines[] = {
        "Move:   A / D  or  Left / Right",
        "Jump:   W / Space  or  Up",
        "Shoot:  Left Mouse Button",
        "Stun enemies with shards - don't touch them!"
    };
    int count = (int)(sizeof(lines) / sizeof(lines[0]));

    for (int i = 0; i < count; i++) {
        DrawText(lines[i],
                 SCREEN_WIDTH / 2 - MeasureText(lines[i], 24) / 2,
                 170 + i * 44, 24, (Color){ 200, 225, 245, 255 });
    }

    const char *back = "Press Esc or click to go back";
    DrawText(back,
             SCREEN_WIDTH / 2 - MeasureText(back, 18) / 2,
             SCREEN_HEIGHT - 50, 18, (Color){ 170, 210, 240, 220 });
}

/* ============================================================
 *  Level setup
 * ============================================================ */

static void BuildLevel(Game *game) {
    Platform_InitAll(game->platforms);

    Platform_Add(game->platforms, (Rectangle){   0, 440, 800, 40 });
    Platform_Add(game->platforms, (Rectangle){  80, 360, 200, 24 });
    Platform_Add(game->platforms, (Rectangle){ 520, 360, 200, 24 });
    Platform_Add(game->platforms, (Rectangle){ 300, 280, 200, 24 });
    Platform_Add(game->platforms, (Rectangle){  40, 200, 160, 24 });
    Platform_Add(game->platforms, (Rectangle){ 600, 200, 160, 24 });
    Platform_Add(game->platforms, (Rectangle){ 320, 120, 160, 24 });
}

/* ============================================================
 *  Enemy placement - all enemies spawn once at level start
 * ============================================================ */

static void SpawnAllEnemies(Game *game) {
    /* Skip the ground floor (index 0) so enemies start on raised platforms.
       One enemy per platform from index 1 onward. */
    for (int i = 1; i < MAX_PLATFORMS; i++) {
        if (!game->platforms[i].active) continue;
        Enemy_SpawnOnPlatform(game->enemies, game->platforms[i].bounds);
    }
}

/* ============================================================
 *  Collision helpers
 * ============================================================ */

static void ResolveJackPlatformCollisions(Game *game) {
    for (int i = 0; i < MAX_PLATFORMS; i++) {
        if (!game->platforms[i].active) continue;

        Rectangle p  = game->platforms[i].bounds;
        Rectangle jr = {
            game->jack.position.x - JACK_RADIUS,
            game->jack.position.y - JACK_RADIUS,
            JACK_RADIUS * 2,
            JACK_RADIUS * 2
        };

        if (CheckCollisionRecs(jr, p)) {
            float overlapTop    = (p.y + p.height) - jr.y;
            float overlapBottom = (jr.y + jr.height) - p.y;
            float overlapLeft   = (jr.x + jr.width)  - p.x;
            float overlapRight  = (p.x + p.width)    - jr.x;

            float minX = (overlapLeft < overlapRight) ? overlapLeft : -overlapRight;
            float minY = (overlapTop  < overlapBottom) ? overlapTop  : -overlapBottom;

            if (fabsf(minX) < fabsf(minY)) {
                game->jack.position.x += minX;
            } else {
                game->jack.position.y += minY;
            }
        }
    }
}

static void ResolveShardPlatformCollisions(Game *game) {
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (!game->shards[i].active) continue;

        Rectangle sr = {
            game->shards[i].position.x - SHARD_RADIUS,
            game->shards[i].position.y - SHARD_RADIUS,
            SHARD_RADIUS * 2,
            SHARD_RADIUS * 2
        };

        for (int j = 0; j < MAX_PLATFORMS; j++) {
            if (!game->platforms[j].active) continue;

            if (CheckCollisionRecs(sr, game->platforms[j].bounds)) {
                game->shards[i].active = false;
                break;
            }
        }
    }
}

static void ResolveShardEnemyCollisions(Game *game) {
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (!game->shards[i].active) continue;

        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!game->enemies[j].active) continue;

            Rectangle er = Enemy_GetRect(&game->enemies[j]);
            if (CheckCollisionCircleRec(game->shards[i].position, SHARD_RADIUS, er)) {
                /* Consume the shard; stun (do NOT kill) the enemy */
                game->shards[i].active = false;

                if (Enemy_Stun(&game->enemies[j])) {
                    game->score += 10;   /* reward still given on stun */
                }
                break;
            }
        }
    }
}

/* ============================================================
 *  Public API
 * ============================================================ */

void Game_Init(Game *game) {
    Jack_Init(&game->jack);
    Shard_InitAll(game->shards);
    Enemy_InitAll(game->enemies);

    BuildLevel(game);
    SpawnAllEnemies(game);

    game->score        = 0;
    game->active       = true;
    game->frameCounter = 0.0f;
    game->state        = GAME_STATE_PLAYING;
}

void Game_Restart(Game *game) {
    Game_Init(game);
}

void Game_Update(Game *game) {
    if (!game->active) return;

    /* 1. Player */
    Jack_Update(&game->jack, game->platforms);
    ResolveJackPlatformCollisions(game);

    Shard_UpdateAll(game->shards, SCREEN_WIDTH);
    ResolveShardPlatformCollisions(game);

    Rectangle playerRect = {
        game->jack.position.x - JACK_RADIUS * 0.75f,
        game->jack.position.y - JACK_RADIUS * 0.75f,
        JACK_RADIUS * 1.5f,
        JACK_RADIUS * 1.5f
    };

    if (Enemy_UpdateAll(game->enemies, playerRect)) {
        game->active = false;
        return;
    }

    ResolveShardEnemyCollisions(game);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Shard_Shoot(game->shards, game->jack.position);
    }
}

void Game_Draw(const Game *game) {
    DrawBackground();

    switch (game->state) {
        case GAME_STATE_MENU:
            DrawMenu(game);
            break;

        case GAME_STATE_CONTROLS:
            DrawControls();
            break;

        case GAME_STATE_PLAYING:
            Platform_DrawAll(game->platforms);
            Shard_DrawAll(game->shards);
            Enemy_DrawAll(game->enemies);
            Jack_Draw(&game->jack);
            DrawHUD(game);
            break;
    }
}

/* ============================================================
 *  Application lifecycle
 * ============================================================ */

void Game_Run(Game *game) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);

    /* Esc is used to go back to the menu, so don't let it close the window */
    SetExitKey(KEY_NULL);

    LoadBackgroundAsset();
    Game_Init(game);

    /* Start on the menu instead of jumping straight into play */
    game->state         = GAME_STATE_MENU;
    game->menuSelection = MENU_PLAY;
    game->quitRequested = false;

    while (!WindowShouldClose() && !game->quitRequested) {
        switch (game->state) {
            case GAME_STATE_MENU:
                UpdateMenu(game);
                break;

            case GAME_STATE_CONTROLS:
                UpdateControls(game);
                break;

            case GAME_STATE_PLAYING:
                if (IsKeyPressed(KEY_ESCAPE)) {
                    game->state = GAME_STATE_MENU;
                } else if (game->active) {
                    Game_Update(game);
                } else if (IsKeyPressed(KEY_R)) {
                    Game_Restart(game);
                } else if (IsKeyPressed(KEY_M)) {
                    game->state = GAME_STATE_MENU;
                }
                break;
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        Game_Draw(game);
        EndDrawing();
    }

    UnloadBackgroundAsset();
    CloseWindow();
}
