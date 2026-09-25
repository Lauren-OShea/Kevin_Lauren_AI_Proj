#include "game.h"
#include <math.h>

/* ============================================================
 *  Background asset
 * ============================================================ */

static void LoadBackground(Game *game) {
    game->background = LoadTexture("assets/background.png");

    if (game->background.id == 0) {
        TraceLog(LOG_ERROR, "FAILED to load assets/background.png");
        Image img = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, MAGENTA);
        game->background = LoadTextureFromImage(img);
        UnloadImage(img);
    } else {
        SetTextureFilter(game->background, TEXTURE_FILTER_POINT);
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

static void DrawBackground(const Game *game) {
    Rectangle src = { 0, 0,
                      (float)game->background.width,
                      (float)game->background.height };
    Rectangle dst = { 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };

    DrawTexturePro(game->background, src, dst, (Vector2){ 0, 0 }, 0.0f, WHITE);
    DrawAnimatedOverlay();
}

/* ============================================================
 *  HUD
 * ============================================================ */

static void DrawLivesRow(int lives, int xStart, int y, Color full) {
    for (int i = 0; i < JACK_MAX_LIVES; i++) {
        int cx = xStart + i * 26;

        Color face = (i < lives)
                     ? full
                     : (Color){ 60, 40, 40, 255 };

        DrawCircle(cx, y, 9, face);
        DrawCircleLines(cx, y, 9, (Color){ 20, 15, 15, 255 });
    }
}

static void DrawHUD(const Game *game) {
    DrawText(TextFormat("SCORE: %d", game->score),
             20, 20, 30, (Color){ 232, 245, 255, 255 });

    /* Lives — P1 top-left, P2 top-right */
    DrawLivesRow(game->players[0].lives, 30, 70,
                 (Color){ 120, 200, 255, 255 });

    int p2StartX = SCREEN_WIDTH - 30 - (JACK_MAX_LIVES - 1) * 26;
    DrawLivesRow(game->players[1].lives, p2StartX, 70,
                 (Color){ 255, 110, 110, 255 });

    DrawText("P1: A/D  SPACE  E",
             20, SCREEN_HEIGHT - 30, 18, (Color){ 200, 220, 240, 200 });
    DrawText("P2: ARROWS  UP  M",
             SCREEN_WIDTH - 200, SCREEN_HEIGHT - 30, 18,
             (Color){ 240, 200, 220, 200 });

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

static void SpawnAllEnemies(Game *game) {
    for (int i = 1; i < MAX_PLATFORMS; i++) {
        if (!game->platforms[i].active) continue;

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
 *  Player-vs-threat collision
 * ============================================================ */

static void ApplyPlayerThreatCollisions(Game *game,
                                        const Rectangle playerHitboxes[PLAYER_COUNT])
{
    for (int p = 0; p < PLAYER_COUNT; p++) {
        if (!Jack_IsVulnerable(&game->players[p])) continue;

        bool hit = false;

        /* vs. enemies — stunned enemies cannot hurt the player */
        for (int i = 0; i < MAX_ENEMIES && !hit; i++) {
            if (!game->enemies[i].active)           continue;
            if ( game->enemies[i].stunTimer > 0.0f) continue;
            if (CheckCollisionRecs(Enemy_GetRect(&game->enemies[i]),
                                   playerHitboxes[p])) {
                hit = true;
            }
        }

        /* vs. enemy projectiles */
        for (int j = 0; j < MAX_ENEMY_PROJECTILES && !hit; j++) {
            if (!game->enemyProjectiles[j].active) continue;
            if (CheckCollisionCircleRec(game->enemyProjectiles[j].position,
                                        ENEMY_PROJECTILE_RADIUS,
                                        playerHitboxes[p])) {
                game->enemyProjectiles[j].active = false;
                hit = true;
            }
        }

        if (hit) Jack_Hit(&game->players[p]);
    }
}

/* ============================================================
 *  Public API
 * ============================================================ */

static const JackControls P1_CONTROLS = { KEY_A, KEY_D, KEY_SPACE };
static const JackControls P2_CONTROLS = { KEY_LEFT, KEY_RIGHT, KEY_UP };

void Game_Init(Game *game) {
    Jack_Init(&game->players[0], (Vector2){ 130, 400 }, WHITE);
    Jack_Init(&game->players[1], (Vector2){ 670, 400 },
              (Color){ 255, 110, 110, 255 });

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

    /* 1. Players */
    Jack_Update(&game->players[0], game->platforms, &P1_CONTROLS);
    Jack_Update(&game->players[1], game->platforms, &P2_CONTROLS);

    /* 2. Shards */
    Shard_UpdateAll(game->shards, SCREEN_WIDTH);
    ResolveShardPlatformCollisions(game);

    /* 3. Player data arrays */
    Vector2   playerPositions[PLAYER_COUNT];
    Rectangle playerHitboxes [PLAYER_COUNT];
    for (int p = 0; p < PLAYER_COUNT; p++) {
        playerPositions[p] = game->players[p].position;
        playerHitboxes[p]  = (Rectangle){
            game->players[p].position.x - JACK_RADIUS * 0.75f,
            game->players[p].position.y - JACK_RADIUS * 0.75f,
            JACK_RADIUS * 1.5f,
            JACK_RADIUS * 1.5f
        };
    }

    /* 4. Enemies move & shoot (null hitboxes; game.c handles hits) */
    Rectangle nullHitboxes[PLAYER_COUNT];
    for (int p = 0; p < PLAYER_COUNT; p++) {
        nullHitboxes[p] = (Rectangle){ -10000, -10000, 0, 0 };
    }
    Enemy_UpdateAll(game->enemies, playerPositions, nullHitboxes,
                    PLAYER_COUNT, game->enemyProjectiles);

    EnemyProjectile_UpdateAll(game->enemyProjectiles, nullHitboxes,
                              PLAYER_COUNT, SCREEN_WIDTH, SCREEN_HEIGHT);

    /* 5. Player-vs-threat (lives loss + fling) */
    ApplyPlayerThreatCollisions(game, playerHitboxes);

    /* 6. Both players eliminated? End the level. */
    if (game->players[0].state == JACK_STATE_GONE &&
        game->players[1].state == JACK_STATE_GONE) {
        game->active = false;
        return;
    }

    /* 7. Shard ↔ Enemy */
    ResolveShardEnemyCollisions(game);

    /* 8. Shard ↔ Enemy projectile (deflect) */
    ResolveShardProjectileCollisions(game);

    /* 9. Shooting — only alive players can shoot */
    if (IsKeyPressed(KEY_E) && Jack_IsPlayable(&game->players[0])) {
        Shard_Shoot(game->shards, game->players[0].position,
                    game->players[0].facingDir);
        Jack_TriggerShoot(&game->players[0]);
    }
    if (IsKeyPressed(KEY_M) && Jack_IsPlayable(&game->players[1])) {
        Shard_Shoot(game->shards, game->players[1].position,
                    game->players[1].facingDir);
        Jack_TriggerShoot(&game->players[1]);
    }
}

void Game_Draw(const Game *game) {
    DrawBackground(game);
    Platform_DrawAll(game->platforms);
    Shard_DrawAll(game->shards);
    EnemyProjectile_DrawAll(game->enemyProjectiles);
    Enemy_DrawAll(game->enemies);
    Jack_Draw(&game->players[0], &game->sprites);
    Jack_Draw(&game->players[1], &game->sprites);
    DrawHUD(game);
}

/* ============================================================
 *  Application lifecycle
 * ============================================================ */

void Game_Run(Game *game) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    SetTargetFPS(TARGET_FPS);

    LoadBackground(game);
    PlayerSprites_Load(&game->sprites);
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

    PlayerSprites_Unload(&game->sprites);
    if (game->background.id != 0) UnloadTexture(game->background);
    CloseWindow();
}