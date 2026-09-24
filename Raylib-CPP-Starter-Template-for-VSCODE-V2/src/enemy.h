#ifndef JACKFROST_ENEMY_H
#define JACKFROST_ENEMY_H

#include <stdbool.h>
#include "raylib.h"

#define MAX_ENEMIES      32
#define ENEMY_WIDTH      32
#define ENEMY_HEIGHT     36
#define ENEMY_SPEED      1.5f
#define ENEMY_STUN_TIME  6.5f

/* ---- Shooter-specific constants ---- */
#define MAX_ENEMY_PROJECTILES          32
#define ENEMY_SHOOT_RANGE              160.0f   /* 4 tiles × 40px, horizontal */
#define ENEMY_SHOOT_VERTICAL_TOLERANCE  40.0f   /* 1 tile — must be on same level */
#define ENEMY_SHOOT_COOLDOWN             1.4f
#define ENEMY_PROJECTILE_SPEED           3.8f
#define ENEMY_PROJECTILE_RADIUS          6.0f

typedef enum EnemyType {
    ENEMY_WALKER = 0,
    ENEMY_SHOOTER
} EnemyType;

typedef struct Enemy {
    EnemyType type;
    Vector2   position;
    float     speed;
    float     dir;
    float     patrolLeft;
    float     patrolRight;
    float     phase;
    float     stunTimer;
    float     shootCooldown;
    bool      active;
} Enemy;

typedef struct EnemyProjectile {
    Vector2 position;
    Vector2 velocity;
    bool    active;
} EnemyProjectile;

void Enemy_InitAll(Enemy enemies[MAX_ENEMIES]);

bool Enemy_SpawnOnPlatform       (Enemy enemies[MAX_ENEMIES], Rectangle platform);
bool Enemy_SpawnShooterOnPlatform(Enemy enemies[MAX_ENEMIES], Rectangle platform);

bool Enemy_UpdateAll(Enemy enemies[MAX_ENEMIES],
                     Vector2 playerPos, Rectangle playerRect,
                     EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES]);

void Enemy_DrawAll(const Enemy enemies[MAX_ENEMIES]);

Rectangle Enemy_GetRect(const Enemy *enemy);
bool      Enemy_Stun   (Enemy *enemy);

void EnemyProjectile_InitAll(EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES]);

bool EnemyProjectile_UpdateAll(EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES],
                               Rectangle playerRect,
                               int screenWidth, int screenHeight);

void EnemyProjectile_DrawAll(const EnemyProjectile projectiles[MAX_ENEMY_PROJECTILES]);

#endif /* JACKFROST_ENEMY_H */