#ifndef ENEMY_H
#define ENEMY_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_ENEMIES 64
#define ENEMY_BASE_SPEED 2.5f
#define ENEMY_RADIUS 28.0f

typedef struct Enemy {
    Vector2 position;
    float speed;
    float phase;
    bool active;
} Enemy;

// Initialize all enemies to inactive
void Enemy_InitAll(Enemy enemies[MAX_ENEMIES]);

// Spawn a single enemy on the right side
void Enemy_Spawn(Enemy enemies[MAX_ENEMIES], int screenWidth, int screenHeight);

// Update all active enemies; returns true if any enemy hits the player
bool Enemy_UpdateAll(Enemy enemies[MAX_ENEMIES], Vector2 playerPos, float playerRadius);

// Draw all active enemies
void Enemy_DrawAll(const Enemy enemies[MAX_ENEMIES]);

#endif // ENEMY_H