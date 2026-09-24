#ifndef JACKFROST_GAME_H
#define JACKFROST_GAME_H

#include <stdbool.h>
#include "raylib.h"
#include "jack.h"
#include "shard.h"
#include "enemy.h"
#include "platform.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 500
#define JACK_RADIUS   26.0f
#define GAME_TITLE    "Jack Frost - Raylib"
#define TARGET_FPS    60

typedef struct Game {
    Jack            jack;
    Shard           shards[MAX_SHARDS];
    Enemy           enemies[MAX_ENEMIES];
    EnemyProjectile enemyProjectiles[MAX_ENEMY_PROJECTILES];
    Platform        platforms[MAX_PLATFORMS];
    int             score;
    bool            active;
    float           frameCounter;
} Game;

void Game_Init(Game *game);
void Game_Restart(Game *game);
void Game_Update(Game *game);
void Game_Draw(const Game *game);
void Game_Run(Game *game);

#endif /* JACKFROST_GAME_H */