#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "jack.h"
#include "shard.h"
#include "enemy.h"
#include <stdbool.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 500
#define JACK_RADIUS 26.0f

typedef struct Game {
    Jack jack;
    Shard shards[MAX_SHARDS];
    Enemy enemies[MAX_ENEMIES];
    int score;
    bool active;
    float frameCounter;
} Game;

// Initialize the entire game state
void Game_Init(Game *game);

// Restart the game (same as Init)
void Game_Restart(Game *game);

// Update game logic
void Game_Update(Game *game);

// Draw everything (background, entities, HUD)
void Game_Draw(const Game *game);

#endif // GAME_H