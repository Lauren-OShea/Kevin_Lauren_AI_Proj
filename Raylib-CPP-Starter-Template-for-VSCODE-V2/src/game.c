#include "game.h"
#include <math.h>

// --- Internal helpers ---
static void DrawBackground(void) {
    DrawRectangleGradientV(
        0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
        (Color){ 194, 226, 255, 255 },
        (Color){ 77, 126, 168, 255 }
    );

    // Static snowflakes
    for (int i = 0; i < 70; i++) {
        int x = (i * 37) % SCREEN_WIDTH;
        int y = (i * 53) % SCREEN_HEIGHT;
        float r = 1.5f + (i % 3);
        DrawCircle(x, y, r, (Color){ 255, 255, 255, 180 });
    }
}

static void DrawHUD(const Game *game) {
    DrawText(TextFormat("SCORE: %d", game->score), 20, 20, 30,
             (Color){ 232, 245, 255, 255 });

    if (!game->active) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 0, 0, 0, 150 });
        DrawText("GAME OVER",
                 SCREEN_WIDTH / 2 - MeasureText("GAME OVER", 60) / 2,
                 SCREEN_HEIGHT / 2 - 60, 60,
                 (Color){ 255, 255, 255, 255 });
        DrawText("Press R to Restart",
                 SCREEN_WIDTH / 2 - MeasureText("Press R to Restart", 30) / 2,
                 SCREEN_HEIGHT / 2 + 20, 30,
                 (Color){ 200, 220, 240, 255 });
    }
}

// --- Public API ---
void Game_Init(Game *game) {
    Jack_Init(&game->jack);
    Shard_InitAll(game->shards);
    Enemy_InitAll(game->enemies);

    game->score = 0;
    game->active = true;
    game->frameCounter = 0.0f;
}

void Game_Restart(Game *game) {
    Game_Init(game);
}

void Game_Update(Game *game) {
    if (!game->active) return;

    // 1. Player
    Jack_Update(&game->jack);

    // 2. Shards
    Shard_UpdateAll(game->shards, SCREEN_WIDTH);

    // 3. Enemies + collision with player
    if (Enemy_UpdateAll(game->enemies, game->jack.position, JACK_RADIUS)) {
        game->active = false;
    }

    // 4. Shard vs Enemy collisions
    for (int i = 0; i < MAX_SHARDS; i++) {
        if (!game->shards[i].active) continue;

        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!game->enemies[j].active) continue;

            float dx = game->shards[i].position.x - game->enemies[j].position.x;
            float dy = game->shards[i].position.y - game->enemies[j].position.y;
            if (sqrtf(dx * dx + dy * dy) < SHARD_RADIUS + ENEMY_RADIUS) {
                game->enemies[j].active = false;
                game->shards[i].active = false;
                game->score += 10;
                break;
            }
        }
    }

    // 5. Spawning (dynamic difficulty)
    game->frameCounter += GetFrameTime() * 60.0f;

    int dynamicSpawnRate = 46 - (game->score / 70);
    if (dynamicSpawnRate < 22) dynamicSpawnRate = 22;

    if ((int)game->frameCounter % dynamicSpawnRate == 0) {
        Enemy_Spawn(game->enemies, SCREEN_WIDTH, SCREEN_HEIGHT);
        if (game->score > 180 && GetRandomValue(0, 100) < 35) {
            Enemy_Spawn(game->enemies, SCREEN_WIDTH, SCREEN_HEIGHT);
        }
    }

    // 6. Shooting
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Shard_Shoot(game->shards, game->jack.position);
    }
}

void Game_Draw(const Game *game) {
    DrawBackground();
    Shard_DrawAll(game->shards);
    Enemy_DrawAll(game->enemies);
    Jack_Draw(&game->jack);
    DrawHUD(game);
}