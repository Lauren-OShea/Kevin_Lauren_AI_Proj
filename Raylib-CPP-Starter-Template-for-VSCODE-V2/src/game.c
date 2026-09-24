#include "game.h"
#include <math.h>

/* ============================================================
 *  Background asset
 * ============================================================ */

static Texture2D g_backgroundTex    = { 0 };
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

    Platform_Add(game->platforms, (Rectangle){   0, 440, 800, 40 });
    Platform_Add(game->platforms, (Rectangle){  80, 360, 200, 24 });
    Platform_Add(game->platforms, (Rectangle){ 520, 360, 200, 24 });
    Platform_Add(game->platforms, (Rectangle){ 300, 280, 200, 24 });
    Platform_Add(game->platforms, (Rectangle){  40, 200, 160, 24 });
    Platform_Add(game->platforms, (Rectangle){ 600, 200, 160, 24 });
    Platform_Add(game->platforms, (Rectangle){ 320, 120, 160, 24 });
}

/* ============================================================
 *  Enemy placement — all enemies spawn once at level start
 * ============================================================ */

static void SpawnAllEnemies(Game *game) {
    /* Skip ground floor (index 0). Each raised platform gets a random type. */
    for (int i = 1; i < MAX_PLATFORMS; i++) {
        if (!game->platforms[i].active) continue;

        /* ~40% chance of a shooter, 60% walker */
        bool useShooter = (GetRandomValue(0, 99) < 40);

        if (useShooter) {
            Enemy_SpawnShooterOnPlatform(game->enemies, game->platforms[i].bounds);
        } else {
            Enemy_SpawnOnPlatform(game->enemies, game->platforms[i].bounds);
        }
    }
}

/* ============================================================
 *  Collision helpers
 * ============================================================ */

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
                game->shards[i].active = false;
                if (Enemy_Stun(&game->enemies[j])) {
                    game->score += 10;
                }
                break;
            }
        }
    }
}

/* Shard vs enemy projectile — shards can shield the player */
static void ResolveShardProjectileCollisions(Game *game) {
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (!game->shards[i].active) continue;

        for (int j = 0; j < MAX_ENEMY_PROJECTILES; j++) {
            if (!game->enemyProjectiles[j].active) continue;

            float dx = game->shards[i].position.x - game->enemyProjectiles[j].position.x;
            float dy = game->shards[i].position.y - game->enemyProjectiles[j].position.y;
            float rr = SHARD_RADIUS + ENEMY_PROJECTILE_RADIUS;

            if (dx * dx + dy * dy < rr * rr) {
                game->shards[i].active = false;
                game->enemyProjectiles[j].active = false;
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
    EnemyProjectile_InitAll(game->enemyProjectiles);

    BuildLevel(game);
    SpawnAllEnemies(game);

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

    /* Shards */
    Shard_UpdateAll(game->shards, SCREEN_WIDTH);
    ResolveShardPlatformCollisions(game);

    /* Player rect (slightly forgiving) */
    Rectangle playerRect = {
        game->jack.position.x - JACK_RADIUS * 0.75f,
        game->jack.position.y - JACK_RADIUS * 0.75f,
        JACK_RADIUS * 1.5f,
        JACK_RADIUS * 1.5f
    };

    /* Enemies (walkers + shooters) — may spawn projectiles */
    if (Enemy_UpdateAll(game->enemies, game->jack.position,
                        playerRect, game->enemyProjectiles)) {
        game->active = false;
        return;
    }

    /* Enemy projectiles — hit the player? */
    if (EnemyProjectile_UpdateAll(game->enemyProjectiles, playerRect,
                                  SCREEN_WIDTH, SCREEN_HEIGHT)) {
        game->active = false;
        return;
    }

    /* Shard ↔ Enemy */
    ResolveShardEnemyCollisions(game);

    /* Shard ↔ Projectile (deflect) */
    ResolveShardProjectileCollisions(game);

    /* Shooting */
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Shard_Shoot(game->shards, game->jack.position);
    }
}

void Game_Draw(const Game *game) {
    DrawBackground();
    Platform_DrawAll(game->platforms);
    Shard_DrawAll(game->shards);
    EnemyProjectile_DrawAll(game->enemyProjectiles);
    Enemy_DrawAll(game->enemies);
    Jack_Draw(&game->jack);
    DrawHUD(game);
}

/* ============================================================
 *  Application lifecycle
 * ============================================================ */

void Game_Run(Game *game) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);

    LoadBackgroundAsset();
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

    UnloadBackgroundAsset();
    CloseWindow();
}