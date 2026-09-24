#include "game.h"
#include <math.h>

/* ============================================================
 *  Internal drawing helpers
 * ============================================================ */

static void DrawBackground(void) {
    DrawRectangleGradientV(
        0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
        (Color){ 194, 226, 255, 255 },
        (Color){  77, 126, 168, 255 }
    );

    // Static snowflakes
    for (int i = 0; i < 70; i++) {
        int   x = (i * 37) % SCREEN_WIDTH;
        int   y = (i * 53) % SCREEN_HEIGHT;
        float r = 1.5f + (i % 3);
        DrawCircle(x, y, r, (Color){ 255, 255, 255, 180 });
    }
}

static void DrawHUD(const Game *game) {
    DrawText(TextFormat("SCORE: %d", game->score),
             20, 20, 30, (Color){ 232, 245, 255, 255 });

    if (!game->active) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                      (Color){ 0, 0, 0, 150 });

        const char *over    = "GAME OVER";
        const char *restart = "Press R to Restart";

        DrawText(over,
                 SCREEN_WIDTH / 2 - MeasureText(over, 60) / 2,
                 SCREEN_HEIGHT / 2 - 60, 60,
                 (Color){ 255, 255, 255, 255 });

        DrawText(restart,
                 SCREEN_WIDTH / 2 - MeasureText(restart, 30) / 2,
                 SCREEN_HEIGHT / 2 + 20, 30,
                 (Color){ 200, 220, 240, 255 });
    }
}

/* ============================================================
 *  Level setup
 * ============================================================ */

static void BuildLevel(Game *game) {
    Platform_InitAll(game->platforms);

    // --- Ground floor (spans bottom, leaving a small gutter) ---
    Platform_Add(game->platforms, (Rectangle){   0, 440, 800, 40 });

    // --- Mid-low ledges ---
    Platform_Add(game->platforms, (Rectangle){  80, 360, 200, 24 });
    Platform_Add(game->platforms, (Rectangle){ 520, 360, 200, 24 });

    // --- Mid-high ledges ---
    Platform_Add(game->platforms, (Rectangle){ 300, 280, 200, 24 });

    // --- Upper ledges ---
    Platform_Add(game->platforms, (Rectangle){  40, 200, 160, 24 });
    Platform_Add(game->platforms, (Rectangle){ 600, 200, 160, 24 });

    // --- Top shelf ---
    Platform_Add(game->platforms, (Rectangle){ 320, 120, 160, 24 });
}

/* ============================================================
 *  Internal update helpers
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
            // Push out along the shortest axis
            float overlapTop    = (p.y + p.height) - jr.y;             // push down
            float overlapBottom = (jr.y + jr.height) - p.y;            // push up
            float overlapLeft   = (jr.x + jr.width)  - p.x;            // push right
            float overlapRight  = (p.x + p.width)    - jr.x;           // push left

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

            float dx = game->shards[i].position.x - game->enemies[j].position.x;
            float dy = game->shards[i].position.y - game->enemies[j].position.y;

            if (sqrtf(dx * dx + dy * dy) < SHARD_RADIUS + ENEMY_RADIUS) {
                game->enemies[j].active = false;
                game->shards[i].active  = false;
                game->score += 10;
                break;
            }
        }
    }
}

static void HandleSpawning(Game *game) {
    game->frameCounter += GetFrameTime() * 60.0f;

    int rate = 46 - (game->score / 70);
    if (rate < 22) rate = 22;

    if ((int)game->frameCounter % rate == 0) {
        Enemy_Spawn(game->enemies, SCREEN_WIDTH, SCREEN_HEIGHT);

        if (game->score > 180 && GetRandomValue(0, 100) < 35) {
            Enemy_Spawn(game->enemies, SCREEN_WIDTH, SCREEN_HEIGHT);
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

    game->score        = 0;
    game->active       = true;
    game->frameCounter = 0.0f;
}

void Game_Restart(Game *game) {
    Game_Init(game);
}

void Game_Update(Game *game) {
    if (!game->active) return;

    // 1. Player
    Jack_Update(&game->jack, game->platforms);
    ResolveJackPlatformCollisions(game);

    // 2. Shards
    Shard_UpdateAll(game->shards, SCREEN_WIDTH);
    ResolveShardPlatformCollisions(game);

    // 3. Enemies + collision with player
    if (Enemy_UpdateAll(game->enemies, game->jack.position, JACK_RADIUS)) {
        game->active = false;
        return;
    }

    // 4. Shard vs Enemy collisions
    ResolveShardEnemyCollisions(game);

    // 5. Spawning
    HandleSpawning(game);

    // 6. Shooting
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Shard_Shoot(game->shards, game->jack.position);
    }
}

void Game_Draw(const Game *game) {
    DrawBackground();
    Platform_DrawAll(game->platforms);
    Shard_DrawAll(game->shards);
    Enemy_DrawAll(game->enemies);
    Jack_Draw(&game->jack);
    DrawHUD(game);
}

/* ============================================================
 *  The entire application lifecycle lives here
 * ============================================================ */

void Game_Run(Game *game) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);
    Game_Init(game);

    while (!WindowShouldClose()) {
        if (game->active) {
            Game_Update(game);
        } else if (IsKeyPressed(KEY_R)) {
            Game_Restart(game);
        }

        BeginDrawing();
        ClearBackground(RAYWHITE);
        Game_Draw(game);
        EndDrawing();
    }

    CloseWindow();
}