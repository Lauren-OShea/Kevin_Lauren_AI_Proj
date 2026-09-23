#include "game.h"
#include <math.h>

/* ============================================================
 *  Background: parallax helpers
 * ============================================================ */

// Draws a jagged mountain silhouette as a solid strip of quads.
static void DrawMountainRange(float baseY, float amplitude,
                              Color color, int seed, float scroll)
{
    const int segments = 40;
    float stepX = (float)SCREEN_WIDTH / segments;

    float prevX = 0.0f;
    float prevY = baseY
                + sinf((0 + seed + scroll) * 0.5f * 1.7f) * amplitude
                + sinf((0 + seed + scroll) * 0.5f * 0.6f) * amplitude * 0.6f;

    for (int i = 1; i <= segments; i++) {
        float x = i * stepX;
        float t = (i + seed + scroll) * 0.5f;
        float y = baseY
                + sinf(t * 1.7f) * amplitude
                + sinf(t * 0.6f) * amplitude * 0.6f;

        // Quad between (prevX, prevY) and (x, y), extending to the floor.
        DrawTriangle(
            (Vector2){ prevX, prevY },
            (Vector2){ x,     y     },
            (Vector2){ x,     SCREEN_HEIGHT },
            color
        );
        DrawTriangle(
            (Vector2){ prevX, prevY          },
            (Vector2){ x,     SCREEN_HEIGHT  },
            (Vector2){ prevX, SCREEN_HEIGHT  },
            color
        );

        prevX = x;
        prevY = y;
    }
}

// Solid silhouette tree (spiky canopy).
static void DrawTreeSilhouette(float x, float baseY,
                               float scale, Color color)
{
    float trunkW = 8.0f  * scale;
    float trunkH = 60.0f * scale;

    // Trunk
    DrawRectangle((int)(x - trunkW / 2),
                  (int)(baseY - trunkH),
                  (int)trunkW, (int)trunkH, color);

    // Top tier
    float topApexY = baseY - trunkH - 80.0f * scale;
    float topBaseY = baseY - trunkH - 20.0f * scale;
    float topHalfW = 55.0f * scale;

    DrawTriangle(
        (Vector2){ x,            topApexY },
        (Vector2){ x - topHalfW, topBaseY },
        (Vector2){ x + topHalfW, topBaseY },
        color
    );

    // Middle tier
    float midApexY = baseY - trunkH - 50.0f * scale;
    float midBaseY = baseY - trunkH - 0.0f  * scale;
    float midHalfW = 70.0f * scale;

    DrawTriangle(
        (Vector2){ x,            midApexY },
        (Vector2){ x - midHalfW, midBaseY },
        (Vector2){ x + midHalfW, midBaseY },
        color
    );
}

// Small bare/dead tree for the far background.
static void DrawSmallDeadTree(float x, float baseY, float scale, Color color)
{
    float trunkW = 4.0f * scale;
    float trunkH = 30.0f * scale;

    // Trunk
    DrawRectangle((int)(x - trunkW / 2),
                  (int)(baseY - trunkH),
                  (int)trunkW, (int)trunkH, color);

    // Branches as thin filled triangles (no rogue line artifacts)
    float bw = 1.5f * scale;

    DrawTriangle(
        (Vector2){ x - trunkW / 2,        baseY - trunkH * 0.60f },
        (Vector2){ x - trunkW / 2 - bw,   baseY - trunkH * 0.60f - bw },
        (Vector2){ x - 14 * scale,        baseY - trunkH - 8 * scale },
        color
    );
    DrawTriangle(
        (Vector2){ x + trunkW / 2,        baseY - trunkH * 0.75f },
        (Vector2){ x + trunkW / 2 + bw,   baseY - trunkH * 0.75f - bw },
        (Vector2){ x + 14 * scale,        baseY - trunkH - 4 * scale },
        color
    );
    DrawTriangle(
        (Vector2){ x - trunkW / 2,        baseY - trunkH * 0.85f },
        (Vector2){ x - trunkW / 2 - bw,   baseY - trunkH * 0.85f - bw },
        (Vector2){ x - 8 * scale,         baseY - trunkH - 16 * scale },
        color
    );
}

/* ============================================================
 *  Background renderer
 * ============================================================ */

static void DrawBackground(void) {
    float t = (float)GetTime();

    // ---------- 1. Night sky gradient ----------
    DrawRectangleGradientV(
        0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
        (Color){  38,  18,  72, 255 },
        (Color){ 110,  55, 130, 255 }
    );

    // ---------- 2. Big soft moon ----------
    DrawCircle(680, 90, 55, (Color){ 210, 200, 245,  40 });
    DrawCircle(680, 90, 42, (Color){ 230, 225, 255,  90 });
    DrawCircle(680, 90, 30, (Color){ 245, 240, 255, 160 });

    // ---------- 3. Twinkling stars ----------
    for (int i = 0; i < 90; i++) {
        int   x     = (i * 137) % SCREEN_WIDTH;
        int   y     = (i * 79)  % (SCREEN_HEIGHT / 2);
        float phase = (float)(i * 13);
        float tw    = 0.5f + 0.5f * sinf(t * 2.0f + phase);

        int alpha = 80 + (int)(tw * 175);
        float r   = 0.8f + (i % 3) * 0.5f;

        DrawCircle(x, y, r, (Color){ 255, 255, 255, (unsigned char)alpha });
    }

    // ---------- 4. Far mountain range ----------
    DrawMountainRange(300, 40, (Color){  45,  25,  75, 255 }, 0, 0.0f);

    // ---------- 5. Mid mountain range ----------
    DrawMountainRange(340, 55, (Color){  70,  40, 110, 255 }, 7, 0.0f);

    // ---------- 6. Distant bare trees ----------
    for (int i = 0; i < 12; i++) {
        float x = 40.0f + i * 68.0f;
        float s = 0.7f + (i % 3) * 0.15f;
        DrawSmallDeadTree(x, 400, s, (Color){ 30, 15, 50, 200 });
    }

    // ---------- 7. Near tree silhouettes ----------
    for (int i = 0; i < 6; i++) {
        float x = 80.0f + i * 140.0f;
        float s = 0.9f + (i % 2) * 0.3f;
        DrawTreeSilhouette(x, 440, s, (Color){ 20, 10, 35, 255 });
    }

    // ---------- 8. Falling snow ----------
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

    // ---------- 9. Vignette ----------
    DrawRectangleGradientV(
        0, 0, SCREEN_WIDTH, 90,
        (Color){ 0, 0, 0, 80 },
        (Color){ 0, 0, 0, 0 }
    );
    DrawRectangleGradientV(
        0, SCREEN_HEIGHT - 120, SCREEN_WIDTH, 120,
        (Color){ 0, 0, 0, 0 },
        (Color){ 0, 0, 0, 120 }
    );
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

    Jack_Update(&game->jack);
    ResolveJackPlatformCollisions(game);

    Shard_UpdateAll(game->shards, SCREEN_WIDTH);
    ResolveShardPlatformCollisions(game);

    if (Enemy_UpdateAll(game->enemies, game->jack.position, JACK_RADIUS)) {
        game->active = false;
        return;
    }

    ResolveShardEnemyCollisions(game);
    HandleSpawning(game);

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
 *  Application lifecycle
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