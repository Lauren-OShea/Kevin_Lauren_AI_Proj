#ifndef JACKFROST_GAME_H
#define JACKFROST_GAME_H

#include <stdbool.h>
#include "raylib.h"
#include "jack.h"
#include "shard.h"
#include "enemy.h"
#include "platform.h"
#include "frozen.h"
#include "particle.h"

#define SCREEN_WIDTH   800
#define SCREEN_HEIGHT  500
#define JACK_RADIUS    26.0f
#define GAME_TITLE     "Jack Frost - Raylib"
#define TARGET_FPS     60
#define PLAYER_COUNT   2


typedef enum GameState {
    GAME_STATE_MENU = 0,
    GAME_STATE_CONTROLS,
    GAME_STATE_PLAYING
} GameState;



typedef struct Game {
    Jack      players[PLAYER_COUNT];
    Shard     shards[MAX_SHARDS];
    Enemy     enemies[MAX_ENEMIES];
    EnemyProjectile enemyProjectiles[MAX_ENEMY_PROJECTILES];
    Platform  platforms[MAX_PLATFORMS];
    Ladder    ladders[MAX_LADDERS];
    PlayerSprites   sprites;      /* shared between both players */
    Texture2D       background;
    FrozenMap      frozen;
    ParticleSystem particles;
    int       score;
    bool      active;          /* false while playing = game over */
    float     frameCounter;

    GameState state;           /* which screen we're on */
    int       menuSelection;   /* highlighted menu item */
    bool      quitRequested;   /* set by the Quit menu item */

} Game;

void Game_Init(Game *game);
void Game_Restart(Game *game);
void Game_Update(Game *game);
void Game_Draw(const Game *game);
void Game_Run(Game *game);

#endif /* JACKFROST_GAME_H */
