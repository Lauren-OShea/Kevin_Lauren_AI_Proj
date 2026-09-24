#ifndef JACKFROST_ENEMY_H
#define JACKFROST_ENEMY_H

#include <stdbool.h>
#include "raylib.h"

#define MAX_ENEMIES      32
#define ENEMY_WIDTH      32
#define ENEMY_HEIGHT     36
#define ENEMY_SPEED      1.5f
#define ENEMY_STUN_TIME  6.5f

typedef struct Enemy {
    Vector2 position;      // feet position (bottom-center)
    float   speed;
    float   dir;           // +1 = right, -1 = left
    float   patrolLeft;    // left patrol bound (world x)
    float   patrolRight;   // right patrol bound (world x)
    float   phase;         // walk animation phase
    float   stunTimer;     // seconds remaining stunned (0 = not stunned)
    bool    active;
} Enemy;

void Enemy_InitAll(Enemy enemies[MAX_ENEMIES]);

bool Enemy_SpawnOnPlatform(Enemy enemies[MAX_ENEMIES], Rectangle platform);

bool Enemy_UpdateAll(Enemy enemies[MAX_ENEMIES], Rectangle playerRect);

void Enemy_DrawAll(const Enemy enemies[MAX_ENEMIES]);

Rectangle Enemy_GetRect(const Enemy *enemy);

// Returns true if the enemy was successfully stunned (was not already stunned).
bool Enemy_Stun(Enemy *enemy);

#endif // JACKFROST_ENEMY_H